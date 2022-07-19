// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "RTX.h"
#include <Headers/RTXBuild.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Graph/Base/BaseNode.h>
#include <Systems/CommonSystem.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanWindow.h>
#include <Graph/Base/NodeSlot.h>

#include <Nodes/RtxPbrRendererNode.h>
#include <Nodes/ModelToAccelStructNode.h>

#ifndef USE_STATIC_LINKING_OF_PLUGINS
// needed for plugin creating / destroying
extern "C" // needed for avoid renaming of funcs by the compiler
{
#ifdef WIN32
#define PLUGIN_PREFIX __declspec (dllexport)
#else
#define PLUGIN_PREFIX
#endif

	PLUGIN_PREFIX RTX* allocator()
	{
		return new RTX();
	}

	PLUGIN_PREFIX void deleter(RTX* ptr)
	{
		delete ptr;
	}
}
#endif // USE_STATIC_LINKING_OF_PLUGINS

RTX::RTX()
{
#ifdef _MSC_VER
	// active memory leak detector
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void RTX::ActionAfterInit()
{
	NodeSlot::GetSlotColors()->AddSlotColor("RTX_ACCEL_STRUCTURE", ImVec4(0.8f, 0.5f, 0.8f, 1.0f));
}

uint32_t RTX::GetVersionMajor() const
{
	return RTX_MinorNumber;
}

uint32_t RTX::GetVersionMinor() const
{
	return RTX_MajorNumber;
}

uint32_t RTX::GetVersionBuild() const
{
	return RTX_BuildNumber;
}

std::string RTX::GetName() const
{
	return "RTX";
}

std::string RTX::GetVersion() const
{
	return RTX_BuildId;
}

std::string RTX::GetDescription() const
{
	return "Ray Tracing (RTX) plugin";
}

std::vector<std::string> RTX::GetNodes() const
{
	return
	{
		"RTX_PBR_RENDERER",
		"RTX_MODEL_TO_ACCELERATION_STRUCTURE"
	};
}

std::vector<LibraryEntry> RTX::GetLibrary() const
{
	std::vector<LibraryEntry> res;

	LibraryEntry entry_RTX_PBR_RENDERER;
	entry_RTX_PBR_RENDERER.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_PLUGIN;
	entry_RTX_PBR_RENDERER.first = "plugins";
	entry_RTX_PBR_RENDERER.second.nodeLabel = "PBR";
	entry_RTX_PBR_RENDERER.second.nodeType = "RTX_PBR_RENDERER";
	entry_RTX_PBR_RENDERER.second.color = ct::fvec4(0.0f);
	entry_RTX_PBR_RENDERER.second.categoryPath = "RTX";
	res.push_back(entry_RTX_PBR_RENDERER);


	LibraryEntry entry_RTX_MODEL_TO_ACCELERATION_STRUCTURE;
	entry_RTX_MODEL_TO_ACCELERATION_STRUCTURE.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_PLUGIN;
	entry_RTX_MODEL_TO_ACCELERATION_STRUCTURE.first = "plugins";
	entry_RTX_MODEL_TO_ACCELERATION_STRUCTURE.second.nodeLabel = "AccelStruct Builder" ;
	entry_RTX_MODEL_TO_ACCELERATION_STRUCTURE.second.nodeType = "RTX_MODEL_TO_ACCELERATION_STRUCTURE";
	entry_RTX_MODEL_TO_ACCELERATION_STRUCTURE.second.color = ct::fvec4(0.0f);
	entry_RTX_MODEL_TO_ACCELERATION_STRUCTURE.second.categoryPath = "RTX";
	res.push_back(entry_RTX_MODEL_TO_ACCELERATION_STRUCTURE);

	return res;
}

BaseNodePtr RTX::CreatePluginNode(const std::string& vPluginNodeName)
{
	BaseNodePtr res = nullptr;

	auto vkCorePtr = m_VulkanCoreWeak.getValidShared();

	if (vPluginNodeName == "RTX_PBR_RENDERER")
		res = RtxPbrRendererNode::Create(vkCorePtr);
	else if (vPluginNodeName == "RTX_MODEL_TO_ACCELERATION_STRUCTURE")
		res = ModelToAccelStructNode::Create(vkCorePtr);

	return res;
}

int RTX::ResetImGuiID(const int& vWidgetId)
{
	auto ids = ImGui::CustomStyle::Instance()->pushId;
	ImGui::CustomStyle::Instance()->pushId = vWidgetId;
	return ids;
}
