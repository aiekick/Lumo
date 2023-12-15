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

#include "BloomModule_Comp_2D_Pass.h"

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
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<BloomModule_Comp_2D_Pass> BloomModule_Comp_2D_Pass::Create(const ct::uvec2& vSize, GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res_ptr = std::make_shared<BloomModule_Comp_2D_Pass>(vVulkanCore);
    if (!res_ptr->InitCompute2D(vSize, 2U, false, vk::Format::eR32G32B32A32Sfloat)) {
        res_ptr.reset();
    }
    return res_ptr;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

BloomModule_Comp_2D_Pass::BloomModule_Comp_2D_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : EffectPass(vVulkanCore) {
    SetRenderDocDebugName("Comp 2D Pass : Bloom", COMPUTE_SHADER_PASS_DEBUG_COLOR);
    m_DontUseShaderFilesOnDisk = true;
}

BloomModule_Comp_2D_Pass::~BloomModule_Comp_2D_Pass() {
    Unit();
}

void BloomModule_Comp_2D_Pass::ActionBeforeInit() {
    ReComputeGaussianBlurWeights();
}

void BloomModule_Comp_2D_Pass::ActionBeforeCompilation() {
    ClearShaderEntryPoints();
    AddShaderEntryPoints(vk::ShaderStageFlagBits::eCompute, "high_freq_thresholding");
    AddShaderEntryPoints(vk::ShaderStageFlagBits::eCompute, "horizontal_blur");
    AddShaderEntryPoints(vk::ShaderStageFlagBits::eCompute, "vertical_blur");
    AddShaderEntryPoints(vk::ShaderStageFlagBits::eCompute, "gamma_correction");
}

bool BloomModule_Comp_2D_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (ImGui::CollapsingHeader_CheckBox("Bloom##BloomModule_Comp_2D_Pass", -1.0f, false, true, IsEffectEnabled())) {
        bool change = false;

        static UBOComp s_DefaultUBOComp;
        change |= ImGui::SliderUIntDefaultCompact(0.0f, "Radius", &m_UBOComp.u_blur_radius, 1U, 200U, 4U);
        change |= ImGui::CheckBoxBoolDefault("Gaussian Sigma Auto", &m_GaussianSigmAuto, true);
        if (!m_GaussianSigmAuto) {
            change |= ImGui::SliderFloatDefaultCompact(0.0f, "Gaussian Sigma", &m_GaussianSigma, 0.01f, 2.0f, 1.0f);
        }

        if (ImGui::ColorEdit3Default(0.0f, "High Freq Threshold", &m_UBOComp.u_high_freq_threshold.x, &s_DefaultUBOComp.u_high_freq_threshold.x)) {
            NeedNewUBOUpload();
        }
        change |= ImGui::SliderFloatDefaultCompact(0.0f, "Exposure Correction", &m_UBOComp.u_exposure, 0.01f, 2.0f, 1.0f);
        change |= ImGui::SliderFloatDefaultCompact(0.0f, "Gamma Correction", &m_UBOComp.u_gamma_correction, 0.01f, 4.0f, 2.2f);
        if (change) {
            m_UBOComp.u_exposure = ct::maxi(m_UBOComp.u_exposure, 0.01f);
            m_UBOComp.u_gamma_correction = ct::maxi(m_UBOComp.u_gamma_correction, 0.01f);
            m_UBOComp.u_blur_radius = ct::maxi(m_UBOComp.u_blur_radius, 1U);
            NeedNewUBOUpload();

            m_GaussianSigma = ct::maxi(m_GaussianSigma, 0.01f);
            ReComputeGaussianBlurWeights();
        }

        // DrawInputTexture(m_VulkanCore, "Input", 0U, m_OutputRatio);
        // DrawInputTexture(m_VulkanCore, "Output Blur", 0U, m_OutputRatio);

        return change;
    }

    return false;
}

bool BloomModule_Comp_2D_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool BloomModule_Comp_2D_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void BloomModule_Comp_2D_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
    ZoneScoped;

    if (m_Loaded) {
        if (vBindingPoint < m_ImageInfos.size()) {
            if (vImageInfo) {
                if (vTextureSize) {
                    m_ImageInfosSize[vBindingPoint] = *vTextureSize;
                    NeedResizeByHandIfChanged(m_ImageInfosSize[vBindingPoint]);
                }
                m_ImageInfos[vBindingPoint] = *vImageInfo;
            } else {
                auto corePtr = m_VulkanCore.lock();
                assert(corePtr != nullptr);

                m_ImageInfos[vBindingPoint] = *corePtr->getEmptyTexture2DDescriptorImageInfo();
            }
        }
    }
}

vk::DescriptorImageInfo* BloomModule_Comp_2D_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    if (m_ComputeBufferPtr) {
        AutoResizeBuffer(std::dynamic_pointer_cast<OutputSizeInterface>(m_ComputeBufferPtr).get(), vOutSize);

        return m_ComputeBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
    }

    return nullptr;
}

void BloomModule_Comp_2D_Pass::SwapMultiPassFrontBackDescriptors() {
    m_DescriptorSets[0].m_WriteDescriptorSets[0].pImageInfo = m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U);  // output
}

bool BloomModule_Comp_2D_Pass::CanUpdateDescriptors() {
    return (m_ComputeBufferPtr != nullptr);
}

void BloomModule_Comp_2D_Pass::Compute_High_Freq_Thresholding(vk::CommandBuffer* vCmdBufferPtr) {
    if (vCmdBufferPtr) {
        vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
        vCmdBufferPtr->bindDescriptorSets(
            vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
        Dispatch(vCmdBufferPtr, "High_Freq_Thresholding");
    }
}

void BloomModule_Comp_2D_Pass::Compute_Horizontal_Blur(vk::CommandBuffer* vCmdBufferPtr) {
    if (vCmdBufferPtr) {
        vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[1].m_Pipeline);
        vCmdBufferPtr->bindDescriptorSets(
            vk::PipelineBindPoint::eCompute, m_Pipelines[1].m_PipelineLayout, 0, m_DescriptorSets[1].m_DescriptorSet, nullptr);
        Dispatch(vCmdBufferPtr, "Horizontal_Blur");
    }
}

void BloomModule_Comp_2D_Pass::Compute_Vertical_Blur(vk::CommandBuffer* vCmdBufferPtr) {
    if (vCmdBufferPtr) {
        vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[2].m_Pipeline);
        vCmdBufferPtr->bindDescriptorSets(
            vk::PipelineBindPoint::eCompute, m_Pipelines[2].m_PipelineLayout, 0, m_DescriptorSets[2].m_DescriptorSet, nullptr);
        Dispatch(vCmdBufferPtr, "Vertical_Blur");
    }
}

void BloomModule_Comp_2D_Pass::Compute_Gamma_Correction(vk::CommandBuffer* vCmdBufferPtr) {
    if (vCmdBufferPtr) {
        vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[3].m_Pipeline);
        vCmdBufferPtr->bindDescriptorSets(
            vk::PipelineBindPoint::eCompute, m_Pipelines[3].m_PipelineLayout, 0, m_DescriptorSets[3].m_DescriptorSet, nullptr);
        Dispatch(vCmdBufferPtr, "Gamma_Correction");
    }
}

void BloomModule_Comp_2D_Pass::Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) {
    if (vCmdBufferPtr) {
#ifdef _MSC_VER
#undef MemoryBarrier
#endif
        Compute_High_Freq_Thresholding(vCmdBufferPtr);

        vCmdBufferPtr->pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(),
            vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead), nullptr, nullptr);

        Compute_Horizontal_Blur(vCmdBufferPtr);

        vCmdBufferPtr->pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(),
            vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead), nullptr, nullptr);

        Compute_Vertical_Blur(vCmdBufferPtr);

        vCmdBufferPtr->pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(),
            vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead), nullptr, nullptr);

        Compute_Gamma_Correction(vCmdBufferPtr);

        vCmdBufferPtr->pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(),
            vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead), nullptr, nullptr);
    }
}

bool BloomModule_Comp_2D_Pass::CreateUBO() {
    ZoneScoped;

    auto size_in_bytes = sizeof(UBOComp);
    m_UBOCompPtr = VulkanRessource::createUniformBufferObject(m_VulkanCore, size_in_bytes, "BloomModule_Comp_2D_Pass");
    m_DescriptorBufferInfo_Comp.buffer = m_UBOCompPtr->buffer;
    m_DescriptorBufferInfo_Comp.range = size_in_bytes;
    m_DescriptorBufferInfo_Comp.offset = 0;

    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);

    for (auto& info : m_ImageInfos) {
        info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
    }

    NeedNewUBOUpload();

    return true;
}

void BloomModule_Comp_2D_Pass::UploadUBO() {
    ZoneScoped;
    assert(IsEffectEnabled() != nullptr);
    m_UBOComp.u_enabled = (*IsEffectEnabled()) ? 1.0f : 0.0f;
    VulkanRessource::upload(m_VulkanCore, m_UBOCompPtr, &m_UBOComp, sizeof(UBOComp));
}

void BloomModule_Comp_2D_Pass::DestroyUBO() {
    ZoneScoped;

    m_UBOCompPtr.reset();
}

void BloomModule_Comp_2D_Pass::ReComputeGaussianBlurWeights() {
    // gauss curve : exp(-x�)
    // integration of curve give sqrt(pi)
    // exp(-x�/(2*sigma�)
    // here we will compute weights value for X Samples from 0 to
    m_GaussianWeights.clear();
    m_GaussianWeights.resize((size_t)m_UBOComp.u_blur_radius);

    float sig_inv, x_step, sum = 0.0f;
    if (m_GaussianSigmAuto)  // m_GaussianSigma and conpensated step for whole x range
    {
        sig_inv = -0.5f;
        x_step = 4.0f / m_UBOComp.u_blur_radius;
    } else {
        sig_inv = -1.0f / (2.0f * m_GaussianSigma * m_GaussianSigma);
        x_step = 4.0f / m_UBOComp.u_blur_radius;
        // float x_step_auto = 4.0f * m_GaussianSigma  / m_UBOComp.u_blur_radius;
    }

    for (uint32_t idx = 0U; idx < m_UBOComp.u_blur_radius; ++idx) {
        float x = (float)idx * x_step;
        sum += m_GaussianWeights[idx] = std::exp(x * x * sig_inv);
    }

    if (sum > 0.0f) {
        for (auto& weight : m_GaussianWeights) {
            weight /= sum;
        }
    }

    if (m_Loaded) {
        CreateSBO();
    }
}

bool BloomModule_Comp_2D_Pass::CreateSBO() {
    ZoneScoped;

    m_SBO_GaussianWeights.reset();

    const auto sizeInBytes = sizeof(float) * m_GaussianWeights.size();
    m_SBO_GaussianWeights = VulkanRessource::createStorageBufferObject(
        m_VulkanCore, sizeInBytes, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU, "BloomModule_Comp_2D_Pass");
    if (m_SBO_GaussianWeights && m_SBO_GaussianWeights->buffer) {
        m_SBO_GaussianWeightsBufferInfo = vk::DescriptorBufferInfo{m_SBO_GaussianWeights->buffer, 0, sizeInBytes};
    } else {
        m_SBO_GaussianWeightsBufferInfo = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
    }

    NeedNewSBOUpload();

    return true;
}

void BloomModule_Comp_2D_Pass::UploadSBO() {
    if (m_SBO_GaussianWeights) {
        const auto sizeInBytes = sizeof(float) * m_GaussianWeights.size();
        VulkanRessource::upload(m_VulkanCore, m_SBO_GaussianWeights, m_GaussianWeights.data(), sizeInBytes);
    }
}

void BloomModule_Comp_2D_Pass::DestroySBO() {
    ZoneScoped;

    m_SBO_GaussianWeights.reset();
}

bool BloomModule_Comp_2D_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    if (m_ComputeBufferPtr) {
        bool res = true;
        for (uint32_t i = 0; i < (uint32_t)m_DescriptorSets.size(); ++i) {
            res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute, 1U, i);
            res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute, 1U, i);
            res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1U, i);
            res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute, 1U, i);
            res &= AddOrSetLayoutDescriptor(4U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1U, i);
        }
        return res;
    }

    return false;
}

bool BloomModule_Comp_2D_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    if (m_ComputeBufferPtr) {
        bool res = true;

        // clearing, and fixed descriptors
        for (uint32_t i = 0; i < (uint32_t)m_DescriptorSets.size(); ++i) {
            res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, &m_DescriptorBufferInfo_Comp, 1U, i);  // fixed 0U : UBO
            res &= AddOrSetWriteDescriptorImage(
                1U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], 1U, i);  // fixed 1U : input image of pass 0
            res &= AddOrSetWriteDescriptorBuffer(
                2U, vk::DescriptorType::eStorageBuffer, &m_SBO_GaussianWeightsBufferInfo, 1U, i);  // fixed 2U : gaussian weights
        }

        // high_freq_thresholding 0U
        // variable 3U : read from input image
        res &= AddOrSetWriteDescriptorImage(3U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], 1U, 0U);
        // variable 4U : write to front 1U
        res &= AddOrSetWriteDescriptorImage(4U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(1U), 1U, 0U);

        // horizontal_blur 1U
        // variable 3U : read from front 1U
        res &=
            AddOrSetWriteDescriptorImage(3U, vk::DescriptorType::eCombinedImageSampler, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(1U), 1U, 1U);
        // variable 4U : write to front 0U
        res &= AddOrSetWriteDescriptorImage(4U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U), 1U, 1U);

        // vertical_blur 2U
        // variable 3U : read from front 0U
        // image to bright // the input of pass 0
        res &=
            AddOrSetWriteDescriptorImage(3U, vk::DescriptorType::eCombinedImageSampler, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U), 1U, 2U);
        // variable 4U : write to front 1U
        // output
        res &= AddOrSetWriteDescriptorImage(4U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(1U), 1U, 2U);

        // gamma_correction 3U
        // variable 3U : read from front 1U
        // image to bright // the input of pass 0
        res &=
            AddOrSetWriteDescriptorImage(3U, vk::DescriptorType::eCombinedImageSampler, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(1U), 1U, 3U);
        // variable 4U : write to front 0U
        // output
        res &= AddOrSetWriteDescriptorImage(4U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U), 1U, 3U);

        return res;
    }

    return false;
}

std::string BloomModule_Comp_2D_Pass::GetComputeShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "BloomModule_Comp_2D_Pass";

    SetLocalGroupSize(ct::uvec3(8U, 8U, 1U));

    return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1 ) in;

// fixed
layout(std140, binding = 0) uniform UBO_Comp
{
	vec3 u_high_freq_threshold; // default vec3(0.2)
	uint u_blur_radius; // default is 4
	float u_exposure; // default is 1.0
	float u_gamma_correction; // default is 2.2
	uint u_enabled; // default is 1
};

// fixed
layout(binding = 1) uniform sampler2D input_0_map_sampler; // the input of pass 0

// fixed
layout(std430, binding = 2) readonly buffer SBO_Comp
{
	float gaussian_weights[];
};

// Variable read
layout(binding = 3) uniform sampler2D input_pass_map_sampler; // the input of each pass

// Variable write
layout(binding = 4, rgba32f) uniform writeonly image2D outColor; // the output of each pass

void high_freq_thresholding() {
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    vec4 res = texelFetch(input_pass_map_sampler, coords, 0);
    if (u_enabled > 0.5) {
		res *= step(1.0, dot(res.rgb, u_high_freq_threshold));
	}
	imageStore(outColor, coords, res); 
}

void horizontal_blur() {
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    vec4 res = texelFetch(input_pass_map_sampler, coords, 0);
    if (u_enabled > 0.5) {
		ivec2 iv_pos = coords;
		ivec2 iv_neg = coords;
		// for avoid double sum at gaussian_weights[0]
		res *= gaussian_weights[0];
		for(uint idx = 1; idx < u_blur_radius; ++idx) {
			// <---
			--iv_neg.x;
			res += texelFetch(input_pass_map_sampler, iv_neg, 0) * gaussian_weights[idx] * 0.5;
			// --->
			++iv_pos.x;
			res += texelFetch(input_pass_map_sampler, iv_pos, 0) * gaussian_weights[idx] * 0.5;
		}
	}
	imageStore(outColor, coords, res); 
}

void vertical_blur() {
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    vec4 res = texelFetch(input_pass_map_sampler, coords, 0);
    if (u_enabled > 0.5) {
		ivec2 iv_pos = coords;
		ivec2 iv_neg = coords;
		// for avoid double sum at gaussian_weights[0]
		res *= gaussian_weights[0];
		for(uint idx = 1; idx < u_blur_radius; ++idx) {
			// <---
			--iv_neg.y;
			res += texelFetch(input_pass_map_sampler, iv_neg, 0) * gaussian_weights[idx] * 0.5;
			// --->
			++iv_pos.y;
			res += texelFetch(input_pass_map_sampler, iv_pos, 0) * gaussian_weights[idx] * 0.5;
		}
	}
	imageStore(outColor, coords, res); 
}

void gamma_correction() {
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    vec4 res = texelFetch(input_pass_map_sampler, coords, 0);
    if (u_enabled > 0.5) {
		// input image from last blur V pass
		vec4 input_pass = res;
		// input image from pass 0
		res = texelFetch(input_0_map_sampler, coords, 0);
		// additive blending	
		res += input_pass;
		// tone mapping
		res = 1.0 - exp(-res * u_exposure);
		// gamma correction   
		res = pow(res, vec4(1.0 / u_gamma_correction));
	}
	imageStore(outColor, coords, res); 
}

)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / PIPELINE ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BloomModule_Comp_2D_Pass::CreateComputePipeline() {
    ZoneScoped;

    if (m_ShaderCodes[vk::ShaderStageFlagBits::eCompute].empty())
        return false;
    if (m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["high_freq_thresholding"][0].m_SPIRV.empty())
        return false;
    if (m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["horizontal_blur"][0].m_SPIRV.empty())
        return false;
    if (m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["vertical_blur"][0].m_SPIRV.empty())
        return false;
    if (m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["gamma_correction"][0].m_SPIRV.empty())
        return false;

    std::vector<vk::PushConstantRange> push_constants;
    if (m_Internal_PushConstants.size) {
        push_constants.push_back(m_Internal_PushConstants);
    }

    m_Pipelines[0].m_PipelineLayout = m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
        vk::PipelineLayoutCreateFlags(), 1, &m_DescriptorSets[0].m_DescriptorSetLayout, (uint32_t)push_constants.size(), push_constants.data()));

    m_Pipelines[1].m_PipelineLayout = m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
        vk::PipelineLayoutCreateFlags(), 1, &m_DescriptorSets[1].m_DescriptorSetLayout, (uint32_t)push_constants.size(), push_constants.data()));

    m_Pipelines[2].m_PipelineLayout = m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
        vk::PipelineLayoutCreateFlags(), 1, &m_DescriptorSets[2].m_DescriptorSetLayout, (uint32_t)push_constants.size(), push_constants.data()));

    m_Pipelines[3].m_PipelineLayout = m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
        vk::PipelineLayoutCreateFlags(), 1, &m_DescriptorSets[3].m_DescriptorSetLayout, (uint32_t)push_constants.size(), push_constants.data()));

    auto cs_HFT = GaiApi::VulkanCore::sVulkanShader->CreateShaderModule(
        (VkDevice)m_Device, m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["high_freq_thresholding"][0].m_SPIRV);

    auto cs_HBL = GaiApi::VulkanCore::sVulkanShader->CreateShaderModule(
        (VkDevice)m_Device, m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["horizontal_blur"][0].m_SPIRV);

    auto cs_VBL = GaiApi::VulkanCore::sVulkanShader->CreateShaderModule(
        (VkDevice)m_Device, m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["vertical_blur"][0].m_SPIRV);

    auto cs_GCO = GaiApi::VulkanCore::sVulkanShader->CreateShaderModule(
        (VkDevice)m_Device, m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["gamma_correction"][0].m_SPIRV);

    m_ShaderCreateInfos = {
        vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eCompute, cs_HFT, "high_freq_thresholding"),
        vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eCompute, cs_HBL, "horizontal_blur"),
        vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eCompute, cs_VBL, "vertical_blur"),
        vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eCompute, cs_GCO, "gamma_correction")};

    vk::ComputePipelineCreateInfo computePipeInfo_HFT =
        vk::ComputePipelineCreateInfo().setStage(m_ShaderCreateInfos[0]).setLayout(m_Pipelines[0].m_PipelineLayout);
    m_Pipelines[0].m_Pipeline = m_Device.createComputePipeline(nullptr, computePipeInfo_HFT).value;

    vk::ComputePipelineCreateInfo computePipeInfo_HBL =
        vk::ComputePipelineCreateInfo().setStage(m_ShaderCreateInfos[1]).setLayout(m_Pipelines[1].m_PipelineLayout);
    m_Pipelines[1].m_Pipeline = m_Device.createComputePipeline(nullptr, computePipeInfo_HBL).value;

    vk::ComputePipelineCreateInfo computePipeInfo_VBL =
        vk::ComputePipelineCreateInfo().setStage(m_ShaderCreateInfos[2]).setLayout(m_Pipelines[2].m_PipelineLayout);
    m_Pipelines[2].m_Pipeline = m_Device.createComputePipeline(nullptr, computePipeInfo_VBL).value;

    vk::ComputePipelineCreateInfo computePipeInfo_GCO =
        vk::ComputePipelineCreateInfo().setStage(m_ShaderCreateInfos[3]).setLayout(m_Pipelines[3].m_PipelineLayout);
    m_Pipelines[3].m_Pipeline = m_Device.createComputePipeline(nullptr, computePipeInfo_GCO).value;

    GaiApi::VulkanCore::sVulkanShader->DestroyShaderModule((VkDevice)m_Device, cs_HFT);
    GaiApi::VulkanCore::sVulkanShader->DestroyShaderModule((VkDevice)m_Device, cs_HBL);
    GaiApi::VulkanCore::sVulkanShader->DestroyShaderModule((VkDevice)m_Device, cs_VBL);
    GaiApi::VulkanCore::sVulkanShader->DestroyShaderModule((VkDevice)m_Device, cs_GCO);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string BloomModule_Comp_2D_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;

    str += vOffset + "<bloom_pass>\n";
    str += ShaderPass::getXml(vOffset + "\t", vUserDatas);
    str += vOffset + "\t<radius>" + ct::toStr(m_UBOComp.u_blur_radius) + "</radius>\n";
    str += vOffset + "\t<gaussian_sigma_auto>" + (m_GaussianSigmAuto ? "true" : "false") + "</gaussian_sigma_auto>\n";
    str += vOffset + "\t<gaussian_sigma>" + ct::toStr(m_GaussianSigma) + "</gaussian_sigma>\n";
    str += vOffset + "\t<gamma_correction>" + ct::toStr(m_UBOComp.u_gamma_correction) + "</gamma_correction>\n";
    str += vOffset + "\t<exposure_correction>" + ct::toStr(m_UBOComp.u_exposure) + "</exposure_correction>\n";
    str += vOffset + "\t<high_freq_threshold>" + m_UBOComp.u_high_freq_threshold.string() + "</high_freq_threshold>\n";
    str += vOffset + "\t<enabled>" + ct::toStr(m_UBOComp.u_enabled) + "</enabled>\n";
    str += vOffset + "</bloom_pass>\n";

    return str;
}

bool BloomModule_Comp_2D_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "bloom_pass") {
        ShaderPass::setFromXml(vElem, vParent, vUserDatas);
        if (strName == "radius") {
            m_UBOComp.u_blur_radius = ct::uvariant(strValue).GetU();
        } else if (strName == "gaussian_sigma") {
            m_GaussianSigma = ct::fvariant(strValue).GetF();
        } else if (strName == "gaussian_sigma_auto") {
            m_GaussianSigmAuto = ct::ivariant(strValue).GetB();
        } else if (strName == "gamma_correction") {
            m_UBOComp.u_gamma_correction = ct::fvariant(strValue).GetF();
        } else if (strName == "exposure_correction") {
            m_UBOComp.u_exposure = ct::fvariant(strValue).GetF();
        } else if (strName == "high_freq_threshold") {
            m_UBOComp.u_high_freq_threshold = ct::fvariant(strValue).GetV3();
        } else if (strName == "enabled") {
            m_UBOComp.u_enabled = ct::fvariant(strValue).GetF();
            *IsEffectEnabled() = m_UBOComp.u_enabled;
        }
    }

    return true;
}

void BloomModule_Comp_2D_Pass::AfterNodeXmlLoading() {
    ReComputeGaussianBlurWeights();
    NeedNewUBOUpload();
}