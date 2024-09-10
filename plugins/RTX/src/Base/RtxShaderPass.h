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

#pragma once

#include <Gaia/gaia.h>
#include <LumoBackend/Base/ShaderPass.h>

/*
    https://www.gsn-lib.org/docs/nodes/raytracing.php

    -------------------------
    Built-In Variables
    -------------------------
                                      Ray generation 	Closest-hit 	Miss 	Intersection 	 Any-hit
    uvec3 gl_LaunchIDEXT 					x				x			x			x				x
    uvec3 gl_LaunchSizeEXT 					x				x			x			x				x
    int gl_PrimitiveID 										x						x				x
    int gl_InstanceID 										x						x				x
    int gl_InstanceCustomIndexEXT 							x						x				x
    int gl_GeometryIndexEXT 								x						x				x
    vec3 gl_WorldRayOriginEXT 								x			x			x				x
    vec3 gl_WorldRayDirectionEXT 							x			x			x				x
    vec3 gl_ObjectRayOriginEXT 								x						x				x
    vec3 gl_ObjectRayDirectionEXT 							x						x				x
    float gl_RayTminEXT 									x			x			x				x
    float gl_RayTmaxEXT 									x			x			x				x
    uint gl_IncomingRayFlagsEXT 							x			x			x				x
    float gl_HitTEXT 										x										x
    uint gl_HitKindEXT 										x										x
    mat4x3 gl_ObjectToWorldEXT 								x						x				x
    mat4x3 gl_WorldToObjectEXT 								x						x				x

    -------------------------
    Built-In Constants
    -------------------------

    const uint gl_RayFlagsNoneEXT = 0u;
    const uint gl_RayFlagsNoOpaqueEXT = 2u;
    const uint gl_RayFlagsTerminateOnFirstHitEXT = 4u;
    const uint gl_RayFlagsSkipClosestHitShaderEXT = 8u;
    const uint gl_RayFlagsCullBackFacingTrianglesEXT = 16u;
    const uint gl_RayFlagsCullFrontFacingTrianglesEXT = 32u;
    const uint gl_RayFlagsCullOpaqueEXT = 64u;
    const uint gl_RayFlagsCullNoOpaqueEXT = 128u;
    const uint gl_HitKindFrontFacingTriangleEXT = 0xFEu;
    const uint gl_HitKindBackFacingTriangleEXT = 0xFFu;
*/
class RtxShaderPass : public ShaderPass {
protected:
    vk::PhysicalDeviceRayTracingPipelinePropertiesKHR m_RayTracingPipelineProperties;
    VulkanBufferObjectPtr m_RayGenShaderBindingTablePtr = nullptr;
    VulkanBufferObjectPtr m_RayMissShaderBindingTablePtr = nullptr;
    VulkanBufferObjectPtr m_RayHitShaderBindingTablePtr = nullptr;

    vk::StridedDeviceAddressRegionKHR m_RayGenShaderSbtEntry = {};
    vk::StridedDeviceAddressRegionKHR m_MissShaderSbtEntry = {};
    vk::StridedDeviceAddressRegionKHR m_HitShaderSbtEntry = {};
    vk::StridedDeviceAddressRegionKHR m_CallableShaderSbtEntry = {};

public:
    RtxShaderPass(GaiApi::VulkanCoreWeak vVulkanCore);
    RtxShaderPass(GaiApi::VulkanCoreWeak vVulkanCore, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool);

protected:
    // https://developer.nvidia.com/blog/vulkan-raytracing/

    uint32_t GetAlignedSize(uint32_t value, uint32_t alignment);

    void TraceRays(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber) override;

    bool BuildModel() override;
    void DestroyModel(const bool& vReleaseDatas = false) override;

    bool CreateRtxPipeline() override;

    bool CreateShaderBindingTable();
    void DestroyShaderBindingTable();
};