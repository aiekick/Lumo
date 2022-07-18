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

#include <vulkan/vulkan.hpp>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <Utils/RtxShaderPass.h>

#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanDevice.h>
#include <vkFramework/vk_mem_alloc.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/ImGuiTexture.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/VulkanFrameBuffer.h>

#include <SceneGraph/SceneMesh.h>

#include <Interfaces/GuiInterface.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/ShaderInterface.h>
#include <Interfaces/TextureInputInterface.h>
#include <Interfaces/ResizerInterface.h>
#include <Interfaces/TextureOutputInterface.h>
#include <Interfaces/TextureGroupInputInterface.h>
#include <Interfaces/LightGroupInputInterface.h>
#include <Interfaces/ModelInputInterface.h>

class RtxPbrRenderer_Pass :
	public RtxShaderPass,
	public GuiInterface,
	public LightGroupInputInterface,
	public ModelInputInterface,
	public TextureOutputInterface
{
private:
	const vk::DescriptorBufferInfo m_SceneLightGroupDescriptorInfo = { VK_NULL_HANDLE, 0U, VK_WHOLE_SIZE };
	const vk::DescriptorBufferInfo* m_SceneLightGroupDescriptorInfoPtr = &m_SceneLightGroupDescriptorInfo;
	const vk::WriteDescriptorSetAccelerationStructureKHR m_EmptyAccelStructureTopDescriptorInfo = { 1U, VK_NULL_HANDLE };

	VulkanBufferObjectPtr m_ModelAdressesPtr = nullptr;
	vk::DescriptorBufferInfo m_ModelAdressesBufferInfo = { VK_NULL_HANDLE, 0U, VK_WHOLE_SIZE };

public:
	RtxPbrRenderer_Pass(vkApi::VulkanCorePtr vVulkanCorePtr);
	~RtxPbrRenderer_Pass() override;

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;
	void SetModel(SceneModelWeak vSceneModel) override;
	void SetLightGroup(SceneLightGroupWeak vSceneLightGroup = SceneLightGroupWeak()) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

private:
	bool CanRender() override;
	bool CanUpdateDescriptors() override;

	bool BuildModel() override;
	void DestroyModel(const bool& vReleaseDatas = false) override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	std::string GetRayGenerationShaderCode(std::string& vOutShaderName) override;
	std::string GetRayIntersectionShaderCode(std::string& vOutShaderName) override;
	std::string GetRayMissShaderCode(std::string& vOutShaderName) override;
	std::string GetRayAnyHitShaderCode(std::string& vOutShaderName) override;
	std::string GetRayClosestHitShaderCode(std::string& vOutShaderName) override;

};