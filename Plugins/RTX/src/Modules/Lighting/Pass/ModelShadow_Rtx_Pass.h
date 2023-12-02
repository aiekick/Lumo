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

#include <LumoBackend/Headers/LumoBackendDefs.h>

#include <vulkan/vulkan.hpp>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <Base/RtxShaderPass.h>

#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>
#include <Gaia/Core/vk_mem_alloc.h>
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
#include <LumoBackend/Interfaces/ModelInputInterface.h>
#include <Interfaces/AccelStructureInputInterface.h>

class ModelShadow_Rtx_Pass :
	public RtxShaderPass,
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
	ModelShadow_Rtx_Pass(GaiApi::VulkanCoreWeak vVulkanCore);
	~ModelShadow_Rtx_Pass() override;

	void ActionBeforeCompilation() override;

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = "") override;
	bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = "") override;
	bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = "") override;
	
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