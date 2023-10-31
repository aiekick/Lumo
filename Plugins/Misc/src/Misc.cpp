// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "Misc.h"
#include <Headers/MiscBuild.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/VulkanWindow.h>
#include <LumoBackend/Graph/Base/NodeSlot.h>

#include <Nodes/Modifiers/SmoothNormalNode.h>

#include <Nodes/Misc/Layering2DNode.h>
#include <Nodes/Misc/SdfTextureNode.h>

// needed for plugin creating / destroying
extern "C" // needed for avoid renaming of funcs by the compiler
{
#ifdef WIN32
#define PLUGIN_PREFIX __declspec (dllexport)
#else
#define PLUGIN_PREFIX
#endif

	PLUGIN_PREFIX Misc* allocator()
	{
		return new Misc();
	}

	PLUGIN_PREFIX void deleter(Misc* ptr)
	{
		delete ptr;
	}
}

Misc::Misc()
{
#ifdef _MSC_VER
	// active memory leak detector
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void Misc::ActionAfterInit()
{
	NodeSlot::sGetSlotColors()->AddSlotColor("NONE",				ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("MESH",				ImVec4(0.5f, 0.5f, 0.9f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("MESH_GROUP",			ImVec4(0.1f, 0.1f, 0.8f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("LIGHT_GROUP",			ImVec4(0.9f, 0.9f, 0.1f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("ENVIRONMENT",			ImVec4(0.1f, 0.9f, 0.1f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("MERGED",				ImVec4(0.1f, 0.5f, 0.9f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("TEXTURE_2D",			ImVec4(0.9f, 0.5f, 0.1f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("TEXTURE_2D_GROUP",	ImVec4(0.2f, 0.9f, 0.2f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("TEXTURE_3D",			ImVec4(0.9f, 0.8f, 0.3f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("TEXTURE_CUBE",		ImVec4(0.9f, 0.7f, 0.2f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("MIXED",				ImVec4(0.3f, 0.5f, 0.1f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("WIDGET_BOOLEAN",		ImVec4(0.8f, 0.7f, 0.6f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("WIDGET_UINT",			ImVec4(0.8f, 0.7f, 0.6f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("WIDGET_INT",			ImVec4(0.8f, 0.7f, 0.6f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("WIDGET_FLOAT",		ImVec4(0.8f, 0.7f, 0.6f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("DEPTH",				ImVec4(0.2f, 0.7f, 0.6f, 1.0f));
}

uint32_t Misc::GetVersionMajor() const
{
	return Misc_MinorNumber;
}

uint32_t Misc::GetVersionMinor() const
{
	return Misc_MajorNumber;
}

uint32_t Misc::GetVersionBuild() const
{
	return Misc_BuildNumber;
}

std::string Misc::GetName() const
{
	return "Misc";
}

std::string Misc::GetVersion() const
{
	return Misc_BuildId;
}

std::string Misc::GetDescription() const
{
	return "Misc Nodes";
}

std::vector<std::string> Misc::GetNodes() const
{
	return
	{
		""
	};
}

std::vector<LibraryEntry> Misc::GetLibrary() const
{
	std::vector<LibraryEntry> res;

	res.push_back(AddLibraryEntry("2D/Misc", "2D Layering", "2D_LAYERING"));
	res.push_back(AddLibraryEntry("2D/Misc", "Sdf Texture", "SDF_TEXTURE"));

	res.push_back(AddLibraryEntry("3D/Modifiers", "Smooth Normals", "SMOOTH_NORMAL"));

	return res;
}

BaseNodePtr Misc::CreatePluginNode(const std::string& vPluginNodeName)
{
	auto vkMiscPtr = m_VulkanCoreWeak.lock();

	// Misc
	if (vPluginNodeName == "2D_LAYERING")			return Layering2DNode::Create(vkMiscPtr);
	else if (vPluginNodeName == "SDF_TEXTURE")		return SdfTextureNode::Create(vkMiscPtr);
	
	// Modifiers
	else if (vPluginNodeName == "SMOOTH_NORMAL")	return SmoothNormalNode::Create(vkMiscPtr);

	return nullptr;
}

std::vector<PluginPaneConfig> Misc::GetPanes() const
{
	return {};
}

int Misc::ResetImGuiID(const int& vWidgetId)
{
    auto ids = ImGui::GetPUSHID();
    ImGui::SetPUSHID(vWidgetId);
	return ids;
}
