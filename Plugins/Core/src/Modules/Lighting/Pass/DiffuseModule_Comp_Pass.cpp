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

#include "DiffuseModule_Comp_Pass.h"

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

DiffuseModule_Comp_Pass::DiffuseModule_Comp_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : Diffuse", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

DiffuseModule_Comp_Pass::~DiffuseModule_Comp_Pass()
{
	Unit();
}

bool DiffuseModule_Comp_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	return false;
}

void DiffuseModule_Comp_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void DiffuseModule_Comp_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void DiffuseModule_Comp_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				m_ImageInfos[vBinding] = *vImageInfo;
			}
			else
			{
				m_ImageInfos[vBinding] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* DiffuseModule_Comp_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

void DiffuseModule_Comp_Pass::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
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

void DiffuseModule_Comp_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
		
		Dispatch(vCmdBuffer);
	}
}

bool DiffuseModule_Comp_Pass::CreateUBO()
{
	ZoneScoped;

	for (auto& info : m_ImageInfos)
	{
		info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
	}

	NeedNewUBOUpload();

	return true;
}

bool DiffuseModule_Comp_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_DescriptorSets[0].m_LayoutBindings.clear();
	m_DescriptorSets[0].m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute);
	m_DescriptorSets[0].m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);
	m_DescriptorSets[0].m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute);
	m_DescriptorSets[0].m_LayoutBindings.emplace_back(3U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute);

	return true;
}

bool DiffuseModule_Comp_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	m_DescriptorSets[0].m_WriteDescriptorSets.clear();

	assert(m_ComputeBufferPtr);
	m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eStorageImage, 
		m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U), nullptr); // output

	m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eStorageBuffer, 
		nullptr, m_SceneLightGroupDescriptorInfoPtr);

	m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eCombinedImageSampler, 
		&m_ImageInfos[0], nullptr); // pos

	m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 3U, 0, 1, vk::DescriptorType::eCombinedImageSampler,
		&m_ImageInfos[1], nullptr); // nor

	return true;
}

std::string DiffuseModule_Comp_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "DiffuseModule_Comp_Pass";

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
layout(binding = 2) uniform sampler2D pos_map_sampler;
layout(binding = 3) uniform sampler2D nor_map_sampler;

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
		diff = min(max(dot(normal, light_dir), 0.0) * light_intensity, 1.0) * light_col;
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

std::string DiffuseModule_Comp_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	//str += vOffset + "<blur_radius>" + ct::toStr(m_UBOComp.u_blur_radius) + "</blur_radius>\n";
	
	return str;
}

bool DiffuseModule_Comp_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
{
	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strParentName == "diffuse_module")
	{
		//if (strName == "blur_radius")
		//	m_UBOComp.u_blur_radius = ct::uvariant(strValue).GetU();

		NeedNewUBOUpload();
	}

	return true;
}