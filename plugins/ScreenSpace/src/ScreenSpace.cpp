// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "ScreenSpace.h"
#include <Headers/ScreenSpaceBuild.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/VulkanWindow.h>
#include <LumoBackend/Graph/Base/NodeSlot.h>

#include <Nodes/ScreenSpace/Effects/SSAONode.h>
#include <Nodes/ScreenSpace/Effects/SSReflectionNode.h>
#include <Nodes/ScreenSpace/ScreenSpaceNode.h>

// needed for plugin creating / destroying
extern "C"  // needed for avoid renaming of funcs by the compiler
{
#ifdef WIN32
#define PLUGIN_PREFIX __declspec(dllexport)
#else
#define PLUGIN_PREFIX
#endif

PLUGIN_PREFIX ScreenSpace* allocator() {
    return new ScreenSpace();
}

PLUGIN_PREFIX void deleter(ScreenSpace* ptr) {
    delete ptr;
}
}

ScreenSpace::ScreenSpace() {
#ifdef _MSC_VER
    // active memory leak detector
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void ScreenSpace::ActionAfterInit() {
    NodeSlot::sGetSlotColors()->AddSlotColor("NONE", ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    NodeSlot::sGetSlotColors()->AddSlotColor("MESH", ImVec4(0.5f, 0.5f, 0.9f, 1.0f));
    NodeSlot::sGetSlotColors()->AddSlotColor("MESH_GROUP", ImVec4(0.1f, 0.1f, 0.8f, 1.0f));
    NodeSlot::sGetSlotColors()->AddSlotColor("LIGHT_GROUP", ImVec4(0.9f, 0.9f, 0.1f, 1.0f));
    NodeSlot::sGetSlotColors()->AddSlotColor("ENVIRONMENT", ImVec4(0.1f, 0.9f, 0.1f, 1.0f));
    NodeSlot::sGetSlotColors()->AddSlotColor("MERGED", ImVec4(0.1f, 0.5f, 0.9f, 1.0f));
    NodeSlot::sGetSlotColors()->AddSlotColor("TEXTURE_2D", ImVec4(0.9f, 0.5f, 0.1f, 1.0f));
    NodeSlot::sGetSlotColors()->AddSlotColor("TEXTURE_2D_GROUP", ImVec4(0.2f, 0.9f, 0.2f, 1.0f));
    NodeSlot::sGetSlotColors()->AddSlotColor("TEXTURE_3D", ImVec4(0.9f, 0.8f, 0.3f, 1.0f));
    NodeSlot::sGetSlotColors()->AddSlotColor("TEXTURE_CUBE", ImVec4(0.9f, 0.7f, 0.2f, 1.0f));
    NodeSlot::sGetSlotColors()->AddSlotColor("MIXED", ImVec4(0.3f, 0.5f, 0.1f, 1.0f));
    NodeSlot::sGetSlotColors()->AddSlotColor("WIDGET_BOOLEAN", ImVec4(0.8f, 0.7f, 0.6f, 1.0f));
    NodeSlot::sGetSlotColors()->AddSlotColor("WIDGET_UINT", ImVec4(0.8f, 0.7f, 0.6f, 1.0f));
    NodeSlot::sGetSlotColors()->AddSlotColor("WIDGET_INT", ImVec4(0.8f, 0.7f, 0.6f, 1.0f));
    NodeSlot::sGetSlotColors()->AddSlotColor("WIDGET_FLOAT", ImVec4(0.8f, 0.7f, 0.6f, 1.0f));
    NodeSlot::sGetSlotColors()->AddSlotColor("DEPTH", ImVec4(0.2f, 0.7f, 0.6f, 1.0f));
}

uint32_t ScreenSpace::GetVersionMajor() const {
    return ScreenSpace_MinorNumber;
}

uint32_t ScreenSpace::GetVersionMinor() const {
    return ScreenSpace_MajorNumber;
}

uint32_t ScreenSpace::GetVersionBuild() const {
    return ScreenSpace_BuildNumber;
}

std::string ScreenSpace::GetName() const {
    return "ScreenSpace";
}

std::string ScreenSpace::GetVersion() const {
    return ScreenSpace_BuildId;
}

std::string ScreenSpace::GetDescription() const {
    return "ScreenSpace Nodes";
}

std::vector<std::string> ScreenSpace::GetNodes() const {
    return {""};
}

std::vector<LibraryEntry> ScreenSpace::GetLibrary() const {
    std::vector<LibraryEntry> res;

    res.push_back(AddLibraryEntry("ScreenSpace/Effects", "Screen Space Reflection (SSRe)", "SS_REFLECTION"));
    res.push_back(AddLibraryEntry("ScreenSpace/Effects", "Screen Space Ambient Occlusion (SSAO)", "SS_AO"));

    res.push_back(AddLibraryEntry("ScreenSpace", "ScreenSpace", "SCREEN_SPACE"));

    return res;
}

BaseNodePtr ScreenSpace::CreatePluginNode(const std::string& vPluginNodeName) {
    auto vkScreenSpacePtr = m_VulkanCoreWeak.lock();

    if (vPluginNodeName == "SS_AO") {
        return SSAONode::Create(vkScreenSpacePtr);
    } else if (vPluginNodeName == "SS_REFLECTION") {
        return SSReflectionNode::Create(vkScreenSpacePtr);
    } else if (vPluginNodeName == "SCREEN_SPACE") {
        return ScreenSpaceNode::Create(vkScreenSpacePtr);
    }

    return nullptr;
}

std::vector<PluginPaneConfig> ScreenSpace::GetPanes() const {
    return {};
}

int ScreenSpace::ResetImGuiID(const int& vWidgetId) {
    auto ids = ImGui::GetPUSHID();
    ImGui::SetPUSHID(vWidgetId);
    return ids;
}
