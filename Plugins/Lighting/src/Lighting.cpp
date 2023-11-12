// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "Lighting.h"
#include <Headers/LightingBuild.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/VulkanWindow.h>
#include <LumoBackend/Graph/Base/NodeSlot.h>

#include <Nodes/Breaks/BreakTexturesGroupNode.h>

#include <Nodes/Exporter/TextureGroupExporterNode.h>

#include <Nodes/Lighting/LightGroupNode.h>
#include <Nodes/Lighting/DiffuseNode.h>
#include <Nodes/Lighting/SpecularNode.h>
#include <Nodes/Lighting/ShadowMapNode.h>
#include <Nodes/Lighting/ModelShadowNode.h>
#include <Nodes/Lighting/ReflectionNode.h>
#include <Nodes/Lighting/RefractionNode.h>
#include <Nodes/Lighting/CellShadingNode.h>

#include <Nodes/Preview/CubeMapPreviewNode.h>
#include <Nodes/Preview/LongLatPeviewNode.h>

#include <Nodes/Renderers/BillBoardRendererNode.h>
#include <Nodes/Renderers/DeferredRendererNode.h>
#include <Nodes/Renderers/PBRRendererNode.h>

#include <Nodes/Utils/DepthConvNode.h>
#include <Nodes/Utils/PosToDepthNode.h>
#include <Nodes/Utils/FlatGradientNode.h>

// needed for plugin creating / destroying
extern "C"  // needed for avoid renaming of funcs by the compiler
{
#ifdef WIN32
#define PLUGIN_PREFIX __declspec(dllexport)
#else
#define PLUGIN_PREFIX
#endif

PLUGIN_PREFIX Lighting* allocator() {
    return new Lighting();
}

PLUGIN_PREFIX void deleter(Lighting* ptr) {
    delete ptr;
}
}

Lighting::Lighting() {
#ifdef _MSC_VER
    // active memory leak detector
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void Lighting::ActionAfterInit() {
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

uint32_t Lighting::GetVersionMajor() const {
    return Lighting_MinorNumber;
}

uint32_t Lighting::GetVersionMinor() const {
    return Lighting_MajorNumber;
}

uint32_t Lighting::GetVersionBuild() const {
    return Lighting_BuildNumber;
}

std::string Lighting::GetName() const {
    return "Lighting";
}

std::string Lighting::GetVersion() const {
    return Lighting_BuildId;
}

std::string Lighting::GetDescription() const {
    return "Lighting Nodes";
}

std::vector<std::string> Lighting::GetNodes() const {
    return {""};
}

std::vector<LibraryEntry> Lighting::GetLibrary() const {
    std::vector<LibraryEntry> res;

    res.push_back(AddLibraryEntry("Breaks", "Break Textures 2D Group", "BREAK_TEXTURE_2D_GROUP"));

    res.push_back(AddLibraryEntry("Lighting", "Lights", "LIGHT_GROUP"));
    res.push_back(AddLibraryEntry("Lighting/Shadow", "Shadow Mapping", "SHADOW_MAPPING"));
    res.push_back(AddLibraryEntry("Lighting/Shadow", "Model Shadow", "MODEL_SHADOW"));
    res.push_back(AddLibraryEntry("Lighting/Effects", "Diffuse", "DIFFUSE"));
    res.push_back(AddLibraryEntry("Lighting/Effects", "Reflection", "REFLECTION"));
    res.push_back(AddLibraryEntry("Lighting/Effects", "Refraction", "REFRACTION"));
    res.push_back(AddLibraryEntry("Lighting/Effects", "Specular", "SPECULAR"));
    res.push_back(AddLibraryEntry("Lighting/Effects", "Cell Shading", "CELL_SHADING"));

    res.push_back(AddLibraryEntry("Preview", "CubeMap Preview", "CUBE_MAP_PREVIEW"));
    res.push_back(AddLibraryEntry("Preview", "LongLat Preview", "LONG_LAT_PREVIEW"));

    res.push_back(AddLibraryEntry("Exporter", "Texture Group 2D Exporter", "TEXTURE_2D_GROUP_EXPORTER"));

    res.push_back(AddLibraryEntry("Renderers", "Deferred", "DEFERRED_RENDERER"));
    res.push_back(AddLibraryEntry("Renderers", "PBR", "PBR_RENDERER"));
    res.push_back(AddLibraryEntry("Renderers", "Billboard", "BILLBOARD_RENDERER"));

    res.push_back(AddLibraryEntry("Utils", "Depth Conversion", "DEPTH_CONVERSION"));
    res.push_back(AddLibraryEntry("Utils", "Pos to Depth", "POS_TO_DEPTH"));
    res.push_back(AddLibraryEntry("Utils", "Flat gradients", "FLAT_GRADIENT"));

    return res;
}

BaseNodePtr Lighting::CreatePluginNode(const std::string& vPluginNodeName) {
    auto vkCorePtr = m_VulkanCoreWeak.lock();

    // Divers
    if (vPluginNodeName == "BREAK_TEXTURE_2D_GROUP")
        return BreakTexturesGroupNode::Create(vkCorePtr);

    // Exporters
    else if (vPluginNodeName == "TEXTURE_2D_GROUP_EXPORTER")
        return TextureGroupExporterNode::Create(vkCorePtr);

    // Lighting
    else if (vPluginNodeName == "LIGHT_GROUP")
        return LightGroupNode::Create(vkCorePtr);
    else if (vPluginNodeName == "SHADOW_MAPPING")
        return ShadowMapNode::Create(vkCorePtr);
    else if (vPluginNodeName == "MODEL_SHADOW")
        return ModelShadowNode::Create(vkCorePtr);
    else if (vPluginNodeName == "DIFFUSE")
        return DiffuseNode::Create(vkCorePtr);
    else if (vPluginNodeName == "REFLECTION")
        return ReflectionNode::Create(vkCorePtr);
    else if (vPluginNodeName == "REFRACTION")
        return RefractionNode::Create(vkCorePtr);
    else if (vPluginNodeName == "SPECULAR")
        return SpecularNode::Create(vkCorePtr);
    else if (vPluginNodeName == "CELL_SHADING")
        return CellShadingNode::Create(vkCorePtr);

    // Preview
    else if (vPluginNodeName == "DEFERRED_RENDERER")
        return DeferredRendererNode::Create(vkCorePtr);
    else if (vPluginNodeName == "PBR_RENDERER")
        return PBRRendererNode::Create(vkCorePtr);
    else if (vPluginNodeName == "BILLBOARD_RENDERER")
        return BillBoardRendererNode::Create(vkCorePtr);

    // Renderers
    else if (vPluginNodeName == "CUBE_MAP_PREVIEW")
        return CubeMapPreviewNode::Create(vkCorePtr);
    else if (vPluginNodeName == "LONG_LAT_PREVIEW")
        return LongLatPeviewNode::Create(vkCorePtr);

    // Utils
    else if (vPluginNodeName == "DEPTH_CONVERSION")
        return DepthConvNode::Create(vkCorePtr);
    else if (vPluginNodeName == "POS_TO_DEPTH")
        return PosToDepthNode::Create(vkCorePtr);
    else if (vPluginNodeName == "FLAT_GRADIENT")
        return FlatGradientNode::Create(vkCorePtr);

    return nullptr;
}

std::vector<PluginPaneConfig> Lighting::GetPanes() const {
    return {};
}

int Lighting::ResetImGuiID(const int& vWidgetId) {
    auto ids = ImGui::GetPUSHID();
    ImGui::SetPUSHID(vWidgetId);
    return ids;
}
