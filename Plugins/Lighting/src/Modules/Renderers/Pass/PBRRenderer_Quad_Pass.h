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



#include <Gaia/gaia.h>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Base/QuadShaderPass.h>

#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>

#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/SceneGraph/SceneMesh.hpp>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/TaskInterface.h>
#include <LumoBackend/Interfaces/ShaderInterface.h>
#include <LumoBackend/Interfaces/TextureInputInterface.h>
#include <LumoBackend/Interfaces/ResizerInterface.h>
#include <LumoBackend/Interfaces/TextureOutputInterface.h>
#include <LumoBackend/Interfaces/TextureGroupInputInterface.h>
#include <LumoBackend/Interfaces/LightGroupInputInterface.h>

class ShadowMapModule;

class PBRRenderer_Quad_Pass :
	public QuadShaderPass,
	
	public TextureInputInterface<5U>,
	public TextureGroupInputInterface<8U>,
	public LightGroupInputInterface,
	public TextureOutputInterface
{
private:
	VulkanBufferObjectPtr m_UBOFragPtr = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo_Frag;

	struct UBOFrag {
		alignas(4) float u_light_intensity_factor = 100.0f;
		alignas(4) float u_shadow_strength = 0.5f;
		alignas(4) float u_bias = 0.01f;
		alignas(4) float u_poisson_scale = 5000.0f;
		alignas(4) float u_use_pcf = 1.0f;
		alignas(4) float u_diffuse_factor = 1.0f;
		alignas(4) float u_metallic_factor = 1.0f;
		alignas(4) float u_rugosity_factor = 1.0f;
		alignas(4) float u_ao_factor = 1.0f;
		alignas(4) float use_sampler_position = 0.0f;		// position
		alignas(4) float use_sampler_normal = 0.0f;			// normal
		alignas(4) float use_sampler_albedo = 0.0f;			// albedo
		alignas(4) float use_sampler_mask = 0.0f;			// mask
		alignas(4) float use_sampler_ao = 0.0f;				// ao
		alignas(4) float use_sampler_shadow_maps = 0.0f;	// 8 shadow maps
	} m_UBOFrag;

	std::string m_VertexShaderCode;
	std::string m_FragmentShaderCode;

public:
	PBRRenderer_Quad_Pass(GaiApi::VulkanCorePtr vVulkanCorePtr);
	~PBRRenderer_Quad_Pass() override;

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;
	void SetTextures(const uint32_t& vBindingPoint, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes) override;
	void SetLightGroup(SceneLightGroupWeak vSceneLightGroup = SceneLightGroupWeak()) override;
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