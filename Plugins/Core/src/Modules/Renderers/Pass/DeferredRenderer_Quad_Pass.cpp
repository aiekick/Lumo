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

#include "DeferredRenderer_Quad_Pass.h"

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

DeferredRenderer_Quad_Pass::DeferredRenderer_Quad_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: QuadShaderPass(vVulkanCorePtr, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Quad Pass 1 : Deferred", QUAD_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

DeferredRenderer_Quad_Pass::~DeferredRenderer_Quad_Pass()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool DeferredRenderer_Quad_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

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

void DeferredRenderer_Quad_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void DeferredRenderer_Quad_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void DeferredRenderer_Quad_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBindingPoint < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				m_ImageInfos[vBindingPoint] = *vImageInfo;

				if ((&m_UBOFrag.use_sampler_position)[vBindingPoint] < 1.0f)
				{
					(&m_UBOFrag.use_sampler_position)[vBindingPoint] = 1.0f;
					NeedNewUBOUpload();
				}
			}
			else
			{
				if ((&m_UBOFrag.use_sampler_position)[vBindingPoint] > 0.0f)
				{
					(&m_UBOFrag.use_sampler_position)[vBindingPoint] = 0.0f;
					NeedNewUBOUpload();
				}
				
				m_ImageInfos[vBindingPoint] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* DeferredRenderer_Quad_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_FrameBufferPtr)
	{
		AutoResizeBuffer(m_FrameBufferPtr.get(), vOutSize);

		return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string DeferredRenderer_Quad_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	return str;
}

bool DeferredRenderer_Quad_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

bool DeferredRenderer_Quad_Pass::CreateUBO()
{
	ZoneScoped;

	auto size_in_bytes = sizeof(UBOFrag);
	m_UBOFragPtr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, size_in_bytes);
	m_DescriptorBufferInfo_Frag.buffer = m_UBOFragPtr->buffer;
	m_DescriptorBufferInfo_Frag.range = size_in_bytes;
	m_DescriptorBufferInfo_Frag.offset = 0;

	for (auto& info : m_ImageInfos)
	{
		info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
	}

	NeedNewUBOUpload();

	return true;
}

void DeferredRenderer_Quad_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBOFragPtr, &m_UBOFrag, sizeof(UBOFrag));
}

void DeferredRenderer_Quad_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBOFragPtr.reset();
}

bool DeferredRenderer_Quad_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	res &= AddOrSetLayoutDescriptor(4U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	res &= AddOrSetLayoutDescriptor(5U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	res &= AddOrSetLayoutDescriptor(6U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	res &= AddOrSetLayoutDescriptor(7U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	res &= AddOrSetLayoutDescriptor(8U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	res &= AddOrSetLayoutDescriptor(9U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	res &= AddOrSetLayoutDescriptor(10U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	return res;
}

bool DeferredRenderer_Quad_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo());
	res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_DescriptorBufferInfo_Frag);
	res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]); // position
	res &= AddOrSetWriteDescriptorImage(3U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[1]); // normal
	res &= AddOrSetWriteDescriptorImage(4U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[2]); // albedo
	res &= AddOrSetWriteDescriptorImage(5U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[3]); // diffuse
	res &= AddOrSetWriteDescriptorImage(6U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[4]); // specular
	res &= AddOrSetWriteDescriptorImage(7U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[5]); // attenuation
	res &= AddOrSetWriteDescriptorImage(8U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[6]); // mask
	res &= AddOrSetWriteDescriptorImage(9U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[7]); // ao
	res &= AddOrSetWriteDescriptorImage(10U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[8]); // shadow
	return res;
}

std::string DeferredRenderer_Quad_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "DeferredRenderer_Quad_Pass_Vertex";

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

std::string DeferredRenderer_Quad_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "DeferredRenderer_Quad_Pass_Fragment";

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
