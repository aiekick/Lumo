/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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
	static std::shared_ptr<Texture2DModule> Create(vkApi::VulkanCore* vVulkanCore, BaseNodeWeak vParentNode);

private:
	ct::cWeak<Texture2DModule> m_This;
	std::string m_FilePathName;
	std::string m_FilePath;
	std::string m_FileName;
	std::string unique_OpenPictureFileDialog_id;
	Texture2DPtr m_Texture2DPtr = nullptr;
	ImGuiTexture m_ImGuiTexture;
	vkApi::VulkanCore* m_VulkanCore = nullptr;

public:
	Texture2DModule(vkApi::VulkanCore* vVulkanCore);
	~Texture2DModule();

	bool Init();
	void Unit();

	std::string GetFileName() { return m_FileName; }

	void NeedResize(ct::ivec2* vNewSize);

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;

	void DrawTexture2D(ct::ivec2 vMaxSize);

	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint)  override;

private:
	void LoadTexture2D(const std::string& vFilePathName);

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};