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
//#include <Nodes/Renderers/ParticlesBillBoardRendererNode.h>
//#include <Nodes/Simulation/ParticlesSimulationNode.h>
//#include <Nodes/Primitives/PrimitiveFibonacciNode.h>
#include <Nodes/Emitters/MeshEmitterNode.h>

#ifndef USE_PLUGIN_STATIC_LINKING
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
#endif // USE_PLUGIN_STATIC_LINKING

Particles::Particles()
{
#ifdef _MSC_VER
	// active memory leak detector
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void Particles::ActionAfterInit()
{
	NodeSlot::sGetSlotColors()->AddSlotColor("PARTICLES", ImVec4(0.2f, 0.5f, 0.8f, 1.0f));
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
		"PARTICLES_MESH_EMITTER",
		//"PARTICLES_SIMULATION",
		"PARTICLES_POINT_RENDERER",
		//"PARTICLES_BILLBOARDS_RENDERER",
		//"PARTICLES_PRIMITIVE_FIBONACCI"
	};
}

std::vector<LibraryEntry> Particles::GetLibrary() const
{
	std::vector<LibraryEntry> res;

	res.push_back(AddLibraryEntry("Particles/3D/Emitters",	"Mesh Emitter",		"PARTICLES_MESH_EMITTER"));
	res.push_back(AddLibraryEntry("Particles/3D/Renderers",	"Point Renderer",	"PARTICLES_POINT_RENDERER"));
	//res.push_back(AddLibraryEntry("Particles/3D/Simulation",	"Simulation",			"PARTICLES_SIMULATION"));
	//res.push_back(AddLibraryEntry("Particles/3D/Renderers",	"Billboards Renderer",	"PARTICLES_BILLBOARDS_RENDERER"));
	//res.push_back(AddLibraryEntry("Particles/3D/Primitives",	"Fibonacci Ball",		"PARTICLES_PRIMITIVE_FIBONACCI"));
	
	return res;
}

BaseNodePtr Particles::CreatePluginNode(const std::string& vPluginNodeName)
{
	if (vPluginNodeName == "PARTICLES_MESH_EMITTER")
		return MeshEmitterNode::Create(m_VulkanCoreWeak.getValidShared());
	else if (vPluginNodeName == "PARTICLES_POINT_RENDERER")
		return ParticlesPointRendererNode::Create(m_VulkanCoreWeak.getValidShared());
	//if (vPluginNodeName == "PARTICLES_SIMULATION")
	//	return ParticlesSimulationNode::Create(m_VulkanCoreWeak.getValidShared());
	//else if (vPluginNodeName == "PARTICLES_BILLBOARDS_RENDERER")
	//	return ParticlesBillBoardRendererNode::Create(m_VulkanCoreWeak.getValidShared());
	//else if (vPluginNodeName == "PARTICLES_PRIMITIVE_FIBONACCI")
	//	return PrimitiveFibonacciNode::Create(m_VulkanCoreWeak.getValidShared());
	
	return nullptr;
}

int Particles::ResetImGuiID(const int& vWidgetId)
{
	auto ids = ImGui::CustomStyle::Instance()->pushId;
	ImGui::CustomStyle::Instance()->pushId = vWidgetId;
	return ids;
}
