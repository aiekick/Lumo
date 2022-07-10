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
#include <ctools/ConfigAbstract.h>

#include <Headers/Globals.h>
#include <vulkan/vulkan.hpp>
#include <vkFramework/vk_mem_alloc.h>
#include <ctools/cTools.h>
#include <Base/ShaderPass.h>
#include <ctools/ConfigAbstract.h>

#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanDevice.h>
#include <vkFramework/VulkanFrameBuffer.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/ImGuiTexture.h>

#include <SceneGraph/SceneMesh.h>

#include <Interfaces/GuiInterface.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/TextureGroupOutputInterface.h>
#include <Interfaces/ModelInputInterface.h>
#include <Interfaces/LightGroupInputInterface.h>
#include <Interfaces/LightGroupOutputInterface.h>

class ShadowMapModule_Pass :
	public ShaderPass,
	public GuiInterface,
	public TextureGroupOutputInterface,
	public ModelInputInterface,
	public LightGroupInputInterface,
	public LightGroupOutputInterface
{
protected:
	const vk::DescriptorBufferInfo m_SceneLightGroupDescriptorInfo = { VK_NULL_HANDLE, 0U, VK_WHOLE_SIZE };
	const vk::DescriptorBufferInfo* m_SceneLightGroupDescriptorInfoPtr = &m_SceneLightGroupDescriptorInfo;

	VulkanBufferObjectPtr m_UBO_Vert = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo_Vert;
	struct UBOVert {
		alignas(16) uint32_t light_id_to_use = 0U;
	} m_UBOVert;

	struct PushConstants {
		uint32_t light_id_to_use = 0U;
	} m_PushConstants;

public:
	ShadowMapModule_Pass(vkApi::VulkanCorePtr vVulkanCorePtr);
	~ShadowMapModule_Pass();

	void ActionBeforeInit() override;
	void DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void SetModel(SceneModelWeak vSceneModel = SceneModelWeak()) override;
	DescriptorImageInfoVector* GetDescriptorImageInfos(const uint32_t& vBindingPoint, fvec2Vector* vOutSizes) override;
	void SetLightGroup(SceneLightGroupWeak vSceneLightGroup) override;
	SceneLightGroupWeak GetLightGroup() override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

private:
	void DestroyModel(const bool& vReleaseDatas = false) override;

	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override; 

	void SetInputStateBeforePipelineCreation() override;

	std::string GetVertexShaderCode(std::string& vOutShaderName) override;
	std::string GetFragmentShaderCode(std::string& vOutShaderName) override;

private:
	void DrawModelForOneLightGroup(const uint32_t vLigthNumber, vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber);
};