#pragma once

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

class Output3DModule;
typedef std::shared_ptr<Output3DModule> Output3DModulePtr;
typedef ct::cWeak<Output3DModule> Output3DModuleWeak;

class Output3DModule :
	public conf::ConfigAbstract,
	public GuiInterface,
	public TaskInterface,
	public NodeInterface,
	public TextureInputInterface<0U>, // 0, because no need of items here
	public TextureOutputInterface, // le output n'est pas dans le graph, mais appelé par la vue, ce node conlue le graph, il est unique
	public ResizerInterface
{
public:
	static Output3DModulePtr Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode);

private:
	Output3DModuleWeak m_This;
	ImGuiTexture m_ImGuiTexture;
	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;

public:
	Output3DModule(vkApi::VulkanCorePtr vVulkanCorePtr);
	~Output3DModule();

	bool Init();
	void Unit();

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
		
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;
	void SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;

	void NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) override;

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};