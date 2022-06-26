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

#include "HeatmapRenderer_Pass_1.h"

#include <functional>
#include <Gui/MainFrame.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <Generic/FrameBuffer.h>
using namespace vkApi;

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

HeatmapRenderer_Pass_1::HeatmapRenderer_Pass_1(vkApi::VulkanCore* vVulkanCore)
	: ShaderPass(vVulkanCore)
{
	SetRenderDocDebugName("Mesh Pass 1 : Heatmap", MESH_SHADER_PASS_DEBUG_COLOR);
}

HeatmapRenderer_Pass_1::~HeatmapRenderer_Pass_1()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void HeatmapRenderer_Pass_1::ActionBeforeInit()
{
	ClearColorBuffer();
	AddColorToBuffer(ct::fvec4(0.32f, 0.00f, 0.32f, 1.00f));
	AddColorToBuffer(ct::fvec4(0.00f, 0.00f, 1.00f, 1.00f));
	AddColorToBuffer(ct::fvec4(0.00f, 1.00f, 0.00f, 1.00f));
	AddColorToBuffer(ct::fvec4(1.00f, 1.00f, 0.00f, 1.00f));
	AddColorToBuffer(ct::fvec4(1.00f, 0.60f, 0.00f, 1.00f));
	AddColorToBuffer(ct::fvec4(1.00f, 0.00f, 0.00f, 1.00f));
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void HeatmapRenderer_Pass_1::DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	ZoneScoped;

	if (!m_Loaded) return;

	if (vCmdBuffer)
	{
		auto modelPtr = m_SceneModel.getValidShared();
		if (!modelPtr || modelPtr->empty()) return;

		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
		{
			VKFPScoped(*vCmdBuffer, "HeatmapRenderer", "DrawModel");

			vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout, 0, m_DescriptorSet, nullptr);

			for (auto meshPtr : *modelPtr)
			{
				if (meshPtr)
				{
					vk::DeviceSize offsets = 0;
					vCmdBuffer->bindVertexBuffers(0, meshPtr->GetVerticesBuffer(), offsets);

					if (meshPtr->GetIndicesCount())
					{
						vCmdBuffer->bindIndexBuffer(meshPtr->GetIndicesBuffer(), 0, vk::IndexType::eUint32);
						vCmdBuffer->drawIndexed(meshPtr->GetIndicesCount(), 1, 0, 0, 0);
					}
					else
					{
						vCmdBuffer->draw(meshPtr->GetVerticesCount(), 1, 0, 0);
					}
				}
			}
		}
	}
}

bool HeatmapRenderer_Pass_1::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	bool change = false;

	ImGui::Text("Component : ");
	ImGui::SameLine();
	if (ImGui::RadioButtonLabeled(0.0f, "R", m_UBOFrag.channel_idx == 0, false)) { change = true; m_UBOFrag.channel_idx = 0; }
	ImGui::SameLine();
	if (ImGui::RadioButtonLabeled(0.0f, "G", m_UBOFrag.channel_idx == 1, false)) { change = true; m_UBOFrag.channel_idx = 1; }
	ImGui::SameLine();
	if (ImGui::RadioButtonLabeled(0.0f, "B", m_UBOFrag.channel_idx == 2, false)) { change = true; m_UBOFrag.channel_idx = 2; }
	ImGui::SameLine();
	if (ImGui::RadioButtonLabeled(0.0f, "A", m_UBOFrag.channel_idx == 3, false)) { change = true; m_UBOFrag.channel_idx = 3; }

	uint32_t idx = 0;
	for (uint32_t i = 0; i < m_UBOFrag.count_colors; ++i)
	{
		ImGui::PushID(ImGui::IncPUSHID());
		change |= ImGui::ColorEdit4Default(0.0f, ct::toStr("Color %u", i).c_str(), &m_Colors[i].x, &m_DefaultColors[i].x);
		ImGui::PopID();
	}

	if (change)
	{
		NeedNewSBOUpload();
	}

	return change;
}

void HeatmapRenderer_Pass_1::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{

}

void HeatmapRenderer_Pass_1::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{

}

void HeatmapRenderer_Pass_1::SetModel(SceneModelWeak vSceneModel)
{
	ZoneScoped;

	m_SceneModel = vSceneModel;

	m_NeedModelUpdate = true;
}

vk::DescriptorImageInfo* HeatmapRenderer_Pass_1::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	if (m_FrameBufferPtr)
	{
		return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HeatmapRenderer_Pass_1::ClearColorBuffer()
{
	m_Colors.clear();
	m_DefaultColors.clear();
}

void HeatmapRenderer_Pass_1::AddColorToBuffer(const ct::fvec4& vColor)
{
	m_Colors.push_back(vColor);
	m_DefaultColors.push_back(vColor);
	m_UBOFrag.count_colors = (int32_t)m_Colors.size();
}

void HeatmapRenderer_Pass_1::DestroyModel(const bool& vReleaseDatas)
{
	ZoneScoped;

	if (vReleaseDatas)
	{
		m_SceneModel.reset();
	}
}

bool HeatmapRenderer_Pass_1::CreateSBO()
{
	ZoneScoped;

	m_SBO_Colors.reset();

	const auto sizeInBytes = sizeof(ct::fvec4) * m_Colors.size();
	m_SBO_Colors = VulkanRessource::createStorageBufferObject(m_VulkanCore, sizeInBytes, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU);
	
	const auto color_buffer_size = sizeof(ct::fvec4);
	m_SBO_Empty_Colors = VulkanRessource::createStorageBufferObject(m_VulkanCore, color_buffer_size, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);

	NeedNewSBOUpload();

	return true;
}

void HeatmapRenderer_Pass_1::UploadSBO()
{
	if (m_SBO_Colors)
	{
		const auto sizeInBytes = sizeof(ct::fvec4) * m_Colors.size();
		VulkanRessource::upload(m_VulkanCore, *m_SBO_Colors, m_Colors.data(), sizeInBytes);
	}
}

void HeatmapRenderer_Pass_1::DestroySBO()
{
	ZoneScoped;

	m_SBO_Colors.reset();
	m_SBO_Empty_Colors.reset();
}

bool HeatmapRenderer_Pass_1::CreateUBO()
{
	ZoneScoped;

	m_UBO_Vert = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBOVert));
	if (m_UBO_Vert)
	{
		m_DescriptorBufferInfo_Vert.buffer = m_UBO_Vert->buffer;
		m_DescriptorBufferInfo_Vert.range = sizeof(UBOVert);
		m_DescriptorBufferInfo_Vert.offset = 0;
	}

	m_UBO_Frag = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBOFrag));
	if (m_UBO_Frag)
	{
		m_DescriptorBufferInfo_Frag.buffer = m_UBO_Frag->buffer;
		m_DescriptorBufferInfo_Frag.range = sizeof(UBOFrag);
		m_DescriptorBufferInfo_Frag.offset = 0;
	}

	NeedNewUBOUpload();

	return true;
}

void HeatmapRenderer_Pass_1::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCore, *m_UBO_Vert, &m_UBOVert, sizeof(UBOVert));
	VulkanRessource::upload(m_VulkanCore, *m_UBO_Frag, &m_UBOFrag, sizeof(UBOFrag));
}

void HeatmapRenderer_Pass_1::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Vert.reset();
	m_UBO_Frag.reset();
}

bool HeatmapRenderer_Pass_1::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(3U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eFragment);

	return true;
}

bool HeatmapRenderer_Pass_1::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();
	writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, CommonSystem::Instance()->GetBufferInfo());
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Vert);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Frag);
	if (m_Colors.empty())
	{
		writeDescriptorSets.emplace_back(m_DescriptorSet, 3U, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &m_SBO_Empty_Colors->bufferInfo);
	}
	else
	{
		writeDescriptorSets.emplace_back(m_DescriptorSet, 3U, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &m_SBO_Colors->bufferInfo);
	}

	return true;
}

void HeatmapRenderer_Pass_1::SetInputStateBeforePipelineCreation()
{
	VertexStruct::P3_N3_TA3_BTA3_T2_C4::GetInputState(m_InputState);
}

std::string HeatmapRenderer_Pass_1::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "HeatmapRenderer_Pass_1_Vertex";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBiTangent;
layout(location = 4) in vec2 aUv;
layout(location = 5) in vec4 aColor;

layout(location = 0) out vec4 vertColor;
)"
+ CommonSystem::Instance()->GetBufferObjectStructureHeader(0U) +
u8R"(
layout (std140, binding = 1) uniform UBO_Vert 
{ 
	mat4 transform;
};

void main() 
{
	vertColor = aColor;
	gl_Position = cam * transform * vec4(aPosition, 1.0);
}
)";
}

std::string HeatmapRenderer_Pass_1::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "HeatmapRenderer_Pass_1_Fragment";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec4 vertColor;
)"
+ CommonSystem::Instance()->GetBufferObjectStructureHeader(0U) +
u8R"(
layout(std140, binding = 2) uniform UBO_Frag 
{ 
	uint channel_idx;	// 0..3
	uint color_count;	// 0..N
};

layout(std140, binding = 3) readonly buffer SBO_Frag 
{ 
	vec4 colors[];		// N Colors
};

vec4 HeatMapColor(float value, float minValue, float maxValue)
{
    float ratio = (color_count-1.0) * clamp((value-minValue) / (maxValue-minValue), 0.0, 1.0);
    uint indexMin = uint(floor(ratio));
    uint indexMax = min(indexMin+1,color_count-1);
    return mix(colors[indexMin], colors[indexMax], ratio-indexMin);
}

void main() 
{
	fragColor = vec4(0);

	if (vertColor[channel_idx] > 0.0)
	{
		fragColor = HeatMapColor(vertColor[channel_idx], 0.0, 1.0);
		fragColor.a = 1.0;
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

std::string HeatmapRenderer_Pass_1::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + ct::toStr("<component>%u</component>\n", m_UBOFrag.channel_idx);
	str += vOffset + "<color_levels>\n";
	for (auto color : m_Colors)
	{
		str += vOffset + ct::toStr("\t<color>%s</color>\n", color.string().c_str());
	}
	str += vOffset + "</color_levels>\n";

	return str;
}

bool HeatmapRenderer_Pass_1::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strParentName == "heatmap_renderer")
	{
		if (strName == "color_levels")
			m_Colors.clear();
		else if (strName == "component")
			m_UBOFrag.channel_idx = ct::ivariant(strValue).GetI();

		NeedNewUBOUpload();
	}

	if (strParentName == "color_levels")
	{
		if (strName == "color")
			m_Colors.push_back(ct::fvariant(strValue).GetV4());

		NeedNewSBOUpload();
	}

	return true;
}

void HeatmapRenderer_Pass_1::DrawTexture(const char* vLabel, const uint32_t& vIdx)
{
	/*if (vLabel && vIdx >= 0U && vIdx <= m_SamplesImageInfos.size())
	{
		auto imguiRendererPtr = m_VulkanCore->GetVulkanImGuiRenderer().getValidShared();
		if (imguiRendererPtr)
		{
			if (ImGui::CollapsingHeader(vLabel, ImGuiTreeNodeFlags_DefaultOpen))
			{
				m_ImGuiTexture[vIdx].SetDescriptor(imguiRendererPtr.get(),
					&m_SamplesImageInfos[vIdx],
					ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y).ratioXY<float>());

				if (m_ImGuiTexture[vIdx].canDisplayPreview)
				{
					int w = (int)ImGui::GetContentRegionAvail().x;
					auto rect = ct::GetScreenRectWithRatio<int32_t>(m_ImGuiTexture[vIdx].ratio, ct::ivec2(w, w), false);
					const ImVec2 pos = ImVec2((float)rect.x, (float)rect.y);
					const ImVec2 siz = ImVec2((float)rect.w, (float)rect.h);
					ImGui::ImageRect((ImTextureID)&m_ImGuiTexture[vIdx].descriptor, pos, siz);
				}
			}
		}
	}*/
}