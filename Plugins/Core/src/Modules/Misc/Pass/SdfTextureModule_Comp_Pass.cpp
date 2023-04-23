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

#include "SdfTextureModule_Comp_Pass.h"

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

//////////////////////////////////////////////////////////////
//// SSAO SECOND PASS : BLUR /////////////////////////////////
//////////////////////////////////////////////////////////////

SdfTextureModule_Comp_Pass::SdfTextureModule_Comp_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : SdfTexture", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	//m_DontUseShaderFilesOnDisk = true;
}

SdfTextureModule_Comp_Pass::~SdfTextureModule_Comp_Pass()
{
	Unit();
}

void SdfTextureModule_Comp_Pass::ActionBeforeInit()
{
	ZoneScoped;

	m_Pipelines.resize(4U);
	m_DescriptorSets.resize(4U);

	vk::PushConstantRange push_constant;
	push_constant.offset = 0;
	push_constant.size = sizeof(PushConstants);
	push_constant.stageFlags = vk::ShaderStageFlagBits::eCompute;

	SetPushConstantRange(push_constant);
}

void SdfTextureModule_Comp_Pass::ActionBeforeCompilation()
{
	ZoneScoped;

	ClearShaderEntryPoints();
	AddShaderEntryPoints(vk::ShaderStageFlagBits::eCompute, "Init");
	AddShaderEntryPoints(vk::ShaderStageFlagBits::eCompute, "DistancePassH");
	AddShaderEntryPoints(vk::ShaderStageFlagBits::eCompute, "DistancePassV");
	AddShaderEntryPoints(vk::ShaderStageFlagBits::eCompute, "DisplaySDF");
}

bool SdfTextureModule_Comp_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (ImGui::CollapsingHeader("SdfTexture", ImGuiTreeNodeFlags_DefaultOpen))
	{
		return false;
	}

	return false;
}

void SdfTextureModule_Comp_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void SdfTextureModule_Comp_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); ImGui::SetCurrentContext(vContext);
}

void SdfTextureModule_Comp_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				if (vTextureSize)
				{
					m_ImageInfosSize[vBinding] = *vTextureSize;
					SetDispatchSize2D(ct::uvec2((uint32_t)vTextureSize->x, (uint32_t)vTextureSize->y));
					NeedResizeByHandIfChanged(m_ImageInfosSize[0]);
				}
				else
				{
					m_ImageInfosSize[vBinding] = ct::fvec2();
				}

				m_ImageInfos[vBinding] = *vImageInfo;
			}
			else
			{
				SetDispatchSize2D(0);
				m_ImageInfosSize[vBinding] = ct::fvec2();
				m_ImageInfos[vBinding] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* SdfTextureModule_Comp_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

bool SdfTextureModule_Comp_Pass::CanUpdateDescriptors()
{
	ZoneScoped;

	return (m_ComputeBufferPtr != nullptr);
}

void SdfTextureModule_Comp_Pass::Compute_SdfTexture(vk::CommandBuffer* vCmdBuffer, const uint32_t& vIdx)
{
	ZoneScoped;

	if (vCmdBuffer && !GetDispatchSize().emptyOR())
	{
#ifdef _MSC_VER
#undef MemoryBarrier
#endif 
		if (vIdx == 0U || vIdx == 3U)
		{
			vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[vIdx].m_Pipeline);
			vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[vIdx].m_PipelineLayout, 0, m_DescriptorSets[vIdx].m_DescriptorSet, nullptr);
			vCmdBuffer->pushConstants(m_Pipelines[vIdx].m_PipelineLayout,
				vk::ShaderStageFlagBits::eCompute,
				0, sizeof(PushConstants), &m_PushConstants);
			Dispatch(vCmdBuffer);
		}
		else
		{
			for (uint32_t idx = 0; idx < m_MaxIterations; ++idx)
			{
				m_PushConstants.iteration_count = idx;

				vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[vIdx].m_Pipeline);
				vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[vIdx].m_PipelineLayout, 0, m_DescriptorSets[vIdx].m_DescriptorSet, nullptr);
				vCmdBuffer->pushConstants(m_Pipelines[vIdx].m_PipelineLayout,
					vk::ShaderStageFlagBits::eCompute,
					0, sizeof(PushConstants), &m_PushConstants);
				Dispatch(vCmdBuffer);
			}
		}
	}
}

void SdfTextureModule_Comp_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	ZoneScoped;

	if (vCmdBuffer)
	{
		// Init
		Compute_SdfTexture(vCmdBuffer, 0U);
		vCmdBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eComputeShader,
			vk::PipelineStageFlagBits::eComputeShader,
			vk::DependencyFlags(),
			vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead),
			nullptr, nullptr);

		// DistancePassH
		m_MaxIterations = (uint32_t)m_ImageInfosSize[0].x;
		Compute_SdfTexture(vCmdBuffer, 1U);
		vCmdBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eComputeShader,
			vk::PipelineStageFlagBits::eComputeShader,
			vk::DependencyFlags(),
			vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead),
			nullptr, nullptr);
		
		// DistancePassV
		m_MaxIterations = (uint32_t)m_ImageInfosSize[0].y;
		Compute_SdfTexture(vCmdBuffer, 2U);
		vCmdBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eComputeShader,
			vk::PipelineStageFlagBits::eComputeShader,
			vk::DependencyFlags(),
			vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead),
			nullptr, nullptr);
		
		// DisplaySDF
		Compute_SdfTexture(vCmdBuffer, 3U);
		vCmdBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eComputeShader,
			vk::PipelineStageFlagBits::eComputeShader,
			vk::DependencyFlags(),
			vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead),
			nullptr, nullptr);
	}
}

bool SdfTextureModule_Comp_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	if (m_ComputeBufferPtr)
	{
		bool res = true;

		// Init
		res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1U, 0U);
		res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute, 1U, 0U);

		// DistancePassH
		res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1U, 1U);
		res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute, 1U, 1U);

		// DistancePassV
		res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1U, 2U);
		res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute, 1U, 2U);

		// DisplaySDF
		res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1U, 3U);
		res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute, 1U, 3U);

		return res;
	}

	return false;
}

bool SdfTextureModule_Comp_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	if (m_ComputeBufferPtr)
	{
		//ClearWriteDescriptors();

		bool res = true;
		
		// Init
		res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(1U), 1U, 0U); // output
		res &= AddOrSetWriteDescriptorImage(1U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], 1U, 0U); // image to sdfize
		
		// DistancePassH
		res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U), 1U, 1U); // output
		res &= AddOrSetWriteDescriptorImage(1U, vk::DescriptorType::eCombinedImageSampler, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(1U), 1U, 1U); // image to sdfize H
		
		// DistancePassV
		res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(1U), 1U, 2U); // output
		res &= AddOrSetWriteDescriptorImage(1U, vk::DescriptorType::eCombinedImageSampler, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U), 1U, 2U); // image to sdfize V
		
		// DisplaySDF
		res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U), 1U, 3U); // output
		res &= AddOrSetWriteDescriptorImage(1U, vk::DescriptorType::eCombinedImageSampler, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(1U), 1U, 3U); // image to sdfize V
		
		return res;
	}

	return false;
}

std::string SdfTextureModule_Comp_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	ZoneScoped;

	vOutShaderName = "SdfTextureModule_Compute_Pass";

	SetLocalGroupSize(ct::uvec3(8U, 8U, 1U));

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1 ) in;
layout(binding = 0, rgba32f) uniform writeonly image2D outColor;
layout(binding = 1) uniform sampler2D input_map_sampler;
layout(push_constant) uniform constants { int iteration_count; };

//////////////////////////////////////////////////////////////////////
//// algo from here : https://prideout.net/blog/distance_fields/ /////
//////////////////////////////////////////////////////////////////////

#define EPSILON 1e-7

void DistancePass(in ivec2 offset)
{
	ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	float beta = float(iteration_count) * 2.0 + 1.0;

	vec3 this_pixel = texelFetch(input_map_sampler, coords, 0).rgb;
    vec3 east_pixel = texelFetch(input_map_sampler, coords + offset, 0).rgb;
    vec3 west_pixel = texelFetch(input_map_sampler, coords - offset, 0).rgb;

    float A = this_pixel.b;
    float e = east_pixel.b + beta;
    float w = west_pixel.b + beta;
    float B = min(min(A, e), w);

    // If there is no change, discard the pixel.
    // Convergence can be detected using GL_ANY_SAMPLES_PASSED.
    if (abs(A - B) > EPSILON) 
	{
		vec4 res = vec4(west_pixel.rg, B, 1);

		// Closest point coordinate is stored in the RED-GREEN channels.
		if (A <= e && e <= w) res.rg = this_pixel.rg;
		if (A <= w && w <= e) res.rg = this_pixel.rg;
		if (e <= A && A <= w) res.rg = east_pixel.rg;
		if (e <= w && w <= A) res.rg = east_pixel.rg;

		imageStore(outColor, coords, res);
    }
}

void Init() 
{
	ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	float mask = texelFetch(input_map_sampler, coords, 0).r;
	if (mask < 0.5)
	{
		imageStore(outColor, coords, vec4(0,0,0,1));
	}
	else
	{
		imageStore(outColor, coords, vec4(1,1,1e8,1));
	}
}

void DistancePassH()
{
	DistancePass(ivec2(1,0));
}

void DistancePassV()
{
	DistancePass(ivec2(0,1));
}

void DisplaySDF()
{
	ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	vec4 res = texelFetch(input_map_sampler, coords, 0);
	res.rgb = normalize(res.rgb) * 0.5 + 0.5;
	res.a = 1.0;
	imageStore(outColor, coords, res);
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / PIPELINE ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdfTextureModule_Comp_Pass::CreateComputePipeline()
{
	ZoneScoped;

	if (m_ShaderCodes[vk::ShaderStageFlagBits::eCompute].empty()) return false;
	if (m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["Init"][0].m_SPIRV.empty()) return false;
	if (m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["DistancePassH"][0].m_SPIRV.empty()) return false;
	if (m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["DistancePassV"][0].m_SPIRV.empty()) return false;
	if (m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["DisplaySDF"][0].m_SPIRV.empty()) return false;

	std::vector<vk::PushConstantRange> push_constants;
	if (m_Internal_PushConstants.size)
	{
		push_constants.push_back(m_Internal_PushConstants);
	}

	m_Pipelines[0].m_PipelineLayout =
		m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			1, &m_DescriptorSets[0].m_DescriptorSetLayout,
			(uint32_t)push_constants.size(), push_constants.data()
		)); 

	m_Pipelines[1].m_PipelineLayout =
		m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			1, &m_DescriptorSets[1].m_DescriptorSetLayout,
			(uint32_t)push_constants.size(), push_constants.data()
		));

	m_Pipelines[2].m_PipelineLayout =
		m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			1, &m_DescriptorSets[2].m_DescriptorSetLayout,
			(uint32_t)push_constants.size(), push_constants.data()
		));

	m_Pipelines[3].m_PipelineLayout =
		m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			1, &m_DescriptorSets[3].m_DescriptorSetLayout,
			(uint32_t)push_constants.size(), push_constants.data()
		));

	auto cs_I = vkApi::VulkanCore::sVulkanShader->CreateShaderModule(
		(VkDevice)m_Device, m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["Init"][0].m_SPIRV);

	auto cs_H = vkApi::VulkanCore::sVulkanShader->CreateShaderModule(
		(VkDevice)m_Device, m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["DistancePassH"][0].m_SPIRV);
	
	auto cs_V = vkApi::VulkanCore::sVulkanShader->CreateShaderModule(
		(VkDevice)m_Device, m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["DistancePassV"][0].m_SPIRV);

	auto cs_S = vkApi::VulkanCore::sVulkanShader->CreateShaderModule(
		(VkDevice)m_Device, m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["DisplaySDF"][0].m_SPIRV);


	m_ShaderCreateInfos = {
	   vk::PipelineShaderStageCreateInfo(
		   vk::PipelineShaderStageCreateFlags(),
		   vk::ShaderStageFlagBits::eCompute,
		   cs_I, "Init"
	   ),
	   vk::PipelineShaderStageCreateInfo(
		   vk::PipelineShaderStageCreateFlags(),
		   vk::ShaderStageFlagBits::eCompute,
		   cs_H, "DistancePassH"
	   ),
	   vk::PipelineShaderStageCreateInfo(
		   vk::PipelineShaderStageCreateFlags(),
		   vk::ShaderStageFlagBits::eCompute,
		   cs_V, "DistancePassV"
	   ),
	   vk::PipelineShaderStageCreateInfo(
		   vk::PipelineShaderStageCreateFlags(),
		   vk::ShaderStageFlagBits::eCompute,
		   cs_S, "DisplaySDF"
	   )
	};

	vk::ComputePipelineCreateInfo computePipeInfo_I = vk::ComputePipelineCreateInfo()
		.setStage(m_ShaderCreateInfos[0]).setLayout(m_Pipelines[0].m_PipelineLayout);
	m_Pipelines[0].m_Pipeline = m_Device.createComputePipeline(nullptr, computePipeInfo_I).value;

	vk::ComputePipelineCreateInfo computePipeInfo_H = vk::ComputePipelineCreateInfo()
		.setStage(m_ShaderCreateInfos[1]).setLayout(m_Pipelines[1].m_PipelineLayout);
	m_Pipelines[1].m_Pipeline = m_Device.createComputePipeline(nullptr, computePipeInfo_H).value;
	
	vk::ComputePipelineCreateInfo computePipeInfo_V = vk::ComputePipelineCreateInfo()
		.setStage(m_ShaderCreateInfos[2]).setLayout(m_Pipelines[2].m_PipelineLayout);
	m_Pipelines[2].m_Pipeline = m_Device.createComputePipeline(nullptr, computePipeInfo_V).value;

	vk::ComputePipelineCreateInfo computePipeInfo_S = vk::ComputePipelineCreateInfo()
		.setStage(m_ShaderCreateInfos[3]).setLayout(m_Pipelines[3].m_PipelineLayout);
	m_Pipelines[3].m_Pipeline = m_Device.createComputePipeline(nullptr, computePipeInfo_S).value;

	vkApi::VulkanCore::sVulkanShader->DestroyShaderModule((VkDevice)m_Device, cs_I);
	vkApi::VulkanCore::sVulkanShader->DestroyShaderModule((VkDevice)m_Device, cs_H);
	vkApi::VulkanCore::sVulkanShader->DestroyShaderModule((VkDevice)m_Device, cs_V);
	vkApi::VulkanCore::sVulkanShader->DestroyShaderModule((VkDevice)m_Device, cs_S);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string SdfTextureModule_Comp_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	ZoneScoped;

	std::string str;
	
	return str;
}

bool SdfTextureModule_Comp_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	return true;
}

void SdfTextureModule_Comp_Pass::AfterNodeXmlLoading()
{
	ZoneScoped;
}