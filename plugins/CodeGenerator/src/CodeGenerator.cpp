// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
#include "CodeGenerator.h"
#include <Headers/CodeGeneratorBuild.h>
#include <ctools/FileHelper.h>
#include <ImGuiPack.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <LumoBackend/Graph/Base/NodeSlot.h>
#include <Panes/CodeGeneratorPane.h>

// needed for plugin creating / destroying
extern "C"  // needed for avoid renaming of funcs by the compiler
{
#ifdef WIN32
#define PLUGIN_PREFIX __declspec(dllexport)
#else
#define PLUGIN_PREFIX
#endif

PLUGIN_PREFIX CodeGenerator* allocator() {
    return new CodeGenerator();
}

PLUGIN_PREFIX void deleter(CodeGenerator* ptr) {
    delete ptr;
}
}

CodeGenerator::CodeGenerator() {
#ifdef _MSC_VER
    // active memory leak detector
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

bool CodeGenerator::Init(GaiApi::VulkanCoreWeak vVulkanCoreWeak) {
    if (PluginInterface::Init(vVulkanCoreWeak)) {
        m_NodeGraphPtr = GeneratorNode::Create(vVulkanCoreWeak.lock());
        if (m_NodeGraphPtr != nullptr) {
            CodeGeneratorPane::Instance()->setVulkanCore(m_VulkanCoreWeak);
            CodeGeneratorPane::Instance()->SetNodeGraph(m_NodeGraphPtr);
            return true;
        }
    }
    return false;
}

void CodeGenerator::Unit() {
    m_NodeGraphPtr.reset();
}

bool CodeGenerator::AuthorizeLoading() {
    return true;
}

void CodeGenerator::ActionAfterInit() {
    CodeGeneratorPane::Instance()->setVulkanCore(m_VulkanCoreWeak);
}

uint32_t CodeGenerator::GetVersionMajor() const {
    return CodeGenerator_MinorNumber;
}

uint32_t CodeGenerator::GetVersionMinor() const {
    return CodeGenerator_MajorNumber;
}

uint32_t CodeGenerator::GetVersionBuild() const {
    return CodeGenerator_BuildNumber;
}

std::string CodeGenerator::GetName() const {
    return "CodeGenerator";
}

std::string CodeGenerator::GetVersion() const {
    return CodeGenerator_BuildId;
}

std::string CodeGenerator::GetDescription() const {
    return "Nodes Code Generator plugin for developpers";
}

std::vector<std::string> CodeGenerator::GetNodes() const {
    return {};
}

std::vector<LibraryEntry> CodeGenerator::GetLibrary() const {
    return {};
}

BaseNodePtr CodeGenerator::CreatePluginNode(const std::string& vPluginNodeName) {
    return nullptr;
}

std::vector<PluginPaneConfig> CodeGenerator::GetPanes() const {
    std::vector<PluginPaneConfig> res;
    res.push_back(PluginPaneConfig(CodeGeneratorPane::Instance(), "Code Generator", "CodeGenerator", "CENTRAL", 0.0f, false, false));
    return res;
}

int CodeGenerator::ResetImGuiID(const int& vWidgetId) {
    auto ids = ImGui::GetPUSHID();
    ImGui::SetPUSHID(vWidgetId);
    return ids;
}

std::string CodeGenerator::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;
    str += vOffset + "<" + GetName() + ">\n";
    str += CodeGeneratorPane::Instance()->getXml(vOffset + "\t", vUserDatas);
    if (m_NodeGraphPtr) {
        str += vOffset + "\t<scene>\n";
        if (!m_NodeGraphPtr->m_ChildNodes.empty()) {
            str += m_NodeGraphPtr->getXml(vOffset + "\t\t", "project");
        }
        str += vOffset + "\t</scene>\n";
    }
    str += vOffset + "</" + GetName() + ">\n";
    return str;
}

bool CodeGenerator::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != 0)
        strParentName = vParent->Value();

    CodeGeneratorPane::Instance()->setFromXml(vElem, vParent, vUserDatas);

    if (strName == "graph" && strParentName == "scene") {
        m_NodeGraphPtr->RecursParsingConfig(vElem, vParent, "project");
        m_NodeGraphPtr->FinalizeGraphLoading();
    }

    return true;
}