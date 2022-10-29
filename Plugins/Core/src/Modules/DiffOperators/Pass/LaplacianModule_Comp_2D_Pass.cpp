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

#include "LaplacianModule_Comp_2D_Pass.h"

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

LaplacianModule_Comp_2D_Pass::LaplacianModule_Comp_2D_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	ZoneScoped;

	SetRenderDocDebugName("Comp Pass : Laplacian", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

LaplacianModule_Comp_2D_Pass::~LaplacianModule_Comp_2D_Pass()
{
	ZoneScoped;

	Unit();
}

void LaplacianModule_Comp_2D_Pass::ActionBeforeInit()
{
	ZoneScoped;

	for (auto& info : m_ImageInfos)
	{
		info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
	}
}

bool LaplacianModule_Comp_2D_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (ImGui::CollapsingHeader("Laplacian", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool change = false;

		if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen))
		{
			change |= ImGui::ContrastedComboVectorDefault(0.0f, "Method", &m_UBOComp.method, m_MethodNames, 0);
			change |= ImGui::SliderFloatDefaultCompact(0.0f, "Corner", &m_UBOComp.u_lap_corner, 0.0f, 1.0f, 0.2f);
			change |= ImGui::SliderIntDefaultCompact(0.0f, "Offset", &m_UBOComp.u_lap_offset, 0, 10, 1);
			change |= ImGui::CheckBoxFloatDefault("Discard Zero values", &m_UBOComp.u_discard_zero, false);

			if (change)
			{
				NeedNewUBOUpload();
			}
		}

		return change;
	}

	return false;
}

void LaplacianModule_Comp_2D_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
}

void LaplacianModule_Comp_2D_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void LaplacianModule_Comp_2D_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
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

vk::DescriptorImageInfo* LaplacianModule_Comp_2D_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;
	if (m_ComputeBufferPtr)
	{
		if (vOutSize)
		{
			*vOutSize = m_ComputeBufferPtr->GetOutputSize();
		}

		return m_ComputeBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LaplacianModule_Comp_2D_Pass::WasJustResized()
{
	ZoneScoped;
}

void LaplacianModule_Comp_2D_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
		{
			VKFPScoped(*vCmdBuffer, "Laplacian", "Compute");

			vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);

			for (uint32_t iter = 0; iter < m_CountIterations.w; iter++)
			{
				Dispatch(vCmdBuffer);
			}
		}
	}
}


bool LaplacianModule_Comp_2D_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBOComp_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Comp));
	m_UBOComp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBOComp_Ptr)
	{
		m_UBOComp_BufferInfos.buffer = m_UBOComp_Ptr->buffer;
		m_UBOComp_BufferInfos.range = sizeof(UBO_Comp);
		m_UBOComp_BufferInfos.offset = 0;
	}

	NeedNewUBOUpload();

	return true;
}

void LaplacianModule_Comp_2D_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBOComp_Ptr, &m_UBOComp, sizeof(UBO_Comp));
}

void LaplacianModule_Comp_2D_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBOComp_Ptr.reset();
	m_UBOComp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
}

bool LaplacianModule_Comp_2D_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);

	return res;
}

bool LaplacianModule_Comp_2D_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U)); // output
	res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_UBOComp_BufferInfos);
	res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]); // ssao

	return res;
}

std::string LaplacianModule_Comp_2D_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "LaplacianModule_Comp_2D_Pass_Compute";

	SetLocalGroupSize(ct::uvec3(1U, 1U, 1U));

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform image2D colorBuffer;

layout(std140, binding = 1) uniform UBO_Comp
{
	int method;			// 0 -> 7
	int u_lap_offset; // default is 1
	float u_lap_corner; // default is 0.2
	float u_discard_zero; // default is 0.0 (false)
};

layout(binding = 2) uniform sampler2D input_map_sampler;

float getValue(vec4 v) 
{
	switch(method)
	{
	case 0: // r
		return v.r;
	case 1: // g
		return v.g;
	case 2: // b
		return v.b;
	case 3: // a
		return v.a;
	case 4: // length(rg)
		return length(v.rg);
	case 5: // length(rgb)
		return length(v.rgb);
	case 6: // length(rgba)
		return length(v);
	case 7: // median(rgb)
		// https://github.com/Chlumsky/msdfgen
		return max(min(v.r, v.g), min(max(v.r, v.g), v.b));
	}
	return v.r;
}

float getSam(ivec2 co, int x, int y)
{
	return getValue(texelFetch(input_map_sampler, co + ivec2(x,y), 0));
}

float getLap(ivec2 co)
{
	float l  = getSam(co, -u_lap_offset,	0);
	float lt = getSam(co, -u_lap_offset,	u_lap_offset);
	float t  = getSam(co, 0,				u_lap_offset);
	float rt = getSam(co, u_lap_offset,		u_lap_offset);
	float r  = getSam(co, u_lap_offset,		0);
	float rb = getSam(co, u_lap_offset,		-u_lap_offset);
	float b  = getSam(co, 0,				-u_lap_offset);
	float lb = getSam(co, -u_lap_offset,	-u_lap_offset);
	
	float lap_corner = clamp(u_lap_corner, 0.0, 1.0);
	float lap_side = 1.0 - lap_corner;
	return (l + t + r + b) * 0.25 * lap_side + (lt + rt + rb + lb) * 0.25 * lap_corner; // - c; done in external
}

void main()
{
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

	float c = getSam(coords, 0, 0);
	
	float value = 0.0;

	if (u_discard_zero > 0.5)
	{
		if (dot(c, c) > 0.0)
		{
			value = getLap(coords) - c;
		}
	}
	else
	{
		value = getLap(coords) - c;
	}

	imageStore(colorBuffer, coords, vec4(value)); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string LaplacianModule_Comp_2D_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += ShaderPass::getXml(vOffset, vUserDatas);

	str += vOffset + "<method>" + ct::toStr(m_UBOComp.method) + "</method>\n";
	str += vOffset + "<lap_corner>" + ct::toStr(m_UBOComp.u_lap_corner) + "</lap_corner>\n";
	str += vOffset + "<lap_offset>" + ct::toStr(m_UBOComp.u_lap_offset) + "</lap_offset>\n";
	str += vOffset + "<discard_zeros>" + ct::toStr(m_UBOComp.u_discard_zero) + "</discard_zeros>\n";

	return str;
}

bool LaplacianModule_Comp_2D_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "laplacian_module")
	{
		if (strName == "method")
			m_UBOComp.method = ct::ivariant(strValue).GetI();
		else if (strName == "lap_corner")
			m_UBOComp.u_lap_corner = ct::fvariant(strValue).GetF();
		else if (strName == "lap_offset")
			m_UBOComp.u_lap_offset = ct::ivariant(strValue).GetI();
		else if (strName == "discard_zeros")
			m_UBOComp.u_discard_zero = ct::ivariant(strValue).GetB();
	}

	return true;
}

void LaplacianModule_Comp_2D_Pass::AfterNodeXmlLoading()
{
	ZoneScoped;

	// code to do after end of the xml loading of this node
	// by ex :
	NeedNewUBOUpload();
}
