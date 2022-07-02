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

#include <ctools/cTools.h>
#include <vulkan/vulkan.hpp>
#include <ctools/ConfigAbstract.h>
#include <Base/BaseRenderer.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/VulkanDevice.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/GuiInterface.h>
#include <Interfaces/ResizerInterface.h>
#include <Interfaces/TextureOutputInterface.h>

namespace vkApi { class VulkanCore; }
class GridModule_Pass;
class GridModule :
	public BaseRenderer,
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
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint) override;

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};
