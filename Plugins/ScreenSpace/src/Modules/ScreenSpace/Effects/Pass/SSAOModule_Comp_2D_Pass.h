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

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Base/BaseRenderer.h>
#include <LumoBackend/Base/EffectPass.h>

#include <Gaia/gaia.h>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>

#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/TextureInputInterface.h>
#include <LumoBackend/Interfaces/TextureOutputInterface.h>

class SSAOModule_Comp_2D_Pass : 
    public EffectPass<4U>, // 1:pos, 2:nor, 3:noise, 4:source_color for blending only with pass not with nodes
	public NodeInterface {
public:
    static std::shared_ptr<SSAOModule_Comp_2D_Pass> Create(const ct::uvec2& vSize, GaiApi::VulkanCorePtr vVulkanCorePtr);

private:
	VulkanBufferObjectPtr m_UBOCompPtr = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo_Comp;

	struct UBOComp {
        alignas(4) float use_sampler_color = 0.0f;
		alignas(4) float use_sampler_pos = 0.0f;
		alignas(4) float use_sampler_nor = 0.0f;
        alignas(4) float use_sampler_noise = 0.0f;
		alignas(4) float u_noise_scale = 1.0f;
		alignas(4) float u_ao_radius = 0.01f;
		alignas(4) float u_ao_scale = 1.0f;
		alignas(4) float u_ao_bias = 0.01f;
        alignas(4) float u_ao_intensity = 2.0f;
        alignas(4) float u_enabled = 1.0f;
	} m_UBOComp;

public:
	SSAOModule_Comp_2D_Pass(GaiApi::VulkanCorePtr vVulkanCorePtr);
	~SSAOModule_Comp_2D_Pass() override;

	void Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) override;
    
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	
	void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;
	
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

    std::string GetComputeShaderCode(std::string& vOutShaderName) override;
};
