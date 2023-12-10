// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "Smoke.h"
#include <Headers/SmokeBuild.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/VulkanWindow.h>
#include <LumoBackend/Graph/Base/NodeSlot.h>

// needed for plugin creating / destroying
extern "C"  // needed for avoid renaming of funcs by the compiler
{
#ifdef WIN32
#define PLUGIN_PREFIX __declspec(dllexport)
#else
#define PLUGIN_PREFIX
#endif

PLUGIN_PREFIX Smoke* allocator() {
    return new Smoke();
}

PLUGIN_PREFIX void deleter(Smoke* ptr) {
    delete ptr;
}
}

Smoke::Smoke() {
#ifdef _MSC_VER
    // active memory leak detector
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void Smoke::ActionAfterInit() {
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

uint32_t Smoke::GetVersionMajor() const {
    return Smoke_MinorNumber;
}

uint32_t Smoke::GetVersionMinor() const {
    return Smoke_MajorNumber;
}

uint32_t Smoke::GetVersionBuild() const {
    return Smoke_BuildNumber;
}

std::string Smoke::GetName() const {
    return "Smoke";
}

std::string Smoke::GetVersion() const {
    return Smoke_BuildId;
}

std::string Smoke::GetDescription() const {
    return "Smoke Nodes";
}

std::vector<std::string> Smoke::GetNodes() const {
    return {""};
}

std::vector<LibraryEntry> Smoke::GetLibrary() const {
    std::vector<LibraryEntry> res;
	
    return res;
}

BaseNodePtr Smoke::CreatePluginNode(const std::string& vPluginNodeName) {
    auto vkCorePtr = m_VulkanCoreWeak.lock();

    //if (vPluginNodeName == "BREAK_TEXTURE_2D_GROUP")
    //    return BreakTexturesGroupNode::Create(vkCorePtr);

    return nullptr;
}

std::vector<PluginPaneConfig> Smoke::GetPanes() const {
    return {};
}

int Smoke::ResetImGuiID(const int& vWidgetId) {
    auto ids = ImGui::GetPUSHID();
    ImGui::SetPUSHID(vWidgetId);
    return ids;
}
