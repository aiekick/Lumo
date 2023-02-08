// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

//#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "MeshGen.h"
#include <Headers/MeshGenBuild.h>
#include <Graph/Base/BaseNode.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <ctools/FileHelper.h>
#include <Systems/CommonSystem.h>
#include <ImWidgets/ImWidgets.h>
#include <vkFramework/VulkanWindow.h>

#include <Nodes/Curve/ParametricCurveNode.h>
#include <Nodes/Curve/ParametricCurveDiffNode.h>
#include <Nodes/Mesh/PrimitiveNode.h>
#include <Nodes/Surface/ParametricSurfaceUVNode.h>

// needed for plugin creating / destroying
extern "C" // needed for avoid renaming of funcs by the compiler
{
	#ifdef WIN32
	#define PLUGIN_PREFIX __declspec (dllexport)
	#else
	#define PLUGIN_PREFIX
	#endif

	PLUGIN_PREFIX MeshGen* allocator()
	{
		return new MeshGen();
	}

	PLUGIN_PREFIX void deleter(MeshGen* ptr)
	{
		delete ptr;
	}
}

MeshGen::MeshGen()
{
#ifdef _MSC_VER
	// active memory leak detector
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void MeshGen::ActionAfterInit()
{
	NodeSlot::sGetSlotColors()->AddSlotColor("MESH", ImVec4(0.5f, 0.5f, 0.9f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("CURVE", ImVec4(0.4f, 0.5f, 0.9f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("SURFACE", ImVec4(0.3f, 0.5f, 0.9f, 1.0f));
}

uint32_t MeshGen::GetVersionMajor() const
{
	return MeshGen_MinorNumber;
}

uint32_t MeshGen::GetVersionMinor() const
{
	return MeshGen_MajorNumber;
}

uint32_t MeshGen::GetVersionBuild() const
{
	return MeshGen_BuildNumber;
}

std::string MeshGen::GetName() const
{
	return "MeshGen";
}

std::string MeshGen::GetVersion() const
{
	return MeshGen_BuildId;
}

std::string MeshGen::GetDescription() const
{
	return "Mesh Generation plugin";
}

std::vector<std::string> MeshGen::GetNodes() const
{
	return 
	{
		"PARAMETRIC_CURVE",
		"PARAMETRIC_CURVE_DIFF",
		"PARAMETRIC_SURFACE_UV",
		"PRIMITIVE"
	};
}

std::vector<LibraryEntry> MeshGen::GetLibrary() const
{
	std::vector<LibraryEntry> res;

	res.push_back(AddLibraryEntry("MeshGen/Parametric/Curve", "Parametric Curve", "PARAMETRIC_CURVE"));
	res.push_back(AddLibraryEntry("MeshGen/Parametric/Curve", "Parametric Curve Differential", "PARAMETRIC_CURVE_DIFF"));
	res.push_back(AddLibraryEntry("MeshGen/Parametric/Surface", "Parametric Surface UV", "PARAMETRIC_SURFACE_UV"));
	res.push_back(AddLibraryEntry("MeshGen/Mesh/Primitive", "Primitive", "PRIMITIVE"));
	
	return res;
}

BaseNodePtr MeshGen::CreatePluginNode(const std::string& vPluginNodeName)
{
	BaseNodePtr res = nullptr;

	if (vPluginNodeName == "PARAMETRIC_CURVE")
		res = ParametricCurveNode::Create(m_VulkanCoreWeak.getValidShared());
	else if (vPluginNodeName == "PARAMETRIC_CURVE_DIFF")
		res = ParametricCurveDiffNode::Create(m_VulkanCoreWeak.getValidShared());
	else if (vPluginNodeName == "PARAMETRIC_SURFACE_UV")
		res = ParametricSurfaceUVNode::Create(m_VulkanCoreWeak.getValidShared());
	else if (vPluginNodeName == "PRIMITIVE")
		res = PrimitiveNode::Create(m_VulkanCoreWeak.getValidShared());

	return res;
}

std::vector<PluginPane> MeshGen::GetPanes() const
{
	std::vector<PluginPane> res;

	return res;
}

int MeshGen::ResetImGuiID(const int& vWidgetId)
{
	auto ids = ImGui::CustomStyle::Instance()->pushId;
	ImGui::CustomStyle::Instance()->pushId = vWidgetId;
	return ids;
}