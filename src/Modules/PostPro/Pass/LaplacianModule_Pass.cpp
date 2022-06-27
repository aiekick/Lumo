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

#include "LaplacianModule_Pass.h"

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
//// Laplacian SECOND PASS : BLUR ////////////////////////////
//////////////////////////////////////////////////////////////

LaplacianModule_Pass::LaplacianModule_Pass(vkApi::VulkanCore* vVulkanCore)
	: QuadShaderPass(vVulkanCore, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Quad Pass : Laplacian", QUAD_SHADER_PASS_DEBUG_COLOR);
}

LaplacianModule_Pass::~LaplacianModule_Pass()
{
	Unit();
}

bool LaplacianModule_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (ImGui::CollapsingHeader("Laplacian", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool change = false;

		if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen))
		{
			change |= ImGui::SliderFloatDefaultCompact(0.0f, "Corner", &m_UBOFrag.u_lap_corner, 0.0f, 1.0f, 0.2f);
			change |= ImGui::SliderFloatDefaultCompact(0.0f, "Offset", &m_UBOFrag.u_lap_offset, 0.0f, 10.0f, 1.0f);
			change |= ImGui::CheckBoxFloatDefault("Discard Zero values", &m_UBOFrag.u_discard_zero, false);

			if (change)
			{
				NeedNewUBOUpload();
			}
		}

		DrawInputTexture(m_VulkanCore, "Input", 0U, m_OutputRatio);
		//DrawInputTexture(m_VulkanCore, "Output Blur", 0U, m_OutputRatio);

		return change;
	}

	return false;
}

void LaplacianModule_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{

}

void LaplacianModule_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{

}

void LaplacianModule_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding < m_SamplerImageInfos.size())
		{
			if (vImageInfo)
			{
				m_SamplerImageInfos[vBinding] = *vImageInfo;
			}
			else
			{
				if (m_EmptyTexturePtr)
				{
					m_SamplerImageInfos[vBinding] = m_EmptyTexturePtr->m_DescriptorImageInfo;
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

vk::DescriptorImageInfo* LaplacianModule_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	if (m_FrameBufferPtr)
	{
		return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	CTOOL_DEBUG_BREAK;

	return nullptr;
}

bool LaplacianModule_Pass::CreateUBO()
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

void LaplacianModule_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCore, *m_UBO_Frag, &m_UBOFrag, sizeof(UBOFrag));
}

void LaplacianModule_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Frag.reset();
	m_EmptyTexturePtr.reset();
}

bool LaplacianModule_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);

	return true;
}

bool LaplacianModule_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Frag);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_SamplerImageInfos[0], nullptr); // ssao

	return true;
}

std::string LaplacianModule_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "LaplacianModule_Vertex";

	auto shader_path = FileHelper::Instance()->GetAppPath() + "/shaders/LaplacianModule.vert";

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

std::string LaplacianModule_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "LaplacianModule_Pass";

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
	float u_lap_offset; // default is 1.0
	float u_lap_corner; // default is 0.2
	float u_discard_zero; // default is 0.0 (false)
};
layout(binding = 2) uniform sampler2D input_map_sampler;

vec4 getSam(float x, float y)
{
	return texture(input_map_sampler, v_uv + vec2(x,y));
}

vec4 getLap()
{
	vec2 p = u_lap_offset / textureSize(input_map_sampler, 0);

	vec4 l  = 	getSam(-p.x,  0.0);
	vec4 lt = 	getSam(-p.x,  p.y);
	vec4 t  = 	getSam(0.0,  p.y);
	vec4 rt = 	getSam(p.x,  p.y);
	vec4 r  = 	getSam(p.x,  0.0);
	vec4 rb = 	getSam(p.x, -p.y);
	vec4 b  = 	getSam(0.0, -p.y);
	vec4 lb = 	getSam(-p.x, -p.y);
	
	float lap_corner = clamp(u_lap_corner, 0.0, 1.0);
	float lap_side = 1.0 - lap_corner;
	return (l + t + r + b) * 0.25 * lap_side + (lt + rt + rb + lb) * 0.25 * lap_corner; // - c; done in external
}

void main() 
{
	fragColor = vec4(0.0);
	
	vec4 c = texture(input_map_sampler, v_uv);
	
	if (u_discard_zero > 0.5)
	{
		if (dot(c, c) > 0.0)
		{
			fragColor = getLap() - c;
		}
		else
		{
			discard;
		}
	}
	else
	{
		fragColor = getLap() - c;
	}
	
	fragColor.a = c.a;
}
)";
		FileHelper::Instance()->SaveStringToFile(m_FragmentShaderCode, shader_path);
	}

	return m_FragmentShaderCode;
}

void LaplacianModule_Pass::UpdateShaders(const std::set<std::string>& vFiles)
{
	bool needReCompil = false;

	if (vFiles.find("shaders/LaplacianModule.vert") != vFiles.end())
	{
		auto shader_path = FileHelper::Instance()->GetAppPath() + "/shaders/LaplacianModule.vert";
		if (FileHelper::Instance()->IsFileExist(shader_path))
		{
			m_VertexShaderCode = FileHelper::Instance()->LoadFileToString(shader_path);
			needReCompil = true;
		}

	}
	else if (vFiles.find("shaders/LaplacianModule_Pass.frag") != vFiles.end())
	{
		auto shader_path = FileHelper::Instance()->GetAppPath() + "/shaders/LaplacianModule_Pass.frag";
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

std::string LaplacianModule_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<lap_corner>" + ct::toStr(m_UBOFrag.u_lap_corner) + "</lap_corner>\n";
	str += vOffset + "<lap_offset>" + ct::toStr(m_UBOFrag.u_lap_offset) + "</lap_offset>\n";
	str += vOffset + "<discard_zeros>" + ct::toStr(m_UBOFrag.u_discard_zero) + "</discard_zeros>\n";

	return str;
}

bool LaplacianModule_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strParentName == "laplacian_module")
	{
		if (strName == "lap_corner")
			m_UBOFrag.u_lap_corner = ct::fvariant(strValue).GetF();
		else if (strName == "lap_offset")
			m_UBOFrag.u_lap_offset = ct::fvariant(strValue).GetF();
		else if (strName == "discard_zeros")
			m_UBOFrag.u_discard_zero = ct::ivariant(strValue).GetB();

		NeedNewUBOUpload();
	}

	return true;
}