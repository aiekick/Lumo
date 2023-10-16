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

#include "VisuHexGridModule_Vertex_Pass.h"

#include <cinttypes>
#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImGuiPack.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Gaia/Buffer/FrameBuffer.h>

using namespace GaiApi;

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

VisuHexGridModule_Vertex_Pass::VisuHexGridModule_Vertex_Pass(GaiApi::VulkanCorePtr vVulkanCorePtr)
	: VertexShaderPass(vVulkanCorePtr)
{
	ZoneScoped;

	SetRenderDocDebugName("Vertex Pass : Visu Hex Grid", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	//m_DontUseShaderFilesOnDisk = true;
}

VisuHexGridModule_Vertex_Pass::~VisuHexGridModule_Vertex_Pass()
{
	ZoneScoped;

	Unit();
}

void VisuHexGridModule_Vertex_Pass::ActionBeforeInit()
{
	ZoneScoped;

	//m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);

	//SetPrimitveTopology(vk::PrimitiveTopology::eTriangleList); // display Triangles
	//m_LineWidth.x = 0.5f;	// min value
	//m_LineWidth.y = 10.0f;	// max value
	//m_LineWidth.z = 2.0f;	// default value
	//m_LineWidth.w;			// value to change
	for (auto& info : m_ImageInfos)
	{
		info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
	}
}

bool VisuHexGridModule_Vertex_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	ZoneScoped;

	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);

	bool change = false;

	change |= DrawResizeWidget();

	change |= ImGui::SliderFloatDefaultCompact(0.0f, "Name1", &m_UBO_Frag.u_Name1, 0.000f, 0.000f, 0.000f, 0.0f, "%.3f");
	change |= ImGui::SliderFloatDefaultCompact(0.0f, "Name2", &m_UBO_Vert.u_Name2, 0.000f, 0.000f, 0.000f, 0.0f, "%.3f");

	if (change)
	{
		NeedNewUBOUpload();
	}

	return change;
}

bool VisuHexGridModule_Vertex_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;

	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool VisuHexGridModule_Vertex_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;

	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void VisuHexGridModule_Vertex_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{	
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBindingPoint < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				if (vTextureSize)
				{
					m_ImageInfosSize[vBindingPoint] = *vTextureSize;

					NeedResizeByHandIfChanged(m_ImageInfosSize[vBindingPoint]);
				}

				m_ImageInfos[vBindingPoint] = *vImageInfo;
			}
			else
			{
				m_ImageInfos[vBindingPoint] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* VisuHexGridModule_Vertex_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

void VisuHexGridModule_Vertex_Pass::WasJustResized()
{
	ZoneScoped;
}


bool VisuHexGridModule_Vertex_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBO_Frag_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Frag));
	m_UBO_Frag_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_Frag_Ptr)
	{
		m_UBO_Frag_BufferInfos.buffer = m_UBO_Frag_Ptr->buffer;
		m_UBO_Frag_BufferInfos.range = sizeof(UBO_Frag);
		m_UBO_Frag_BufferInfos.offset = 0;
	}

	m_UBO_Vert_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Vert));
	m_UBO_Vert_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_Vert_Ptr)
	{
		m_UBO_Vert_BufferInfos.buffer = m_UBO_Vert_Ptr->buffer;
		m_UBO_Vert_BufferInfos.range = sizeof(UBO_Vert);
		m_UBO_Vert_BufferInfos.offset = 0;
	}

	NeedNewUBOUpload();

	return true;
}

void VisuHexGridModule_Vertex_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_Frag_Ptr, &m_UBO_Frag, sizeof(UBO_Frag));
	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_Vert_Ptr, &m_UBO_Vert, sizeof(UBO_Vert));
}

void VisuHexGridModule_Vertex_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Frag_Ptr.reset();
	m_UBO_Frag_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	m_UBO_Vert_Ptr.reset();
	m_UBO_Vert_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
}

bool VisuHexGridModule_Vertex_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex);

	return res;
}

bool VisuHexGridModule_Vertex_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, &m_UBO_Frag_BufferInfos);
	res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_UBO_Vert_BufferInfos);
	
	return res;
}

std::string VisuHexGridModule_Vertex_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "VisuHexGridModule_Vertex_Pass_Vertex";

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 vertColor;

layout(std140, binding = 1) uniform UBO_Vert
{
	float u_Name2;
};

)"
+ CommonSystem::GetBufferObjectStructureHeader(0U) +
u8R"(
layout(std140, binding = 1) uniform UBOStruct {
	
	float axisSize;
};

void main() 
{
	float vertexId = float(gl_VertexIndex);
	
	float astep = 3.14159 * 2.0 / 70.;
	float a = astep * float(vertexId) * (uTime+10.) * .3;
	vec2 d = a  * vec2(cos(a), sin(a)) / 100.;
	d.x *= screenSize.y/screenSize.x;
	if (vertexId<150.) gl_Position = cam * vec4(d,0,1);
	else gl_Position = cam * vec4(0,0,0,1);
	vertColor = vec4(1,1,1,1);
)";
		}

std::string VisuHexGridModule_Vertex_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "VisuHexGridModule_Vertex_Pass_Fragment";

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec4 vertColor;

layout(std140, binding = 0) uniform UBO_Frag
{
	float u_Name1;
};

void main() 
{
	fragColor = vertColor; 
}
)";
		}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string VisuHexGridModule_Vertex_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += ShaderPass::getXml(vOffset, vUserDatas);

	str += vOffset + "<name1>" + ct::toStr(m_UBO_Frag.u_Name1) + "</name1>\n";
	str += vOffset + "<name2>" + ct::toStr(m_UBO_Vert.u_Name2) + "</name2>\n";

	return str;
}

bool VisuHexGridModule_Vertex_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "visu_hex_grid_module")
	{

		if (strName == "name1")
			m_UBO_Frag.u_Name1 = ct::fvariant(strValue).GetF();
		else if (strName == "name2")
			m_UBO_Vert.u_Name2 = ct::fvariant(strValue).GetF();
	}

	return true;
}

void VisuHexGridModule_Vertex_Pass::AfterNodeXmlLoading()
{
	ZoneScoped;

	// code to do after end of the xml loading of this node
	// by ex :
	NeedNewUBOUpload();
}
