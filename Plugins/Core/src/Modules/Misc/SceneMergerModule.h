/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/


#pragma once

#include <set>
#include <array>
#include <string>
#include <memory>

#include <Headers/Globals.h>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <Base/BaseRenderer.h>
#include <Base/QuadShaderPass.h>

#include <vulkan/vulkan.hpp>
#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanDevice.h>
#include <vkFramework/vk_mem_alloc.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/ImGuiTexture.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/VulkanFrameBuffer.h>

#include <Interfaces/GuiInterface.h>
#include <Interfaces/NodeInterface.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/NodeInterface.h>
#include <Interfaces/ResizerInterface.h>

#include <Interfaces/TextureInputInterface.h>
#include <Interfaces/ShaderPassInputInterface.h>
#include <Interfaces/TextureOutputInterface.h>

class SceneMergerModule_Quad_Pass;
class SceneMergerModule :
	public NodeInterface,
	public BaseRenderer,	
	public TaskInterface,
	public TextureInputInterface<0U>,
	public ShaderPassInputInterface,
	public TextureOutputInterface,
	public GuiInterface
{
public:
	static std::shared_ptr<SceneMergerModule> Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode);

private:
	ct::cWeak<SceneMergerModule> m_This;
	FrameBufferPtr m_FrameBufferPtr = nullptr;
	SceneShaderPassContainer m_SceneShaderPassContainer;
	std::vector<uint32_t> m_JustDeletedSceneShaderPassSlots;

public:
	SceneMergerModule(vkApi::VulkanCorePtr vVulkanCorePtr);
	~SceneMergerModule() override;

	bool Init();

	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
	bool ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
	void RenderShaderPasses(vk::CommandBuffer* vCmdBuffer) override;

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;

	void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
	void NeedResizeByHand(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
	bool ResizeIfNeeded() override;

	// Interfaces Setters
	void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize = nullptr) override;
	void SetShaderPasses(const uint32_t& vSlotID, SceneShaderPassWeak vShaderPasses) override;

	// Interfaces Getters
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;

	SceneShaderPassContainer& GetSceneShaderPassContainerRef();
	const std::vector<uint32_t>& GetJustDeletedSceneShaderPassSlots() const;

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
	void AfterNodeXmlLoading() override;
};
