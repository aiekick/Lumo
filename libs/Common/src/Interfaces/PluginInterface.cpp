// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "PluginInterface.h"

#include <Graph/Base/BaseNode.h>
#include <gaia/VulkanCore.h>
#include <gaia/VulkanShader.h>
#include <ctools/FileHelper.h>
#include <Systems/CommonSystem.h>
#include <ImWidgets/ImWidgets.h>
#include <gaia/VulkanWindow.h>
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
	ImGuiFileDialog* ImGuiFileDialog,
	SlotColor *vSlotColor,
	ImGui::CustomStyle* vCustomStyle)
{
	// on transfere les singleton dans l'espace memoire static de la dll
	// ca evitera dans recreer des vides et d'avoir des erreurs partout
	// car les statics contenus dans ces classes sont null quand ils arrivent ici
	
	m_VulkanCoreWeak = vVulkanCoreWeak;
	auto corePtr = m_VulkanCoreWeak.getValidShared();
	if (corePtr && AuthorizeLoading())
	{
		ActionBeforeInit();

		if (vkApi::VulkanCore::sAllocator == nullptr)
		{
			assert(vFileHelper);
			assert(vCommonSystem);
			assert(vContext); ImGui::SetCurrentContext(vContext);
			assert(ImGuiFileDialog);
			assert(vSlotColor);
			assert(vCustomStyle);

			vk::DynamicLoader dl;
			PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
			VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
			VULKAN_HPP_DEFAULT_DISPATCHER.init(corePtr->getInstance(), vkGetInstanceProcAddr);
			VULKAN_HPP_DEFAULT_DISPATCHER.init(corePtr->getDevice());

			iSinAPlugin = true;
			corePtr->setupMemoryAllocator();
			FileHelper::Instance(vFileHelper);
			CommonSystem::Instance(vCommonSystem);
			ImGui::SetCurrentContext(vContext);
			ImGuiFileDialog::Instance(ImGuiFileDialog);
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

bool PluginInterface::AuthorizeLoading()
{
	return true;
}

void PluginInterface::ActionBeforeInit()
{

}

void PluginInterface::ActionAfterInit()
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