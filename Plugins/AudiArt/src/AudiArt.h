#pragma once

#include <LumoBackend/Interfaces/PluginInterface.h>

#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Core/vk_mem_alloc.h>

class AudiArt : public PluginInterface
{
public:
	AudiArt();
	void ActionAfterInit() override;
	uint32_t GetVersionMajor() const override;
	uint32_t GetVersionMinor() const override;
	uint32_t GetVersionBuild() const override;
	std::string GetName() const override;
	std::string GetVersion() const override;
	std::string GetDescription() const override;
	std::vector<std::string> GetNodes() const override;
	std::vector<LibraryEntry> GetLibrary() const override;
	BaseNodePtr CreatePluginNode(const std::string& vPluginNodeName) override;
	std::vector<PluginPane> GetPanes() const override;
	int ResetImGuiID(const int& vWidgetId) override;
};
