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

#include "LaplacianModule_Quad_Pass.h"

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

LaplacianModule_Quad_Pass::LaplacianModule_Quad_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: QuadShaderPass(vVulkanCorePtr, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Quad Pass : Laplacian", QUAD_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

LaplacianModule_Quad_Pass::~LaplacianModule_Quad_Pass()
{
	Unit();
}

bool LaplacianModule_Quad_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

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

		DrawInputTexture(m_VulkanCorePtr, "Input", 0U, m_OutputRatio);
		//DrawInputTexture(m_VulkanCorePtr, "Output Blur", 0U, m_OutputRatio);

		return change;
	}

	return false;
}

void LaplacianModule_Quad_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

}

void LaplacianModule_Quad_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

}

void LaplacianModule_Quad_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
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
		}
	}
}

vk::DescriptorImageInfo* LaplacianModule_Quad_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_FrameBufferPtr)
	{
		if (vOutSize)
		{
			*vOutSize = m_FrameBufferPtr->GetOutputSize();
		}

		return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	CTOOL_DEBUG_BREAK;

	return nullptr;
}

bool LaplacianModule_Quad_Pass::CreateUBO()
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

void LaplacianModule_Quad_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, *m_UBO_Frag, &m_UBOFrag, sizeof(UBOFrag));
}

void LaplacianModule_Quad_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Frag.reset();
}

bool LaplacianModule_Quad_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);

	return true;
}

bool LaplacianModule_Quad_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Frag);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], nullptr); // ssao

	return true;
}

std::string LaplacianModule_Quad_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "LaplacianModule_Vertex";

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

std::string LaplacianModule_Quad_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "LaplacianModule_Quad_Pass";

	return u8R"(#version 450
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
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string LaplacianModule_Quad_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<lap_corner>" + ct::toStr(m_UBOFrag.u_lap_corner) + "</lap_corner>\n";
	str += vOffset + "<lap_offset>" + ct::toStr(m_UBOFrag.u_lap_offset) + "</lap_offset>\n";
	str += vOffset + "<discard_zeros>" + ct::toStr(m_UBOFrag.u_discard_zero) + "</discard_zeros>\n";

	return str;
}

bool LaplacianModule_Quad_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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