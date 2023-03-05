// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "Planet.h"
#include <Headers/PlanetBuild.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Graph/Base/BaseNode.h>
#include <Systems/CommonSystem.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanWindow.h>

#ifndef USE_PLUGIN_STATIC_LINKING
// needed for plugin creating / destroying
extern "C" // needed for avoid renaming of funcs by the compiler
{
#ifdef WIN32
#define PLUGIN_PREFIX __declspec (dllexport)
#else
#define PLUGIN_PREFIX
#endif

	PLUGIN_PREFIX Planet* allocator()
	{
		return new Planet();
	}

	PLUGIN_PREFIX void deleter(Planet* ptr)
	{
		delete ptr;
	}
}
#endif // USE_PLUGIN_STATIC_LINKING

Planet::Planet()
{
#ifdef _MSC_VER
	// active memory leak detector
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void Planet::ActionAfterInit()
{
	NodeSlot::sGetSlotColors()->AddSlotColor("SCENEPlanet", ImVec4(0.2f, 0.6f, 0.4f, 1.0f));
}

uint32_t Planet::GetVersionMajor() const
{
	return Planet_MinorNumber;
}

uint32_t Planet::GetVersionMinor() const
{
	return Planet_MajorNumber;
}

uint32_t Planet::GetVersionBuild() const
{
	return Planet_BuildNumber;
}

std::string Planet::GetName() const
{
	return "Planet";
}

std::string Planet::GetVersion() const
{
	return Planet_BuildId;
}

std::string Planet::GetDescription() const
{
	return "Planet plugin";
}

std::vector<std::string> Planet::GetNodes() const
{
	return
	{
		/*
		"MESH_SIM_RENDERER",
		"COMPUTE_MESH_SIM"
		*/
	};
}

std::vector<LibraryEntry> Planet::GetLibrary() const
{
	std::vector<LibraryEntry> res;

	//res.push_back(AddLibraryEntry("Planet/Renderer", "Planet Renderer", "PLANET_RENDERER"));

	return res;
}

BaseNodePtr Planet::CreatePluginNode(const std::string& vPluginNodeName)
{
	auto vkCorePtr = m_VulkanCoreWeak.getValidShared();

	//if (vPluginNodeName == "PLANET_RENDERER")				return PlanetRendererNode::Create(vkCorePtr);

	return nullptr;
}

std::vector<PluginPane> Planet::GetPanes() const
{
	std::vector<PluginPane> res;

	return res;
}

int Planet::ResetImGuiID(const int& vWidgetId)
{
	auto ids = ImGui::CustomStyle::Instance()->pushId;
	ImGui::CustomStyle::Instance()->pushId = vWidgetId;
	return ids;
}
