// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "PlanetSystem.h"
#include <Headers/PlanetSystemBuild.h>
#include <ctools/FileHelper.h>
#include <ImGuiPack.h>
#include <Graph/Base/BaseNode.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/VulkanWindow.h>

#include <Nodes/Planet/PlanetNode.h>

#ifndef USE_PLUGIN_STATIC_LINKING
// needed for plugin creating / destroying
extern "C" // needed for avoid renaming of funcs by the compiler
{
#ifdef WIN32
#define PLUGIN_PREFIX __declspec (dllexport)
#else
#define PLUGIN_PREFIX
#endif

	PLUGIN_PREFIX PlanetSystem* allocator()
	{
		return new PlanetSystem();
	}

	PLUGIN_PREFIX void deleter(PlanetSystem* ptr)
	{
		delete ptr;
	}
}
#endif // USE_PLUGIN_STATIC_LINKING

PlanetSystem::PlanetSystem()
{
#ifdef _MSC_VER
	// active memory leak detector
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void PlanetSystem::ActionAfterInit()
{
	NodeSlot::sGetSlotColors()->AddSlotColor("SCENEPlanetSystem", ImVec4(0.2f, 0.6f, 0.4f, 1.0f));
}

uint32_t PlanetSystem::GetVersionMajor() const
{
	return PlanetSystem_MinorNumber;
}

uint32_t PlanetSystem::GetVersionMinor() const
{
	return PlanetSystem_MajorNumber;
}

uint32_t PlanetSystem::GetVersionBuild() const
{
	return PlanetSystem_BuildNumber;
}

std::string PlanetSystem::GetName() const
{
	return "PlanetSystem";
}

std::string PlanetSystem::GetVersion() const
{
	return PlanetSystem_BuildId;
}

std::string PlanetSystem::GetDescription() const
{
	return "PlanetSystem plugin";
}

std::vector<std::string> PlanetSystem::GetNodes() const
{
	return
	{
		/*
		"MESH_SIM_RENDERER",
		"COMPUTE_MESH_SIM"
		*/
	};
}

std::vector<LibraryEntry> PlanetSystem::GetLibrary() const
{
	std::vector<LibraryEntry> res;

	res.push_back(AddLibraryEntry("PlanetSystem", "Planet Renderer", "PLANET_RENDERER"));

	return res;
}

BaseNodePtr PlanetSystem::CreatePluginNode(const std::string& vPluginNodeName)
{
	auto vkCorePtr = m_VulkanCoreWeak.lock();

	if (vPluginNodeName == "PLANET_RENDERER")	return PlanetNode::Create(vkCorePtr);

	return nullptr;
}

std::vector<PluginPane> PlanetSystem::GetPanes() const
{
	std::vector<PluginPane> res;

	return res;
}

int PlanetSystem::ResetImGuiID(const int& vWidgetId)
{
	auto ids = ImGui::CustomStyle::Instance()->pushId;
	ImGui::CustomStyle::Instance()->pushId = vWidgetId;
	return ids;
}
