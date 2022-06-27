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

#include <Base/ShaderPass.h>
#include <Base/BaseRenderer.h>

#include <vkFramework/Texture2D.h>
#include <vkFramework/ImGuiTexture.h>
#include <vkFramework/VulkanDevice.h>
#include <vkFramework/VulkanRessource.h>

#include <Interfaces/GuiInterface.h>
#include <Interfaces/NodeInterface.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/CameraInterface.h>
#include <Interfaces/ModelInputInterface.h>
#include <Interfaces/TextureInputInterface.h>
#include <Interfaces/TextureOutputInterface.h>
#include <Interfaces/ResizerInterface.h>
#include <Interfaces/ShaderUpdateInterface.h>

namespace vkApi { class VulkanCore; }

class MeshAttributesModule_Pass :
	public ShaderPass,
	public GuiInterface,
	public ModelInputInterface,
	public TextureInputInterface<1U>,
	public TextureOutputInterface,
	public ShaderUpdateInterface
{
private:
	VulkanBufferObjectPtr m_UBO_Vert = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo_Vert;

	struct UBOVert {
		alignas(16) glm::mat4x4 transform = glm::mat4x4(1.0f);
	} m_UBOVert;

	VulkanBufferObjectPtr m_UBO_Frag = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo_Frag;

	struct UBOFrag {
		alignas(4) float use_sampler_mask = 0.0f;
	} m_UBOFrag;

public:
	MeshAttributesModule_Pass(vkApi::VulkanCore* vVulkanCore);
	~MeshAttributesModule_Pass() override;

	void DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void SetModel(SceneModelWeak vSceneModel = SceneModelWeak()) override;
	void SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint)  override;
	void UpdateShaders(const std::set<std::string>& vFiles) override;

private:
	void DestroyModel(const bool& vReleaseDatas = false) override;

	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	void SetInputStateBeforePipelineCreation() override;

	std::string GetVertexShaderCode(std::string& vOutShaderName) override;
	std::string GetFragmentShaderCode(std::string& vOutShaderName) override;
};
