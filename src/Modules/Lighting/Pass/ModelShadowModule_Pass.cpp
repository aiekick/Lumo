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

#include "ModelShadowModule_Pass.h"

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
#include <Base/FrameBuffer.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

ModelShadowModule_Pass::ModelShadowModule_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: QuadShaderPass(vVulkanCorePtr, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Quad Pass 1 : Model Shadow", QUAD_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

ModelShadowModule_Pass::~ModelShadowModule_Pass()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ModelShadowModule_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	bool change = false;

	if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen))
	{
		change |= ImGui::SliderFloatDefaultCompact(0.0f, "Strength", &m_UBOFrag.u_shadow_strength, 0.0f, 1.0f, 0.5f);
		change |= ImGui::SliderFloatDefaultCompact(0.0f, "Bias", &m_UBOFrag.u_bias, 0.0f, 0.02f, 0.01f);
		change |= ImGui::SliderFloatDefaultCompact(0.0f, "Noise Scale", &m_UBOFrag.u_poisson_scale, 1.0f, 10000.0f, 2000.0f);

		if (change)
		{
			NeedNewUBOUpload();
		}
	}

	DrawInputTexture(m_VulkanCorePtr, "Input Position", 0U, m_OutputRatio);

	return change;
}

void ModelShadowModule_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{

}

void ModelShadowModule_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{

}

void ModelShadowModule_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding == 0U)
		{
			if (vBinding < m_ImageInfos.size())
			{
				if (vImageInfo)
				{
					m_ImageInfos[vBinding] = *vImageInfo;

					if (m_UBOFrag.use_sampler_pos < 1.0f)
					{
						m_UBOFrag.use_sampler_pos = 1.0f;

						NeedNewUBOUpload();
					}
				}
				else
				{
					if (m_UBOFrag.use_sampler_pos > 0.0f)
					{
						m_UBOFrag.use_sampler_pos = 0.0f;

						NeedNewUBOUpload();
					}

					m_ImageInfos[vBinding] = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
				}
			}
		}
	}
}

void ModelShadowModule_Pass::SetTextures(const uint32_t& vBinding, std::vector<vk::DescriptorImageInfo>* vImageInfos)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding == 1U)
		{
			if (vImageInfos && 
				vImageInfos->size() == m_ImageGroupInfos.size())
			{
				for (size_t i = 0U; i < vImageInfos->size(); ++i)
				{
					m_ImageGroupInfos[i] = vImageInfos->at(i);
				}
			}
			else
			{
				for (auto& info : m_ImageGroupInfos)
				{
					info = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
				}

				if (m_UBOFrag.use_sampler_pos > 0.0f)
				{
					m_UBOFrag.use_sampler_pos = 0.0f;

					NeedNewUBOUpload();
				}
			}			
		}
	}
}

vk::DescriptorImageInfo* ModelShadowModule_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	ZoneScoped;

	if (m_FrameBufferPtr)
	{
		return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

void ModelShadowModule_Pass::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	m_SceneLightGroup = vSceneLightGroup;

	m_SceneLightGroupDescriptorInfoPtr = &m_SceneLightGroupDescriptorInfo;

	auto lightGroupPtr = m_SceneLightGroup.getValidShared();
	if (lightGroupPtr &&
		lightGroupPtr->GetBufferInfo())
	{
		m_SceneLightGroupDescriptorInfoPtr = lightGroupPtr->GetBufferInfo();
	}

	UpdateBufferInfoInRessourceDescriptor();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ModelShadowModule_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<bias>" + ct::toStr(m_UBOFrag.u_bias) + "</bias>\n";
	str += vOffset + "<strength>" + ct::toStr(m_UBOFrag.u_shadow_strength) + "</strength>\n";
	str += vOffset + "<noise_scale>" + ct::toStr(m_UBOFrag.u_poisson_scale) + "</noise_scale>\n";

	return str;
}

bool ModelShadowModule_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strParentName == "model_shadow_module")
	{
		if (strName == "bias")
			m_UBOFrag.u_bias = ct::fvariant(strValue).GetF();
		else if (strName == "strength")
			m_UBOFrag.u_shadow_strength = ct::fvariant(strValue).GetF();
		else if (strName == "noise_scale")
			m_UBOFrag.u_poisson_scale = ct::fvariant(strValue).GetF();

		NeedNewUBOUpload();
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ModelShadowModule_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBO_Frag = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBOFrag));
	m_DescriptorBufferInfo_Frag.buffer = m_UBO_Frag->buffer;
	m_DescriptorBufferInfo_Frag.range = sizeof(UBOFrag);
	m_DescriptorBufferInfo_Frag.offset = 0;

	for (auto& info : m_ImageInfos)
	{
		info = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
	}

	for (auto& info : m_ImageGroupInfos)
	{
		info = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
		//info.imageView = VK_NULL_HANDLE;
	}

	NeedNewUBOUpload();

	return true;
}

void ModelShadowModule_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, *m_UBO_Frag, &m_UBOFrag, sizeof(UBOFrag));
}

void ModelShadowModule_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Frag.reset();
}

bool ModelShadowModule_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eUniformBuffer, 1U, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1U, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1U, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(3U, vk::DescriptorType::eStorageBuffer, 1U, vk::ShaderStageFlagBits::eFragment);

	// the shadow maps
	m_LayoutBindings.emplace_back(4U, vk::DescriptorType::eCombinedImageSampler,
		(uint32_t)m_ImageGroupInfos.size(), vk::ShaderStageFlagBits::eFragment);

	// next binding will be 4 + 8 => 12
	
	return true;
}

bool ModelShadowModule_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();
	writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, CommonSystem::Instance()->GetBufferInfo());
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Frag);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], nullptr);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 3U, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, m_SceneLightGroupDescriptorInfoPtr);
	
	// the shadow maps
	writeDescriptorSets.emplace_back(m_DescriptorSet, 4U, 0,
		(uint32_t)m_ImageGroupInfos.size(), vk::DescriptorType::eCombinedImageSampler, m_ImageGroupInfos.data(), nullptr);
	
	// next binding will be 4 + 8 => 12

	return true;
}

std::string ModelShadowModule_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ModelShadowModule_Pass_Vertex";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 vertPosition;
layout(location = 1) in vec2 vertUv;
layout(location = 0) out vec2 v_uv;

void main() 
{
	v_uv = vertUv;
	gl_Position = vec4(vertPosition, 0.0, 1.0);
}
)";
}

std::string ModelShadowModule_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ModelShadowModule_Pass";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragShadow;
layout(location = 0) in vec2 v_uv;
)"
+ 
CommonSystem::GetBufferObjectStructureHeader(0U) 
+
u8R"(
layout (std140, binding = 1) uniform UBO_Frag 
{ 
	float u_shadow_strength;
	float u_bias;
	float u_poisson_scale;
	float use_sampler_pos;
	float use_sampler_shadow_map;
};
layout(binding = 2) uniform sampler2D position_map_sampler;
)"
+
SceneLightGroup::GetBufferObjectStructureHeader(3U)
+
u8R"(
layout(binding = 4) uniform sampler2D light_shadow_map_samplers[8]; // binding 4 + 8
const vec2 poissonDisk[16] = vec2[]
( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

float random(vec3 seed, int i)
{
	vec4 seed4 = vec4(seed, i);
	float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
	return fract(sin(dot_product) * 43758.5453);
}

void main() 
{
	fragShadow = vec4(0);
	
	fragShadow = texture(position_map_sampler, v_uv);
	
	//fragShadow = texture(light_shadow_map_samplers[0], v_uv);

	/*if (use_sampler_pos > 0.5 && 
		use_sampler_shadow_map > 0.5)
	{
		vec3 pos = texture(position_map_sampler, v_uv).xyz;
		if (dot(pos, pos) > 0.0)
		{
			vec4 shadowCoord = lightDatas[0].lightView * vec4(pos, 1.0);
			shadowCoord.xyz /= shadowCoord.w;
			shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;

			const float poisson_scale = max(u_poisson_scale, 1.0); // for div by zero
			const float bias = u_bias * 0.01;

			float sha_vis = 1.0;
			float sha_step = 1.0 / mix(16.0, 8.0, u_shadow_strength);
			for (int i=0;i<8;i++)
			{
				int index = int(16.0 * random(gl_FragCoord.xyy, i)) % 16;
				float sha = texture(light_shadow_map_sampler, shadowCoord.xy + poissonDisk[index] / poisson_scale, 0.0).r;
				if (sha * cam_far < (shadowCoord.z - bias)/shadowCoord.w)
				{
					sha_vis -= sha_step * (1.0 - sha);
				}
			}

			fragShadow = vec4(sha_vis);
		}
		else
		{
			discard;
		}
	}	
	else
	{
		discard;
	}*/
}
)";
}
