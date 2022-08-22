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

#include <Base/RtxShaderPass.h>

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
#include <Interfaces/AccelStructureInputInterface.h>

class ModelShadow_Rtx_Pass :
	public RtxShaderPass,
	public GuiInterface,
	public LightGroupInputInterface,
	public TextureOutputInterface,
	public AccelStructureInputInterface
{
private:
	const vk::WriteDescriptorSetAccelerationStructureKHR m_EmptyAccelStructureTopDescriptorInfo = { 1U, VK_NULL_HANDLE };

	struct UBO_Rtx {
		alignas(4) float u_light_intensity_factor = 100.0f;
		alignas(4) float u_enable_light_attenuation = 1.0f;
		alignas(4) float u_enable_light_color = 1.0f;
		alignas(4) float u_shadow_strength = 0.5f;
	} m_UBO_Rtx;
	const UBO_Rtx m_Default_UBO_Rtx;
	VulkanBufferObjectPtr m_UBO_Rtx_Ptr = nullptr;
	vk::DescriptorBufferInfo m_UBO_Rtx_BufferInfos;

public:
	ModelShadow_Rtx_Pass(vkApi::VulkanCorePtr vVulkanCorePtr);
	~ModelShadow_Rtx_Pass() override;

	void ActionBeforeCompilation() override;

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;
	
	void SetAccelStructure(SceneAccelStructureWeak vSceneAccelStructure) override;
	void SetLightGroup(SceneLightGroupWeak vSceneLightGroup = SceneLightGroupWeak()) override;

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

private:
	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;

	bool CanRender() override;
	bool CanUpdateDescriptors() override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	std::string GetHitPayLoadCode();
	std::string GetRayGenerationShaderCode(std::string& vOutShaderName) override;
	std::string GetRayIntersectionShaderCode(std::string& vOutShaderName) override;
	std::string GetRayMissShaderCode(std::string& vOutShaderName) override;
	std::string GetRayAnyHitShaderCode(std::string& vOutShaderName) override;
	std::string GetRayClosestHitShaderCode(std::string& vOutShaderName) override;

};