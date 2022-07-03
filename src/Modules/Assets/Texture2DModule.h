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

#include <functional>
#include <ctools/cTools.h>
#include <vulkan/vulkan.hpp>
#include <Graph/Base/BaseNode.h>
#include <vkFramework/Texture2D.h>
#include <ctools/ConfigAbstract.h>
#include <Interfaces/NodeInterface.h>
#include <vkFramework/ImGuiTexture.h>
#include <Interfaces/CameraInterface.h>
#include <Interfaces/GuiInterface.h>
#include <Interfaces/TextureOutputInterface.h>

namespace vkApi { class VulkanCore; }
class Texture2DModule :
	public conf::ConfigAbstract,
	public GuiInterface,
	public NodeInterface,
	public TextureOutputInterface
{
public:
	static std::shared_ptr<Texture2DModule> Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode);

private:
	ct::cWeak<Texture2DModule> m_This;
	std::string m_FilePathName;
	std::string m_FilePath;
	std::string m_FileName;
	std::string unique_OpenPictureFileDialog_id;
	Texture2DPtr m_Texture2DPtr = nullptr;
	ImGuiTexture m_ImGuiTexture;
	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;

public:
	Texture2DModule(vkApi::VulkanCorePtr vVulkanCorePtr);
	~Texture2DModule();

	bool Init();
	void Unit();

	std::string GetFileName() { return m_FileName; }

	void NeedResize(ct::ivec2* vNewSize);

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;

	void DrawTexture(ct::ivec2 vMaxSize);

	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint) override;

private:
	void LoadTexture2D(const std::string& vFilePathName);

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};