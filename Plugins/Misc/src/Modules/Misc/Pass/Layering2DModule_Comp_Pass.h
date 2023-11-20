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
#include <LumoBackend/Base/ShaderPass.h>

#include <Gaia/gaia.h>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>

#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/TextureInputInterface.h>
#include <LumoBackend/Interfaces/TextureOutputInterface.h>
#include <LumoBackend/Interfaces/LightGroupInputInterface.h>

class Layering2DModule_Comp_Pass :
	public ShaderPass,
	
	public TextureInputInterface<2U>,
	public TextureOutputInterface
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

	VulkanBufferObjectPtr m_UBOCompPtr = nullptr;
	vk::DescriptorBufferInfo m_UBOComp_BufferInfos = { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	struct UBOComp {
		alignas(4) int32_t method = 0;
		alignas(4) float smoothness = 0.5f;
		alignas(4) float use_input_color_buffer = 0.0f;
		alignas(4) uint32_t layer_count = 50U;
		alignas(4) float step_scale = 0.005f;
		alignas(4) float center_offset_step = 2.0f;
		alignas(8) ct::ivec2 image_size;
		alignas(8) ct::ivec2 center_offset;
	} m_UBOComp;

public:
	Layering2DModule_Comp_Pass(GaiApi::VulkanCorePtr vVulkanCorePtr);
	~Layering2DModule_Comp_Pass() override;

	void WasJustResized() override;
	void Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

protected:
	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	std::string GetComputeShaderCode(std::string& vOutShaderName) override;
};