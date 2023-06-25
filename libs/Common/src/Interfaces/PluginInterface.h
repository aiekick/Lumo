/*
Copyright 2022-2022 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <Graph/Graph.h>
#include <ctools/cTools.h>
#include <Graph/Library/LibraryCategory.h>
#include <vkFramework/vkFramework.h>
#include <Panes/Abstract/AbstractPane.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <Common/Globals.h>

class PluginInterface;
typedef ct::cWeak<PluginInterface> PluginInterfaceWeak;
typedef std::shared_ptr<PluginInterface> PluginInterfacePtr;

class COMMON_API PluginPane
{
public:
	AbstractPaneWeak paneWeak;
	std::string paneName;
	std::string paneCategory;
	PaneDisposal paneDisposal = PaneDisposal::CENTRAL;
	bool isPaneOpenedDefault = false;
	bool isPaneFocusedDefault = false;

public:
	PluginPane(
		AbstractPaneWeak vPaneWeak,
		std::string vName, 
		std::string vPaneCategory,
		PaneDisposal vPaneDisposal,
		bool vIsOpenedDefault,
		bool vIsFocusedDefault)
		: 
		paneWeak(vPaneWeak), 
		paneName(vName),
		paneCategory(vPaneCategory),
		paneDisposal(vPaneDisposal),
		isPaneOpenedDefault(vIsOpenedDefault),
		isPaneFocusedDefault(vIsFocusedDefault)
	{}
};

class SlotColor;
class FileHelper;
class VulkanShader;
class CommonSystem;
struct ImGuiContext;
namespace ImGui { class CustomStyle; }
class COMMON_API PluginInterface
{
protected:
	vkApi::VulkanCoreWeak m_VulkanCoreWeak;
	bool iSinAPlugin = false;

protected:
	LibraryEntry AddLibraryEntry(
		const std::string& vCategoryPath,
		const std::string& vNodeLabel,
		const std::string& vNodeType,
		const ct::fvec4& vColor = 0.0f) const;

public:
	PluginInterface() = default;
	virtual ~PluginInterface();

	virtual bool Init(
		vkApi::VulkanCoreWeak vVulkanCoreWeak,
		FileHelper* vFileHelper, 
		CommonSystem* vCommonSystem,
		ImGuiContext* vContext,
		ImGuiFileDialog* vImGuiFileDialog,
		SlotColor* vSlotColor,
		ImGui::CustomStyle* vCustomStyle);
	virtual void Unit();

	// if false, will prevent plugin loading, ex if a feature is not available
	virtual bool AuthorizeLoading();

	virtual void ActionBeforeInit();
	virtual void ActionAfterInit();

	virtual uint32_t GetVersionMajor() const = 0;
	virtual uint32_t GetVersionMinor() const = 0;
	virtual uint32_t GetVersionBuild() const = 0;
	virtual std::string GetName() const = 0;
	virtual std::string GetVersion() const = 0;
	virtual std::string GetDescription() const = 0;
	virtual std::vector<std::string> GetNodes() const = 0;
	virtual std::vector<LibraryEntry> GetLibrary() const = 0;
	virtual BaseNodePtr CreatePluginNode(const std::string& vPluginNodeName) = 0; // factory for nodes
	virtual std::vector<PluginPane> GetPanes() const = 0;

	// will reset the ids but will return the id count pre reset
	virtual int ResetImGuiID(const int& vWidgetId) = 0;

};
