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



#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Base/BaseRenderer.h>
#include <LumoBackend/Base/QuadShaderPass.h>

#include <Gaia/gaia.h>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>

#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/TaskInterface.h>
#include <LumoBackend/Interfaces/ResizerInterface.h>
#include <LumoBackend/Interfaces/ModelInputInterface.h>
#include <LumoBackend/Interfaces/LightGroupInputInterface.h>
#include <LumoBackend/Interfaces/LightGroupOutputInterface.h>
#include <LumoBackend/Interfaces/TextureGroupOutputInterface.h>

class ShadowMapModule_Mesh_Pass;
class ShadowMapModule :
	public BaseRenderer,
	
	public TaskInterface,
	public ModelInputInterface,
	public TextureGroupOutputInterface,
	public LightGroupInputInterface,
	public LightGroupOutputInterface
{
public:
	static std::shared_ptr<ShadowMapModule> Create(GaiApi::VulkanCorePtr vVulkanCorePtr);

private:
	std::weak_ptr<ShadowMapModule> m_This;
	DescriptorImageInfoVector m_ImageInfos;
	std::array<FrameBufferPtr, 8U> m_FrameBuffers; // 8 Lights
	std::shared_ptr<ShadowMapModule_Mesh_Pass> m_ShadowMapModule_Mesh_Pass_Ptr = nullptr;
	SceneLightGroupWeak m_SceneLightGroupWeak;

private:
	bool Init();
	void Unit();

public:
	ShadowMapModule(GaiApi::VulkanCorePtr vVulkanCorePtr);
	~ShadowMapModule() override;

	void RenderShaderPasses(vk::CommandBuffer* vCmdBufferPtr) override;
	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
	void SetModel(SceneModelWeak vSceneModel = SceneModelWeak()) override;
	DescriptorImageInfoVector* GetDescriptorImageInfos(const uint32_t& vBindingPoint, fvec2Vector* vOutSizes) override;
	void SetLightGroup(SceneLightGroupWeak vSceneLightGroup) override;
	SceneLightGroupWeak GetLightGroup() override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};