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

#include <Nodes/Renderers/ParticlesPointRendererNode.h>
#include <Nodes/Renderers/ParticlesSpriteRendererNode.h>
#include <Nodes/Simulation/ParticlesSimulationNode.h>
#include <Nodes/Primitives/PrimitiveFibonacciNode.h>

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

void Particles::ActionAfterInit()
{
	NodeSlot::GetSlotColors()->AddSlotColor("PARTICLES", ImVec4(0.2f, 0.5f, 0.8f, 1.0f));
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
		"PARTICLES_SIMULATION",
		"PARTICLES_RENDERER"
	};
}

std::vector<LibraryEntry> Particles::GetLibrary() const
{
	std::vector<LibraryEntry> res;

	{
		LibraryEntry entry;
		entry.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_PLUGIN;
		entry.first = "plugins";
		entry.second.nodeLabel = "Simulation";
		entry.second.nodeType = "PARTICLES_SIMULATION";
		entry.second.color = ct::fvec4(0.0f);
		entry.second.categoryPath = "Particles";
		res.push_back(entry);
	}

	{
		LibraryEntry entry;
		entry.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_PLUGIN;
		entry.first = "plugins";
		entry.second.nodeLabel = "Point Renderer";
		entry.second.nodeType = "PARTICLES_POINT_RENDERER";
		entry.second.color = ct::fvec4(0.0f);
		entry.second.categoryPath = "Particles";
		res.push_back(entry);
	}

	{
		LibraryEntry entry;
		entry.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_PLUGIN;
		entry.first = "plugins";
		entry.second.nodeLabel = "Sprite Renderer";
		entry.second.nodeType = "PARTICLES_SPRITE_RENDERER";
		entry.second.color = ct::fvec4(0.0f);
		entry.second.categoryPath = "Particles";
		res.push_back(entry);
	}

	{
		LibraryEntry entry;
		entry.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_PLUGIN;
		entry.first = "plugins";
		entry.second.nodeLabel = "Fibonacci Ball";
		entry.second.nodeType = "PRIMITIVE_FIBONACCI";
		entry.second.color = ct::fvec4(0.0f);
		entry.second.categoryPath = "Particles";
		res.push_back(entry);
	}

	return res;
}

BaseNodePtr Particles::CreatePluginNode(const std::string& vPluginNodeName)
{
	BaseNodePtr res = nullptr;

	if (vPluginNodeName == "PARTICLES_SIMULATION")
		res = ParticlesSimulationNode::Create(m_VulkanCoreWeak.getValidShared());
	else if (vPluginNodeName == "PARTICLES_POINT_RENDERER")
		res = ParticlesPointRendererNode::Create(m_VulkanCoreWeak.getValidShared());
	else if (vPluginNodeName == "PARTICLES_SPRITE_RENDERER")
		res = ParticlesSpriteRendererNode::Create(m_VulkanCoreWeak.getValidShared());
	else if (vPluginNodeName == "PRIMITIVE_FIBONACCI")
		res = PrimitiveFibonacciNode::Create(m_VulkanCoreWeak.getValidShared());

	return res;
}

int Particles::ResetImGuiID(const int& vWidgetId)
{
	auto ids = ImGui::CustomStyle::Instance()->pushId;
	ImGui::CustomStyle::Instance()->pushId = vWidgetId;
	return ids;
}
