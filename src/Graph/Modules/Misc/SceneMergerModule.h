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

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Base/BaseRenderer.h>
#include <LumoBackend/Base/QuadShaderPass.h>

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
#include <LumoBackend/Interfaces/TaskInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/ResizerInterface.h>

#include <LumoBackend/Interfaces/Texture2DInputInterface.h>
#include <LumoBackend/Interfaces/ShaderPassInputInterface.h>
#include <LumoBackend/Interfaces/Texture2DOutputInterface.h>

class SceneMergerModule_Quad_Pass;
class SceneMergerModule : public NodeInterface,
                          public BaseRenderer,
                          public TaskInterface,
                          public Texture2DInputInterface<0U>,
                          public ShaderPassInputInterface,
                          public Texture2DOutputInterface {
public:
    static std::shared_ptr<SceneMergerModule> Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode);

private:
    std::weak_ptr<SceneMergerModule> m_This;
    FrameBufferPtr m_FrameBufferPtr = nullptr;
    SceneShaderPassContainer m_SceneShaderPassContainer;
    std::vector<uint32_t> m_JustDeletedSceneShaderPassSlots;

public:
    SceneMergerModule(GaiApi::VulkanCoreWeak vVulkanCore);
    virtual ~SceneMergerModule() override;

    bool Init();

    bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
    bool ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
    void RenderShaderPasses(vk::CommandBuffer* vCmdBufferPtr) override;

    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

    void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
    void NeedResizeByHand(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
    bool ResizeIfNeeded() override;

    // Interfaces Setters
    void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas = nullptr) override;
    void SetShaderPasses(const uint32_t& vSlotID, SceneShaderPassWeak vShaderPasses) override;

    // Interfaces Getters
    vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr, void* vUserDatas = nullptr) override;

    SceneShaderPassContainer& GetSceneShaderPassContainerRef();
    const std::vector<uint32_t>& GetJustDeletedSceneShaderPassSlots() const;

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
    void AfterNodeXmlLoading() override;
};
