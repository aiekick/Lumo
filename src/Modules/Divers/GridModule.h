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

#include <ctools/cTools.h>
#include <vulkan/vulkan.hpp>
#include <ctools/ConfigAbstract.h>
#include <Generic/GenericRenderer.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/VulkanDevice.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/GuiInterface.h>
#include <Interfaces/ResizerInterface.h>
#include <Interfaces/TextureOutputInterface.h>

namespace vkApi { class VulkanCore; }
class GridModule_Pass;
class GridModule :
	public GenericRenderer,
	public GuiInterface,
	public TaskInterface,
	public ResizerInterface,
	public TextureOutputInterface
{
public:
	static std::shared_ptr<GridModule> Create(vkApi::VulkanCore* vVulkanCore);

private:
	std::shared_ptr<GridModule_Pass> m_GridModule_Pass_Ptr = nullptr;

private:
	VulkanBufferObjectPtr m_UBO_Vert = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo_Vert;

	struct UBOVert {
		alignas(4) float showGrid = 1.0f; // bool 0:false / 1:true
		alignas(4) float gridSize = 7.5f;
		alignas(4) int gridCount = 20;
		alignas(4) float showAxis = 1.0f; // bool 0:false / 1:true
		alignas(4) float bothSides = 0.0f; // bool 0:false / 1:true
		alignas(4) float axisSize = 5.0f;
	} m_UBOVert;

public:
	GridModule(vkApi::VulkanCore* vVulkanCore);
	~GridModule() override;

	bool Init();

	bool Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer = nullptr) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint)  override;

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};
