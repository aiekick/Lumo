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

#include <LumoBackend/Graph/Graph.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Interfaces/VariableOutputInterface.h>

class VariableModule;
class VariableNode : public BaseNode, public VariableOutputInterface {
public:
    static std::shared_ptr<VariableNode> Create(GaiApi::VulkanCorePtr vVulkanCorePtr, const std::string& vNodeType);

private:
    std::shared_ptr<VariableModule> m_VariableModulePtr = nullptr;

public:
    VariableNode(const std::string& vNodeType);
    ~VariableNode() override;
    bool Init(GaiApi::VulkanCorePtr vVulkanCorePtr) override;
    bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    void DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) override;
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
    void DrawOutputWidget(BaseNodeState* vBaseNodeState, NodeSlotWeak vSlot) override;
    SceneVariableWeak GetVariable(const uint32_t& vVariableIndex) override;
};