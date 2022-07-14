#pragma once

#include <Interfaces/PluginInterface.h>

#include <vkFramework/VulkanRessource.h>
#include <vkFramework/vk_mem_alloc.h>


class SdfMesher : public PluginInterface
{
private:
	vkApi::VulkanCoreWeak m_VulkanCoreWeak;

public:
	SdfMesher();
	virtual ~SdfMesher() override;
	bool Init(
		vkApi::VulkanCoreWeak vVkCore,
		FileHelper* vFileHelper,
		CommonSystem* vCommonSystem,
		ImGuiContext* vContext,
		ImGui::CustomStyle* vCustomStyle) override;
	void Unit() override;
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