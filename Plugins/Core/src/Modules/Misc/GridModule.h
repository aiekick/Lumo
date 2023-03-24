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
#include <Base/TaskRenderer.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/VulkanDevice.h>
#include <Interfaces/GuiInterface.h>
#include <Interfaces/ResizerInterface.h>
#include <Interfaces/TextureOutputInterface.h>
#include <Interfaces/ShaderPassOutputInterface.h>

class GridModule_Vertex_Pass;
class GridModule :
	public TaskRenderer,
	public GuiInterface,	
	public TextureOutputInterface,
	public ShaderPassOutputInterface
{
public:
	static std::shared_ptr<GridModule> Create(vkApi::VulkanCorePtr vVulkanCorePtr);

private:
	std::shared_ptr<GridModule_Vertex_Pass> m_GridModule_Vertex_Pass_Ptr = nullptr;
	SceneShaderPassPtr m_SceneShaderPassPtr = nullptr;

public:
	GridModule(vkApi::VulkanCorePtr vVulkanCorePtr);
	~GridModule() override;

	bool Init();

	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers = nullptr) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;
	SceneShaderPassWeak GetShaderPasses(const uint32_t& vBindingPoint) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};
