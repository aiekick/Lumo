// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "Core.h"
#include <Headers/CoreBuild.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Graph/Base/BaseNode.h>
#include <Systems/CommonSystem.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanWindow.h>
#include <Graph/Base/NodeSlot.h>

#include <Nodes/Assets/MeshNode.h>
#include <Nodes/Assets/Texture2DNode.h>
#include <Nodes/Assets/CubeMapNode.h>

#include <Nodes/Breaks/BreakTexturesGroupNode.h>

#include <Nodes/DiffOperators/LaplacianNode.h>

#include <Nodes/Lighting/LightGroupNode.h>
#include <Nodes/Lighting/DiffuseNode.h>
#include <Nodes/Lighting/SpecularNode.h>
#include <Nodes/Lighting/ShadowMapNode.h>
#include <Nodes/Lighting/ModelShadowNode.h>
#include <Nodes/Lighting/Normal2DNode.h>
#include <Nodes/Lighting/ReflectionNode.h>
#include <Nodes/Lighting/RefractionNode.h>

#include <Nodes/Modifiers/SmoothNormalNode.h>

#include <Nodes/Misc/GridNode.h>
#include <Nodes/Misc/Layering2DNode.h>

#include <Nodes/PostPro/SSAONode.h>
#include <Nodes/PostPro/BlurNode.h>
#include <Nodes/PostPro/ToneMapNode.h>

#include <Nodes/Preview/CubeMapPreviewNode.h>
#include <Nodes/Preview/LongLatPeviewNode.h>

#include <Nodes/Renderers/MatcapRendererNode.h>
#include <Nodes/Renderers/ChannelRendererNode.h>
#include <Nodes/Renderers/HeatmapRendererNode.h>
#include <Nodes/Renderers/DeferredRendererNode.h>
#include <Nodes/Renderers/PBRRendererNode.h>

#include <Nodes/Simulation/GrayScottNode.h>

#include <Nodes/Utils/MathNode.h>
#include <Nodes/Utils/DepthToPosNode.h>
#include <Nodes/Utils/PosToDepthNode.h>
#include <Nodes/Utils/MeshAttributesNode.h>

#include <Nodes/Widgets/VariableNode.h>
#include <Nodes/Widgets/WidgetColorNode.h>

#ifndef USE_PLUGIN_STATIC_LINKING
// needed for plugin creating / destroying
extern "C" // needed for avoid renaming of funcs by the compiler
{
#ifdef WIN32
#define PLUGIN_PREFIX __declspec (dllexport)
#else
#define PLUGIN_PREFIX
#endif

	PLUGIN_PREFIX Core* allocator()
	{
		return new Core();
	}

	PLUGIN_PREFIX void deleter(Core* ptr)
	{
		delete ptr;
	}
}
#endif // USE_PLUGIN_STATIC_LINKING

Core::Core()
{
#ifdef _MSC_VER
	// active memory leak detector
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void Core::ActionAfterInit()
{
	NodeSlot::sGetSlotColors()->AddSlotColor("NONE", ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("MESH", ImVec4(0.5f, 0.5f, 0.9f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("MESH_GROUP", ImVec4(0.1f, 0.1f, 0.8f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("LIGHT_GROUP", ImVec4(0.9f, 0.9f, 0.1f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("ENVIRONMENT", ImVec4(0.1f, 0.9f, 0.1f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("MERGED", ImVec4(0.1f, 0.5f, 0.9f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("TEXTURE_2D", ImVec4(0.9f, 0.5f, 0.1f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("TEXTURE_CUBE", ImVec4(0.9f, 0.7f, 0.2f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("TEXTURE_2D_GROUP", ImVec4(0.2f, 0.9f, 0.2f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("TEXTURE_3D", ImVec4(0.9f, 0.8f, 0.3f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("MIXED", ImVec4(0.3f, 0.5f, 0.1f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("WIDGET_BOOLEAN", ImVec4(0.8f, 0.7f, 0.6f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("WIDGET_UINT", ImVec4(0.8f, 0.7f, 0.6f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("WIDGET_INT", ImVec4(0.8f, 0.7f, 0.6f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("WIDGET_FLOAT", ImVec4(0.8f, 0.7f, 0.6f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("DEPTH", ImVec4(0.2f, 0.7f, 0.6f, 1.0f));
}

uint32_t Core::GetVersionMajor() const
{
	return Core_MinorNumber;
}

uint32_t Core::GetVersionMinor() const
{
	return Core_MajorNumber;
}

uint32_t Core::GetVersionBuild() const
{
	return Core_BuildNumber;
}

std::string Core::GetName() const
{
	return "Core";
}

std::string Core::GetVersion() const
{
	return Core_BuildId;
}

std::string Core::GetDescription() const
{
	return "Core Nodes";
}

std::vector<std::string> Core::GetNodes() const
{
	return
	{
		""
	};
}

std::vector<LibraryEntry> Core::GetLibrary() const
{
	std::vector<LibraryEntry> res;

	res.push_back(AddLibraryEntry("Core/3D/Assets", "3D Model", "MESH"));
	res.push_back(AddLibraryEntry("Core/2D/Assets", "2D Texture", "TEXTURE_2D"));
	res.push_back(AddLibraryEntry("Core/3D/Assets", "CubeMap", "CUBE_MAP"));

	res.push_back(AddLibraryEntry("Core/3D/Breaks", "Break Textures 2D Group", "BREAK_TEXTURE_2D_GROUP"));

	res.push_back(AddLibraryEntry("Core/3D/Misc", "Grid / Axis", "GRID_AXIS"));

	res.push_back(AddLibraryEntry("Core/2D/Misc", "2D Layering", "2D_LAYERING"));
	res.push_back(AddLibraryEntry("Core/2D/Lighting", "Normal 2D", "2D_NORMAL_FROM_TEXTURE"));

	res.push_back(AddLibraryEntry("Core/3D/Lighting", "Lights", "LIGHT_GROUP"));
	res.push_back(AddLibraryEntry("Core/3D/Lighting/Shadow", "Shadow Mapping", "SHADOW_MAPPING"));
	res.push_back(AddLibraryEntry("Core/3D/Lighting/Shadow", "Model Shadow", "MODEL_SHADOW"));
	res.push_back(AddLibraryEntry("Core/3D/Lighting", "Diffuse", "DIFFUSE"));
	res.push_back(AddLibraryEntry("Core/3D/Lighting", "Reflection", "REFLECTION"));
	res.push_back(AddLibraryEntry("Core/3D/Lighting", "Refraction", "REFRACTION"));
	res.push_back(AddLibraryEntry("Core/3D/Lighting", "Specular", "SPECULAR"));
	res.push_back(AddLibraryEntry("Core/3D/Lighting", "SSAO", "SSAO"));

	res.push_back(AddLibraryEntry("Core/3D/Modifiers", "Smooth Normals", "SMOOTH_NORMAL"));

	res.push_back(AddLibraryEntry("Core/3D/Preview", "CubeMap Preview", "CUBE_MAP_PREVIEW"));
	res.push_back(AddLibraryEntry("Core/3D/Preview", "LongLat Preview", "LONG_LAT_PREVIEW"));

	res.push_back(AddLibraryEntry("Core/2D/PostPro", "Blur", "BLUR"));
	res.push_back(AddLibraryEntry("Core/2D/PostPro", "Laplacian", "LAPLACIAN"));
	res.push_back(AddLibraryEntry("Core/2D/PostPro", "Tone Map", "TONE_MAP"));

	res.push_back(AddLibraryEntry("Core/3D/Renderers", "Channels", "CHANNEL_RENDERER"));
	res.push_back(AddLibraryEntry("Core/3D/Renderers", "Deferred", "DEFERRED_RENDERER"));
	res.push_back(AddLibraryEntry("Core/3D/Renderers", "Heatmap", "HEATMAP_RENDERER"));
	res.push_back(AddLibraryEntry("Core/3D/Renderers", "Matcap", "MATCAP_RENDERER"));
	res.push_back(AddLibraryEntry("Core/3D/Renderers", "PBR", "PBR_RENDERER"));

	res.push_back(AddLibraryEntry("Core/2D/Simulation", "GrayScott", "2D_SIMULATION_GRAY_SCOTT"));

	res.push_back(AddLibraryEntry("Core/3D/Utils", "3D Model Attributes", "MESH_ATTRIBUTES"));
	res.push_back(AddLibraryEntry("Core/3D/Utils", "Depth to Pos", "DEPTH_TO_POS"));
	res.push_back(AddLibraryEntry("Core/3D/Utils", "Pos to Depth", "POS_TO_DEPTH"));
	res.push_back(AddLibraryEntry("Core/2D/Utils", "Math", "MATH"));

	res.push_back(AddLibraryEntry("Core/Widgets", "Boolean", "WIDGET_BOOLEAN"));
	res.push_back(AddLibraryEntry("Core/Widgets", "Color", "WIDGET_COLOR"));

	return res;
}

BaseNodePtr Core::CreatePluginNode(const std::string& vPluginNodeName)
{
	auto vkCorePtr = m_VulkanCoreWeak.getValidShared();

	// assets
	if (vPluginNodeName == "MESH")								return MeshNode::Create(vkCorePtr);
	else if (vPluginNodeName == "TEXTURE_2D")					return Texture2DNode::Create(vkCorePtr);
	else if (vPluginNodeName == "CUBE_MAP")						return CubeMapNode::Create(vkCorePtr);

	// Divers
	else if (vPluginNodeName == "BREAK_TEXTURE_2D_GROUP")		return BreakTexturesGroupNode::Create(vkCorePtr);

	// Divers
	else if (vPluginNodeName == "GRID_AXIS")					return GridNode::Create(vkCorePtr);

	// Lighting
	else if (vPluginNodeName == "LIGHT_GROUP")					return LightGroupNode::Create(vkCorePtr);
	else if (vPluginNodeName == "SHADOW_MAPPING")				return ShadowMapNode::Create(vkCorePtr);
	else if (vPluginNodeName == "MODEL_SHADOW")					return ModelShadowNode::Create(vkCorePtr);
	else if (vPluginNodeName == "DIFFUSE")						return DiffuseNode::Create(vkCorePtr);
	else if (vPluginNodeName == "REFLECTION")					return ReflectionNode::Create(vkCorePtr);
	else if (vPluginNodeName == "REFRACTION")					return RefractionNode::Create(vkCorePtr);
	else if (vPluginNodeName == "SPECULAR")						return SpecularNode::Create(vkCorePtr);
	else if (vPluginNodeName == "SSAO")							return SSAONode::Create(vkCorePtr);

	// Modifiers
	else if (vPluginNodeName == "SMOOTH_NORMAL")				return SmoothNormalNode::Create(vkCorePtr);

	// Post Processing
	else if (vPluginNodeName == "BLUR")							return BlurNode::Create(vkCorePtr);
	else if (vPluginNodeName == "LAPLACIAN")					return LaplacianNode::Create(vkCorePtr);
	else if (vPluginNodeName == "TONE_MAP")						return ToneMapNode::Create(vkCorePtr);

	// Preview
	else if (vPluginNodeName == "CUBE_MAP_PREVIEW")				return CubeMapPreviewNode::Create(vkCorePtr);
	else if (vPluginNodeName == "LONG_LAT_PREVIEW")				return LongLatPeviewNode::Create(vkCorePtr);

	// renderers
	else if (vPluginNodeName == "CHANNEL_RENDERER")				return ChannelRendererNode::Create(vkCorePtr);
	else if (vPluginNodeName == "DEFERRED_RENDERER")			return DeferredRendererNode::Create(vkCorePtr);
	else if (vPluginNodeName == "HEATMAP_RENDERER")				return HeatmapRendererNode::Create(vkCorePtr);
	else if (vPluginNodeName == "MATCAP_RENDERER")				return MatcapRendererNode::Create(vkCorePtr);
	else if (vPluginNodeName == "PBR_RENDERER")					return PBRRendererNode::Create(vkCorePtr);

	// Simulations
	else if (vPluginNodeName == "2D_SIMULATION_GRAY_SCOTT")		return GrayScottNode::Create(vkCorePtr);
	else if (vPluginNodeName == "2D_NORMAL_FROM_TEXTURE")		return Normal2DNode::Create(vkCorePtr);
	else if (vPluginNodeName == "2D_LAYERING")					return Layering2DNode::Create(vkCorePtr);

	// Utils
	else if (vPluginNodeName == "DEPTH_TO_POS")					return DepthToPosNode::Create(vkCorePtr);
	else if (vPluginNodeName == "MESH_ATTRIBUTES")				return MeshAttributesNode::Create(vkCorePtr);
	else if (vPluginNodeName == "POS_TO_DEPTH")					return PosToDepthNode::Create(vkCorePtr);
	else if (vPluginNodeName == "MATH")							return MathNode::Create(vkCorePtr);

	// Variables
	else if (vPluginNodeName == "WIDGET_BOOLEAN")				return VariableNode::Create(vkCorePtr, vPluginNodeName);
	else if (vPluginNodeName == "WIDGET_FLOAT")					return VariableNode::Create(vkCorePtr, vPluginNodeName);
	else if (vPluginNodeName == "WIDGET_INT")					return VariableNode::Create(vkCorePtr, vPluginNodeName);
	else if (vPluginNodeName == "WIDGET_UINT")					return VariableNode::Create(vkCorePtr, vPluginNodeName);
	else if (vPluginNodeName == "WIDGET_COLOR")					return WidgetColorNode::Create(vkCorePtr);

	return nullptr;
}

int Core::ResetImGuiID(const int& vWidgetId)
{
	auto ids = ImGui::CustomStyle::Instance()->pushId;
	ImGui::CustomStyle::Instance()->pushId = vWidgetId;
	return ids;
}
