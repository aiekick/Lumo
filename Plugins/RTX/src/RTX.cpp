// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "SdfMesher.h"
#include <Headers/Build.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Graph/Base/BaseNode.h>
#include <Systems/CommonSystem.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanWindow.h>

// needed for plugin creating / destroying
extern "C" // needed for avoid renaming of funcs by the compiler
{
#ifdef WIN32
#define PLUGIN_PREFIX __declspec (dllexport)
#else
#define PLUGIN_PREFIX
#endif

	PLUGIN_PREFIX SdfMesher* allocator()
	{
		return new SdfMesher();
	}

	PLUGIN_PREFIX void deleter(SdfMesher* ptr)
	{
		delete ptr;
	}
}

SdfMesher::SdfMesher()
{
#ifdef _MSC_VER
	// active memory leak detector
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

SdfMesher::~SdfMesher()
{
	Unit();
}

bool SdfMesher::Init(
	vkApi::VulkanCoreWeak vVulkanCoreWeak,
	FileHelper* vFileHelper,
	CommonSystem* vCommonSystem,
	ImGuiContext* vContext,
	ImGui::CustomStyle* vCustomStyle)
{
	// on transfere les singleton dans l'espace memoire static de la dll
	// ca evitera dans recreer des vides et d'avoir des erreurs partout
	// car les statics contenus dans ces classes sont null quand ils arrivent ici

	m_VulkanCoreWeak = vVulkanCoreWeak;

	auto corePtr = m_VulkanCoreWeak.getValidShared();
	if (corePtr)
	{
		if (vkApi::VulkanCore::sAllocator == nullptr)
		{
			corePtr->setupMemoryAllocator();
		}
		else
		{
			CTOOL_DEBUG_BREAK;
			LogVarInfo("le static VulkanCore::sAllocator n'est pas null..");
			// a tien ? ca a changé ?
		}
		vkApi::VulkanCore::sVulkanShader = VulkanShader::Create();

		FileHelper::Instance(vFileHelper);
		CommonSystem::Instance(vCommonSystem);
		ImGui::SetCurrentContext(vContext);
		ImGui::CustomStyle::Instance(vCustomStyle);

		return true;
	}

	return false;
}

void SdfMesher::Unit()
{
	auto corePtr = m_VulkanCoreWeak.getValidShared();
	if (corePtr)
	{
		corePtr->getDevice().waitIdle();

		ImGui::CustomStyle::Instance(nullptr, true);
		CommonSystem::Instance(nullptr, true);
		FileHelper::Instance(nullptr, true);

		ImGui::SetCurrentContext(nullptr);

		vkApi::VulkanCore::sVulkanShader.reset();

		corePtr = nullptr;
		vmaDestroyAllocator(vkApi::VulkanCore::sAllocator);
	}
}

uint32_t SdfMesher::GetVersionMajor() const
{
	return SdfMesher_MinorNumber;
}

uint32_t SdfMesher::GetVersionMinor() const
{
	return SdfMesher_MajorNumber;
}

uint32_t SdfMesher::GetVersionBuild() const
{
	return SdfMesher_BuildNumber;
}

std::string SdfMesher::GetName() const
{
	return "SdfMesher";
}

std::string SdfMesher::GetVersion() const
{
	return SdfMesher_BuildId;
}

std::string SdfMesher::GetDescription() const
{
	return "Sdf To Mesh Conversion plugin";
}

std::vector<std::string> SdfMesher::GetNodes() const
{
	return
	{
		/*
		"MESH_SIM_RENDERER",
		"COMPUTE_MESH_SIM"
		*/
	};
}

std::vector<LibraryEntry> SdfMesher::GetLibrary() const
{
	std::vector<LibraryEntry> res;

	/*
	LibraryEntry entry_MESH_SIM_RENDERER;
	entry_MESH_SIM_RENDERER.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_PLUGIN;
	entry_MESH_SIM_RENDERER.first = "plugins";
	entry_MESH_SIM_RENDERER.second.nodeLabel = "Mesh Simulation";
	entry_MESH_SIM_RENDERER.second.nodeType = "MESH_SIM_RENDERER";
	entry_MESH_SIM_RENDERER.second.color = ct::fvec4(0.0f);
	entry_MESH_SIM_RENDERER.second.categoryPath = "SdfMesher/Renderers";
	res.push_back(entry_MESH_SIM_RENDERER);

	LibraryEntry entry_COMPUTE_MESH_SIM;
	entry_COMPUTE_MESH_SIM.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_PLUGIN;
	entry_COMPUTE_MESH_SIM.first = "plugins";
	entry_COMPUTE_MESH_SIM.second.nodeLabel = "Mesh Simulation";
	entry_COMPUTE_MESH_SIM.second.nodeType = "COMPUTE_MESH_SIM";
	entry_COMPUTE_MESH_SIM.second.color = ct::fvec4(0.0f);
	entry_COMPUTE_MESH_SIM.second.categoryPath = "SdfMesher/Generators";
	res.push_back(entry_COMPUTE_MESH_SIM);
	*/

	return res;
}

BaseNodePtr SdfMesher::CreatePluginNode(const std::string& vPluginNodeName)
{
	BaseNodePtr res = nullptr;

	/*
	if (vPluginNodeName == "MESH_SIM_RENDERER")
		res = SdfMesherRendererNode::Create(m_VulkanCorePtr);
	else if (vPluginNodeName == "COMPUTE_MESH_SIM")
		res = ComputeSdfMesherNode::Create(m_VulkanCorePtr);
	*/

	return res;
}

int SdfMesher::ResetImGuiID(const int& vWidgetId)
{
	auto ids = ImGui::CustomStyle::Instance()->pushId;
	ImGui::CustomStyle::Instance()->pushId = vWidgetId;
	return ids;
}
