// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "PluginInterface.h"

#include <Graph/Base/BaseNode.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <ctools/FileHelper.h>
#include <Systems/CommonSystem.h>
#include <ImWidgets/ImWidgets.h>
#include <vkFramework/VulkanWindow.h>
#include <Graph/Base/NodeSlot.h>

PluginInterface::~PluginInterface()
{
	Unit();
}

bool PluginInterface::Init(
	vkApi::VulkanCoreWeak vVulkanCoreWeak,
	FileHelper* vFileHelper, 
	CommonSystem* vCommonSystem,
	ImGuiContext* vContext,
	SlotColor *vSlotColor,
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
			assert(vFileHelper);
			assert(vCommonSystem);
			assert(vContext);
			assert(vSlotColor);
			assert(vCustomStyle);

			iSinAPlugin = true;
			corePtr->setupMemoryAllocator();
			FileHelper::Instance(vFileHelper);
			CommonSystem::Instance(vCommonSystem);
			ImGui::SetCurrentContext(vContext);
			NodeSlot::sGetSlotColors(vSlotColor);
			ImGui::CustomStyle::Instance(vCustomStyle);
			vkApi::VulkanCore::sVulkanShader = VulkanShader::Create();
		}
		else
		{
#ifndef USE_PLUGIN_STATIC_LINKING	
			CTOOL_DEBUG_BREAK;
			LogVarInfo("le static VulkanCore::sAllocator n'est pas null..");
			// a tien ? ca a changé ?
#endif // USE_PLUGIN_STATIC_LINKING
		}

		ActionAfterInit();

		return true;
	}

	return false;
}

void PluginInterface::Unit()
{
	auto corePtr = m_VulkanCoreWeak.getValidShared();
	if (corePtr)
	{
		corePtr->getDevice().waitIdle();

#ifndef USE_PLUGIN_STATIC_LINKING	
		// for avoid issue when its a normal class 
		// like PlugiManager who inherit from PluginInterface
		if (iSinAPlugin)
		{
			ImGui::CustomStyle::Instance(nullptr, true);
			NodeSlot::sGetSlotColors(nullptr, true);
			CommonSystem::Instance(nullptr, true);
			FileHelper::Instance(nullptr, true);

			ImGui::SetCurrentContext(nullptr);

			vkApi::VulkanCore::sVulkanShader.reset();

			corePtr = nullptr;
			vmaDestroyAllocator(vkApi::VulkanCore::sAllocator);
		}
#endif
	}
}

void  PluginInterface::ActionAfterInit()
{

}

LibraryEntry PluginInterface::AddLibraryEntry(
	const std::string& vCategoryPath,
	const std::string& vNodeLabel,
	const std::string& vNodeType,
	const ct::fvec4& vColor) const
{
	LibraryEntry entry;

	assert(!vCategoryPath.empty());
	assert(!vNodeLabel.empty());
	assert(!vNodeType.empty());

	entry.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_PLUGIN;
	entry.first = "plugins";
	entry.second.nodeLabel = vNodeLabel;
	entry.second.nodeType = vNodeType;
	entry.second.color = vColor;
	entry.second.categoryPath = vCategoryPath;

	return entry;
}