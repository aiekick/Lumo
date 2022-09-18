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

class LaplacianModule_Comp_2D_Pass :
	public ShaderPass,
	public TextureInputInterface<1>,
	public TextureOutputInterface,
	public GuiInterface,
	public NodeInterface
{
private:
	std::vector<std::string> m_MethodNames =
	{
		"r",
		"g",
		"b",
		"a",
		"length(rg)",
		"length(rgb)" ,
		"length(rga)" ,
		"median(rgb)"
	};
	struct UBO_Comp {
		alignas(4) int32_t method = 0;
		alignas(4) int32_t u_lap_offset = 1;
		alignas(4) float u_lap_corner = 0.2f;
		alignas(4) float u_discard_zero = 0.0f;
	} m_UBOComp;
	VulkanBufferObjectPtr m_UBOComp_Ptr = nullptr;
	vk::DescriptorBufferInfo m_UBOComp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };


public:
	LaplacianModule_Comp_2D_Pass(vkApi::VulkanCorePtr vVulkanCorePtr);
	~LaplacianModule_Comp_2D_Pass() override;

	void ActionBeforeInit() override;
	void WasJustResized() override;

	void Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;

	// Interfaces Setters
	void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize = nullptr) override;

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

	std::string GetComputeShaderCode(std::string& vOutShaderName) override;
};