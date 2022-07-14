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

#include "DeferredRenderer_Pass.h"

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

DeferredRenderer_Pass::DeferredRenderer_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: QuadShaderPass(vVulkanCorePtr, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Quad Pass 1 : Deferred", QUAD_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

DeferredRenderer_Pass::~DeferredRenderer_Pass()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool DeferredRenderer_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	/*DrawInputTexture(m_VulkanCorePtr, "Position", 0U, m_OutputRatio);
	DrawInputTexture(m_VulkanCorePtr, "Normal", 1U, m_OutputRatio);
	DrawInputTexture(m_VulkanCorePtr, "Albedo", 2U, m_OutputRatio);
	DrawInputTexture(m_VulkanCorePtr, "Diffuse", 3U, m_OutputRatio);
	DrawInputTexture(m_VulkanCorePtr, "Specular", 4U, m_OutputRatio);
	DrawInputTexture(m_VulkanCorePtr, "Attenuation", 5U, m_OutputRatio);
	DrawInputTexture(m_VulkanCorePtr, "Mask", 6U, m_OutputRatio);
	DrawInputTexture(m_VulkanCorePtr, "Ao", 7U, m_OutputRatio);
	DrawInputTexture(m_VulkanCorePtr, "shadow", 8U, m_OutputRatio);*/

	return false;
}

void DeferredRenderer_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

}

void DeferredRenderer_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

}

void DeferredRenderer_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				m_ImageInfos[vBinding] = *vImageInfo;

				if ((&m_UBOFrag.use_sampler_position)[vBinding] < 1.0f)
				{
					(&m_UBOFrag.use_sampler_position)[vBinding] = 1.0f;
					NeedNewUBOUpload();
				}
			}
			else
			{
				if ((&m_UBOFrag.use_sampler_position)[vBinding] > 0.0f)
				{
					(&m_UBOFrag.use_sampler_position)[vBinding] = 0.0f;
					NeedNewUBOUpload();
				}
				
				m_ImageInfos[vBinding] = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* DeferredRenderer_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string DeferredRenderer_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	return str;
}

bool DeferredRenderer_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DeferredRenderer_Pass::CreateUBO()
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

void DeferredRenderer_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, *m_UBO_Frag, &m_UBOFrag, sizeof(UBOFrag));
}

void DeferredRenderer_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Frag.reset();
}

bool DeferredRenderer_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(3U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(4U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(5U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(6U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(7U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(8U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(9U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(10U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);

	return true;
}

bool DeferredRenderer_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();
	writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, CommonSystem::Instance()->GetBufferInfo());
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Frag);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], nullptr); // position
	writeDescriptorSets.emplace_back(m_DescriptorSet, 3U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[1], nullptr); // normal
	writeDescriptorSets.emplace_back(m_DescriptorSet, 4U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[2], nullptr); // albedo
	writeDescriptorSets.emplace_back(m_DescriptorSet, 5U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[3], nullptr); // diffuse
	writeDescriptorSets.emplace_back(m_DescriptorSet, 6U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[4], nullptr); // specular
	writeDescriptorSets.emplace_back(m_DescriptorSet, 7U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[5], nullptr); // attenuation
	writeDescriptorSets.emplace_back(m_DescriptorSet, 8U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[6], nullptr); // mask
	writeDescriptorSets.emplace_back(m_DescriptorSet, 9U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[7], nullptr); // ao
	writeDescriptorSets.emplace_back(m_DescriptorSet, 10U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[8], nullptr); // shadow

	return true;
}

std::string DeferredRenderer_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "DeferredRenderer_Pass_Vertex";

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

std::string DeferredRenderer_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "DeferredRenderer_Pass_Fragment";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 v_uv;
)"
+ CommonSystem::GetBufferObjectStructureHeader(0U) +
u8R"(
layout (std140, binding = 1) uniform UBO_Frag 
{ 
	float use_sampler_position;		// position
	float use_sampler_normal;		// normal
	float use_sampler_albedo;		// albedo
	float use_sampler_diffuse;		// diffuse
	float use_sampler_specular;		// specular
	float use_sampler_attenuation;	// attenuation
	float use_sampler_mask;			// mask
	float use_sampler_ssao;			// ssao
	float use_sampler_shadow;		// shadow
};

layout(binding = 2) uniform sampler2D position_map_sampler;
layout(binding = 3) uniform sampler2D normal_map_sampler;
layout(binding = 4) uniform sampler2D albedo_map_sampler;
layout(binding = 5) uniform sampler2D diffuse_map_sampler;
layout(binding = 6) uniform sampler2D specular_map_sampler;
layout(binding = 7) uniform sampler2D attenuation_map_sampler;
layout(binding = 8) uniform sampler2D mask_map_sampler;
layout(binding = 9) uniform sampler2D ssao_map_sampler;
layout(binding = 10) uniform sampler2D shadow_map_sampler;

void main() 
{
	fragColor = vec4(0);
	
	if (use_sampler_diffuse > 0.5)
	{
		fragColor += texture(diffuse_map_sampler, v_uv);
	}

	if (use_sampler_specular > 0.5)
	{
		fragColor += texture(specular_map_sampler, v_uv);
	}

	if (use_sampler_attenuation > 0.5)
	{
		fragColor *= texture(attenuation_map_sampler, v_uv);
	}

	if (use_sampler_albedo > 0.5)
	{
		fragColor *= texture(albedo_map_sampler, v_uv);
	}
	
	if (use_sampler_shadow > 0.5)
	{
		fragColor.rgb *= texture(shadow_map_sampler, v_uv).r;
	}

	if (use_sampler_ssao > 0.5)
	{
		fragColor.rgb *= texture(ssao_map_sampler, v_uv).r;
	}
	
	fragColor.a = 1.0;

	if (use_sampler_mask > 0.5)
	{
		float mask =  texture(mask_map_sampler, v_uv).r;
		if (mask < 0.5)
		{
			discard; // a faire en dernier
		}
	}
}
)";
}
