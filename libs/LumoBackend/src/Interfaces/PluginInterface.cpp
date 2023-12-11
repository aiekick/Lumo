// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <LumoBackend/Interfaces/PluginInterface.h>

#include <LumoBackend/Graph/Base/BaseNode.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <ctools/FileHelper.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <ImWidgets.h>
#include <Gaia/Gui/VulkanWindow.h>
#include <LumoBackend/Graph/Base/NodeSlot.h>

PluginInterface::~PluginInterface() {
    Unit();
}

bool PluginInterface::Init(GaiApi::VulkanCoreWeak vVulkanCoreWeak) {
    m_VulkanCoreWeak = vVulkanCoreWeak;
    auto corePtr = m_VulkanCoreWeak.lock();
    if (corePtr && AuthorizeLoading()) {
        ActionBeforeInit();

        // init

        ActionAfterInit();

        return true;
    }

    return false;
}

void PluginInterface::Unit() {
    auto corePtr = m_VulkanCoreWeak.lock();
    if (corePtr) {
        corePtr->getDevice().waitIdle();
    }
}

bool PluginInterface::AuthorizeLoading() {
    return true;
}

void PluginInterface::ActionBeforeInit() {
}

void PluginInterface::ActionAfterInit() {
}

LibraryEntry PluginInterface::AddLibraryEntry(
    const std::string& vCategoryPath, const std::string& vNodeLabel, const std::string& vNodeType, const ct::fvec4& vColor) const {
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