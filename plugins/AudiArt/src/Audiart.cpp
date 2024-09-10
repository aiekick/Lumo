// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "Audiart.h"
#include <Headers/AudiartBuild.h>
#include <ctools/FileHelper.h>
#include <ImGuiPack.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>

#include <Nodes/Effect/SoundFFTNode.h>
#include <Nodes/Operations/HistorizeNode.h>
#include <Nodes/Operations/AudioTextureNode.h>
#include <Nodes/Source/SpeakerSourceNode.h>
#include <Nodes/Viewer/SourcePreviewNode.h>
#include <Nodes/Viewer/VisuHexGridNode.h>
#include <Nodes/Windowing/BlackmanFilterNode.h>

#ifndef USE_PLUGIN_STATIC_LINKING
// needed for plugin creating / destroying
extern "C"  // needed for avoid renaming of funcs by the compiler
{
#ifdef WIN32
#define PLUGIN_PREFIX __declspec(dllexport)
#else
#define PLUGIN_PREFIX
#endif

PLUGIN_PREFIX Audiart* allocator() {
    return new Audiart();
}

PLUGIN_PREFIX void deleter(Audiart* ptr) {
    delete ptr;
}
}
#endif  // USE_PLUGIN_STATIC_LINKING

Audiart::Audiart() {
#ifdef _MSC_VER
    // active memory leak detector
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void Audiart::ActionAfterInit() {
    NodeSlot::sGetSlotColors()->AddSlotColor("SCENEAUDIART", ImVec4(0.2f, 0.6f, 0.4f, 1.0f));
}

uint32_t Audiart::GetVersionMajor() const {
    return Audiart_MinorNumber;
}

uint32_t Audiart::GetVersionMinor() const {
    return Audiart_MajorNumber;
}

uint32_t Audiart::GetVersionBuild() const {
    return Audiart_BuildNumber;
}

std::string Audiart::GetName() const {
    return "Audiart";
}

std::string Audiart::GetVersion() const {
    return Audiart_BuildId;
}

std::string Audiart::GetDescription() const {
    return "Audio Art plugin";
}

std::vector<std::string> Audiart::GetNodes() const {
    return {
        /*
        "MESH_SIM_RENDERER",
        "COMPUTE_MESH_SIM"
        */
    };
}

std::vector<LibraryEntry> Audiart::GetLibrary() const {
    std::vector<LibraryEntry> res;

    res.push_back(AddLibraryEntry("Audiart/Effects", "FFT (audio to audio)", "SOUND_FFT"));
    res.push_back(AddLibraryEntry("Audiart/Operations", "Historize (audio to texture)", "HISTORIZE"));
    res.push_back(AddLibraryEntry("Audiart/Operations", "Audiart to Texture", "AUDIO_TEXTURE"));
    res.push_back(AddLibraryEntry("Audiart/Sources", "Speacker (to audio)", "SPEAKER_SOURCE"));
    res.push_back(AddLibraryEntry("Audiart/Viewers", "Source Preview (audio plot display)", "SOURCE_PREVIEW"));
    res.push_back(AddLibraryEntry("Audiart/Viewers", "Visu Hex Grid (texture to texture)", "VISU_HEX_GRID"));
    res.push_back(AddLibraryEntry("Audiart/Windowing", "Blackman FIlter", "BLACKMAN_FILTER"));

    return res;
}

BaseNodePtr Audiart::CreatePluginNode(const std::string& vPluginNodeName) {
    auto vkCorePtr = m_VulkanCoreWeak.lock();

    if (vPluginNodeName == "SOUND_FFT")
        return SoundFFTNode::Create(vkCorePtr);
    else if (vPluginNodeName == "HISTORIZE")
        return HistorizeNode::Create(vkCorePtr);
    else if (vPluginNodeName == "AUDIO_TEXTURE")
        return AudioTextureNode::Create(vkCorePtr);
    else if (vPluginNodeName == "SPEAKER_SOURCE")
        return SpeakerSourceNode::Create(vkCorePtr);
    else if (vPluginNodeName == "SOURCE_PREVIEW")
        return SourcePreviewNode::Create(vkCorePtr);
    else if (vPluginNodeName == "VISU_HEX_GRID")
        return VisuHexGridNode::Create(vkCorePtr);
    else if (vPluginNodeName == "BLACKMAN_FILTER")
        return BlackmanFilterNode::Create(vkCorePtr);

    return nullptr;
}

std::vector<PluginPaneConfig> Audiart::GetPanes() const {
    std::vector<PluginPaneConfig> res;

    return res;
}

int Audiart::ResetImGuiID(const int& vWidgetId) {
    auto ids = ImGui::GetPUSHID();
    ImGui::SetPUSHID(vWidgetId);
    return ids;
}
