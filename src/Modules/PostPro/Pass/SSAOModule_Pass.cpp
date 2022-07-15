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

#include "SSAOModule_Pass.h"

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

using namespace vkApi;

#define COUNT_BUFFERS 2

//////////////////////////////////////////////////////////////
//// SSAO FIRST PASS : AO ////////////////////////////////////
//////////////////////////////////////////////////////////////

SSAOModule_Pass::SSAOModule_Pass(vkApi::VulkanCorePtr vVulkanCorePtr) 
	: QuadShaderPass(vVulkanCorePtr, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Quad Pass : SSAO", QUAD_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

SSAOModule_Pass::~SSAOModule_Pass()
{
	Unit();
}

bool SSAOModule_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (ImGui::CollapsingHeader("SSAO", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool change = false;

		if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen))
		{
			change |= ImGui::SliderFloatDefaultCompact(0.0f, "Noise Scale", &m_UBOFrag.u_noise_scale, 0.0f, 2.0f, 1.0f);
			change |= ImGui::SliderFloatDefaultCompact(0.0f, "Radius", &m_UBOFrag.u_ao_radius, 0.0f, 0.25f, 0.01f);
			change |= ImGui::SliderFloatDefaultCompact(0.0f, "Scale", &m_UBOFrag.u_ao_scale, 0.0f, 1.0f, 1.0f);
			change |= ImGui::SliderFloatDefaultCompact(0.0f, "Bias", &m_UBOFrag.u_ao_bias, 0.0f, 0.1f, 0.001f);
			change |= ImGui::SliderFloatDefaultCompact(0.0f, "Intensity", &m_UBOFrag.u_ao_intensity, 0.0f, 5.0f, 2.0f);

			if (change)
			{
				NeedNewUBOUpload();
			}
		}

		DrawInputTexture(m_VulkanCorePtr, "Input Position", 0U, m_OutputRatio);
		DrawInputTexture(m_VulkanCorePtr, "Input Normal", 1U, m_OutputRatio);
		DrawInputTexture(m_VulkanCorePtr, "Input Blue Noise", 2U, m_OutputRatio);

		return change;
	}

	return false;
}

void SSAOModule_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

}

void SSAOModule_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

}

void SSAOModule_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				m_ImageInfos[vBinding] = *vImageInfo;

				if ((&m_UBOFrag.use_sampler_pos)[vBinding] < 1.0f)
				{
					(&m_UBOFrag.use_sampler_pos)[vBinding] = 1.0f;
					NeedNewUBOUpload();
				}
			}
			else
			{
				if ((&m_UBOFrag.use_sampler_pos)[vBinding] > 0.0f)
				{
					(&m_UBOFrag.use_sampler_pos)[vBinding] = 0.0f;
					NeedNewUBOUpload();
				}
				
				m_ImageInfos[vBinding] = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* SSAOModule_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_FrameBufferPtr)
	{
		if (vOutSize)
		{
			*vOutSize = m_FrameBufferPtr->GetOutputSize();
		}

		return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

bool SSAOModule_Pass::CreateUBO()
{
	ZoneScoped;

	auto size_in_bytes = sizeof(UBOFrag);
	m_UBO_Frag = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, size_in_bytes);
	m_DescriptorBufferInfo_Frag.buffer = m_UBO_Frag->buffer;
	m_DescriptorBufferInfo_Frag.range = size_in_bytes;
	m_DescriptorBufferInfo_Frag.offset = 0;

	for (auto& info : m_ImageInfos)
	{
		info = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
	}

	NeedNewUBOUpload();

	return true;
}

void SSAOModule_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, *m_UBO_Frag, &m_UBOFrag, sizeof(UBOFrag));
}

void SSAOModule_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Frag.reset();
}

bool SSAOModule_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(3U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(4U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);

	return true;
}

bool SSAOModule_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Frag);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], nullptr); // depth
	writeDescriptorSets.emplace_back(m_DescriptorSet, 3U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[1], nullptr); // normal
	writeDescriptorSets.emplace_back(m_DescriptorSet, 4U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[2], nullptr); // RGB noise

	return true;
}

std::string SSAOModule_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "SSAOModule_Vertex";

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

std::string SSAOModule_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "SSAOModule_Pass";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 v_uv;

layout (std140, binding = 1) uniform UBO_Frag
{
	float use_sampler_pos;
	float use_sampler_nor;
	float use_sampler_noise;
	float u_noise_scale;
	float u_ao_radius;
	float u_ao_scale;
	float u_ao_bias;
	float u_ao_intensity;
};
layout(binding = 2) uniform sampler2D pos_map_sampler;
layout(binding = 3) uniform sampler2D nor_map_sampler;
layout(binding = 4) uniform sampler2D noise_map_sampler;

// https://www.gamedev.net/tutorials/programming/graphics/a-simple-and-practical-approach-to-ssao-r2753/

vec3 getPos(vec2 uv)
{
	return texture(pos_map_sampler, uv).xyz;
}

vec3 getNor(vec2 uv)
{
	return texture(nor_map_sampler, uv).xyz * 2.0 - 1.0;
}

vec2 getNoi(vec2 uv) 
{
	return normalize(texture(noise_map_sampler, uv * u_noise_scale).xy * 2.0 - 1.0); 
}
	
float getAO(vec2 uv, vec2 off, vec3 p, vec3 n) 
{
	vec3 diff = getPos(uv + off) - p; 
	vec3 v = normalize(diff); 
	float d = length(diff) * u_ao_scale; 
	return max(0.0, dot(n, v) - u_ao_bias) * (1.0 / (1.0 + d)) * u_ao_intensity;
}

void main() 
{
	fragColor = vec4(0.0);

	if (use_sampler_pos > 0.5 && use_sampler_nor > 0.5)
	{
		vec3 pos = getPos(v_uv);
		if (dot(pos, pos) > 0.0)
		{	
			const vec2 vec[4] = { vec2(1, 0), vec2(-1, 0), vec2(0, 1), vec2(0, -1) };
			vec3 p = getPos(v_uv); 
			vec3 n = getNor(v_uv); 
			vec2 rand = getNoi(v_uv); 

			float occ = 0.0; 
			
			const int iterations = 4; 
			for (int j = 0; j < iterations; ++j) 
			{	
				vec2 c1 = reflect(vec[j], rand) * u_ao_radius; 
				vec2 c2 = vec2(c1.x - c1.y, c1.x + c1.y) * 0.707; 
				occ += getAO(v_uv, c1 * 0.25, p, n); 
				occ += getAO(v_uv, c2 * 0.50, p, n); 
				occ += getAO(v_uv, c1 * 0.75, p, n); 
				occ += getAO(v_uv, c2 * 1.00, p, n); 
			}
		  
			occ /= float(iterations) * 4.0; 
					
			fragColor = vec4(1.0 - occ);
		}
		else
		{
			//discard;
		}
	}
	else
	{
		//discard;
	}
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string SSAOModule_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<noise_scale>" + ct::toStr(m_UBOFrag.u_noise_scale) + "</noise_scale>\n";
	str += vOffset + "<ao_radius>" + ct::toStr(m_UBOFrag.u_ao_radius) + "</ao_radius>\n";
	str += vOffset + "<ao_scale>" + ct::toStr(m_UBOFrag.u_ao_scale) + "</ao_scale>\n";
	str += vOffset + "<ao_bias>" + ct::toStr(m_UBOFrag.u_ao_bias) + "</ao_bias>\n";
	str += vOffset + "<ao_intensity>" + ct::toStr(m_UBOFrag.u_ao_intensity) + "</ao_intensity>\n";

	return str;
}

bool SSAOModule_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strParentName == "ssao_module")
	{
		if (strName == "noise_scale")
			m_UBOFrag.u_noise_scale = ct::fvariant(strValue).GetF();
		else if (strName == "ao_radius")
			m_UBOFrag.u_ao_radius = ct::fvariant(strValue).GetF();
		else if (strName == "ao_scale")
			m_UBOFrag.u_ao_scale = ct::fvariant(strValue).GetF();
		else if (strName == "ao_bias")
			m_UBOFrag.u_ao_bias = ct::fvariant(strValue).GetF();
		else if (strName == "ao_intensity")
			m_UBOFrag.u_ao_intensity = ct::fvariant(strValue).GetF();

		NeedNewUBOUpload();
	}

	return true;
}