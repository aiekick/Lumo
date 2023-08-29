#pragma once

#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Gaia/Core/VulkanCore.h>
#include <LumoBackend/Base/QuadShaderPass.h>
#include <Gaia/gaia.h>

class DisplaySizeQuadPass : public QuadShaderPass {
private:
    vk::DescriptorImageInfo m_ImageInfos;

public:
    DisplaySizeQuadPass(GaiApi::VulkanCorePtr vVulkanCorePtr);
    virtual ~DisplaySizeQuadPass();

    void SetImageInfos(const vk::DescriptorImageInfo* vImageInfosPtr);

protected:
    void ActionBeforeInit() override;

    bool UpdateLayoutBindingInRessourceDescriptor() override;
    bool UpdateBufferInfoInRessourceDescriptor() override;

    void SetInputStateBeforePipelineCreation() override;

    std::string GetVertexShaderCode(std::string& vOutShaderName) override;
    std::string GetFragmentShaderCode(std::string& vOutShaderName) override;
};