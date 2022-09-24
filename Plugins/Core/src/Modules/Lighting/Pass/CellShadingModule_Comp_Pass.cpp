/*
Copyright 2022-2022 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "CellShadingModule_Comp_Pass.h"

#include <functional>
#include <Gui/MainFrame.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanSubmitter.h>
#include <utils/Mesh/VertexStruct.h>
#include <cinttypes>
#include <Base/FrameBuffer.h>
#include <Modules/Lighting/LightGroupModule.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////
//// SSAO SECOND PASS : BLUR /////////////////////////////////
//////////////////////////////////////////////////////////////

CellShadingModule_Comp_Pass::CellShadingModule_Comp_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : Cell Shading", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

CellShadingModule_Comp_Pass::~CellShadingModule_Comp_Pass()
{
	Unit();
}

void CellShadingModule_Comp_Pass::ActionBeforeInit()
{
	ZoneScoped;

	for (auto& info : m_ImageInfos)
	{
		info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
	}
}

bool CellShadingModule_Comp_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	bool change = false;

	change |= ImGui::SliderUIntDefaultCompact(0.0f, "Count_Step", &m_UBOComp.u_Count_Step, 1U, 128U, 4U);

	if (change)
	{
		NeedNewUBOUpload();
	}

	return change;
}

void CellShadingModule_Comp_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
}

void CellShadingModule_Comp_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
}

void CellShadingModule_Comp_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				if (vTextureSize)
				{
					m_ImageInfosSize[vBinding] = *vTextureSize;
				}

				m_ImageInfos[vBinding] = *vImageInfo;

				if ((&m_UBOComp.u_use_pos_map)[vBinding] < 1.0f)
				{
					(&m_UBOComp.u_use_pos_map)[vBinding] = 1.0f;
					NeedNewUBOUpload();
				}
			}
			else
			{
				if ((&m_UBOComp.u_use_pos_map)[vBinding] > 0.0f)
				{
					(&m_UBOComp.u_use_pos_map)[vBinding] = 0.0f;
					NeedNewUBOUpload();
				}

				m_ImageInfos[vBinding] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* CellShadingModule_Comp_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_ComputeBufferPtr)
	{
		if (vOutSize)
		{
			*vOutSize = m_ComputeBufferPtr->GetOutputSize();
		}

		return m_ComputeBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

void CellShadingModule_Comp_Pass::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	m_SceneLightGroup = vSceneLightGroup;

	m_SceneLightGroupDescriptorInfoPtr = &m_SceneEmptyLightGroupDescriptorInfo;

	auto lightGroupPtr = m_SceneLightGroup.getValidShared();
	if (lightGroupPtr && 
		lightGroupPtr->GetBufferInfo())
	{
		m_SceneLightGroupDescriptorInfoPtr = lightGroupPtr->GetBufferInfo();
	}

	UpdateBufferInfoInRessourceDescriptor();
}

void CellShadingModule_Comp_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
		
		Dispatch(vCmdBuffer);
	}
}

bool CellShadingModule_Comp_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBOComp_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Comp));
	m_UBOComp_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
	if (m_UBOComp_Ptr)
	{
		m_UBOComp_BufferInfos.buffer = m_UBOComp_Ptr->buffer;
		m_UBOComp_BufferInfos.range = sizeof(UBO_Comp);
		m_UBOComp_BufferInfos.offset = 0;
	}

	NeedNewUBOUpload();

	return true;
}

void CellShadingModule_Comp_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBOComp_Ptr, &m_UBOComp, sizeof(UBO_Comp));
}

void CellShadingModule_Comp_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBOComp_Ptr.reset();
	m_UBOComp_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
}

bool CellShadingModule_Comp_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute); // pos
	res &= AddOrSetLayoutDescriptor(4U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute); // not
	res &= AddOrSetLayoutDescriptor(5U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute); // tint
	return res;
}

bool CellShadingModule_Comp_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U)); // output
	res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eStorageBuffer, m_SceneLightGroupDescriptorInfoPtr);
	res &= AddOrSetWriteDescriptorBuffer(2U, vk::DescriptorType::eUniformBuffer, &m_UBOComp_BufferInfos);
	res &= AddOrSetWriteDescriptorImage(3U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]); // pos
	res &= AddOrSetWriteDescriptorImage(4U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[1]); // nor
	res &= AddOrSetWriteDescriptorImage(5U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[2]); // tint
	return res;
}

std::string CellShadingModule_Comp_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "CellShadingModule_Comp_Pass";

	SetLocalGroupSize(ct::uvec3(8U, 8U, 1U));

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColor;
)" 
+
SceneLightGroup::GetBufferObjectStructureHeader(1U)
+ 
u8R"(
layout(std140, binding = 2) uniform UBO_Comp
{
	uint u_count_step;
	float u_use_pos_map;
	float u_use_nor_map;
	float u_use_tint_map;
};

layout(binding = 3) uniform sampler2D pos_map_sampler;
layout(binding = 4) uniform sampler2D nor_map_sampler;
layout(binding = 5) uniform sampler2D tint_map_sampler;

vec4 getLightGroup(uint id, ivec2 coords, vec3 pos)
{
	vec4 diff = vec4(1.0);

	if (lightDatas[id].lightActive > 0.5)
	{
		vec3 light_pos = lightDatas[id].lightGizmo[3].xyz;
		float light_intensity = lightDatas[id].lightIntensity;
		vec4 light_col = lightDatas[id].lightColor;
	
		vec3 normal = normalize(texelFetch(nor_map_sampler, coords, 0).xyz * 2.0 - 1.0);
		if (lightDatas[id].is_inside > 0.5) // inside mesh
			normal *= - 1.0;
		vec3 light_dir = normalize(light_pos - pos);
		float angle = clamp(dot(normal, light_dir), 0.0, 1.0);

		const float f_u_count_step = float(u_count_step);
		angle = floor(angle * f_u_count_step) / f_u_count_step;

		if (u_use_tint_map > 0.5)
			diff = texture(tint_map_sampler, vec2(0.5)); // tint

		diff *= light_col * light_intensity * angle;
	}

	return diff;
}

void main()
{
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	
	vec4 res = vec4(0.0);
	
	vec3 pos = texelFetch(pos_map_sampler, coords, 0).xyz;
	if (dot(pos, pos) > 0.0)
	{
		uint count = uint(lightsCount) % 8; // maxi 8 lights in this system
		for (int i=0;i<count;++i)
		{
			res += getLightGroup(i, coords, pos);
		}
	}
	
	imageStore(outColor, coords, res); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string CellShadingModule_Comp_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += ShaderPass::getXml(vOffset, vUserDatas);

	str += vOffset + "<count_step>" + ct::toStr(m_UBOComp.u_Count_Step) + "</count_step>\n";

	return str;
}

bool CellShadingModule_Comp_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	ZoneScoped;

	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	ShaderPass::setFromXml(vElem, vParent, vUserDatas);

	if (strParentName == "cell_shading_module")
	{
		if (strName == "count_step")
			m_UBOComp.u_Count_Step = ct::uvariant(strValue).GetU();
	}

	return true;
}

void CellShadingModule_Comp_Pass::AfterNodeXmlLoading()
{
	ZoneScoped;

	// code to do after end of the xml loading of this node
	// by ex :
	NeedNewUBOUpload();
}