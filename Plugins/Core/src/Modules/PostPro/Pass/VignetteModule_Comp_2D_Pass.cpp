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

#include "VignetteModule_Comp_2D_Pass.h"

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

VignetteModule_Comp_2D_Pass::VignetteModule_Comp_2D_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : Vignette", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

VignetteModule_Comp_2D_Pass::~VignetteModule_Comp_2D_Pass()
{
	Unit();
}

void VignetteModule_Comp_2D_Pass::ActionBeforeInit()
{
	//m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);

	for (auto& info : m_ImageInfos)
	{
		info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
	}
}

bool VignetteModule_Comp_2D_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	ZoneScoped;

	bool change = false;

	change |= DrawResizeWidget();

	change |= ImGui::SliderFloatDefaultCompact(0.0f, "Width", &m_UBO_1_Comp.u_Width, 0.000f, 0.500f, 0.250f, 0.0f, "%.3f");
	change |= ImGui::SliderFloatDefaultCompact(0.0f, "Intensity", &m_UBO_1_Comp.u_Intensity, 0.000f, 30.000f, 15.000f, 0.0f, "%.3f");
	if (change)
	{
		NeedNewUBOUpload();
	}


	return change;
}

void VignetteModule_Comp_2D_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	ZoneScoped;
}

void VignetteModule_Comp_2D_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	ZoneScoped;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void VignetteModule_Comp_2D_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
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

					NeedResizeByHandIfChanged(m_ImageInfosSize[0]);
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

vk::DescriptorImageInfo* VignetteModule_Comp_2D_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;
	if (m_ComputeBufferPtr)
	{
		AutoResizeBuffer(m_ComputeBufferPtr.get(), vOutSize);

		return m_ComputeBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}
	return nullptr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void VignetteModule_Comp_2D_Pass::WasJustResized()
{
	ZoneScoped;
}

void VignetteModule_Comp_2D_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
		{
			VKFPScoped(*vCmdBuffer, "Vignette", "Compute");

			vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);

			for (uint32_t iter = 0; iter < m_CountIterations.w; iter++)
			{
				Dispatch(vCmdBuffer);
			}
		}
	}
}

bool VignetteModule_Comp_2D_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBO_1_Comp_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_1_Comp));
	m_UBO_1_Comp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_1_Comp_Ptr)
	{
		m_UBO_1_Comp_BufferInfos.buffer = m_UBO_1_Comp_Ptr->buffer;
		m_UBO_1_Comp_BufferInfos.range = sizeof(UBO_1_Comp);
		m_UBO_1_Comp_BufferInfos.offset = 0;
	}

	NeedNewUBOUpload();

	return true;
}

void VignetteModule_Comp_2D_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_1_Comp_Ptr, &m_UBO_1_Comp, sizeof(UBO_1_Comp));
}

void VignetteModule_Comp_2D_Pass::DestroyUBO()
{
	ZoneScoped;
	m_UBO_1_Comp_Ptr.reset();
	m_UBO_1_Comp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
}

bool VignetteModule_Comp_2D_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);
	return res;
}

bool VignetteModule_Comp_2D_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	if (m_ComputeBufferPtr)
	{
		bool res = true;
		res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U)); // output
		res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_UBO_1_Comp_BufferInfos);
		res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]);
		return res;
	}

	return false;
}
std::string VignetteModule_Comp_2D_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "VignetteModule_Comp_2D_Pass_Compute";

	SetLocalGroupSize(ct::uvec3(8U, 8U, 1U));

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform image2D colorBuffer;

layout(std140, binding = 1) uniform UBO_1_Comp
{
	float u_Width;
	float u_Intensity;
};

layout(binding = 2) uniform sampler2D input_mask;

void main()
{
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

	vec2 uv = vec2(coords) / textureSize(input_mask, 0);
    uv *= 1.0 - uv.yx;
    float vignette_effect = pow(uv.x * uv.y * u_Intensity, u_Width);

	vec4 res = texelFetch(input_mask, coords, 0) * vignette_effect;

	imageStore(colorBuffer, coords, res); 
}
)";
		}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string VignetteModule_Comp_2D_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += ShaderPass::getXml(vOffset, vUserDatas);

	str += vOffset + "<width>" + ct::toStr(m_UBO_1_Comp.u_Width) + "</width>\n";
	str += vOffset + "<intensity>" + ct::toStr(m_UBO_1_Comp.u_Intensity) + "</intensity>\n";
	return str;
}

bool VignetteModule_Comp_2D_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "vignette_module")
	{
		if (strName == "width")
			m_UBO_1_Comp.u_Width = ct::fvariant(strValue).GetF();
		else if (strName == "intensity")
			m_UBO_1_Comp.u_Intensity = ct::fvariant(strValue).GetF();
	}

	return true;
}

void VignetteModule_Comp_2D_Pass::AfterNodeXmlLoading()
{
	// code to do after end of the xml loading of this node
}
