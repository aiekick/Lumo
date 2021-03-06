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

class ToneMapModule_Quad_Pass :
	public QuadShaderPass,
	public GuiInterface,
	public TextureInputInterface<1U>,
	public TextureOutputInterface
{
private:
	VulkanBufferObjectPtr m_UBO_Frag = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo_Frag;

	struct UBOFrag {
		alignas(4) int32_t u_tone_map_algo_idx = 0U; // default is 0 (aces)

		alignas(4) float u_aces_a = 2.51f;			// default is 2.51
		alignas(4) float u_aces_b = 0.03f;			// default is 0.03
		alignas(4) float u_aces_c = 2.43f;			// default is 2.43
		alignas(4) float u_aces_d = 0.59f;			// default is 0.59
		alignas(4) float u_aces_e = 0.14f;			// default is 0.14

		alignas(4) float u_filmic_a = 0.004f;		// default is 0.004
		alignas(4) float u_filmic_b = 6.2f;			// default is 6.2
		alignas(4) float u_filmic_c = 0.5f;			// default is 0.5
		alignas(4) float u_filmic_d = 1.7f;			// default is 1.7
		alignas(4) float u_filmic_e = 0.06f;		// default is 0.06
		alignas(4) float u_filmic_f = 2.2f;			// default is 2.2

		alignas(4) float u_lottes_a = 1.6f;			// default is 1.6
		alignas(4) float u_lottes_d = 0.977f;		// default is 0.977
		alignas(4) float u_lottes_hdrMax = 8.0f;	// default is 8.0
		alignas(4) float u_lottes_midIn = 0.18f;	// default is 0.18
		alignas(4) float u_lottes_midOut = 0.267f;	// default is 0.267

		alignas(4) float u_reinhard2_L_white = 1.6f;	// default is 4.0

		alignas(4) float u_uchimura_max_brightness = 1.0f;			// max display brightness // default is 1.0
		alignas(4) float u_uchimura_contrast = 1.0f;				// contrast // default is 1.0
		alignas(4) float u_uchimura_linear_section_start = 0.22f;	// linear section start // default is 0.22
		alignas(4) float u_uchimura_linear_section_length = 0.4f;	// linear section length // default is 0.4
		alignas(4) float u_uchimura_black = 1.33f;					// black // default is 1.33
		alignas(4) float u_uchimura_pedestal = 0.0f;				// pedestal // default is 0.0

		alignas(4) float u_uncharted2_a = 0.15f;		// default is 0.15
		alignas(4) float u_uncharted2_b = 0.50f;		// default is 0.50
		alignas(4) float u_uncharted2_c = 0.10f;		// default is 0.10
		alignas(4) float u_uncharted2_d = 0.20f;		// default is 0.20
		alignas(4) float u_uncharted2_e = 0.02f;		// default is 0.02
		alignas(4) float u_uncharted2_f = 0.30f;		// default is 0.30
		alignas(4) float u_uncharted2_w = 11.2f;		// default is 11.2
		alignas(4) float u_uncharted2_exposure = 2.0f;	// default is 2.0

		alignas(4) float u_unreal_a = 0.154f;	// default is 0.154
		alignas(4) float u_unreal_b = 1.019f;	// default is 1.019
	}; 
	
	UBOFrag m_UBOFrag;
	UBOFrag m_DefaultUBOFrag;
	std::vector<std::string> m_ToneMap_Algos;


public:
	ToneMapModule_Quad_Pass(vkApi::VulkanCorePtr vVulkanCorePtr);
	~ToneMapModule_Quad_Pass() override;

	void ActionBeforeInit() override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

private:
	std::string GetStructToXMLString(const char* vBalise, float* vStartItem, size_t vCountItem);
	void LoadStructFromToXMLString(const std::string& vXMLString, float* vStartItem, size_t vCountItem);

protected:
	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	std::string GetVertexShaderCode(std::string& vOutShaderName) override;
	std::string GetFragmentShaderCode(std::string& vOutShaderName) override;
};