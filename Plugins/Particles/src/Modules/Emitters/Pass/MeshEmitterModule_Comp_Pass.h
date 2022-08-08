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
#include <Interfaces/ModelInputInterface.h>
#include <Interfaces/ParticlesOutputInterface.h>

#include <SceneGraph/SceneParticles.h>

#include <Utils/GpuOnlyStorageBuffer.h>

class MeshEmitterModule_Comp_Pass :
	public ShaderPass,
	public GuiInterface,
	public NodeInterface,
	public ModelInputInterface,
	public ParticlesOutputInterface
{
private:
	SceneMeshWeak m_InputMesh;
	SceneParticlesPtr m_ParticlesPtr = nullptr;

	VulkanBufferObjectPtr m_UBOCompPtr = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo_Comp = { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	SceneParticles::CounterStruct* m_CountersPtr = nullptr;
	VkDrawIndexedIndirectCommand* m_IndexedIndirectCommandPtr = nullptr;

	struct UBOComp 
	{
		alignas(4) uint32_t max_particles_count = 100000U; // 100K by default
		alignas(4) uint32_t current_vertexs_count = 0U;
		alignas(4) float reset = 0.0f;
		alignas(4) uint32_t emission_count = 100U; // 100 by default
		alignas(4) float base_min_life = 1.0f;
		alignas(4) float base_max_life = 1.0f;
		alignas(4) float base_speed = 0.1f;
		alignas(4) float spawn_rate = 0.1f;
		alignas(4) float spawn_mass = 0.1f;
	} 
	m_UBOComp;

	struct m_SBOComp
	{
		uint32_t pending_emission_count = 0U;
	} m_SBOComp;

	struct PushConstants 
	{
		uint32_t pass_number = 0U;	// compute pass (reset, then emit)
		float absolute_time = 0.0f;
		float delta_time = 0.0f;
	} 
	m_PushConstants;

public:
	MeshEmitterModule_Comp_Pass(vkApi::VulkanCorePtr vVulkanCorePtr);
	~MeshEmitterModule_Comp_Pass() override;

	void ActionBeforeInit() override;
	void ActionAfterInitSucceed() override;
	void Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void SetModel(SceneModelWeak vSceneModel = SceneModelWeak()) override;
	SceneParticlesWeak GetParticles() override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

private:
	void ComputePass(vk::CommandBuffer* vCmd, const uint32_t& vPassNumber);

	bool BuildModel() override;
	void DestroyModel(const bool& vReleaseDatas = false) override;

	bool CreateUBO();
	void UploadUBO();
	void DestroyUBO();

	bool CanUpdateDescriptors() override;
	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	std::string GetComputeShaderCode(std::string& vOutShaderName) override;
};