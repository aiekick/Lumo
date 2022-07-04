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

#include "SpecularModule_Pass.h"

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

#define COUNT_BUFFERS 2

//////////////////////////////////////////////////////////////
//// SSAO SECOND PASS : BLUR /////////////////////////////////
//////////////////////////////////////////////////////////////

SpecularModule_Pass::SpecularModule_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : Specular", COMPUTE_SHADER_PASS_DEBUG_COLOR);
}

SpecularModule_Pass::~SpecularModule_Pass()
{
	Unit();
}

bool SpecularModule_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	return false;
}

void SpecularModule_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{

}

void SpecularModule_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{

}

void SpecularModule_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
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
				m_ImageInfos[vBinding] = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
			}

			m_NeedSamplerUpdate = true;
		}
	}
}

vk::DescriptorImageInfo* SpecularModule_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	if (m_ComputeBufferPtr)
	{
		return m_ComputeBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

void SpecularModule_Pass::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	m_SceneLightGroup = vSceneLightGroup;
}

void SpecularModule_Pass::SwapOutputDescriptors()
{
	writeDescriptorSets[0].pImageInfo = m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U); // output
}

void SpecularModule_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipeline);
		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_PipelineLayout, 0, m_DescriptorSet, nullptr);
		vCmdBuffer->dispatch(m_DispatchSize.x, m_DispatchSize.y, m_DispatchSize.z);
	}
}

bool SpecularModule_Pass::CreateUBO()
{
	for (auto& info : m_ImageInfos)
	{
		info = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
	}

	return true;
}

bool SpecularModule_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute);
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);
	m_LayoutBindings.emplace_back(3U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute);
	m_LayoutBindings.emplace_back(4U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute);

	return true;
}

bool SpecularModule_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();

	assert(m_ComputeBufferPtr);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eStorageImage,
		m_ComputeBufferPtr->GetBackDescriptorImageInfo(0U), nullptr); // output
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, 
		nullptr, CommonSystem::Instance()->GetBufferInfo()); // output

	auto lightGrouPtr = m_SceneLightGroup.getValidShared();
	if (lightGrouPtr && lightGrouPtr->GetBufferInfo())
	{
		writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eStorageBuffer, 
			nullptr, lightGrouPtr->GetBufferInfo());
	}
	else
	{
		writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eStorageBuffer,
			nullptr, m_VulkanCorePtr->getEmptyDescriptorBufferInfo());
	}

	writeDescriptorSets.emplace_back(m_DescriptorSet, 3U, 0, 1, vk::DescriptorType::eCombinedImageSampler, 
		&m_ImageInfos[0], nullptr); // pos

	writeDescriptorSets.emplace_back(m_DescriptorSet, 4U, 0, 1, vk::DescriptorType::eCombinedImageSampler,
		&m_ImageInfos[1], nullptr); // nor

	return true;
}

std::string SpecularModule_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "SpecularModule_Pass";

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColor;
)" 
+
CommonSystem::GetBufferObjectStructureHeader(1U)
+
SceneLightGroup::GetBufferObjectStructureHeader(2U)
+ 
u8R"(
layout(binding = 3) uniform sampler2D pos_map_sampler;
layout(binding = 4) uniform sampler2D nor_map_sampler;

vec3 getRayOrigin()
{
	vec3 ro = view[3].xyz + model[3].xyz;
	ro *= mat3(view * model);
	return -ro;
}

vec4 getLight(uint id, ivec2 coords, vec3 pos)
{
	vec3 light_pos = lightDatas[id].lightGizmo[3].xyz;
	float light_intensity = lightDatas[id].lightIntensity;
	vec4 light_col = lightDatas[id].lightColor;
	
	vec3 normal = normalize(texelFetch(nor_map_sampler, coords, 0).xyz * 2.0 - 1.0);
	vec3 light_dir = normalize(light_pos - pos);
	
	vec3 ro = getRayOrigin();
	vec3 rd = normalize(ro - pos);
	vec3 refl = reflect(-light_dir, normal);  
	vec4 spec = min(pow(max(dot(rd, refl), 0.0), 32.0) * light_intensity, 1.0) * light_col;
	
	return spec;
}

void main()
{
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	
	vec4 res = vec4(0.0);
	
	vec3 pos = texelFetch(pos_map_sampler, coords, 0).xyz;
	if (dot(pos, pos) > 0.0)
	{
		// why length() return 0 ???
		// there is always one light at least...
		// so we do length() + 1
		for (int i=0;i<lightDatas.length() + 1;++i)
		{
			res += getLight(i, coords, pos);
		}
	}
	
	imageStore(outColor, coords, res); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string SpecularModule_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	//str += vOffset + "<blur_radius>" + ct::toStr(m_UBOComp.u_blur_radius) + "</blur_radius>\n";
	
	return str;
}

bool SpecularModule_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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