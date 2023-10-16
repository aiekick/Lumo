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

#include "FlatGradientModule_Comp_2D_Pass.h"

#include <cinttypes>
#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
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

FlatGradientModule_Comp_2D_Pass::FlatGradientModule_Comp_2D_Pass(GaiApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	ZoneScoped;

	SetRenderDocDebugName("Comp Pass : Flat Gradient", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

FlatGradientModule_Comp_2D_Pass::~FlatGradientModule_Comp_2D_Pass()
{
	ZoneScoped;

	Unit();
}

void FlatGradientModule_Comp_2D_Pass::ActionBeforeInit()
{
	ZoneScoped;

	for (auto& info : m_ImageInfos)
	{
		info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
	}
}

bool FlatGradientModule_Comp_2D_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	ZoneScoped;

	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);

	bool change = false;

	if (m_SceneVariables[0].expired())
	{
		change |= ImGui::SliderUIntDefaultCompact(0.0f, "Count_Step", &m_UBOComp.u_Count_Step, 1U, 128U, 10U);
	}

	if (change)
	{
		NeedNewUBOUpload();
	}

	return change;
}

bool FlatGradientModule_Comp_2D_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool FlatGradientModule_Comp_2D_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void FlatGradientModule_Comp_2D_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
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
//// VARIABLE SLOT INPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void FlatGradientModule_Comp_2D_Pass::SetVariable(const uint32_t& vVarIndex, SceneVariableWeak vSceneVariable)
{	
	ZoneScoped;

	if (vVarIndex < m_SceneVariables.size())
	{
		m_SceneVariables[vVarIndex] = vSceneVariable;

		auto varPtr = vSceneVariable.lock();
		if (varPtr)
		{
			m_UBOComp.u_Count_Step = varPtr->GetDatas().m_Uint32;
			NeedNewUBOUpload();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* FlatGradientModule_Comp_2D_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

void FlatGradientModule_Comp_2D_Pass::WasJustResized()
{
	ZoneScoped;
}

void FlatGradientModule_Comp_2D_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
		{
			//VKFPScoped(*vCmdBuffer, "Flat Gradient", "Compute");

			vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
			Dispatch(vCmdBuffer);
		}
	}
}

bool FlatGradientModule_Comp_2D_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBOComp_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Comp));
	m_UBOComp_BufferInfos = vk::DescriptorBufferInfo { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBOComp_Ptr)
	{
		m_UBOComp_BufferInfos.buffer = m_UBOComp_Ptr->buffer;
		m_UBOComp_BufferInfos.range = sizeof(UBO_Comp);
		m_UBOComp_BufferInfos.offset = 0;
	}

	NeedNewUBOUpload();

	return true;
}

void FlatGradientModule_Comp_2D_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBOComp_Ptr, &m_UBOComp, sizeof(UBO_Comp));
}

void FlatGradientModule_Comp_2D_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBOComp_Ptr.reset();
	m_UBOComp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
}

bool FlatGradientModule_Comp_2D_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);
	return res;
}

bool FlatGradientModule_Comp_2D_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U)); // output
	res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_UBOComp_BufferInfos);
	res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]);
	res &= AddOrSetWriteDescriptorImage(3U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[1]);
	return res;
}

std::string FlatGradientModule_Comp_2D_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "FlatGradientModule_Comp_2D_Pass_Compute";

	SetLocalGroupSize(ct::uvec3(1U, 1U, 1U));

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform image2D colorBuffer;

layout(std140, binding = 1) uniform UBO_Comp
{
	uint u_Count_Step;
};

layout(binding = 2) uniform sampler2D u_start_sampler;
layout(binding = 3) uniform sampler2D u_end_sampler;

void main()
{
	const vec2 size = vec2(imageSize(colorBuffer));
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	const vec2 v_nor_uv = clamp(vec2(coords) / size, 0.0, 1.0);
	
	const vec4 start_color = texture(u_start_sampler, v_nor_uv);
	const vec4 end_color = texture(u_end_sampler, v_nor_uv);

	const float f_u_Count_Step = float(u_Count_Step);
	const float ratio = floor(v_nor_uv.x * f_u_Count_Step) / f_u_Count_Step;

	vec4 color = mix(start_color, end_color, vec4(ratio));

	imageStore(colorBuffer, coords, color); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string FlatGradientModule_Comp_2D_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += ShaderPass::getXml(vOffset, vUserDatas);

	str += vOffset + "<count_step>" + ct::toStr(m_UBOComp.u_Count_Step) + "</count_step>\n";

	return str;
}

bool FlatGradientModule_Comp_2D_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "flat_gradient_module")
	{

		if (strName == "count_step")
			m_UBOComp.u_Count_Step = ct::uvariant(strValue).GetU();
	}

	return true;
}

void FlatGradientModule_Comp_2D_Pass::AfterNodeXmlLoading()
{
	ZoneScoped;

	// code to do after end of the xml loading of this node
	// by ex :
	NeedNewUBOUpload();
}
