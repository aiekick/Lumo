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

#include <ImGuiPack.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/PluginInterface.h>
#include <LumoBackend/Graph/Graph.h>
#include <LumoBackend/Interfaces/ProjectInterface.h>
#include <Editor/UBOEditor.h>
#include <Editor/SlotEditor.h>
#include <stdint.h>
#include <string>
#include <memory>
#include <map>
#include <ctools/ConfigAbstract.h>

class ProjectFile;
class CodeGeneratorPane : public PluginPane,     //
                          public NodeInterface,  //
                          public conf::ConfigAbstract {
private:
    GaiApi::VulkanCoreWeak m_VulkanCore;
    GeneratorNodeWeak m_NodeGraph;
    GeneratorNodeWeak m_SelectedNode;
    bool m_NeedToApplyLayout = false;
    SlotEditor m_InputSlotEditor;
    SlotEditor m_OutputSlotEditor;
    NodeSlotInputWeak m_SelectedNodeSlotInput;
    NodeSlotOutputWeak m_SelectedNodeSlotOutput;
    ImWidgets::InputText m_NodeDisplayNameInputText;
    ImWidgets::InputText m_NodeCreationNameInputText;
    ImWidgets::InputText m_NodeCategoryNameInputText;
    ImWidgets::InputText m_ClassNameInputText;
    ImWidgets::InputText m_ModuleDisplayNameInputText;
    ImWidgets::InputText m_ModuleXmlNameInputText;
    std::vector<ImWidgets::InputText> m_CustomTypeInputTexts;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(
        const uint32_t& vCurrentFrame, bool* vOpened, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    void SetNodeGraph(GeneratorNodeWeak vNodeGraph);
    void Select(BaseNodeWeak vObjet);
    std::vector<ImWidgets::InputText> GetCustomTypeInputTexts() {
        return m_CustomTypeInputTexts;
    }
    void setVulkanCore(GaiApi::VulkanCoreWeak vVulkanCoreWeak);

private:
    void DrawContent();
    void DrawGraph();
    void DrawPluginCreationPane();
    void DrawNodeCreationPane();
    void SelectNode(const BaseNodeWeak& vNode);
    void SelectSlot(const NodeSlotWeak& vSlot, const ImGuiMouseButton& vButton);

public:  // configuration
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
    bool LoadNodeFromXML(BaseNodeWeak vBaseNodeWeak,
        tinyxml2::XMLElement* vElem,
        tinyxml2::XMLElement* vParent,
        const std::string& vNodeName,
        const std::string& vNodeType,
        const ct::fvec2& vPos,
        const size_t& vNodeId);

public:  // singleton
    static std::shared_ptr<CodeGeneratorPane> Instance() {
        static std::shared_ptr<CodeGeneratorPane> _instance = std::make_shared<CodeGeneratorPane>();
        return _instance;
    }

public:
    CodeGeneratorPane();                                    // Prevent construction
    CodeGeneratorPane(const CodeGeneratorPane&) = default;  // Prevent construction by copying
    CodeGeneratorPane& operator=(const CodeGeneratorPane&) {
        return *this;
    };                     // Prevent assignment
    ~CodeGeneratorPane();  // Prevent unwanted destruction};
};
