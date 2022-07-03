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

#include <array>
#include <memory>
#include <ctools/cTools.h>
#include <vkFramework/Texture2D.h>
#include <ctools/ConfigAbstract.h>
#include <vkFramework/ImGuiTexture.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/GuiInterface.h>
#include <Interfaces/NodeInterface.h>
#include <Interfaces/ResizerInterface.h>
#include <Interfaces/TextureInputInterface.h>
#include <Interfaces/TextureOutputInterface.h>

class OutputModule;
typedef std::shared_ptr<OutputModule> OutputModulePtr;
typedef ct::cWeak<OutputModule> OutputModuleWeak;


class OutputModule :
	public conf::ConfigAbstract,
	public GuiInterface,
	public TaskInterface,
	public NodeInterface,
	public TextureInputInterface<0U>, // 0, because no need of items here
	public TextureOutputInterface, // le output n'est pas dans le graph, mais appelé par la vue, ce node conlue le graph, il est unique
	public ResizerInterface
{
public:
	static OutputModulePtr Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode);

private:
	OutputModuleWeak m_This;
	ImGuiTexture m_ImGuiTexture;
	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;

public:
	OutputModule(vkApi::VulkanCorePtr vVulkanCorePtr);
	~OutputModule();

	bool Init();
	void Unit();

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
		
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint) override;
	void SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo) override;

	void NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer) override;
	bool Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer *vCmd);

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};