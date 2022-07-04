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

#include "GrayScottModule_Pass.h"

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
//// SSAO SECOND PASS : BLUR /////////////////////////////////
//////////////////////////////////////////////////////////////

GrayScottModule_Pass::GrayScottModule_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: QuadShaderPass(vVulkanCorePtr, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Quad Pass : GrayScott", QUAD_SHADER_PASS_DEBUG_COLOR);
}

GrayScottModule_Pass::~GrayScottModule_Pass()
{
	Unit();
}

bool GrayScottModule_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (ImGui::CollapsingHeader("GrayScott", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool change = false;

		if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen))
		{
			change |= ImGui::SliderIntDefaultCompact(0.0f, "Radius", &m_UBOFrag.u_blur_radius, 1, 10, 4);
			change |= ImGui::SliderFloatDefaultCompact(0.0f, "Offset", &m_UBOFrag.u_blur_offset, 0.0f, 10.0f, 1.0f);
			change |= ImGui::SliderFloatDefaultCompact(0.0f, "Inf Threshold", &m_UBOFrag.u_blur_smooth_inf, 0.0f, 1.0f, 0.0f);
			change |= ImGui::SliderFloatDefaultCompact(0.0f, "Sup Threshold", &m_UBOFrag.u_blur_smooth_sup, 0.0f, 1.0f, 1.0f);
			change |= ImGui::SliderFloatDefaultCompact(0.0f, "Power", &m_UBOFrag.u_blur_power, 0.0f, 2.0f, 1.0f);

			if (change)
			{
				NeedNewUBOUpload();
			}
		}

		DrawInputTexture(m_VulkanCorePtr, "Input", 0U, m_OutputRatio);
		//DrawInputTexture(m_VulkanCorePtr, "Output GrayScott", 0U, m_OutputRatio);

		return change;
	}

	return false;
}

void GrayScottModule_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{

}

void GrayScottModule_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{

}

void GrayScottModule_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
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

vk::DescriptorImageInfo* GrayScottModule_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	if (m_FrameBufferPtr)
	{
		return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	CTOOL_DEBUG_BREAK;

	return nullptr;
}

bool GrayScottModule_Pass::CreateUBO()
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

void GrayScottModule_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, *m_UBO_Frag, &m_UBOFrag, sizeof(UBOFrag));
}

void GrayScottModule_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Frag.reset();
}

bool GrayScottModule_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);

	return true;
}

bool GrayScottModule_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Frag);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], nullptr); // ssao

	return true;
}

std::string GrayScottModule_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "GrayScottModule_Vertex";

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

std::string GrayScottModule_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "GrayScottModule_Pass";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 v_uv;

layout (std140, binding = 1) uniform UBO_Frag
{
	uint u_blur_radius; // default is 4
	float u_blur_offset; // default is 1.0
	float u_blur_smooth_inf; // default is 0.0
	float u_blur_smooth_sup; // default is 1.0
	float u_blur_power; // default is 1.0
};
layout(binding = 2) uniform sampler2D input_map_sampler;

void main() 
{
	fragColor = vec4(0.0);
	
	vec4 tex = texture(input_map_sampler, v_uv);
	if (dot(tex,tex) > 0.0)
	{
		const uint blur_radius = max(u_blur_radius, 1);
		const uint blur_radius_radius = blur_radius * blur_radius;

		const float blur_radius_f = float(blur_radius);
		const float blur_radius_radius_f = float(blur_radius_radius);
		
		vec2 pix = u_blur_offset / textureSize(input_map_sampler, 0) / blur_radius_f;

		vec4 ao = vec4(0.0);
		
		for (uint i = 0 ; i < blur_radius_radius; ++i)
		{
			float x = floor(i / blur_radius_f);
			float y = mod(float(i), blur_radius_f);
			vec2 p = vec2(x, y) * 2.0 - 1.0;
			vec2 uv_off = v_uv + p * pix;
			ao += texture(input_map_sampler, uv_off);
		}

		ao /= blur_radius_radius_f;
		
		// post pro for remove facets
		ao = smoothstep(vec4(u_blur_smooth_inf), vec4(u_blur_smooth_sup), ao);
		ao = pow(ao, vec4(u_blur_power));
		
		fragColor = ao;
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

std::string GrayScottModule_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<blur_radius>" + ct::toStr(m_UBOFrag.u_blur_radius) + "</blur_radius>\n";
	str += vOffset + "<blur_offset>" + ct::toStr(m_UBOFrag.u_blur_offset) + "</blur_offset>\n";
	str += vOffset + "<blur_smooth_inf>" + ct::toStr(m_UBOFrag.u_blur_smooth_inf) + "</blur_smooth_inf>\n";
	str += vOffset + "<blur_smooth_sup>" + ct::toStr(m_UBOFrag.u_blur_smooth_sup) + "</blur_smooth_sup>\n";
	str += vOffset + "<blur_power>" + ct::toStr(m_UBOFrag.u_blur_power) + "</blur_power>\n";

	return str;
}

bool GrayScottModule_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strParentName == "blur_module")
	{
		if (strName == "blur_radius")
			m_UBOFrag.u_blur_radius = ct::ivariant(strValue).GetI();
		else if (strName == "blur_offset")
			m_UBOFrag.u_blur_offset = ct::fvariant(strValue).GetF();
		else if (strName == "blur_smooth_inf")
			m_UBOFrag.u_blur_smooth_inf = ct::fvariant(strValue).GetF();
		else if (strName == "blur_smooth_sup")
			m_UBOFrag.u_blur_smooth_sup = ct::fvariant(strValue).GetF();
		else if (strName == "blur_power")
			m_UBOFrag.u_blur_power = ct::fvariant(strValue).GetF();

		NeedNewUBOUpload();
	}

	return true;
}