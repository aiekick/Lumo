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
#include <Interfaces/TextureInputInterface.h>
#include <Interfaces/TextureOutputInterface.h>



class SSAOModule_Pass : 
	public QuadShaderPass,
	public GuiInterface,
	public TextureInputInterface<3U>,
	public TextureOutputInterface
{
private:
	VulkanBufferObjectPtr m_UBO_Frag = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo_Frag;

	struct UBOFrag {
		alignas(4) float use_sampler_pos = 0.0f;
		alignas(4) float use_sampler_nor = 0.0f;
		alignas(4) float use_sampler_noise = 0.0f;
		alignas(4) float u_noise_scale = 1.0f;
		alignas(4) float u_ao_radius = 0.01f;
		alignas(4) float u_ao_scale = 1.0f;
		alignas(4) float u_ao_bias = 0.01f;
		alignas(4) float u_ao_intensity = 2.0f;
	} m_UBOFrag;

public:
	SSAOModule_Pass(vkApi::VulkanCorePtr vVulkanCorePtr);
	virtual ~SSAOModule_Pass();

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

protected:
	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	std::string GetVertexShaderCode(std::string& vOutShaderName) override;
	std::string GetFragmentShaderCode(std::string& vOutShaderName) override;
};
