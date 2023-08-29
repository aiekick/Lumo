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

#include "BlurModule_Comp_Pass.h"

#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <cinttypes>
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
//// SSAO SECOND PASS : BLUR /////////////////////////////////
//////////////////////////////////////////////////////////////

BlurModule_Comp_Pass::BlurModule_Comp_Pass(GaiApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : Blur", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

BlurModule_Comp_Pass::~BlurModule_Comp_Pass()
{
	Unit();
}

void BlurModule_Comp_Pass::ActionBeforeInit()
{
	ZoneScoped;

	m_Pipelines.resize(2U);
	m_DescriptorSets.resize(2U);

	ReComputeGaussianBlurWeights(m_Gaussian_H, m_UBOComp.u_blur_radius_H);
	ReComputeGaussianBlurWeights(m_Gaussian_V, m_UBOComp.u_blur_radius_V);
}

void BlurModule_Comp_Pass::ActionBeforeCompilation()
{
	ZoneScoped;

	ClearShaderEntryPoints();
	AddShaderEntryPoints(vk::ShaderStageFlagBits::eCompute, "blur_H");
	AddShaderEntryPoints(vk::ShaderStageFlagBits::eCompute, "blur_V");
}

bool BlurModule_Comp_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext, const std::string& vUserDatas)
{
	ZoneScoped;

	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (ImGui::CollapsingHeader("Blur", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool change_radius = false;
		bool change_gaussian = false;
		bool change_operations = false;

		if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::CheckBoxBoolDefault("Use Distinct Radius for H and V", &m_UseDistinctiveBlurRadiusVH, false))
			{
				if (m_UseDistinctiveBlurRadiusVH == false)
				{
					m_BlurRadius = (uint32_t)(((float)m_UBOComp.u_blur_radius_V + (float)m_UBOComp.u_blur_radius_H) * 0.5f);
					m_UBOComp.u_blur_radius_H = m_BlurRadius;
					m_UBOComp.u_blur_radius_V = m_BlurRadius;
				}

				change_radius = true;
			}

			if (m_UseDistinctiveBlurRadiusVH)
			{
				if (m_UseBlurH)
				{
					change_radius |= ImGui::SliderUIntDefaultCompact(0.0f, "Radius H", &m_UBOComp.u_blur_radius_H, 1U, 32U, 4U);
				}

				if (m_UseBlurV)
				{
					change_radius |= ImGui::SliderUIntDefaultCompact(0.0f, "Radius V", &m_UBOComp.u_blur_radius_V, 1U, 32U, 4U);
				}
			}
			else
			{
				if (ImGui::SliderUIntDefaultCompact(0.0f, "Radius", &m_BlurRadius, 1U, 32U, 4U))
				{
					m_UBOComp.u_blur_radius_H = m_BlurRadius;
					m_UBOComp.u_blur_radius_V = m_BlurRadius;
					change_radius = true;
				}
			}

			change_gaussian |= ImGui::CheckBoxBoolDefault("Gaussian Sigma Auto", &m_GaussianSigmAuto, true);
			if (!m_GaussianSigmAuto)
			{
				change_gaussian |= ImGui::SliderFloatDefaultCompact(0.0f, "Gaussian Sigma", &m_GaussianSigma, 0.01f, 2.0f, 1.0f);
			}

			change_operations |= ImGui::CheckBoxBoolDefault("Use Horizontal Blur", &m_UseBlurH, true);
			change_operations |= ImGui::CheckBoxBoolDefault("Use Vertical Blur", &m_UseBlurV, true);
		}

		DrawInputTexture(m_VulkanCorePtr, "Input", 0U, m_OutputRatio);

		if (change_radius)
		{
			m_UBOComp.u_blur_radius_H = ct::maxi(m_UBOComp.u_blur_radius_H, 1U);
			m_UBOComp.u_blur_radius_V = ct::maxi(m_UBOComp.u_blur_radius_V, 1U);
			NeedNewUBOUpload();
		}

		if (change_gaussian || change_radius)
		{
			m_GaussianSigma = ct::maxi(m_GaussianSigma, 0.01f);
			ReComputeGaussianBlurWeights(m_Gaussian_H, m_UBOComp.u_blur_radius_H);
			ReComputeGaussianBlurWeights(m_Gaussian_V, m_UBOComp.u_blur_radius_V);
		}

		if (change_operations)
		{
			UpdateBufferInfoInRessourceDescriptor();
		}

		return change_radius || change_gaussian || change_operations;
	}

	return false;
}

bool BlurModule_Comp_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContext, const std::string& vUserDatas) {
	ZoneScoped;

	assert(vContext);
    ImGui::SetCurrentContext(vContext);
    return false;

}

bool BlurModule_Comp_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContext, const std::string& vUserDatas) {
	ZoneScoped;

	assert(vContext);
    ImGui::SetCurrentContext(vContext);
    return false;
}

void BlurModule_Comp_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
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

					NeedResizeByHandIfChanged(m_ImageInfosSize[0]);
				}

				m_ImageInfos[vBinding] = *vImageInfo;
			}
			else
			{
				m_ImageInfos[vBinding] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* BlurModule_Comp_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	ZoneScoped;

	if (m_ComputeBufferPtr)
	{
		AutoResizeBuffer(std::dynamic_pointer_cast<OutputSizeInterface>(m_ComputeBufferPtr).get(), vOutSize);

		return m_ComputeBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

bool BlurModule_Comp_Pass::CanUpdateDescriptors()
{
	ZoneScoped;

	return (m_ComputeBufferPtr != nullptr);
}

void BlurModule_Comp_Pass::Compute_Blur_H(vk::CommandBuffer* vCmdBuffer)
{
	ZoneScoped;

	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
		Dispatch(vCmdBuffer);
	}
}

void BlurModule_Comp_Pass::Compute_Blur_V(vk::CommandBuffer* vCmdBuffer)
{
	ZoneScoped;

	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[1].m_Pipeline);
		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[1].m_PipelineLayout, 0, m_DescriptorSets[1].m_DescriptorSet, nullptr);
		Dispatch(vCmdBuffer);
	}
}

void BlurModule_Comp_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	ZoneScoped;

	if (vCmdBuffer)
	{
#ifdef _MSC_VER
#undef MemoryBarrier
#endif
		if (m_UseBlurH)
		{
			Compute_Blur_H(vCmdBuffer);

			vCmdBuffer->pipelineBarrier(
				vk::PipelineStageFlagBits::eComputeShader,
				vk::PipelineStageFlagBits::eComputeShader,
				vk::DependencyFlags(),
				vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead),
				nullptr,
				nullptr);
		}

		if (m_UseBlurV)
		{
			Compute_Blur_V(vCmdBuffer);

			vCmdBuffer->pipelineBarrier(
				vk::PipelineStageFlagBits::eComputeShader,
				vk::PipelineStageFlagBits::eComputeShader,
				vk::DependencyFlags(),
				vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead),
				nullptr,
				nullptr);
		}
	}
}

bool BlurModule_Comp_Pass::CreateUBO()
{
	ZoneScoped;

	auto size_in_bytes = sizeof(UBOComp);
	m_UBOCompPtr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, size_in_bytes);
	m_DescriptorBufferInfo_Comp.buffer = m_UBOCompPtr->buffer;
	m_DescriptorBufferInfo_Comp.range = size_in_bytes;
	m_DescriptorBufferInfo_Comp.offset = 0;

	for (auto& info : m_ImageInfos)
	{
		info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
	}

	NeedNewUBOUpload();

	return true;
}

void BlurModule_Comp_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBOCompPtr, &m_UBOComp, sizeof(UBOComp));
}

void BlurModule_Comp_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBOCompPtr.reset();
}

void BlurModule_Comp_Pass::ReComputeGaussianBlurWeights(GaussianKernel& vOutGaussian, const uint32_t& vRadius)
{
	// gauss curve : exp(-x�)
	// integration of curve give sqrt(pi)
	// exp(-x�/(2*sigma�)
	// here we will compute weights value for X Samples from 0 to  
	vOutGaussian.m_GaussianWeights.clear();
	vOutGaussian.m_GaussianWeights.resize((size_t)vRadius);

	float sig_inv, x_step, sum = 0.0f;
	if (m_GaussianSigmAuto) // m_GaussianSigma and conpensated step for whole x range
	{
		sig_inv = -0.5f;
		x_step = 4.0f / vRadius;
	}
	else
	{
		sig_inv = -1.0f / (2.0f * m_GaussianSigma * m_GaussianSigma);
		x_step = 4.0f / vRadius;
		//float x_step_auto = 4.0f * m_GaussianSigma  / m_UBOComp.u_blur_radius;
	}
	
	for (uint32_t idx = 0U; idx < vRadius; ++idx)
	{
		float x = (float)idx * x_step;
		sum += vOutGaussian.m_GaussianWeights[idx] = std::exp(x * x * sig_inv);
	}

	if (sum > 0.0f)
	{
		for (auto& weight : vOutGaussian.m_GaussianWeights)
		{
			weight /= sum;
		}
	}

	if (m_Loaded)
	{
		CreateSBO(vOutGaussian);
	}
}

bool BlurModule_Comp_Pass::CreateSBO(GaussianKernel& vOutGaussian)
{
	ZoneScoped;

	vOutGaussian.m_SBO_GaussianWeights.reset();
	const auto sizeInBytes = sizeof(float) * vOutGaussian.m_GaussianWeights.size();
	vOutGaussian.m_SBO_GaussianWeights = VulkanRessource::createStorageBufferObject(m_VulkanCorePtr, sizeInBytes, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU);
	if (vOutGaussian.m_SBO_GaussianWeights && vOutGaussian.m_SBO_GaussianWeights->buffer)
	{
		vOutGaussian.m_SBO_GaussianWeightsBufferInfo = vk::DescriptorBufferInfo{vOutGaussian.m_SBO_GaussianWeights->buffer, 0, sizeInBytes};
	}
	else
	{
		vOutGaussian.m_SBO_GaussianWeightsBufferInfo = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
	}

	NeedNewSBOUpload();

	return true;
}

void BlurModule_Comp_Pass::UploadSBO(GaussianKernel& vOutGaussian)
{
	ZoneScoped;

	if (vOutGaussian.m_SBO_GaussianWeights)
	{
		const auto sizeInBytes = sizeof(float) * vOutGaussian.m_GaussianWeights.size();
		VulkanRessource::upload(m_VulkanCorePtr, vOutGaussian.m_SBO_GaussianWeights, vOutGaussian.m_GaussianWeights.data(), sizeInBytes);
	}
}

void BlurModule_Comp_Pass::DestroySBO(GaussianKernel& vOutGaussian)
{
	ZoneScoped;

	vOutGaussian.m_SBO_GaussianWeights.reset();
}

bool BlurModule_Comp_Pass::CreateSBO()
{
	ZoneScoped;

	CreateSBO(m_Gaussian_H);
	CreateSBO(m_Gaussian_V);

	NeedNewSBOUpload();

	return true;
}

void BlurModule_Comp_Pass::UploadSBO()
{
	ZoneScoped;

	UploadSBO(m_Gaussian_H);
	UploadSBO(m_Gaussian_V);
}

void BlurModule_Comp_Pass::DestroySBO()
{
	ZoneScoped;

	DestroySBO(m_Gaussian_H);
	DestroySBO(m_Gaussian_V);
}

bool BlurModule_Comp_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	if (m_ComputeBufferPtr)
	{
		bool res = true;

		// blur H
		res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1U, 0U);
		res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute, 1U, 0U);
		res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute, 1U, 0U);
		res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1U, 0U);

		// blur V
		res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1U, 1U);
		res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute, 1U, 1U);
		res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute, 1U, 1U);
		res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1U, 1U);

		return res;
	}

	return false;
}

bool BlurModule_Comp_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	if (m_ComputeBufferPtr)
	{
		//ClearWriteDescriptors();

		bool res = true;
		
		// blur H
		res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(1U), 1U, 0U); // output
		res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_DescriptorBufferInfo_Comp, 1U, 0U);
		res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], 1U, 0U); // image to blur
		res &= AddOrSetWriteDescriptorBuffer(3U, vk::DescriptorType::eStorageBuffer, &m_Gaussian_H.m_SBO_GaussianWeightsBufferInfo, 1U, 0U); // gaussian weights

		// blur V
		res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U), 1U, 1U); // output
		res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_DescriptorBufferInfo_Comp, 1U, 1U);
		res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(1U), 1U, 1U); // image to blur
		res &= AddOrSetWriteDescriptorBuffer(3U, vk::DescriptorType::eStorageBuffer, &m_Gaussian_V.m_SBO_GaussianWeightsBufferInfo, 1U, 1U); // gaussian weights

		if (m_UseBlurH && m_UseBlurV)
		{
			// blur H
			res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(1U), 1U, 0U); // output
			res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], 1U, 0U); // image to blur
			
			// blur V
			res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U), 1U, 1U); // output
			res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(1U), 1U, 1U); // image to blur
		}
		else if (m_UseBlurH)
		{
			// blur H
			res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U), 1U, 0U); // output
			res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], 1U, 0U); // image to blur
		}
		else if (m_UseBlurV)
		{
			// blur V
			res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U), 1U, 1U); // output
			res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], 1U, 1U); // image to blur
		}
		
		return res;
	}

	return false;
}

std::string BlurModule_Comp_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	ZoneScoped;

	vOutShaderName = "BlurModule_Compute_Pass";

	SetLocalGroupSize(ct::uvec3(8U, 8U, 1U));

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColor;

layout(std140, binding = 1) uniform UBO_Comp
{
	uint u_blur_radius_H; // default is 4
	uint u_blur_radius_V; // default is 4
};

layout(binding = 2) uniform sampler2D input_map_sampler;

layout(std430, binding = 3) readonly buffer SBO_Comp
{
	float gaussian_weights[];
};

void blur_H()
{
	vec4 res = vec4(0.0);
	
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	
	ivec2 iv_neg = coords;
	ivec2 iv_pos = coords;
	
	// for avoid double sum at gaussian_weights[0]
	res = texelFetch(input_map_sampler, coords, 0) * gaussian_weights[0];

	for(uint idx = 1; idx < u_blur_radius_H; ++idx)
	{
		// <---
		--iv_neg.x;
		res += texelFetch(input_map_sampler, iv_neg, 0) * gaussian_weights[idx];

		// --->
		++iv_pos.x;
		res += texelFetch(input_map_sampler, iv_pos, 0) * gaussian_weights[idx];
	}

	imageStore(outColor, coords, res); 
}

void blur_V()
{
	vec4 res = vec4(0.0);
	
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	
	ivec2 iv_neg = coords;
	ivec2 iv_pos = coords;

	// for avoid double sum at gaussian_weights[0]
	res = texelFetch(input_map_sampler, coords, 0) * gaussian_weights[0];

	for(uint idx = 1; idx < u_blur_radius_V; ++idx)
	{
		// <---
		--iv_neg.y;
		res += texelFetch(input_map_sampler, iv_neg, 0) * gaussian_weights[idx];

		// --->
		++iv_pos.y;
		res += texelFetch(input_map_sampler, iv_pos, 0) * gaussian_weights[idx];
	}

	imageStore(outColor, coords, res); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / PIPELINE ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BlurModule_Comp_Pass::CreateComputePipeline()
{
	ZoneScoped;

	if (m_ShaderCodes[vk::ShaderStageFlagBits::eCompute].empty()) return false;
	if (m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["blur_H"][0].m_SPIRV.empty()) return false;
	if (m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["blur_V"][0].m_SPIRV.empty()) return false;

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

	auto cs_H = GaiApi::VulkanCore::sVulkanShader->CreateShaderModule(
		(VkDevice)m_Device, m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["blur_H"][0].m_SPIRV);
	
	auto cs_V = GaiApi::VulkanCore::sVulkanShader->CreateShaderModule(
		(VkDevice)m_Device, m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["blur_V"][0].m_SPIRV);


	m_ShaderCreateInfos = {
	   vk::PipelineShaderStageCreateInfo(
		   vk::PipelineShaderStageCreateFlags(),
		   vk::ShaderStageFlagBits::eCompute,
		   cs_H, "blur_H"
	   ),
	   vk::PipelineShaderStageCreateInfo(
		   vk::PipelineShaderStageCreateFlags(),
		   vk::ShaderStageFlagBits::eCompute,
		   cs_V, "blur_V"
	   )
	};

	vk::ComputePipelineCreateInfo computePipeInfo_H = vk::ComputePipelineCreateInfo()
		.setStage(m_ShaderCreateInfos[0]).setLayout(m_Pipelines[0].m_PipelineLayout);
	m_Pipelines[0].m_Pipeline = m_Device.createComputePipeline(nullptr, computePipeInfo_H).value;
	
	vk::ComputePipelineCreateInfo computePipeInfo_V = vk::ComputePipelineCreateInfo()
		.setStage(m_ShaderCreateInfos[1]).setLayout(m_Pipelines[1].m_PipelineLayout);
	m_Pipelines[1].m_Pipeline = m_Device.createComputePipeline(nullptr, computePipeInfo_V).value;

	GaiApi::VulkanCore::sVulkanShader->DestroyShaderModule((VkDevice)m_Device, cs_H);
	GaiApi::VulkanCore::sVulkanShader->DestroyShaderModule((VkDevice)m_Device, cs_V);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string BlurModule_Comp_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	ZoneScoped;

	std::string str;

	str += vOffset + "<blur_radius_h>" + ct::toStr(m_UBOComp.u_blur_radius_H) + "</blur_radius_h>\n";
	str += vOffset + "<blur_radius_v>" + ct::toStr(m_UBOComp.u_blur_radius_V) + "</blur_radius_v>\n"; 
	str += vOffset + "<blur_disctinct_v_and_h>" + (m_UseDistinctiveBlurRadiusVH ? "true" : "false") + "</blur_disctinct_v_and_h>\n";
	str += vOffset + "<blur_gaussian_sigma>" + ct::toStr(m_GaussianSigma) + "</blur_gaussian_sigma>\n";
	str += vOffset + "<blur_gaussian_sigma_auto>" + (m_GaussianSigmAuto ? "true" : "false") + "</blur_gaussian_sigma_auto>\n";
	str += vOffset + "<blur_use_h>" + (m_UseBlurH ? "true" : "false") + "</blur_use_h>\n";
	str += vOffset + "<blur_use_v>" + (m_UseBlurV ? "true" : "false") + "</blur_use_v>\n";
	
	return str;
}

bool BlurModule_Comp_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strParentName == "blur_module")
	{
		if (strName == "blur_radius_h")
		{
			m_UBOComp.u_blur_radius_H = ct::uvariant(strValue).GetU();
		}
		else if (strName == "blur_radius_v")
		{
			m_UBOComp.u_blur_radius_V = ct::uvariant(strValue).GetU();
		}
		else if (strName == "blur_disctinct_v_and_h")
		{
			m_UseDistinctiveBlurRadiusVH = ct::ivariant(strValue).GetB();
		}
		else if (strName == "blur_use_h")
		{
			m_UseBlurH = ct::ivariant(strValue).GetB();
		}
		else if (strName == "blur_use_v")
		{
			m_UseBlurV = ct::ivariant(strValue).GetB();
		}
		else if (strName == "blur_gaussian_sigma")
		{
			m_GaussianSigma = ct::fvariant(strValue).GetF();
		}
		else if (strName == "blur_gaussian_sigma_auto")
		{
			m_GaussianSigmAuto = ct::ivariant(strValue).GetB();
		}

		NeedNewUBOUpload();
	}

	return true;
}

void BlurModule_Comp_Pass::AfterNodeXmlLoading()
{
	ZoneScoped;

	ReComputeGaussianBlurWeights(m_Gaussian_H, m_UBOComp.u_blur_radius_H);
	ReComputeGaussianBlurWeights(m_Gaussian_V, m_UBOComp.u_blur_radius_V);
}