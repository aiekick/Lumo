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

#include <Base/QuadShaderPass.h>

#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanDevice.h>
#include <vkFramework/vk_mem_alloc.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/ImGuiTexture.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/VulkanFrameBuffer.h>

#include <SceneGraph/SceneMesh.hpp>

#include <Interfaces/GuiInterface.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/ShaderInterface.h>
#include <Interfaces/TextureInputInterface.h>
#include <Interfaces/ResizerInterface.h>
#include <Interfaces/TextureOutputInterface.h>

class ShadowMapModule;

class DeferredRenderer_Quad_Pass :
	public QuadShaderPass,
	public GuiInterface,
	public TextureInputInterface<9U>,
	public TextureOutputInterface
{
private:
	VulkanBufferObjectPtr m_UBOFragPtr = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo_Frag;

	struct UBOFrag {
		alignas(4) float use_sampler_position = 0.0f;		// position
		alignas(4) float use_sampler_normal = 0.0f;			// normal
		alignas(4) float use_sampler_albedo = 0.0f;			// albedo
		alignas(4) float use_sampler_diffuse = 0.0f;		// diffuse
		alignas(4) float use_sampler_specular = 0.0f;		// specular
		alignas(4) float use_sampler_attenuation = 0.0f;	// attenuation
		alignas(4) float use_sampler_mask = 0.0f;			// mask
		alignas(4) float use_sampler_ssao = 0.0f;			// ao
		alignas(4) float use_sampler_shadow = 0.0f;			// shadow
	} m_UBOFrag;

	std::string m_VertexShaderCode;
	std::string m_FragmentShaderCode;

public:
	DeferredRenderer_Quad_Pass(vkApi::VulkanCorePtr vVulkanCorePtr);
	~DeferredRenderer_Quad_Pass() override;

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;
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