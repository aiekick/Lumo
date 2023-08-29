#pragma once
#pragma warning(disable : 4251)

#include <Gaia/gaia.h>

#include <ctools/cTools.h>

#include <LumoBackend/Headers/LumoBackendDefs.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>
#include <Gaia/Resources/VulkanComputeImageTarget.h>
#include <Gaia/Buffer/ComputeBuffer.h>
#include <Gaia/Resources/VulkanRessource.h>

#include <string>
#include <memory>

class LUMO_BACKEND_API ShaderInterface {
public:
    ct::uvec3 m_OutputSize;       // size of the output
    uint32_t m_CurrentFrame = 0;  // current frame of double buffering

public:  // virtual pure
    virtual vk::DescriptorImageInfo GetBackImageInfo(uint32_t vIndex) = 0;
    virtual vk::DescriptorImageInfo GetFrontImageInfo(uint32_t vIndex) = 0;
    virtual GaiApi::VulkanFrameBuffer* GetBackFbo() = 0;
    virtual GaiApi::VulkanFrameBuffer* GetFrontFbo() = 0;
    virtual GaiApi::VulkanComputeImageTarget* GetBackTarget(uint32_t vIndex) = 0;
    virtual GaiApi::VulkanComputeImageTarget* GetFrontTarget(uint32_t vIndex) = 0;
};