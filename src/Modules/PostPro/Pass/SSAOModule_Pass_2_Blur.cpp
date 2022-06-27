/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "SSAOModule_Pass_2_Blur.h"

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
#include <Generic/FrameBuffer.h>

using namespace vkApi;

#define COUNT_BUFFERS 2

//////////////////////////////////////////////////////////////
//// SSAO SECOND PASS : BLUR /////////////////////////////////
//////////////////////////////////////////////////////////////

SSAOModule_Pass_2_Blur::SSAOModule_Pass_2_Blur(vkApi::VulkanCore* vVulkanCore)
	: QuadShaderPass(vVulkanCore, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Quad Pass 2 : Blur", QUAD_SHADER_PASS_DEBUG_COLOR);
}

SSAOModule_Pass_2_Blur::~SSAOModule_Pass_2_Blur()
{
	Unit();
}

bool SSAOModule_Pass_2_Blur::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (ImGui::CollapsingHeader("Pass 2 : Blur"))
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

		DrawInputTexture(m_VulkanCore, "Input SSAO", 0U, m_OutputRatio);
		//DrawInputTexture(m_VulkanCore, "Output Blur", 0U, m_OutputRatio);

		return change;
	}

	return false;
}

void SSAOModule_Pass_2_Blur::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{

}

void SSAOModule_Pass_2_Blur::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{

}

void SSAOModule_Pass_2_Blur::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding >= 2U && vBinding <= 2U)
		{
			if (vImageInfo)
			{
				m_SamplerImageInfos[vBinding - 2U] = *vImageInfo;
			}
			else
			{
				if (m_EmptyTexturePtr)
				{
					m_SamplerImageInfos[vBinding - 2U] = m_EmptyTexturePtr->m_DescriptorImageInfo;
				}
				else
				{
					CTOOL_DEBUG_BREAK;
				}
			}

			m_NeedSamplerUpdate = true;
		}
	}
}

vk::DescriptorImageInfo* SSAOModule_Pass_2_Blur::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	if (m_FrameBufferPtr)
	{
		return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	CTOOL_DEBUG_BREAK;

	return nullptr;
}

bool SSAOModule_Pass_2_Blur::CreateUBO()
{
	ZoneScoped;

	auto size_in_bytes = sizeof(UBOFrag);
	m_UBO_Frag = VulkanRessource::createUniformBufferObject(m_VulkanCore, size_in_bytes);
	m_DescriptorBufferInfo_Frag.buffer = m_UBO_Frag->buffer;
	m_DescriptorBufferInfo_Frag.range = size_in_bytes;
	m_DescriptorBufferInfo_Frag.offset = 0;

	m_EmptyTexturePtr = Texture2D::CreateEmptyTexture(m_VulkanCore, ct::uvec2(1, 1), vk::Format::eR8G8B8A8Unorm);

	for (auto& a : m_SamplerImageInfos)
	{
		a = m_EmptyTexturePtr->m_DescriptorImageInfo;
	}

	NeedNewUBOUpload();

	return true;
}

void SSAOModule_Pass_2_Blur::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCore, *m_UBO_Frag, &m_UBOFrag, sizeof(UBOFrag));
}

void SSAOModule_Pass_2_Blur::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Frag.reset();
	m_EmptyTexturePtr.reset();
}

bool SSAOModule_Pass_2_Blur::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);

	return true;
}

bool SSAOModule_Pass_2_Blur::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Frag);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_SamplerImageInfos[0], nullptr); // ssao

	return true;
}

std::string SSAOModule_Pass_2_Blur::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "SSAOModule_Vertex";

	auto shader_path = FileHelper::Instance()->GetAppPath() + "/shaders/SSAOModule.vert";

	if (FileHelper::Instance()->IsFileExist(shader_path))
	{
		m_VertexShaderCode = FileHelper::Instance()->LoadFileToString(shader_path);
	}
	else
	{
		m_VertexShaderCode = u8R"(#version 450
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
		FileHelper::Instance()->SaveStringToFile(m_VertexShaderCode, shader_path);
	}

	return m_VertexShaderCode;
}

std::string SSAOModule_Pass_2_Blur::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "SSAOModule_Pass_2_Blur";

	auto shader_path = FileHelper::Instance()->GetAppPath() + "/shaders/" + vOutShaderName + ".frag";

	if (FileHelper::Instance()->IsFileExist(shader_path))
	{
		m_FragmentShaderCode = FileHelper::Instance()->LoadFileToString(shader_path);
	}
	else
	{
		m_FragmentShaderCode = u8R"(#version 450
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
layout(binding = 2) uniform sampler2D ssao_map_sampler;

void main() 
{
	fragColor = vec4(0.0);
	
	if (texture(ssao_map_sampler, v_uv).r > 0.0)
	{
		const uint blur_radius = max(u_blur_radius, 1);
		const uint blur_radius_radius = blur_radius * blur_radius;

		const float blur_radius_f = float(blur_radius);
		const float blur_radius_radius_f = float(blur_radius_radius);
		
		vec2 pix = u_blur_offset / textureSize(ssao_map_sampler, 0) / blur_radius_f;

		float ao = 0.0;
		
		for (uint i = 0 ; i < blur_radius_radius; ++i)
		{
			float x = floor(i / blur_radius_f);
			float y = mod(float(i), blur_radius_f);
			vec2 p = vec2(x, y) * 2.0 - 1.0;
			vec2 uv_off = v_uv + p * pix;
			ao += texture(ssao_map_sampler, uv_off).r;
		}

		ao /= blur_radius_radius_f;
		
		// post pro for remove facets
		ao = smoothstep(u_blur_smooth_inf, u_blur_smooth_sup, ao);
		ao = pow(ao, u_blur_power);
		
		fragColor = vec4(ao, ao, ao, 1.0);
	}
	else
	{
		//discard;
	}
}
)";
		FileHelper::Instance()->SaveStringToFile(m_FragmentShaderCode, shader_path);
	}

	return m_FragmentShaderCode;
}

void SSAOModule_Pass_2_Blur::UpdateShaders(const std::set<std::string>& vFiles)
{
	bool needReCompil = false;

	if (vFiles.find("shaders/SSAOModule.vert") != vFiles.end())
	{
		auto shader_path = FileHelper::Instance()->GetAppPath() + "/shaders/SSAOModule.vert";
		if (FileHelper::Instance()->IsFileExist(shader_path))
		{
			m_VertexShaderCode = FileHelper::Instance()->LoadFileToString(shader_path);
			needReCompil = true;
		}

	}
	else if (vFiles.find("shaders/SSAOModule_Pass_2_Blur.frag") != vFiles.end())
	{
		auto shader_path = FileHelper::Instance()->GetAppPath() + "/shaders/SSAOModule_Pass_2_Blur.frag";
		if (FileHelper::Instance()->IsFileExist(shader_path))
		{
			m_FragmentShaderCode = FileHelper::Instance()->LoadFileToString(shader_path);
			needReCompil = true;
		}
	}

	if (needReCompil)
	{
		ReCompil();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string SSAOModule_Pass_2_Blur::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<blur_radius>" + ct::toStr(m_UBOFrag.u_blur_radius) + "</blur_radius>\n";
	str += vOffset + "<blur_offset>" + ct::toStr(m_UBOFrag.u_blur_offset) + "</blur_offset>\n";
	str += vOffset + "<blur_smooth_inf>" + ct::toStr(m_UBOFrag.u_blur_smooth_inf) + "</blur_smooth_inf>\n";
	str += vOffset + "<blur_smooth_sup>" + ct::toStr(m_UBOFrag.u_blur_smooth_sup) + "</blur_smooth_sup>\n";
	str += vOffset + "<blur_power>" + ct::toStr(m_UBOFrag.u_blur_power) + "</blur_power>\n";

	return str;
}

bool SSAOModule_Pass_2_Blur::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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