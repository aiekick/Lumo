/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "CurveRendererModule_Mesh_Pass.h"

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

CurveRendererModule_Mesh_Pass::CurveRendererModule_Mesh_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: MeshShaderPass<VertexStruct::P3_N3_TA3_BTA3_T2_C4>(vVulkanCorePtr, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Mesh Pass : Curve Renderer", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	//m_DontUseShaderFilesOnDisk = true;
}

CurveRendererModule_Mesh_Pass::~CurveRendererModule_Mesh_Pass()
{
	Unit();
}

void CurveRendererModule_Mesh_Pass::ActionBeforeInit()
{
	//m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);

}

bool CurveRendererModule_Mesh_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	ZoneScoped;

	bool change = false;

	//change |= DrawResizeWidget();

	change |= ImGui::SliderFloatDefaultCompact(0.0f, "line_thickness", &m_UBO_0_Vert.u_line_thickness, 0.000f, 2.000f, 1.000f, 0.0f, "%.3f");
	if (change)
	{
		NeedNewUBOUpload();
	}


	return change;
}

void CurveRendererModule_Mesh_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	ZoneScoped;
}

void CurveRendererModule_Mesh_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	ZoneScoped;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL INPUT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void CurveRendererModule_Mesh_Pass::SetModel(SceneModelWeak vSceneModel)
{	
	ZoneScoped;

	m_SceneModel = vSceneModel;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* CurveRendererModule_Mesh_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

void CurveRendererModule_Mesh_Pass::WasJustResized()
{
	ZoneScoped;
}

void CurveRendererModule_Mesh_Pass::DrawModel(vk::CommandBuffer * vCmdBuffer, const int& vIterationNumber)
{
	ZoneScoped;

	if (!m_Loaded) return;

	if (vCmdBuffer)
	{
		auto modelPtr = m_SceneModel.getValidShared();
		if (!modelPtr || modelPtr->empty()) return;

		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipelines[0].m_Pipeline);
		{
			VKFPScoped(*vCmdBuffer, "Curve Renderer", "DrawModel");

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


bool CurveRendererModule_Mesh_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBO_0_Vert_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_0_Vert));
	m_UBO_0_Vert_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_0_Vert_Ptr)
	{
		m_UBO_0_Vert_BufferInfos.buffer = m_UBO_0_Vert_Ptr->buffer;
		m_UBO_0_Vert_BufferInfos.range = sizeof(UBO_0_Vert);
		m_UBO_0_Vert_BufferInfos.offset = 0;
	}


	NeedNewUBOUpload();

	return true;
}

void CurveRendererModule_Mesh_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_0_Vert_Ptr, &m_UBO_0_Vert, sizeof(UBO_0_Vert));

}

void CurveRendererModule_Mesh_Pass::DestroyUBO()
{
	ZoneScoped;
	m_UBO_0_Vert_Ptr.reset();
	m_UBO_0_Vert_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

}
bool CurveRendererModule_Mesh_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_DescriptorSets[0].m_LayoutBindings.clear();
	m_DescriptorSets[0].m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);

	return true;
}

bool CurveRendererModule_Mesh_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	m_DescriptorSets[0].m_WriteDescriptorSets.clear();
	m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(
		m_DescriptorSets[0].m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_UBO_0_Vert_BufferInfos);
	
	return true;
}
std::string CurveRendererModule_Mesh_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "CurveRendererModule_Mesh_Pass_Vertex";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBiTangent;
layout(location = 4) in vec2 aUv;
layout(location = 5) in vec4 aColor;


layout(std140, binding = 0) uniform UBO_0_Vert
{
	float u_line_thickness;
};

layout(location = 0) out vec4 vertColor;
)
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

std::string CurveRendererModule_Mesh_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "CurveRendererModule_Mesh_Pass_Fragment";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec4 vertColor;



+ CommonSystem::GetBufferObjectStructureHeader(0U) +
u8R"(
layout(std140, binding = 2) uniform UBO_Frag 
{ 
	//uint channel_idx;	// 0..3
	//uint color_count;	// 0..N
};

void main() 
{
	fragColor = vec4(0);
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string CurveRendererModule_Mesh_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += ShaderPass::getXml(vOffset, vUserDatas);

	str += vOffset + "<line_thickness>" + ct::toStr(m_UBO_0_Vert.u_line_thickness) + "</line_thickness>\n";
	return str;
}

bool CurveRendererModule_Mesh_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "curve_renderer_module")
	{

		if (strName == "line_thickness")
			m_UBO_0_Vert.u_line_thickness = ct::fvariant(strValue).GetF();
	}

	return true;
}

void CurveRendererModule_Mesh_Pass::AfterNodeXmlLoading()
{
	// code to do after end of the xml loading of this node
}
