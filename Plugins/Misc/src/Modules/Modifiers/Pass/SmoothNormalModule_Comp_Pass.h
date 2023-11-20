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
#include <LumoBackend/Interfaces/ModelInputInterface.h>
#include <LumoBackend/Interfaces/ModelOutputInterface.h>

class SmoothNormalModule_Comp_Pass : 
	public ShaderPass,
	public ModelInputInterface,
	public ModelOutputInterface
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
	SmoothNormalModule_Comp_Pass(GaiApi::VulkanCorePtr vVulkanCorePtr);
	~SmoothNormalModule_Comp_Pass() override;

	void ActionBeforeInit();
	void Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
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
