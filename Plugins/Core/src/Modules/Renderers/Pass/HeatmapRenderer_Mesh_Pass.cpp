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

#include "HeatmapRenderer_Mesh_Pass.h"

#include <functional>
#include <Gui/MainFrame.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <Base/FrameBuffer.h>
using namespace vkApi;

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

HeatmapRenderer_Mesh_Pass::HeatmapRenderer_Mesh_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Mesh Pass 1 : Heatmap", MESH_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

HeatmapRenderer_Mesh_Pass::~HeatmapRenderer_Mesh_Pass()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void HeatmapRenderer_Mesh_Pass::ActionBeforeInit()
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

void HeatmapRenderer_Mesh_Pass::DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	ZoneScoped;

	if (!m_Loaded) return;

	if (vCmdBuffer)
	{
		auto modelPtr = m_SceneModel.getValidShared();
		if (!modelPtr || modelPtr->empty()) return;

		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipelines[0].m_Pipeline);
		{
			VKFPScoped(*vCmdBuffer, "HeatmapRenderer", "DrawModel");

			vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);

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

bool HeatmapRenderer_Mesh_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

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

void HeatmapRenderer_Mesh_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void HeatmapRenderer_Mesh_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void HeatmapRenderer_Mesh_Pass::SetModel(SceneModelWeak vSceneModel)
{
	ZoneScoped;

	m_SceneModel = vSceneModel;
}

vk::DescriptorImageInfo* HeatmapRenderer_Mesh_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_FrameBufferPtr)
	{
		AutoResizeBuffer(m_FrameBufferPtr.get(), vOutSize);

		return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HeatmapRenderer_Mesh_Pass::ClearColorBuffer()
{
	m_Colors.clear();
	m_DefaultColors.clear();
}

void HeatmapRenderer_Mesh_Pass::AddColorToBuffer(const ct::fvec4& vColor)
{
	m_Colors.push_back(vColor);
	m_DefaultColors.push_back(vColor);
	m_UBOFrag.count_colors = (int32_t)m_Colors.size();
}

void HeatmapRenderer_Mesh_Pass::DestroyModel(const bool& vReleaseDatas)
{
	ZoneScoped;

	if (vReleaseDatas)
	{
		m_SceneModel.reset();
	}
}

bool HeatmapRenderer_Mesh_Pass::CreateSBO()
{
	ZoneScoped;

	m_SBO_Colors.reset();

	const auto sizeInBytes = sizeof(ct::fvec4) * m_Colors.size();
	m_SBO_Colors = VulkanRessource::createStorageBufferObject(m_VulkanCorePtr, sizeInBytes, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU);
	if (m_SBO_Colors && m_SBO_Colors->buffer)
	{
		m_SBO_ColorsDescriptorBufferInfo = vk::DescriptorBufferInfo{ m_SBO_Colors->buffer, 0, sizeInBytes };
	}
	else
	{
		m_SBO_ColorsDescriptorBufferInfo = vk::DescriptorBufferInfo { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	}

	NeedNewSBOUpload();

	return true;
}

void HeatmapRenderer_Mesh_Pass::UploadSBO()
{
	if (m_SBO_Colors)
	{
		const auto sizeInBytes = sizeof(ct::fvec4) * m_Colors.size();
		VulkanRessource::upload(m_VulkanCorePtr, m_SBO_Colors, m_Colors.data(), sizeInBytes);
	}
}

void HeatmapRenderer_Mesh_Pass::DestroySBO()
{
	ZoneScoped;

	m_SBO_Colors.reset();
}

bool HeatmapRenderer_Mesh_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBOVertPtr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBOVert));
	if (m_UBOVertPtr)
	{
		m_DescriptorBufferInfo_Vert.buffer = m_UBOVertPtr->buffer;
		m_DescriptorBufferInfo_Vert.range = sizeof(UBOVert);
		m_DescriptorBufferInfo_Vert.offset = 0;
	}

	m_UBOFragPtr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBOFrag));
	if (m_UBOFragPtr)
	{
		m_DescriptorBufferInfo_Frag.buffer = m_UBOFragPtr->buffer;
		m_DescriptorBufferInfo_Frag.range = sizeof(UBOFrag);
		m_DescriptorBufferInfo_Frag.offset = 0;
	}

	NeedNewUBOUpload();

	return true;
}

void HeatmapRenderer_Mesh_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBOVertPtr, &m_UBOVert, sizeof(UBOVert));
	VulkanRessource::upload(m_VulkanCorePtr, m_UBOFragPtr, &m_UBOFrag, sizeof(UBOFrag));
}

void HeatmapRenderer_Mesh_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBOVertPtr.reset();
	m_UBOFragPtr.reset();
}

bool HeatmapRenderer_Mesh_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex);
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
	res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment);
	return res;
}

bool HeatmapRenderer_Mesh_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo());
	res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_DescriptorBufferInfo_Vert);
	res &= AddOrSetWriteDescriptorBuffer(2U, vk::DescriptorType::eUniformBuffer, &m_DescriptorBufferInfo_Frag);
	res &= AddOrSetWriteDescriptorBuffer(3U, vk::DescriptorType::eStorageBuffer, &m_SBO_ColorsDescriptorBufferInfo);
	return res;
}

void HeatmapRenderer_Mesh_Pass::SetInputStateBeforePipelineCreation()
{
	VertexStruct::P3_N3_TA3_BTA3_T2_C4::GetInputState(m_InputState);
}

std::string HeatmapRenderer_Mesh_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "HeatmapRenderer_Mesh_Pass_Vertex";

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
+ CommonSystem::GetBufferObjectStructureHeader(0U) +
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

std::string HeatmapRenderer_Mesh_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "HeatmapRenderer_Mesh_Pass_Fragment";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec4 vertColor;
)"
+ CommonSystem::GetBufferObjectStructureHeader(0U) +
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

std::string HeatmapRenderer_Mesh_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
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

bool HeatmapRenderer_Mesh_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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
