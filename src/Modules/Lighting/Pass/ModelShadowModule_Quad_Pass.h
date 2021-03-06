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

#include <array>
#include <memory>
#include <ctools/ConfigAbstract.h>
#include <Interfaces/GuiInterface.h>

#include <set>
#include <string>
#include <Headers/Globals.h>
#include <vulkan/vulkan.hpp>
#include <vkFramework/vk_mem_alloc.h>
#include <ctools/cTools.h>
#include <Base/QuadShaderPass.h>
#include <ctools/ConfigAbstract.h>
#include <Interfaces/ShaderInterface.h>
#include <Interfaces/SerializationInterface.h>
#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanDevice.h>
#include <vkFramework/VulkanFrameBuffer.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/ImGuiTexture.h>
#include <SceneGraph/SceneMesh.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/TextureInputInterface.h>
#include <Interfaces/TextureOutputInterface.h>
#include <Interfaces/TextureGroupInputInterface.h>
#include <Interfaces/LightGroupInputInterface.h>

class ModelShadowModule_Quad_Pass :
	public QuadShaderPass,
	public GuiInterface,
	public TextureInputInterface<2U>,
	public TextureGroupInputInterface<8U>,
	public TextureOutputInterface,
	public LightGroupInputInterface
{
protected:
	const vk::DescriptorBufferInfo m_SceneLightGroupDescriptorInfo = { VK_NULL_HANDLE, 0U, VK_WHOLE_SIZE };
	const vk::DescriptorBufferInfo* m_SceneLightGroupDescriptorInfoPtr = &m_SceneLightGroupDescriptorInfo;

	VulkanBufferObjectPtr m_UBO_Frag = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo_Frag;
	struct UBOFrag {
		alignas(4) float u_shadow_strength = 0.5f;
		alignas(4) float u_bias = 0.01f;
		alignas(4) float u_poisson_scale = 5000.0f;
		alignas(4) float u_use_pcf = 1.0f;
		alignas(4) float use_sampler_pos = 0.0f;
		alignas(4) float use_sampler_nor = 0.0f;
		alignas(4) float use_sampler_shadow_map = 0.0f;
	} m_UBOFrag;

public:
	ModelShadowModule_Quad_Pass(vkApi::VulkanCorePtr vVulkanCorePtr);
	~ModelShadowModule_Quad_Pass() override;

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;
	void SetTextures(const uint32_t& vBinding, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;
	void SetLightGroup(SceneLightGroupWeak vSceneLightGroup) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

private:
	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	std::string GetVertexShaderCode(std::string& vOutShaderName) override;
	std::string GetFragmentShaderCode(std::string& vOutShaderName) override;
};