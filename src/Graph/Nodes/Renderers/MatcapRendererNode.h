/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

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

#include <LumoBackend/Graph/Graph.h>

#include <ctools/cTools.h>

#include <LumoBackend/Graph/Base/BaseNode.h>

#include <LumoBackend/Interfaces/ModelInputInterface.h>
#include <LumoBackend/Interfaces/TextureInputInterface.h>
#include <LumoBackend/Interfaces/TextureOutputInterface.h>
#include <LumoBackend/Interfaces/ShaderPassOutputInterface.h>

class MatcapRenderer;
class MatcapRendererNode : public BaseNode,
                           public ModelInputInterface,
                           public TextureInputInterface<0U>,
                           public TextureOutputInterface,
                           public ShaderPassOutputInterface {
public:
    static std::shared_ptr<MatcapRendererNode> Create(GaiApi::VulkanCoreWeak vVulkanCore);

private:
    std::shared_ptr<MatcapRenderer> m_MatcapRenderer = nullptr;

public:
    MatcapRendererNode();
    ~MatcapRendererNode() override;
    bool Init(GaiApi::VulkanCoreWeak vVulkanCore) override;
    void Unit() override;
    bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    void SetModel(SceneModelWeak vSceneModel = SceneModelWeak()) override;
    void DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) override;
    void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
    void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;
    vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;
    SceneShaderPassWeak GetShaderPasses(const uint32_t& vSlotID) override;
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
};