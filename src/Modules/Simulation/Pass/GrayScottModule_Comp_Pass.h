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
#include <Base/ShaderPass.h>

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
#include <Interfaces/LightGroupInputInterface.h>

class GrayScottModule_Comp_Pass :
	public ShaderPass,
	public GuiInterface,
	public TextureInputInterface<1U>,
	public TextureOutputInterface
{
private:
	// config name, feed, kill
	std::vector<std::string> m_GrayScottConfigNames;
	std::vector<ct::fvec4> m_GrayScottConfigs; // diff x, diff,y, feed, kill
	int32_t m_SelectedGrayScottConfig = 0; // Custom

	VulkanBufferObjectPtr m_UBO_Comp = nullptr;
	vk::DescriptorBufferInfo m_UBO_Comp_BufferInfos = { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	struct UBOComp {
		alignas(4) float mouse_radius = 0.05f;
		alignas(4) float mouse_inversion = 0.0f;
		alignas(4) float reset_substances = 0.0f;
		alignas(4) float grayscott_diffusion_u = 1.0;
		alignas(4) float grayscott_diffusion_v = 1.0;
		alignas(4) float grayscott_feed = 0.026f;
		alignas(4) float grayscott_kill = 0.051f;
		alignas(8) ct::ivec2 image_size = 0;
	} m_UBOComp;

public:
	GrayScottModule_Comp_Pass(vkApi::VulkanCorePtr vVulkanCorePtr);
	~GrayScottModule_Comp_Pass() override;

	void ActionBeforeInit() override;
	void WasJustResized() override;
	void Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

private:
	void ClearGrayScottConfigs();
	void AddGrayScottConfig(const std::string& vConfigName, const float& vDiffXValue, const float& vDiffYValue, const float& vFeedValue, const float& vKillValue);

protected:
	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	std::string GetComputeShaderCode(std::string& vOutShaderName) override;
};