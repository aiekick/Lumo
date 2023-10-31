// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "Simulation.h"
#include <Headers/SimulationBuild.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/VulkanWindow.h>
#include <LumoBackend/Graph/Base/NodeSlot.h>

#include <Nodes/DiffOperators/LaplacianNode.h>
#include <Nodes/DiffOperators/DivergenceNode.h>
#include <Nodes/DiffOperators/GradientNode.h>
#include <Nodes/DiffOperators/CurlNode.h>

#include <Nodes/Simulation/GrayScottNode.h>
#include <Nodes/Simulation/ConwayNode.h>

// needed for plugin creating / destroying
extern "C" // needed for avoid renaming of funcs by the compiler
{
#ifdef WIN32
#define PLUGIN_PREFIX __declspec (dllexport)
#else
#define PLUGIN_PREFIX
#endif

	PLUGIN_PREFIX Simulation* allocator()
	{
		return new Simulation();
	}

	PLUGIN_PREFIX void deleter(Simulation* ptr)
	{
		delete ptr;
	}
}

Simulation::Simulation()
{
#ifdef _MSC_VER
	// active memory leak detector
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void Simulation::ActionAfterInit()
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

uint32_t Simulation::GetVersionMajor() const
{
	return Simulation_MinorNumber;
}

uint32_t Simulation::GetVersionMinor() const
{
	return Simulation_MajorNumber;
}

uint32_t Simulation::GetVersionBuild() const
{
	return Simulation_BuildNumber;
}

std::string Simulation::GetName() const
{
	return "Simulation";
}

std::string Simulation::GetVersion() const
{
	return Simulation_BuildId;
}

std::string Simulation::GetDescription() const
{
	return "Simulation Nodes";
}

std::vector<std::string> Simulation::GetNodes() const
{
	return
	{
		""
	};
}

std::vector<LibraryEntry> Simulation::GetLibrary() const
{
	std::vector<LibraryEntry> res;

	res.push_back(AddLibraryEntry("2D/Differential Ops", "Laplacian", "LAPLACIAN"));
	res.push_back(AddLibraryEntry("2D/Differential Ops", "Divergence", "DIVERGENCE"));
	res.push_back(AddLibraryEntry("2D/Differential Ops", "Gradient", "GRADIENT"));
	res.push_back(AddLibraryEntry("2D/Differential Ops", "Curl (Rotational)", "CURL"));

	res.push_back(AddLibraryEntry("2D/Simulation", "GrayScott", "2D_SIMULATION_GRAY_SCOTT"));
	res.push_back(AddLibraryEntry("2D/Simulation", "Conway", "2D_SIMULATION_CONWAY"));

	return res;
}

BaseNodePtr Simulation::CreatePluginNode(const std::string& vPluginNodeName)
{
	auto vkSimulationPtr = m_VulkanCoreWeak.lock();

	// Differential Operators
	if (vPluginNodeName == "LAPLACIAN")					return LaplacianNode::Create(vkSimulationPtr);
	else if (vPluginNodeName == "DIVERGENCE")					return DivergenceNode::Create(vkSimulationPtr);
	else if (vPluginNodeName == "GRADIENT")						return GradientNode::Create(vkSimulationPtr);
	else if (vPluginNodeName == "CURL")							return CurlNode::Create(vkSimulationPtr);

	// Simulations
	else if (vPluginNodeName == "2D_SIMULATION_GRAY_SCOTT")		return GrayScottNode::Create(vkSimulationPtr);
	else if (vPluginNodeName == "2D_SIMULATION_CONWAY")			return ConwayNode::Create(vkSimulationPtr);

	return nullptr;
}

std::vector<PluginPaneConfig> Simulation::GetPanes() const
{
	return {};
}

int Simulation::ResetImGuiID(const int& vWidgetId)
{
    auto ids = ImGui::GetPUSHID();
    ImGui::SetPUSHID(vWidgetId);
	return ids;
}
