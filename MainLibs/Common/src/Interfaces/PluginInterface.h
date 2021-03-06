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
#include <Graph/Library/LibraryCategory.h>
#include <vkFramework/vkFramework.h>

class PluginInterface;
typedef ct::cWeak<PluginInterface> PluginInterfaceWeak;
typedef std::shared_ptr<PluginInterface> PluginInterfacePtr;

class SlotColor;
class FileHelper;
class VulkanShader;
class CommonSystem;
struct ImGuiContext;
namespace ImGui { class CustomStyle; }
class PluginInterface
{
protected:
	vkApi::VulkanCoreWeak m_VulkanCoreWeak;
	bool iSinAPlugin = false;

public:
	virtual ~PluginInterface();

	virtual bool Init(
		vkApi::VulkanCoreWeak vVulkanCoreWeak,
		FileHelper* vFileHelper, 
		CommonSystem* vCommonSystem,
		ImGuiContext* vContext,
		SlotColor* vSlotColor,
		ImGui::CustomStyle* vCustomStyle);
	virtual void Unit();

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

	// will reset the ids but will return the id count pre reset
	virtual int ResetImGuiID(const int& vWidgetId) = 0;
};
