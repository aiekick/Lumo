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

#include <LumoBackend/Graph/Graph.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Interfaces/TextureInputInterface.h>
#include <LumoBackend/Interfaces/TextureGroupInputInterface.h>
#include <LumoBackend/Interfaces/TextureOutputInterface.h>

class BreakTexturesGroupModule;
class BreakTexturesGroupNode : public BaseNode, public TextureGroupInputInterface<0U>, public TextureOutputInterface {
private:
    DescriptorImageInfoVector m_Textures;

public:
    static std::shared_ptr<BreakTexturesGroupNode> Create(GaiApi::VulkanCorePtr vVulkanCorePtr);

public:
    BreakTexturesGroupNode();
    ~BreakTexturesGroupNode() override;

    bool Init(GaiApi::VulkanCorePtr vVulkanCorePtr) override;
    void Unit() override;

    bool ExecuteAllTime(const uint32_t& vCurrentFrame,
        vk::CommandBuffer* vCmd = nullptr,
        BaseNodeState* vBaseNodeState = nullptr) override;

    bool DrawWidgets(const uint32_t& vCurrentFrame,
        ImGuiContext* vContextPtr = nullptr,
        const std::string& vUserDatas = {}) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame,
        const ImRect& vRect,
        ImGuiContext* vContextPtr = nullptr,
        const std::string& vUserDatas = {}) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame,
        const ImVec2& vMaxSize,
        ImGuiContext* vContextPtr = nullptr,
        const std::string& vUserDatas = {}) override;

    void TreatNotification(
        const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot) override;

    void SetTextures(
        const uint32_t& vBindingPoint, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes) override;

    vk::DescriptorImageInfo* GetDescriptorImageInfo(
        const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
    NodeSlotWeak AddPreDefinedOutput(const NodeSlot& vNodeSlot) override;

private:
    void ReorganizeSlots();
};