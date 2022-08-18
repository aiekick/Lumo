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
#include <Interfaces/NodeInterface.h>
#include <Interfaces/TextureInputInterface.h>
#include <Interfaces/TextureOutputInterface.h>

class BloomModule_Comp_2D_Pass :
	public ShaderPass,
	public NodeInterface,
	public GuiInterface,
	public TextureInputInterface<1U>,
	public TextureOutputInterface
{
private:
	VulkanBufferObjectPtr m_UBOCompPtr = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo_Comp;

	struct UBOComp {
		alignas(16) ct::fvec3 u_high_freq_threshold = 0.8f;
		alignas(4) uint32_t u_blur_radius = 4; // default is 4
		alignas(4) float u_exposure = 1.0f;
		alignas(4) float u_gamma_correction = 2.2f;// linear to srgb correction 
	} m_UBOComp;

	bool m_GaussianSigmAuto = true;
	float m_GaussianSigma = 1.0f;
	std::vector<float> m_GaussianWeights;
	VulkanBufferObjectPtr m_SBO_GaussianWeights = nullptr;
	vk::DescriptorBufferInfo m_SBO_GaussianWeightsBufferInfo = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};

public:
	BloomModule_Comp_2D_Pass(vkApi::VulkanCorePtr vVulkanCorePtr);
	~BloomModule_Comp_2D_Pass() override;
	void ActionBeforeInit();
	void ActionBeforeCompilation();
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;
	void SwapMultiPassFrontBackDescriptors() override;
	bool CanUpdateDescriptors() override;
	void Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
	void AfterNodeXmlLoading() override;

protected:
	void ReComputeGaussianBlurWeights();

	void Compute_High_Freq_Thresholding(vk::CommandBuffer* vCmdBuffer);
	void Compute_Horizontal_Blur(vk::CommandBuffer* vCmdBuffer);
	void Compute_Vertical_Blur(vk::CommandBuffer* vCmdBuffer);
	void Compute_Gamma_Correction(vk::CommandBuffer* vCmdBuffer);

	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;

	bool CreateSBO() override;
	void UploadSBO() override;
	void DestroySBO() override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	bool CreateComputePipeline();

	std::string GetComputeShaderCode(std::string& vOutShaderName) override;
};