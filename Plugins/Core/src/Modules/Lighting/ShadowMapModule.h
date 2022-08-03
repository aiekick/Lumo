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
#include <Interfaces/TaskInterface.h>
#include <Interfaces/ResizerInterface.h>
#include <Interfaces/ModelInputInterface.h>
#include <Interfaces/LightGroupInputInterface.h>
#include <Interfaces/LightGroupOutputInterface.h>
#include <Interfaces/TextureGroupOutputInterface.h>

class ShadowMapModule_Mesh_Pass;
class ShadowMapModule :
	public BaseRenderer,
	public GuiInterface,
	public TaskInterface,
	public ModelInputInterface,
	public TextureGroupOutputInterface,
	public LightGroupInputInterface,
	public LightGroupOutputInterface,
	public ResizerInterface
{
public:
	static std::shared_ptr<ShadowMapModule> Create(vkApi::VulkanCorePtr vVulkanCorePtr);

private:
	ct::cWeak<ShadowMapModule> m_This;
	DescriptorImageInfoVector m_ImageInfos;
	std::array<FrameBufferPtr, 8U> m_FrameBuffers; // 8 Lights
	std::shared_ptr<ShadowMapModule_Mesh_Pass> m_ShadowMapModule_Mesh_Pass_Ptr = nullptr;
	SceneLightGroupWeak m_SceneLightGroupWeak;

private:
	bool Init();
	void Unit();

public:
	ShadowMapModule(vkApi::VulkanCorePtr vVulkanCorePtr);
	~ShadowMapModule() override;

	void RenderShaderPasses(vk::CommandBuffer* vCmdBuffer) override;
	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
	void SetModel(SceneModelWeak vSceneModel = SceneModelWeak()) override;
	DescriptorImageInfoVector* GetDescriptorImageInfos(const uint32_t& vBindingPoint, fvec2Vector* vOutSizes) override;
	void SetLightGroup(SceneLightGroupWeak vSceneLightGroup) override;
	SceneLightGroupWeak GetLightGroup() override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};