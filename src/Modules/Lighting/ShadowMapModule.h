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
#include <Interfaces/LightInputInterface.h>
#include <Interfaces/LightOutputInterface.h>
#include <Interfaces/TextureOutputInterface.h>



class ShadowMapModule_Pass;
class ShadowMapModule :
	public BaseRenderer,
	public GuiInterface,
	public TaskInterface,
	public ModelInputInterface,
	public TextureOutputInterface,
	public LightInputInterface,
	public LightOutputInterface,
	public ResizerInterface
{
public:
	static std::shared_ptr<ShadowMapModule> Create(vkApi::VulkanCorePtr vVulkanCorePtr);

private:
	ct::cWeak<ShadowMapModule> m_This;

	std::shared_ptr<ShadowMapModule_Pass> m_ShadowMapModule_Pass_Ptr = nullptr;

public:
	ShadowMapModule(vkApi::VulkanCorePtr vVulkanCorePtr);
	~ShadowMapModule();

	bool Init();

	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer) override;
	void SetModel(SceneModelWeak vSceneModel = SceneModelWeak()) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint) override;
	void SetLightGroup(SceneLightGroupWeak vSceneLightGroup) override;
	SceneLightGroupWeak GetLightGroup() override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};