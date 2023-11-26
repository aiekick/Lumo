// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "PostProcessing.h"
#include <Headers/PostProcessingBuild.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/VulkanWindow.h>
#include <LumoBackend/Graph/Base/NodeSlot.h>

#include <Nodes/PostPro/Effects/BlurNode.h>
#include <Nodes/PostPro/Effects/BloomNode.h>
#include <Nodes/PostPro/Effects/ChromaticAberrationsNode.h>
#include <Nodes/PostPro/Effects/DilationNode.h>
#include <Nodes/PostPro/Effects/SharpnessNode.h>
#include <Nodes/PostPro/Effects/ToneMapNode.h>
#include <Nodes/PostPro/Effects/VignetteNode.h>
#include <Nodes/PostPro/PostProcessingNode.h>

// needed for plugin creating / destroying
extern "C"  // needed for avoid renaming of funcs by the compiler
{
#ifdef WIN32
#define PLUGIN_PREFIX __declspec(dllexport)
#else
#define PLUGIN_PREFIX
#endif

PLUGIN_PREFIX PostProcessing* allocator() {
    return new PostProcessing();
}

PLUGIN_PREFIX void deleter(PostProcessing* ptr) {
    delete ptr;
}
}

PostProcessing::PostProcessing() {
#ifdef _MSC_VER
    // active memory leak detector
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void PostProcessing::ActionAfterInit() {
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

uint32_t PostProcessing::GetVersionMajor() const {
    return PostProcessing_MinorNumber;
}

uint32_t PostProcessing::GetVersionMinor() const {
    return PostProcessing_MajorNumber;
}

uint32_t PostProcessing::GetVersionBuild() const {
    return PostProcessing_BuildNumber;
}

std::string PostProcessing::GetName() const {
    return "PostProcessing";
}

std::string PostProcessing::GetVersion() const {
    return PostProcessing_BuildId;
}

std::string PostProcessing::GetDescription() const {
    return "PostProcessing Nodes";
}

std::vector<std::string> PostProcessing::GetNodes() const {
    return {""};
}

std::vector<LibraryEntry> PostProcessing::GetLibrary() const {
    std::vector<LibraryEntry> res;

    res.push_back(AddLibraryEntry("PostPro/Effects", "Bloom", "BLOOM"));
    res.push_back(AddLibraryEntry("PostPro/Effects", "Blur", "BLUR"));
    res.push_back(AddLibraryEntry("PostPro/Effects", "Chromatic Aberrations", "CHROMATIC_ABERRATIONS"));
    res.push_back(AddLibraryEntry("PostPro/Effects", "Dilation", "DILATION"));
    res.push_back(AddLibraryEntry("PostPro/Effects", "Sharpness", "SHARPNESS"));
    res.push_back(AddLibraryEntry("PostPro/Effects", "Tone Map", "TONE_MAP"));
    res.push_back(AddLibraryEntry("PostPro/Effects", "Vignette", "VIGNETTE"));

    res.push_back(AddLibraryEntry("PostPro", "PostProcessing", "POST_PROCESSING"));

    return res;
}

BaseNodePtr PostProcessing::CreatePluginNode(const std::string& vPluginNodeName) {
    auto vkPostProcessingPtr = m_VulkanCoreWeak.lock();

    if (vPluginNodeName == "BLOOM") {
        return BloomNode::Create(vkPostProcessingPtr);
    } else if (vPluginNodeName == "BLUR") {
        return BlurNode::Create(vkPostProcessingPtr);
    } else if (vPluginNodeName == "CHROMATIC_ABERRATIONS") {
        return ChromaticAberrationsNode::Create(vkPostProcessingPtr);
    } else if (vPluginNodeName == "DILATION") {
        return DilationNode::Create(vkPostProcessingPtr);
    } else if (vPluginNodeName == "SHARPNESS") {
        return SharpnessNode::Create(vkPostProcessingPtr);
    } else if (vPluginNodeName == "TONE_MAP") {
        return ToneMapNode::Create(vkPostProcessingPtr);
    } else if (vPluginNodeName == "VIGNETTE") {
        return VignetteNode::Create(vkPostProcessingPtr);
    } else if (vPluginNodeName == "POST_PROCESSING") {
        return PostProcessingNode::Create(vkPostProcessingPtr);
    }

    return nullptr;
}

std::vector<PluginPaneConfig> PostProcessing::GetPanes() const {
    return {};
}

int PostProcessing::ResetImGuiID(const int& vWidgetId) {
    auto ids = ImGui::GetPUSHID();
    ImGui::SetPUSHID(vWidgetId);
    return ids;
}
