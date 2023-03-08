/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "ModelRendererModule_Mesh_Pass.h"

#include <cinttypes>
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
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

ModelRendererModule_Mesh_Pass::ModelRendererModule_Mesh_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: MeshShaderPass<VertexStruct::P3_N3_TA3_BTA3_T2_C4>(vVulkanCorePtr, MeshShaderPassType::PIXEL)
{
	ZoneScoped;

	SetRenderDocDebugName("Mesh Pass : Model Renderer", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

ModelRendererModule_Mesh_Pass::~ModelRendererModule_Mesh_Pass()
{
	ZoneScoped;

	Unit();
}

void ModelRendererModule_Mesh_Pass::ActionBeforeInit()
{
	ZoneScoped;

	//m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);

	SetDynamicallyChangePrimitiveTopology(true);
	SetPrimitveTopology(vk::PrimitiveTopology::eTriangleList);
	m_PrimitiveTopologiesIndex = (int32_t)vk::PrimitiveTopology::eTriangleList;
	m_LineWidth.x = 0.5f;	// min value
	m_LineWidth.y = 10.0f;	// max value
	m_LineWidth.z = 2.0f;	// default value
	m_LineWidth.w; // value to change
}

bool ModelRendererModule_Mesh_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	bool change = false;

	change |= DrawResizeWidget();

	ImGui::Separator();

	if (ImGui::ContrastedComboVectorDefault(0.0f, "Display Mode", &m_PrimitiveTopologiesIndex, m_PrimitiveTopologies, 0))
	{
		ChangeDynamicPrimitiveTopology((vk::PrimitiveTopology)m_PrimitiveTopologiesIndex, true);
	}
	
	change |= ImGui::ContrastedComboVectorDefault(0.0f, "Channel", &m_UBO_Frag.u_show_layer, m_Channels, 0);

	if (m_PrimitiveTopologiesIndex == 1 ||
		m_PrimitiveTopologiesIndex == 2)
	{
		change |= ImGui::SliderFloatDefaultCompact(0.0f, "Line Thickness", &m_LineWidth.w, m_LineWidth.x, m_LineWidth.y, m_LineWidth.z);
	}
	else if (m_PrimitiveTopologiesIndex == 0)
	{
		change |= ImGui::SliderFloatDefaultCompact(0.0f, "point_size", &m_UBO_Vert.u_point_size, 0.000f, 2.000f, 1.000f, 0.0f, "%.3f");
	}
	
	if (m_PrimitiveTopologiesIndex > 2) // "Triangle List" or "Triangle Strip" or "Triangle Fan"
	{
		change |= ImGui::CheckBoxIntDefault("Show Shaded Wireframe", &m_UBO_Frag.u_show_shaded_wireframe, 0);
	}

	auto modelPtr = m_SceneModel.getValidShared();
	if (modelPtr || !modelPtr->empty())
	{
		auto meshPtr = modelPtr->at(0).lock();
		if (meshPtr && meshPtr->GetIndicesCount())
		{
			change |= ImGui::CheckBoxBoolDefault("Use Indices Restriction", &m_UseIndiceRestriction, false);
			if (m_UseIndiceRestriction)
			{
				change |= ImGui::SliderUIntDefaultCompact(0.0f, "count indices", &m_RestrictedIndicesCountToDraw, 0, meshPtr->GetIndicesCount(), meshPtr->GetIndicesCount());
			}
		}
	}

	if (change)
	{
		NeedNewUBOUpload();
	}

	return change;
}

void ModelRendererModule_Mesh_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
}

void ModelRendererModule_Mesh_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL INPUT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void ModelRendererModule_Mesh_Pass::SetModel(SceneModelWeak vSceneModel)
{	
	ZoneScoped;

	m_SceneModel = vSceneModel;

	auto modelPtr = m_SceneModel.getValidShared();
	if (modelPtr || !modelPtr->empty())
	{
		auto meshPtr = modelPtr->at(0).lock();
		if (meshPtr && meshPtr->GetIndicesCount())
		{
			if (!m_UseIndiceRestriction)
			{
				m_RestrictedIndicesCountToDraw = meshPtr->GetIndicesCount();
			}
			else
			{
				m_RestrictedIndicesCountToDraw = ct::clamp(m_RestrictedIndicesCountToDraw, 0U, meshPtr->GetIndicesCount());
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* ModelRendererModule_Mesh_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;

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
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ModelRendererModule_Mesh_Pass::WasJustResized()
{
	ZoneScoped;
}

void ModelRendererModule_Mesh_Pass::DrawModel(vk::CommandBuffer * vCmdBuffer, const int& vIterationNumber)
{
	ZoneScoped;

	if (!m_Loaded) return;

	if (vCmdBuffer)
	{
		auto modelPtr = m_SceneModel.getValidShared();
		if (!modelPtr || modelPtr->empty()) return;

		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipelines[0].m_Pipeline);
		{
			VKFPScoped(*vCmdBuffer, "Model Renderer", "DrawModel");

			vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, 
				m_Pipelines[0].m_PipelineLayout, 0, 
				m_DescriptorSets[0].m_DescriptorSet, nullptr);

			for (auto meshPtr : *modelPtr)
			{
				if (meshPtr)
				{
					vk::DeviceSize offsets = 0;
					vCmdBuffer->bindVertexBuffers(0, meshPtr->GetVerticesBuffer(), offsets);

					if (meshPtr->GetIndicesCount())
					{
						vCmdBuffer->bindIndexBuffer(meshPtr->GetIndicesBuffer(), 0, vk::IndexType::eUint32);
						if (m_UseIndiceRestriction)
						{
							vCmdBuffer->drawIndexed(m_RestrictedIndicesCountToDraw, 1, 0, 0, 0);
						}
						else
						{
							vCmdBuffer->drawIndexed(meshPtr->GetIndicesCount(), 1, 0, 0, 0);
						}
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

bool ModelRendererModule_Mesh_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBO_Vert_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Vert));
	m_UBO_Vert_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_Vert_Ptr)
	{
		m_UBO_Vert_BufferInfos.buffer = m_UBO_Vert_Ptr->buffer;
		m_UBO_Vert_BufferInfos.range = sizeof(UBO_Vert);
		m_UBO_Vert_BufferInfos.offset = 0;
	}

	m_UBO_Frag_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Frag));
	m_UBO_Frag_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
	if (m_UBO_Frag_Ptr)
	{
		m_UBO_Frag_BufferInfos.buffer = m_UBO_Frag_Ptr->buffer;
		m_UBO_Frag_BufferInfos.range = sizeof(UBO_Frag);
		m_UBO_Frag_BufferInfos.offset = 0;
	}

	NeedNewUBOUpload();

	return true;
}

void ModelRendererModule_Mesh_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_Vert_Ptr, &m_UBO_Vert, sizeof(UBO_Vert));
	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_Frag_Ptr, &m_UBO_Frag, sizeof(UBO_Frag));
}

void ModelRendererModule_Mesh_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Vert_Ptr.reset();
	m_UBO_Vert_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	m_UBO_Frag_Ptr.reset();
	m_UBO_Frag_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
}

bool ModelRendererModule_Mesh_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex);
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex);
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
	return res;
}

bool ModelRendererModule_Mesh_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo());
	res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_UBO_Vert_BufferInfos);
	res &= AddOrSetWriteDescriptorBuffer(2U, vk::DescriptorType::eUniformBuffer, &m_UBO_Frag_BufferInfos);
	return res;
}

std::string ModelRendererModule_Mesh_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ModelRendererModule_Mesh_Pass_Vertex";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBiTangent;
layout(location = 4) in vec2 aUv;
layout(location = 5) in vec4 aColor;

layout(location = 0) out vec3 vertPosition;
layout(location = 1) out vec3 vertNormal;
layout(location = 2) out vec3 vertTangent;
layout(location = 3) out vec3 vertBiTangent;
layout(location = 4) out vec2 vertUv;
layout(location = 5) out vec4 vertColor;
)"
+ CommonSystem::GetBufferObjectStructureHeader(0U) +
u8R"(
layout(std140, binding = 1) uniform UBO_Vert
{
	float u_point_size;
};

void main() 
{
	gl_PointSize = u_point_size;

	vertPosition = aPosition;
	vertNormal = aNormal;
	vertTangent = aTangent;
	vertBiTangent = aBiTangent;
	vertUv = aUv;
	vertColor = aColor;

	gl_Position = cam * vec4(aPosition, 1.0);
}
)";
}

std::string ModelRendererModule_Mesh_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ModelRendererModule_Mesh_Pass_Fragment";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec3 vertTangent;
layout(location = 3) in vec3 vertBiTangent;
layout(location = 4) in vec2 vertUv;
layout(location = 5) in vec4 vertColor;

layout(std140, binding = 2) uniform UBO_Frag
{
	int u_show_layer;
	int u_show_shaded_wireframe;
};

void main() 
{
	fragColor = vec4(0);

	switch(u_show_layer)
	{
	case 0: // pos
		fragColor.xyz = vertPosition;
		break;
	case 1: // normal
		fragColor.xyz = vertNormal * 0.5 + 0.5;
		break;
	case 2: // tan
		fragColor.xyz = vertTangent;
		break;
	case 3: // bi tan
		fragColor.xyz = vertBiTangent;
		break;
	case 4: // uv
		fragColor.xyz = vec3(vertUv, 0.0);
		break;
	case 5: // vertex color
		fragColor = vertColor;
		break;
	}

	if (u_show_shaded_wireframe == 1 &&
		bitCount(gl_SampleMaskIn[0]) < 2)
	{
		fragColor.rgb = 1.0 - fragColor.rgb;
	}

	if (dot(fragColor.xyz, fragColor.xyz) > 0.0)
	{
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

std::string ModelRendererModule_Mesh_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += ShaderPass::getXml(vOffset, vUserDatas);

	str += vOffset + "<display_mode>" + ct::toStr(m_PrimitiveTopologiesIndex) + "</display_mode>\n";
	str += vOffset + "<line_thickness>" + ct::toStr(m_LineWidth.w) + "</line_thickness>\n";
	str += vOffset + "<point_size>" + ct::toStr(m_UBO_Vert.u_point_size) + "</point_size>\n";
	str += vOffset + "<show_layer>" + ct::toStr(m_UBO_Frag.u_show_layer) + "</show_layer>\n";
	str += vOffset + "<use_indices_restriction>" + (m_UseIndiceRestriction ? "true" : "false") + "</use_indices_restriction>\n";
	str += vOffset + "<restricted_indices_count_to_draw>" + ct::toStr(m_RestrictedIndicesCountToDraw) + "</restricted_indices_count_to_draw>\n";
	
	return str;
}

bool ModelRendererModule_Mesh_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	ZoneScoped;

	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	ShaderPass::setFromXml(vElem, vParent, vUserDatas);

	if (strParentName == "model_renderer_module")
	{
		if (strName == "display_mode")
			m_PrimitiveTopologiesIndex = ct::fvariant(strValue).GetI();
		else if (strName == "line_thickness")
			m_LineWidth.w = ct::fvariant(strValue).GetF();
		else if (strName == "point_size")
			m_UBO_Vert.u_point_size = ct::fvariant(strValue).GetF();
		else if (strName == "show_layer")
			m_UBO_Frag.u_show_layer = ct::ivariant(strValue).GetI();
		else if (strName == "use_indices_restriction")
			m_UseIndiceRestriction = ct::ivariant(strValue).GetB();
		else if (strName == "restricted_indices_count_to_draw")
			m_RestrictedIndicesCountToDraw = ct::ivariant(strValue).GetU();
	}

	return true;
}

void ModelRendererModule_Mesh_Pass::AfterNodeXmlLoading()
{
	ZoneScoped;

	// code to do after end of the xml loading of this node

	ChangeDynamicPrimitiveTopology((vk::PrimitiveTopology)m_PrimitiveTopologiesIndex, true);
	NeedNewUBOUpload();
}
