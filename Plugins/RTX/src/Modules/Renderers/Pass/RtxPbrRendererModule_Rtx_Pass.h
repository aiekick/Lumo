/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

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
#include <Base/RtxShaderPass.h>
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
#include <Interfaces/AccelStructureInputInterface.h>
#include <Interfaces/LightGroupInputInterface.h>
#include <Interfaces/TextureOutputInterface.h>

class RtxPbrRendererModule_Rtx_Pass :
	public RtxShaderPass,
	public AccelStructureInputInterface,
	public LightGroupInputInterface,
	public TextureOutputInterface,
	public GuiInterface,
	public NodeInterface
{
private:
	struct UBO_Ahit {
		alignas(4) float u_Name = 0.0f;
	} m_UBO_Ahit;
	VulkanBufferObjectPtr m_UBO_Ahit_Ptr = nullptr;
	vk::DescriptorBufferInfo m_UBO_Ahit_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	struct UBO_Chit {
		alignas(4) float u_Name = 0.0f;
	} m_UBO_Chit;
	VulkanBufferObjectPtr m_UBO_Chit_Ptr = nullptr;
	vk::DescriptorBufferInfo m_UBO_Chit_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	struct UBO_Inter {
		alignas(4) float u_Name = 0.0f;
	} m_UBO_Inter;
	VulkanBufferObjectPtr m_UBO_Inter_Ptr = nullptr;
	vk::DescriptorBufferInfo m_UBO_Inter_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	struct UBO_Miss {
		alignas(4) float u_Name = 0.0f;
	} m_UBO_Miss;
	VulkanBufferObjectPtr m_UBO_Miss_Ptr = nullptr;
	vk::DescriptorBufferInfo m_UBO_Miss_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	struct UBO_RGen {
		alignas(4) float u_Name = 0.0f;
	} m_UBO_RGen;
	VulkanBufferObjectPtr m_UBO_RGen_Ptr = nullptr;
	vk::DescriptorBufferInfo m_UBO_RGen_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };


public:
	RtxPbrRendererModule_Rtx_Pass(vkApi::VulkanCorePtr vVulkanCorePtr);
	~RtxPbrRendererModule_Rtx_Pass() override;

	void ActionBeforeInit() override;
	void WasJustResized() override;

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;

	// Interfaces Setters
	void SetAccelStructure(SceneAccelStructureWeak vSceneAccelStructure) override;
	void SetLightGroup(SceneLightGroupWeak vSceneLightGroup) override;

	// Interfaces Getters
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;


	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
	void AfterNodeXmlLoading() override;

protected:
	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	std::string GetRayGenerationShaderCode(std::string& vOutShaderName) override;
	std::string GetRayIntersectionShaderCode(std::string& vOutShaderName) override;
	std::string GetRayMissShaderCode(std::string& vOutShaderName) override;
	std::string GetRayAnyHitShaderCode(std::string& vOutShaderName) override;
	std::string GetRayClosestHitShaderCode(std::string& vOutShaderName) override;
};