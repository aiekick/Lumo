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

namespace vkApi { class VulkanCore; }
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
	static OutputModulePtr Create(vkApi::VulkanCore* vVulkanCore, BaseNodeWeak vParentNode);

private:
	OutputModuleWeak m_This;
	ImGuiTexture m_ImGuiTexture;
	vkApi::VulkanCore* m_VulkanCore = nullptr;

public:
	OutputModule(vkApi::VulkanCore* vVulkanCore);
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