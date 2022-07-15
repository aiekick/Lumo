// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "Particles.h"
#include <Headers/ParticlesBuild.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Graph/Base/BaseNode.h>
#include <Systems/CommonSystem.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanWindow.h>

#ifndef USE_STATIC_LINKING_OF_PLUGINS
// needed for plugin creating / destroying
extern "C" // needed for avoid renaming of funcs by the compiler
{
#ifdef WIN32
#define PLUGIN_PREFIX __declspec (dllexport)
#else
#define PLUGIN_PREFIX
#endif

	PLUGIN_PREFIX Particles* allocator()
	{
		return new Particles();
	}

	PLUGIN_PREFIX void deleter(Particles* ptr)
	{
		delete ptr;
	}
}
#endif // USE_STATIC_LINKING_OF_PLUGINS

Particles::Particles()
{
#ifdef _MSC_VER
	// active memory leak detector
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

uint32_t Particles::GetVersionMajor() const
{
	return Particles_MinorNumber;
}

uint32_t Particles::GetVersionMinor() const
{
	return Particles_MajorNumber;
}

uint32_t Particles::GetVersionBuild() const
{
	return Particles_BuildNumber;
}

std::string Particles::GetName() const
{
	return "Particles";
}

std::string Particles::GetVersion() const
{
	return Particles_BuildId;
}

std::string Particles::GetDescription() const
{
	return "Particles system plugin";
}

std::vector<std::string> Particles::GetNodes() const
{
	return
	{
		/*
		"MESH_SIM_RENDERER",
		"COMPUTE_MESH_SIM"
		*/
	};
}

std::vector<LibraryEntry> Particles::GetLibrary() const
{
	std::vector<LibraryEntry> res;

	/*
	LibraryEntry entry_MESH_SIM_RENDERER;
	entry_MESH_SIM_RENDERER.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_PLUGIN;
	entry_MESH_SIM_RENDERER.first = "plugins";
	entry_MESH_SIM_RENDERER.second.nodeLabel = "Mesh Simulation";
	entry_MESH_SIM_RENDERER.second.nodeType = "MESH_SIM_RENDERER";
	entry_MESH_SIM_RENDERER.second.color = ct::fvec4(0.0f);
	entry_MESH_SIM_RENDERER.second.categoryPath = "Particles/Renderers";
	res.push_back(entry_MESH_SIM_RENDERER);

	LibraryEntry entry_COMPUTE_MESH_SIM;
	entry_COMPUTE_MESH_SIM.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_PLUGIN;
	entry_COMPUTE_MESH_SIM.first = "plugins";
	entry_COMPUTE_MESH_SIM.second.nodeLabel = "Mesh Simulation";
	entry_COMPUTE_MESH_SIM.second.nodeType = "COMPUTE_MESH_SIM";
	entry_COMPUTE_MESH_SIM.second.color = ct::fvec4(0.0f);
	entry_COMPUTE_MESH_SIM.second.categoryPath = "Particles/Generators";
	res.push_back(entry_COMPUTE_MESH_SIM);
	*/

	return res;
}

BaseNodePtr Particles::CreatePluginNode(const std::string& vPluginNodeName)
{
	BaseNodePtr res = nullptr;

	/*
	if (vPluginNodeName == "MESH_SIM_RENDERER")
		res = ParticlesRendererNode::Create(m_VulkanCorePtr);
	else if (vPluginNodeName == "COMPUTE_MESH_SIM")
		res = ComputeParticlesNode::Create(m_VulkanCorePtr);
	*/

	return res;
}

int Particles::ResetImGuiID(const int& vWidgetId)
{
	auto ids = ImGui::CustomStyle::Instance()->pushId;
	ImGui::CustomStyle::Instance()->pushId = vWidgetId;
	return ids;
}