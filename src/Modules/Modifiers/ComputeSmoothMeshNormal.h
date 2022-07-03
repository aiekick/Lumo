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
#include <string>
#include <functional>
#include <ctools/cTools.h>
#include <ctools/cTools.h>
#include <vulkan/vulkan.hpp>
#include <Headers/Globals.h>
#include <vulkan/vulkan.hpp>
#include <Graph/Base/BaseNode.h>
#include <SceneGraph/SceneModel.h>
#include <ctools/ConfigAbstract.h>
#include <ctools/ConfigAbstract.h>
#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanCore.h>
#include <Interfaces/GuiInterface.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/NodeInterface.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/vk_mem_alloc.h>
#include <vkFramework/VulkanDevice.h>
#include <Interfaces/ModelInputInterface.h>
#include <Interfaces/ModelOutputInterface.h>
#include <Interfaces/CameraInterface.h>
#include <Interfaces/ComputeInterface.h>
#include <vkFramework/VulkanFrameBuffer.h>

#include <Utils/Mesh/VertexStruct.h>
#include <Interfaces/SerializationInterface.h>


class ComputeSmoothMeshNormal :
	public conf::ConfigAbstract,
	public NodeInterface,
	public ModelInputInterface,
	public ModelOutputInterface,
	public GuiInterface,
	public ComputeInterface,
	public TaskInterface
{
public:
	static std::shared_ptr<ComputeSmoothMeshNormal> Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode, ct::uvec3 vDispatchSize);

protected:
	ct::cWeak<ComputeSmoothMeshNormal> m_This;
	std::vector<uint32_t> m_SPIRV_Comp;
	std::string m_ComputeShaderCode;
	ct::uvec3 m_DispatchSize = 1U;
	bool m_NeedResize = false;
	bool m_Loaded = false;
	bool m_CanWeRender = true;
	bool m_JustReseted = false;
	bool m_FirstRender = true;
	uint32_t m_CurrentFrame = 0;
	uint32_t m_CountIterations = 1U;
	bool m_NeedNewUBOUpload = false;

protected: // mesh
	SceneMeshWeak m_InputMesh;
	SceneModelWeak m_InputModel;
	bool m_NeedModelUpdate = false;

protected: // vulkan creation
	vkApi::VulkanQueue m_Queue;
	vk::CommandPool m_CommandPool;
	vk::DescriptorPool m_DescriptorPool;
	vk::Device m_Device;

	// Submition
	std::vector<vk::Semaphore> m_RenderCompleteSemaphores;
	std::vector<vk::Fence> m_WaitFences;
	std::vector<vk::CommandBuffer> m_CommandBuffers;

	// ressources
	std::vector<vk::DescriptorSetLayoutBinding> m_LayoutBindings;
	vk::DescriptorSetLayout m_DescriptorSetLayout = {};
	vk::DescriptorSet m_DescriptorSet = {};
	std::vector<vk::WriteDescriptorSet> writeDescriptorSets;

	// m_Pipeline
	vk::PipelineLayout m_PipelineLayout = {};
	vk::Pipeline m_Pipeline = {};
	vk::PipelineCache m_PipelineCache = {};
	std::vector<vk::PipelineShaderStageCreateInfo> m_ShaderCreateInfos;
	std::vector<vk::PipelineColorBlendAttachmentState> m_BlendAttachmentStates;

	VulkanBufferObjectPtr m_UBO_Comp = nullptr;
	VulkanBufferObjectPtr m_SBO_Empty_Vertex_Input = nullptr;
	VulkanBufferObjectPtr m_SBO_Empty_Index_Input = nullptr;
	VulkanBufferObjectPtr m_SBO_Normals_Compute_Helper = nullptr;
	VulkanBufferObjectPtr m_SBO_Empty_Normals_Compute_Helper = nullptr;

	std::vector<int> m_NormalDatas; // 3 components

	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;

protected:
	struct PushConstants {
		uint32_t pass_number;
	} m_PushConstants;

public:
	ComputeSmoothMeshNormal(vkApi::VulkanCorePtr vVulkanCorePtr);
	virtual ~ComputeSmoothMeshNormal();

	bool Init(ct::uvec3 vDispatchSize = 1U);
	void Unit();

	void Resize();
	void Compute() override;
	bool Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer *vCmd = nullptr) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void SetModel(SceneModelWeak vSceneModel = SceneModelWeak()) override;
	SceneModelWeak GetModel() override;

private:
	void ComputePass(vk::CommandBuffer* vCmd, const uint32_t& vPassNumber);

	vk::CommandBuffer* GetCommandBuffer();
	void ResetCommandBuffer();
	void BeginComputeCommandBuffer();
	void EndCommandBuffer();

	void SubmitCompute();
	void Swap();
	void ResetFence();
	void WaitFence();

	bool CreateCommandBuffer();
	void DestroyCommandBuffer();

	bool CreateSyncObjects();
	void DestroySyncObjects();

	bool CreateSBO();
	void DestroySBO();

	bool CreateUBO();
	void UploadUBO_Comp();
	void DestroyUBO();

	bool BuildModel();
	void DestroyModel();
	void UpdateModel(bool vLoaded);

	bool CreateRessourceDescriptor();
	bool UpdateBufferInfoInRessourceDescriptor();
	void UpdateRessourceDescriptor();
	void DestroyRessourceDescriptor();

	bool CreateComputePipeline();
	void DestroyPipeline();

	bool CompilCode();
	std::string GetComputeShaderCode();
	bool SetOrUpdateComputeCode(const std::string& vCode);

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};