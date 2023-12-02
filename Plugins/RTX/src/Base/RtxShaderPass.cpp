// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include "RtxShaderPass.h"

#include <map>
#include <Gaia/gaia.h>
#include <glm/gtc/type_ptr.hpp>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Core/VulkanCommandBuffer.h>

using namespace GaiApi;

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

RtxShaderPass::RtxShaderPass(GaiApi::VulkanCoreWeak vVulkanCore) : ShaderPass(vVulkanCore, GenericType::RTX) {
    auto corePtr = vVulkanCore.lock();
    assert(corePtr != nullptr);
    auto devPtr = corePtr->getFrameworkDevice().lock();
    if (devPtr) {
        m_RayTracingPipelineProperties = devPtr->m_RayTracingDeviceProperties;
    }
}

RtxShaderPass::RtxShaderPass(GaiApi::VulkanCoreWeak vVulkanCore, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool)
    : ShaderPass(vVulkanCore, GenericType::RTX, vCommandPool, vDescriptorPool) {
    auto corePtr = vVulkanCore.lock();
    assert(corePtr != nullptr);
    auto devPtr = corePtr->getFrameworkDevice().lock();
    if (devPtr) {
        m_RayTracingPipelineProperties = devPtr->m_RayTracingDeviceProperties;
    }
}

uint32_t RtxShaderPass::GetAlignedSize(uint32_t value, uint32_t alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

void RtxShaderPass::TraceRays(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber) {
    if (m_Pipelines[0].m_Pipeline && m_Pipelines[0].m_PipelineLayout) {
        vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, m_Pipelines[0].m_Pipeline);
        vCmdBuffer->bindDescriptorSets(
            vk::PipelineBindPoint::eRayTracingKHR, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
        auto dispatch_size = GetDispatchSize();
        vCmdBuffer->traceRaysKHR(
            m_RayGenShaderSbtEntry, m_MissShaderSbtEntry, m_HitShaderSbtEntry, m_CallableShaderSbtEntry, dispatch_size.x, dispatch_size.y, 1);
    }
}

bool RtxShaderPass::BuildModel() {
    ZoneScoped;

    return true;
}

void RtxShaderPass::DestroyModel(const bool& vReleaseDatas) {
    ZoneScoped;
}

bool RtxShaderPass::CreateRtxPipeline() {
    ZoneScoped;

    // if (!m_IsShaderCompiled) return false;

    for (const auto& shaders : m_ShaderCodes) {
        for (auto& shaderEntryPoint : shaders.second) {
            for (auto& shader : shaderEntryPoint.second) {
                if (shader.m_Used && shader.m_SPIRV.empty())
                    return false;
            }
        }
    }

    std::vector<vk::PushConstantRange> push_constants;
    if (m_Internal_PushConstants.size) {
        push_constants.push_back(m_Internal_PushConstants);
    }

    m_Pipelines[0].m_PipelineLayout = m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
        vk::PipelineLayoutCreateFlags(), 1, &m_DescriptorSets[0].m_DescriptorSetLayout, (uint32_t)push_constants.size(), push_constants.data()));

    m_ShaderCreateInfos.clear();
    m_RayTracingShaderGroups.clear();
    for (auto& shaders : m_ShaderCodes) {
        for (auto& shaderEntryPoint : shaders.second) {
            for (auto& shader : shaderEntryPoint.second) {
                if (shader.m_Used) {
                    shader.m_ShaderModule = GaiApi::VulkanCore::sVulkanShader->CreateShaderModule((vk::Device)m_Device, shader.m_SPIRV);

                    if (shader.m_ShaderModule) {
                        auto shaderIndex = static_cast<uint32_t>(m_ShaderCreateInfos.size());

                        if (shader.m_ShaderId == vk::ShaderStageFlagBits::eRaygenKHR || shader.m_ShaderId == vk::ShaderStageFlagBits::eMissKHR) {
                            vk::RayTracingShaderGroupCreateInfoKHR shaderGroup;
                            shaderGroup.type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
                            shaderGroup.generalShader = shaderIndex;
                            shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
                            shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
                            shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
                            m_RayTracingShaderGroups.push_back(shaderGroup);
                        } else if (shader.m_ShaderId == vk::ShaderStageFlagBits::eClosestHitKHR ||
                                   shader.m_ShaderId == vk::ShaderStageFlagBits::eAnyHitKHR) {
                            vk::RayTracingShaderGroupCreateInfoKHR shaderGroup;
                            shaderGroup.type = vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;
                            if (shader.m_ShaderId == vk::ShaderStageFlagBits::eAnyHitKHR) {
                                shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
                                shaderGroup.anyHitShader = shaderIndex;
                                shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
                                shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
                            } else  // if (shader.m_ShaderId == vk::ShaderStageFlagBits::eClosestHitKHR)
                            {
                                shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
                                shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
                                shaderGroup.closestHitShader = shaderIndex;
                                shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
                            }
                            m_RayTracingShaderGroups.push_back(shaderGroup);
                        } else if (shader.m_ShaderId == vk::ShaderStageFlagBits::eIntersectionKHR) {
                            vk::RayTracingShaderGroupCreateInfoKHR shaderGroup;
                            shaderGroup.type = vk::RayTracingShaderGroupTypeKHR::eProceduralHitGroup;
                            shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
                            shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
                            shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
                            shaderGroup.intersectionShader = shaderIndex;
                            m_RayTracingShaderGroups.push_back(shaderGroup);
                        }

                        m_ShaderCreateInfos.emplace_back(vk::PipelineShaderStageCreateInfo(
                            vk::PipelineShaderStageCreateFlags(), shader.m_ShaderId, shader.m_ShaderModule, "main"));
                    }
                }
            }
        }
    }

    vk::RayTracingPipelineCreateInfoKHR rayTracingPipeInfo = vk::RayTracingPipelineCreateInfoKHR();
    rayTracingPipeInfo.stageCount = static_cast<uint32_t>(m_ShaderCreateInfos.size());
    rayTracingPipeInfo.pStages = m_ShaderCreateInfos.data();
    rayTracingPipeInfo.groupCount = static_cast<uint32_t>(m_RayTracingShaderGroups.size());
    rayTracingPipeInfo.pGroups = m_RayTracingShaderGroups.data();
    rayTracingPipeInfo.maxPipelineRayRecursionDepth = 2;
    rayTracingPipeInfo.layout = m_Pipelines[0].m_PipelineLayout;

    m_Pipelines[0].m_Pipeline = m_Device.createRayTracingPipelineKHR(nullptr, m_PipelineCache, rayTracingPipeInfo).value;

    // destroy modules
    for (const auto& shaders : m_ShaderCodes) {
        for (auto& shaderEntryPoint : shaders.second) {
            for (auto& shader : shaderEntryPoint.second) {
                if (shader.m_Used && shader.m_ShaderModule) {
                    GaiApi::VulkanCore::sVulkanShader->DestroyShaderModule((vk::Device)m_Device, shader.m_ShaderModule);
                }
            }
        }
    }

    if (m_Pipelines[0].m_Pipeline && m_Pipelines[0].m_PipelineLayout) {
        return CreateShaderBindingTable();
    }

    return false;
}

bool RtxShaderPass::CreateShaderBindingTable() {
    // Index position of the groups in the generated ray tracing pipeline
    // To be generic, this should be pass in parameters
    // todo, faire attention a correler ces ids avec ce qui est fait dans CreateRtxPipeline
    // sion le driver peut crasher le pc apres la commande de rendu traceRaysKHR

    std::vector<uint32_t> rgen_index;
    std::vector<uint32_t> miss_index;
    std::vector<uint32_t> hit_index;

    uint32_t idx = 0U;
    for (const auto& shaders : m_ShaderCodes) {
        for (auto& shaderEntryPoint : shaders.second) {
            for (auto& shader : shaderEntryPoint.second) {
                if (shader.m_ShaderId == vk::ShaderStageFlagBits::eRaygenKHR) {
                    rgen_index.push_back(idx++);
                } else if (shader.m_ShaderId == vk::ShaderStageFlagBits::eMissKHR) {
                    miss_index.push_back(idx++);
                } else if (shader.m_ShaderId == vk::ShaderStageFlagBits::eAnyHitKHR || shader.m_ShaderId == vk::ShaderStageFlagBits::eClosestHitKHR) {
                    hit_index.push_back(idx++);
                }
            }
        }
    }

    const uint32_t handle_size = m_RayTracingPipelineProperties.shaderGroupHandleSize;
    const uint32_t handle_alignment = m_RayTracingPipelineProperties.shaderGroupHandleAlignment;
    const uint32_t handle_size_aligned = GetAlignedSize(handle_size, handle_alignment);

    if (handle_size && handle_alignment && handle_size_aligned) {
        // Create binding table buffers for each shader type
        const auto bufferUsageFlags =
            vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        m_RayGenShaderBindingTablePtr = VulkanRessource::createStorageBufferObject(
            m_VulkanCore, handle_size_aligned * (uint32_t)rgen_index.size(), bufferUsageFlags, VMA_MEMORY_USAGE_CPU_TO_GPU, "RtxShaderPass");
        m_RayMissShaderBindingTablePtr = VulkanRessource::createStorageBufferObject(
            m_VulkanCore, handle_size_aligned * (uint32_t)miss_index.size(), bufferUsageFlags, VMA_MEMORY_USAGE_CPU_TO_GPU, "RtxShaderPass");
        m_RayHitShaderBindingTablePtr = VulkanRessource::createStorageBufferObject(
            m_VulkanCore, handle_size_aligned * (uint32_t)hit_index.size(), bufferUsageFlags, VMA_MEMORY_USAGE_CPU_TO_GPU, "RtxShaderPass");

        if (m_RayGenShaderBindingTablePtr && m_RayMissShaderBindingTablePtr && m_RayHitShaderBindingTablePtr) {
            // Copy the pipeline's shader handles into a host buffer
            const auto group_count = static_cast<uint32_t>(rgen_index.size() + miss_index.size() + hit_index.size());
            const auto sbt_size = group_count * handle_size_aligned;
            std::vector<uint8_t> shader_handle_storage(sbt_size);
            auto corePtr = m_VulkanCore.lock();
            assert(corePtr != nullptr);
            if (corePtr->getDevice().getRayTracingShaderGroupHandlesKHR(
                    m_Pipelines[0].m_Pipeline, 0, group_count, sbt_size, shader_handle_storage.data()) == vk::Result::eSuccess) {
                // Write the handles in the SBT buffer
                auto copyHandles = [&](VulkanBufferObjectPtr vBufferPtr, std::vector<uint32_t>& indices, uint32_t stride) {
                    if (vBufferPtr) {
                        void* mapped_dst = nullptr;
                        if (vBufferPtr->MapMemory(&mapped_dst)) {
                            auto* pBuffer = static_cast<uint8_t*>(mapped_dst);
                            for (uint32_t index = 0; index < static_cast<uint32_t>(indices.size()); index++) {
                                auto* pStart = pBuffer;
                                // Copy the handle
                                memcpy(pBuffer, shader_handle_storage.data() + (indices[(size_t)index] * handle_size), handle_size);
                                pBuffer = pStart + stride;  // Jumping to next group
                            }

                            vBufferPtr->UnmapMemory();
                        }
                    }
                };

                copyHandles(m_RayGenShaderBindingTablePtr, rgen_index, handle_size_aligned);
                copyHandles(m_RayMissShaderBindingTablePtr, miss_index, handle_size_aligned);
                copyHandles(m_RayHitShaderBindingTablePtr, hit_index, handle_size_aligned);

                m_RayGenShaderSbtEntry = vk::StridedDeviceAddressRegionKHR{};
                m_RayGenShaderSbtEntry.deviceAddress = m_RayGenShaderBindingTablePtr->device_address;
                m_RayGenShaderSbtEntry.stride = handle_size_aligned;
                m_RayGenShaderSbtEntry.size = handle_size_aligned * (uint32_t)rgen_index.size();  // * the count of shaders ion this category

                m_MissShaderSbtEntry = vk::StridedDeviceAddressRegionKHR{};
                m_MissShaderSbtEntry.deviceAddress = m_RayMissShaderBindingTablePtr->device_address;
                m_MissShaderSbtEntry.stride = handle_size_aligned;
                m_MissShaderSbtEntry.size = handle_size_aligned * (uint32_t)miss_index.size();  // * the count of shaders ion this category

                m_HitShaderSbtEntry = vk::StridedDeviceAddressRegionKHR{};
                m_HitShaderSbtEntry.deviceAddress = m_RayHitShaderBindingTablePtr->device_address;
                m_HitShaderSbtEntry.stride = handle_size_aligned;
                m_HitShaderSbtEntry.size = handle_size_aligned * (uint32_t)hit_index.size();  // * the count of shaders ion this category

                m_CallableShaderSbtEntry = vk::StridedDeviceAddressRegionKHR{};
                // m_CallableShaderSbtEntry.deviceAddress = ?? ->device_address;
                // m_CallableShaderSbtEntry.stride = handle_size_aligned;
                // m_CallableShaderSbtEntry.size = handle_size_aligned; // * the count of shaders ion this category

                return true;
            }
        }
    }

    return false;
}

void RtxShaderPass::DestroyShaderBindingTable() {
}