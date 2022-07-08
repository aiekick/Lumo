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
#include <Interfaces/ModelInputInterface.h>
#include <Interfaces/ModelOutputInterface.h>

class SmoothNormalModule_Pass : 
	public ShaderPass,
	public ModelInputInterface,
	public ModelOutputInterface,
	public GuiInterface
{
private:
	SceneMeshWeak m_InputMesh;

	VulkanBufferObjectPtr m_SBO_Normals_Compute_Helper = nullptr;
	vk::DescriptorBufferInfo m_SBO_Normals_Compute_Helper_BufferInfos = { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	std::vector<int> m_NormalDatas; // 3 components

	struct PushConstants {
		uint32_t pass_number;
	} m_PushConstants;

public:
	SmoothNormalModule_Pass(vkApi::VulkanCorePtr vVulkanCorePtr);
	virtual ~SmoothNormalModule_Pass();

	void ActionBeforeInit();
	void Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void SetModel(SceneModelWeak vSceneModel = SceneModelWeak()) override;
	SceneModelWeak GetModel() override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

protected:
	bool BuildModel() override;
	void DestroyModel(const bool& vReleaseDatas = false) override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	std::string GetComputeShaderCode(std::string& vOutShaderName) override;

private:
	void ComputePass(vk::CommandBuffer* vCmd, const uint32_t& vPassNumber);

};
