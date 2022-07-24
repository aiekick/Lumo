#pragma once

#include <Interfaces/PluginInterface.h>

#include <vkFramework/VulkanRessource.h>
#include <vkFramework/vk_mem_alloc.h>


class Particles : public PluginInterface
{
public:
	Particles();
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
	int ResetImGuiID(const int& vWidgetId) override;
};