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
#include <Interfaces/TexelBufferInputInterface.h>
#include <Interfaces/TexelBufferOutputInterface.h>
#include <Interfaces/LightGroupInputInterface.h>

class ParticlesSimulationModule_Pass :
	public ShaderPass,
	public GuiInterface,
	public NodeInterface,
	public TexelBufferInputInterface<1U>,
	public TexelBufferOutputInterface
{
private:
	struct PushConstants {
		float DeltaTime = 0.0f;
	} m_PushConstants;

public:
	ParticlesSimulationModule_Pass(vkApi::VulkanCorePtr vVulkanCorePtr);
	~ParticlesSimulationModule_Pass() override;

	void ActionBeforeInit() override;
	void ActionAfterInitSucceed() override;
	void Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void SetTexelBuffer(const uint32_t& vBinding, vk::Buffer* vTexelBuffer, ct::uvec2* vTexelBufferSize) override;
	void SetTexelBufferView(const uint32_t& vBinding, vk::BufferView* vTexelBufferView, ct::uvec2* vTexelBufferSize) override;
	vk::Buffer* GetTexelBuffer(const uint32_t& vBindingPoint, ct::uvec2* vOutSize = nullptr) override;
	vk::BufferView* GetTexelBufferView(const uint32_t& vBindingPoint, ct::uvec2* vOutSize = nullptr) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

protected:
	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	std::string GetComputeShaderCode(std::string& vOutShaderName) override;
};