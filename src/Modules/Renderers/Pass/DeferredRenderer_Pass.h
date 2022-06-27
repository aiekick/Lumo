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

#include <SceneGraph/SceneMesh.h>

#include <Interfaces/GuiInterface.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/ShaderInterface.h>
#include <Interfaces/TextureInputInterface.h>
#include <Interfaces/ResizerInterface.h>
#include <Interfaces/TextureOutputInterface.h>
#include <Interfaces/ShaderUpdateInterface.h>

class ShadowMapModule;
namespace vkApi { class VulkanCore; }
class DeferredRenderer_Pass :
	public QuadShaderPass,
	public GuiInterface,
	public TextureInputInterface<9U>,
	public TextureOutputInterface,
	public ShaderUpdateInterface
{
private:
	VulkanBufferObjectPtr m_UBO_Frag = nullptr;
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
	DeferredRenderer_Pass(vkApi::VulkanCore* vVulkanCore);
	~DeferredRenderer_Pass() override;

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint)  override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
	void UpdateShaders(const std::set<std::string>& vFiles) override;

private:
	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	std::string GetVertexShaderCode(std::string& vOutShaderName) override;
	std::string GetFragmentShaderCode(std::string& vOutShaderName) override;
};