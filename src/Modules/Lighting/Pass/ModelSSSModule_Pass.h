/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <array>
#include <memory>
#include <ctools/ConfigAbstract.h>
#include <Interfaces/GuiInterface.h>

#include <Headers/Globals.h>
#include <vulkan/vulkan.hpp>
#include <vkFramework/vk_mem_alloc.h>
#include <ctools/cTools.h>
#include <Base/QuadShaderPass.h>
#include <ctools/ConfigAbstract.h>
#include <Interfaces/ShaderInterface.h>
#include <Interfaces/SerializationInterface.h>
#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanDevice.h>
#include <vkFramework/VulkanFrameBuffer.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/ImGuiTexture.h>
#include <SceneGraph/SceneMesh.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/TextureInputInterface.h>
#include <Interfaces/TextureOutputInterface.h>
#include <Interfaces/LightInputInterface.h>
#include <Interfaces/ShaderUpdateInterface.h>
#include <set>
#include <string>

namespace vkApi { class VulkanCore; }
class ModelSSSModule_Pass :
	public QuadShaderPass,
	public GuiInterface,
	public TextureInputInterface<2U>,
	public TextureOutputInterface,
	public LightInputInterface,
	public ShaderUpdateInterface
{
protected: // vulkan creation
	VulkanBufferObjectPtr m_UBO_Frag = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo_Frag;

	struct UBOFrag {
		alignas(16) glm::mat4 u_light_cam = glm::mat4(1.0f);
		alignas(4) float u_bias = 0.01f;
		alignas(4) float u_poisson_scale = 5000.0f;
		alignas(4) float use_sampler_pos = 0.0f;
		alignas(4) float use_sampler_shadow_map = 0.0f;
	} m_UBOFrag;

public:
	ModelSSSModule_Pass(vkApi::VulkanCore* vVulkanCore);
	~ModelSSSModule_Pass();

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint) override;
	void SetLightGroup(SceneLightGroupWeak vSceneLightGroup) override;
	void UpdateShaders(const std::set<std::string>& vFiles) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

	void SetLighViewMatrix(const glm::mat4& vLightViewMatrix);

private:
	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;
	void UpdateRessourceDescriptor() override;

	std::string GetVertexShaderCode(std::string& vOutShaderName) override;
	std::string GetFragmentShaderCode(std::string& vOutShaderName) override;
};