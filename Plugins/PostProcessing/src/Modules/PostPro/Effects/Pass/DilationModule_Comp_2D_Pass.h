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

#pragma once

#include <set>
#include <array>
#include <string>
#include <memory>

#include <LumoBackend/Headers/LumoBackendDefs.h>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Base/BaseRenderer.h>
#include <LumoBackend/Base/EffectPass.h>
#include <Gaia/gaia.h>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>

#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/TextureInputInterface.h>
#include <LumoBackend/Interfaces/TextureOutputInterface.h>

class DilationModule_Comp_2D_Pass : 
    public EffectPass<2U>,
    public NodeInterface {
public:
    static std::shared_ptr<DilationModule_Comp_2D_Pass> Create(const ct::uvec2& vSize, GaiApi::VulkanCorePtr vVulkanCorePtr);

private:
    struct UBO_Comp {
        alignas(4) int32_t u_shape = 0;
        alignas(4) int u_size = 5;
        alignas(4) float u_separation = 1.0f;
        alignas(8) ct::fvec2 u_threshold = ct::fvec2(0.0f, 1.0f);
        alignas(16) ct::fvec3 u_brightColor = ct::fvec3(1.0f, 1.0f, 1.0f);
        alignas(4) float u_enabled = 0.0f;
    } m_UBO_Comp;
    VulkanBufferObjectPtr m_UBO_Comp_Ptr = nullptr;
    vk::DescriptorBufferInfo m_UBO_Comp_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};

public:
    DilationModule_Comp_2D_Pass(GaiApi::VulkanCorePtr vVulkanCorePtr);
    ~DilationModule_Comp_2D_Pass() override;

    void ActionBeforeInit() override;
    void WasJustResized() override;

    void Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;

    // Interfaces Setters
    void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize = nullptr) override;

    // Interfaces Getters
    vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
    void AfterNodeXmlLoading() override;

protected:
    bool CreateUBO() override;
    void UploadUBO() override;
    void DestroyUBO() override;

    bool UpdateLayoutBindingInRessourceDescriptor() override;
    bool UpdateBufferInfoInRessourceDescriptor() override;

    std::string GetComputeShaderCode(std::string& vOutShaderName) override;
};