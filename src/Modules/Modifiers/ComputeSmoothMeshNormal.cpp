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

#include "ComputeSmoothMeshNormal.h"
#include <utility>
#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <FontIcons/CustomFont.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanSubmitter.h>

using namespace vkApi;

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

#define COUNT_BUFFERS 2

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<ComputeSmoothMeshNormal> ComputeSmoothMeshNormal::Create(vkApi::VulkanCore* vVulkanCore, BaseNodeWeak vParentNode, ct::uvec3 vDispatchSize)
{
	ZoneScoped;

	auto res = std::make_shared<ComputeSmoothMeshNormal>(vVulkanCore);
	res->m_This = res;
	res->SetParentNode(vParentNode);
	if (!res->Init(vDispatchSize))
	{
		res.reset();
	}
	return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

ComputeSmoothMeshNormal::ComputeSmoothMeshNormal(vkApi::VulkanCore* vVulkanCore)
	: m_VulkanCore(vVulkanCore)
{

}

ComputeSmoothMeshNormal::~ComputeSmoothMeshNormal()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// PUBLIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ComputeSmoothMeshNormal::Init(ct::uvec3 vDispatchSize)
{
	ZoneScoped;

	m_Loaded = false;

	ct::uvec3 dispatchSize = ct::maxi<ct::uvec3>(vDispatchSize, 1u);
	if (!dispatchSize.emptyOR())
	{
		m_Device = m_VulkanCore->getDevice();
		m_Queue = m_VulkanCore->getQueue(vk::QueueFlagBits::eGraphics);
		m_DescriptorPool = m_VulkanCore->getDescriptorPool();
		m_CommandPool = m_Queue.cmdPools;

		m_DispatchSize = vDispatchSize;

		if (CompilCode()) {
			if (CreateCommandBuffer()) {
				if (CreateSyncObjects()) {
					if (BuildModel()) {
						if (CreateSBO()) {
							if (CreateUBO()) {
								if (CreateRessourceDescriptor()) {
									if (CreateComputePipeline()) {
										m_Loaded = true;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return m_Loaded;
}

void ComputeSmoothMeshNormal::Unit()
{
	ZoneScoped;

	m_Device.waitIdle();

	DestroyPipeline();
	DestroyRessourceDescriptor();
	DestroyUBO();
	DestroySBO();
	DestroyModel();
	DestroySyncObjects();
	DestroyCommandBuffer();

	m_InputMesh.reset();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ComputeSmoothMeshNormal::Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer *vCmd)
{
	return false;
}

void ComputeSmoothMeshNormal::Resize()
{
	m_NeedResize = false;

	ZoneScoped;

	if (!m_Loaded) return;

	//	le storage peut avoir changé et necessiter une maj des descripteurs
	if (!m_DispatchSize.emptyAND())
	{
		// on peut pas faire ca car on veut resize que un buffer en particulier
		// et comme dans le cas du compute on a un buffer dasn chaque uniform retailler en donnant l'indice ne rime a rien
		// une fois dans l'uniform on ne pourra pas distinguer qui est l'index
		// m_UniformWidgets.Resize(m_BufferIdToResize,
		// ct::uvec2((uint32_t)vNewSize.x, (uint32_t)vNewSize.y), m_This);

		/*m_UniformWidgets.UpdateBuffers(m_This); // update storage

		bool res = UpdateBufferInfoInRessourceDescriptor();
		if (res)
		{
			UpdateRessourceDescriptor();
		}*/
	}
}

void ComputeSmoothMeshNormal::Compute()
{
	if (!m_Loaded) return;

	if (m_NeedResize)
	{
		Resize();
	}

	UpdateModel(m_Loaded);

	if (m_InputMesh.expired()) return;

	if (m_CanWeRender || m_JustReseted)
	{
		auto meshPtr = m_InputMesh.getValidShared();
		if (meshPtr)
		{
			m_Device.resetFences(1, &m_WaitFences[m_CurrentFrame]);
			auto cmd = GetCommandBuffer();
			if (cmd)
			{
				// use back buffer
				UpdateRessourceDescriptor();

				ResetCommandBuffer();
				BeginComputeCommandBuffer();

				assert(!m_This.expired());

				cmd->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipeline);

				cmd->pipelineBarrier(
					vk::PipelineStageFlagBits::eVertexInput,
					vk::PipelineStageFlagBits::eComputeShader,
					vk::DependencyFlags(),
					nullptr,
					nullptr,
					nullptr);

				// le dispatch c'est les indices sur 3
				m_DispatchSize.x = meshPtr->GetIndicesCount() / 3U / 8U;
				ComputePass(cmd, 0U); // add vertex normals

				// le dispatch c'est le nombre de vertexs
				m_DispatchSize.x = meshPtr->GetVerticesCount() / 8U;
				ComputePass(cmd, 1U); // normalize vertex normals

				cmd->pipelineBarrier(
					vk::PipelineStageFlagBits::eComputeShader,
					vk::PipelineStageFlagBits::eVertexInput,
					vk::DependencyFlags(),
					nullptr,
					nullptr,
					nullptr);

				EndCommandBuffer();

				SubmitCompute();

				WaitFence();

				Swap();
			}
		}
	}
}

void ComputeSmoothMeshNormal::ComputePass(vk::CommandBuffer* vCmd, const uint32_t& vPassNumber)
{
	vCmd->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_PipelineLayout, 0, m_DescriptorSet, nullptr);

	m_PushConstants.pass_number = vPassNumber;
	vCmd->pushConstants(m_PipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(PushConstants), &m_PushConstants);

	vCmd->dispatch(m_DispatchSize.x, m_DispatchSize.y, m_DispatchSize.z);
}

void ComputeSmoothMeshNormal::ResetFence()
{
	ZoneScoped;

	if (!m_Loaded) return;

	m_Device.resetFences(1, &m_WaitFences[m_CurrentFrame]);
}

void ComputeSmoothMeshNormal::WaitFence()
{
	ZoneScoped;

	if (!m_Loaded) return;

	m_Device.waitForFences(1, &m_WaitFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
}

vk::CommandBuffer* ComputeSmoothMeshNormal::GetCommandBuffer()
{
	return &m_CommandBuffers[m_CurrentFrame];
}

void ComputeSmoothMeshNormal::ResetCommandBuffer()
{
	auto cmd = GetCommandBuffer();
	cmd->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
}

void ComputeSmoothMeshNormal::BeginComputeCommandBuffer()
{
	auto cmd = GetCommandBuffer();
	cmd->begin(vk::CommandBufferBeginInfo());

	{
		TracyVkZone(m_TracyContext, *cmd, "Record Compute Command buffer");
	}
}

void ComputeSmoothMeshNormal::EndCommandBuffer()
{
	auto cmd = GetCommandBuffer();
	if (cmd)
	{
		{
			TracyVkCollect(m_TracyContext, *cmd);
		}
		cmd->end();
	}
}

void ComputeSmoothMeshNormal::SubmitCompute()
{
	ZoneScoped;

	if (!m_Loaded) return;

	vk::SubmitInfo submitInfo;
	vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eComputeShader;
	submitInfo
		.setPWaitDstStageMask(&waitDstStageMask)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&m_CommandBuffers[m_CurrentFrame])
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(&m_RenderCompleteSemaphores[0]);

	if (!m_FirstRender)
	{
		submitInfo
			.setWaitSemaphoreCount(1)
			.setPWaitSemaphores(&m_RenderCompleteSemaphores[0]);
	}
	else
	{
		m_NeedNewUBOUpload = true;
		m_FirstRender = false;
	}

	VulkanSubmitter::Submit(m_VulkanCore, vk::QueueFlagBits::eCompute, submitInfo, m_WaitFences[m_CurrentFrame]);
}

void ComputeSmoothMeshNormal::Swap()
{
	ZoneScoped;

	if (!m_Loaded) return;

	m_CurrentFrame = 1 - m_CurrentFrame;

	m_JustReseted = false;
}

bool ComputeSmoothMeshNormal::SetOrUpdateComputeCode(const std::string& vCode)
{
	bool res = false;

	ZoneScoped;

	return res;
}

bool ComputeSmoothMeshNormal::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	bool change = false;

	auto meshPtr = m_InputMesh.getValidShared();
	if (meshPtr)
	{
		ImGui::Header("Infos");

		ImGui::Text("Mesh Vertice Count : %u", (uint32_t)meshPtr->GetVerticesCount());
		if (meshPtr->HasIndices())
		{
			ImGui::Text("Mesh Indice Count : %u", (uint32_t)meshPtr->GetIndicesCount());
		}

		ImGui::Header("Attibutes");

		ImGui::Text("[%s] Normals", meshPtr->HasNormals() ? "x" : " ");
		ImGui::Text("[%s] Tangeants", meshPtr->HasTangeants() ? "x" : " ");
		ImGui::Text("[%s] BiTangeants", meshPtr->HasBiTangeants() ? "x" : " ");
		ImGui::Text("[%s] TextureCoords", meshPtr->HasTextureCoords() ? "x" : " ");
		ImGui::Text("[%s] VertexColors", meshPtr->HasVertexColors() ? "x" : " ");
		ImGui::Text("[%s] Indices", meshPtr->HasIndices() ? "x" : " ");
	}

	if (change)
	{
		m_NeedNewUBOUpload = true;
	}

	return change;
}

void ComputeSmoothMeshNormal::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;
}

void ComputeSmoothMeshNormal::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;
}

void ComputeSmoothMeshNormal::SetModel(SceneModelWeak vSceneModel)
{
	ZoneScoped;

	m_InputModel = vSceneModel;

	auto modelPtr = m_InputModel.getValidShared();
	if (modelPtr && !modelPtr->empty())
	{
		// only take the first messh
		// sine MeshSIm support only one fully connected mesh
		m_InputMesh = modelPtr->Get(0);
	}
	else
	{
		m_InputMesh.reset();
	}

	m_NeedModelUpdate = true;
}

SceneModelWeak ComputeSmoothMeshNormal::GetModel()
{
	ZoneScoped;

	return m_InputModel;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / COMMANDBUFFER ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ComputeSmoothMeshNormal::CreateCommandBuffer()
{
	ZoneScoped;

	m_CommandBuffers = m_Device.allocateCommandBuffers(
		vk::CommandBufferAllocateInfo(
			m_CommandPool,
			vk::CommandBufferLevel::ePrimary,
			COUNT_BUFFERS
		)
	);

	return true;
}

void ComputeSmoothMeshNormal::DestroyCommandBuffer()
{
	ZoneScoped;

	if (!m_CommandBuffers.empty())
	{
		m_Device.freeCommandBuffers(m_CommandPool, m_CommandBuffers);
		m_CommandBuffers.clear();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / SYNC OBJECTS ////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ComputeSmoothMeshNormal::CreateSyncObjects()
{
	ZoneScoped;

	m_RenderCompleteSemaphores.resize(COUNT_BUFFERS);
	m_WaitFences.resize(COUNT_BUFFERS);
	for (size_t i = 0; i < COUNT_BUFFERS; ++i)
	{
		m_RenderCompleteSemaphores[i] = m_Device.createSemaphore(vk::SemaphoreCreateInfo());
		m_WaitFences[i] = m_Device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
	}

	return true;
}

void ComputeSmoothMeshNormal::DestroySyncObjects()
{
	ZoneScoped;

	for (size_t i = 0; i < COUNT_BUFFERS; ++i)
	{
		if (i < m_RenderCompleteSemaphores.size())
			m_Device.destroySemaphore(m_RenderCompleteSemaphores[i]);
		if (i < m_WaitFences.size())
			m_Device.destroyFence(m_WaitFences[i]);
	}

	m_RenderCompleteSemaphores.clear();
	m_WaitFences.clear();
}

bool ComputeSmoothMeshNormal::CreateSBO()
{
	ZoneScoped;

	const auto vertice_size = sizeof(VertexStruct::P3_N3_TA3_BTA3_T2_C4);
	m_SBO_Empty_Vertex_Input = VulkanRessource::createStorageBufferObject(m_VulkanCore, vertice_size, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);

	const auto index_size = sizeof(VertexStruct::I1);
	m_SBO_Empty_Index_Input = VulkanRessource::createStorageBufferObject(m_VulkanCore, index_size, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);

	const auto normal_size = sizeof(ct::ivec3);
	m_SBO_Empty_Normals_Compute_Helper = VulkanRessource::createStorageBufferObject(m_VulkanCore, normal_size, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);

	return 
		m_SBO_Empty_Vertex_Input != nullptr &&
		m_SBO_Empty_Index_Input != nullptr &&
		m_SBO_Empty_Normals_Compute_Helper != nullptr;
}

void ComputeSmoothMeshNormal::DestroySBO()
{
	ZoneScoped;

	m_SBO_Empty_Vertex_Input.reset();
	m_SBO_Empty_Index_Input.reset();
	m_SBO_Empty_Normals_Compute_Helper.reset();
}

bool ComputeSmoothMeshNormal::CreateUBO()
{
	ZoneScoped;

	//m_UBO_Comp = VulkanRessource::createUniformBufferObject(sizeof(UBOComp));
	//m_DescriptorBufferInfo_Comp.buffer = m_UBO_Comp->buffer;
	//m_DescriptorBufferInfo_Comp.range = sizeof(UBOComp);
	//m_DescriptorBufferInfo_Comp.offset = 0;

	m_NeedNewUBOUpload = true;

	return true;
}

void ComputeSmoothMeshNormal::UploadUBO_Comp()
{
	ZoneScoped;

	//VulkanRessource::upload(*m_UBO_Comp, &m_UBOComp, sizeof(UBOComp));
}

void ComputeSmoothMeshNormal::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Comp.reset();
}

bool ComputeSmoothMeshNormal::BuildModel()
{
	auto meshPtr = m_InputMesh.getValidShared();
	if (meshPtr)
	{
		m_SBO_Normals_Compute_Helper.reset();
		
		m_NormalDatas.clear();
		m_NormalDatas.resize(meshPtr->GetVerticesCount() * 3U);
		const auto sizeInBytes = sizeof(int) * m_NormalDatas.size();
		memset(m_NormalDatas.data(), 0U, sizeInBytes);
		m_SBO_Normals_Compute_Helper = VulkanRessource::createGPUOnlyStorageBufferObject(m_VulkanCore, m_NormalDatas.data(), sizeInBytes);
	}

	return true;
}

void ComputeSmoothMeshNormal::DestroyModel()
{
	
}

void ComputeSmoothMeshNormal::UpdateModel(bool vLoaded)
{
	ZoneScoped;

	if (m_NeedModelUpdate)
	{
		if (vLoaded)
		{
			DestroyModel();
		}

		BuildModel();

		UpdateBufferInfoInRessourceDescriptor();

		m_NeedModelUpdate = false;
	}
}

bool ComputeSmoothMeshNormal::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();

	auto outputMeshPtr = m_InputMesh.getValidShared();
	if (outputMeshPtr && outputMeshPtr->GetVerticesBufferInfo()->range > 0U)
	{
		// VertexStruct::P3_N3_TA3_BTA3_T2_C4
		writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, outputMeshPtr->GetVerticesBufferInfo());
		// VertexStruct::I1
		writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, outputMeshPtr->GetIndicesBufferInfo());
		// Normals
		writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &m_SBO_Normals_Compute_Helper->bufferInfo);
	}
	else
	{
		// empty version, almost empty because his size is thr size of 1 VertexStruct::P3_N3_TA3_BTA3_T2_C4
		writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &m_SBO_Empty_Vertex_Input->bufferInfo);
		// empty version, almost empty because his size is thr size of 1 VertexStruct::I1
		writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &m_SBO_Empty_Index_Input->bufferInfo);
		// empty version, almost empty because his size is thr size of 1 uvec3
		writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, &m_SBO_Empty_Normals_Compute_Helper->bufferInfo);
	}

	return true;
}

bool ComputeSmoothMeshNormal::CreateRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);

	m_DescriptorSetLayout =
		m_Device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(
			vk::DescriptorSetLayoutCreateFlags(),
			static_cast<uint32_t>(m_LayoutBindings.size()),
			m_LayoutBindings.data()
		));

	m_DescriptorSet =
		m_Device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo(
			m_DescriptorPool, 1, &m_DescriptorSetLayout))[0];

	if (UpdateBufferInfoInRessourceDescriptor())
	{
		UpdateRessourceDescriptor();
	}

	return true;
}

void ComputeSmoothMeshNormal::UpdateRessourceDescriptor()
{
	ZoneScoped;

	m_Device.waitIdle();

	// il s'agit de maj les buffer info
	// et ca doit ce faire en dehors d'un renderpass
	// ce qui peu etre fait dans un renderpass c'est juste de la maj de data,
	// mais pas des switch de buffer infos

	// update samplers outside of the command buffer
	assert(!m_This.expired());
	//m_UniformWidgets.UpdateSamplers(m_This);
	//m_UniformWidgets.UpdateBuffers(m_This);
	//m_UniformWidgets.UpdateBlocks(m_This, true);

	if (m_NeedNewUBOUpload)
	{
		UploadUBO_Comp();
		m_NeedNewUBOUpload = false;
	}

	// update descriptor
	m_Device.updateDescriptorSets(writeDescriptorSets, nullptr);

	// on le met la avant le rendu plutot qu'apres sinon au reload la 1ere
	// frame vaut 0 et ca peut reset le shader selon ce qu'on a mis dedans
	// par contre on incremente la frame apres le submit
	//m_UniformWidgets.SetFrame(m_Frame);
}

void ComputeSmoothMeshNormal::DestroyRessourceDescriptor()
{
	ZoneScoped;

	m_Device.waitIdle();

	m_Device.freeDescriptorSets(m_DescriptorPool, m_DescriptorSet);
	m_Device.destroyDescriptorSetLayout(m_DescriptorSetLayout);
	m_DescriptorSet = vk::DescriptorSet{};
	m_DescriptorSetLayout = vk::DescriptorSetLayout{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / PIPELINE ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ComputeSmoothMeshNormal::CreateComputePipeline()
{
	ZoneScoped;

	if (m_SPIRV_Comp.empty()) return false;

	
	if (vkApi::VulkanCore::sVulkanShader)
	{
		vk::PushConstantRange push_constant;
		push_constant.offset = 0;
		push_constant.size = sizeof(PushConstants);
		push_constant.stageFlags = vk::ShaderStageFlagBits::eCompute;

		m_PipelineLayout =
			m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
				vk::PipelineLayoutCreateFlags(),
				1, &m_DescriptorSetLayout,
				1, &push_constant
			));

		auto cs = vkApi::VulkanCore::sVulkanShader->CreateShaderModule((VkDevice)m_Device, m_SPIRV_Comp);

		m_ShaderCreateInfos.clear();
		m_ShaderCreateInfos.resize(1);
		m_ShaderCreateInfos[0] = vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eCompute,
			(vk::ShaderModule)cs, "main"
		);

		vk::ComputePipelineCreateInfo computePipeInfo = vk::ComputePipelineCreateInfo()
			.setStage(m_ShaderCreateInfos[0]).setLayout(m_PipelineLayout);
		m_Pipeline = m_Device.createComputePipeline(nullptr, computePipeInfo).value;

		vkApi::VulkanCore::sVulkanShader->DestroyShaderModule((VkDevice)m_Device, cs);

		return true;
	}
	return false;
}

void ComputeSmoothMeshNormal::DestroyPipeline()
{
	ZoneScoped;

	m_Device.destroyPipelineLayout(m_PipelineLayout);
	m_Device.destroyPipeline(m_Pipeline);
	if (m_PipelineCache)
		m_Device.destroyPipelineCache(m_PipelineCache);
	m_PipelineLayout = vk::PipelineLayout{};
	m_Pipeline = vk::Pipeline{};
	m_PipelineCache = vk::PipelineCache{};
}

bool ComputeSmoothMeshNormal::CompilCode()
{
	ZoneScoped;

	bool res = false;

	auto compCode = GetComputeShaderCode();

	FileHelper::Instance()->SaveStringToFile(compCode, "debug/ComputeSmoothMeshNormal_Compute.glsl");

	if (!compCode.empty())
	{
		
		if (vkApi::VulkanCore::sVulkanShader)
		{
			m_SPIRV_Comp = vkApi::VulkanCore::sVulkanShader->CompileGLSLString(compCode, "comp", "masterRenderer");

			if (!m_Loaded)
			{
				res = true;
			}
			else if (!m_SPIRV_Comp.empty())
			{
				m_Device.waitIdle();
				DestroyPipeline();
				DestroyRessourceDescriptor();
				DestroyUBO();
				CreateUBO();
				CreateRessourceDescriptor();
				CreateComputePipeline();
				res = true;
			}
		}
	}

	return res;
}

std::string ComputeSmoothMeshNormal::GetComputeShaderCode()
{
	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

struct V3N3T3B3T2C4 
{
	float px, py, pz;
	float nx, ny, nz;
	float tax, tay, taz;
	float btax, btay, btaz;
	float tx, ty;
	float cx, cy, cz, cw;
};

layout(std430, binding = 0) buffer Vertices
{
	V3N3T3B3T2C4 vertices[];
};

layout(std430, binding = 1) buffer Indices
{
	uint indices[];
};

layout(std430, binding = 2) buffer Normals
{
	int normals[];
};

layout(push_constant) uniform constants
{
	uint pass_number;
};

// count indexs
void first_pass_sum_normals(const in uint id)
{
	const uint i0 = indices[id * 3 + 0];
	const uint i1 = indices[id * 3 + 1];
	const uint i2 = indices[id * 3 + 2];

	const V3N3T3B3T2C4 v0 = vertices[i0];
	const V3N3T3B3T2C4 v1 = vertices[i1];
	const V3N3T3B3T2C4 v2 = vertices[i2];

	const vec3 p0 = vec3(v0.px, v0.py, v0.pz);
	const vec3 p1 = vec3(v1.px, v1.py, v1.pz);
	const vec3 p2 = vec3(v2.px, v2.py, v2.pz);

	// cross product of two edges
	const vec3 n = cross(normalize(p1 - p0), normalize(p2 - p0));
	const int i_n_x = int(n.x * 100000.0);
	const int i_n_y = int(n.y * 100000.0);
	const int i_n_z = int(n.z * 100000.0);
	
	// exclusive add float to int
	atomicAdd(normals[i0 * 3], i_n_x);
	atomicAdd(normals[i0 * 3 + 1], i_n_y);
	atomicAdd(normals[i0 * 3 + 2], i_n_z);

	atomicAdd(normals[i1 * 3], i_n_x);
	atomicAdd(normals[i1 * 3 + 1], i_n_y);
	atomicAdd(normals[i1 * 3 + 2], i_n_z);
	
	atomicAdd(normals[i2 * 3], i_n_x);
	atomicAdd(normals[i2 * 3 + 1], i_n_y);
	atomicAdd(normals[i2 * 3 + 2], i_n_z);
}

// count vertexs
void second_pass_normalize_normals(const in uint id)
{
	// only one thread on i0
	// not atomic needed here

	const float nx = float(normals[id * 3]);
	const float ny = float(normals[id * 3 + 1]);
	const float nz = float(normals[id * 3 + 2]);
	const vec3 n = normalize(vec3(nx, ny, nz));
		
	vertices[id].nx = n.x;
	vertices[id].ny = n.y;
	vertices[id].nz = n.z;

	normals[id * 3] = 0;	
	normals[id * 3 + 1] = 0;
	normals[id * 3 + 2] = 0;
}

void main()
{
	const uint id = gl_GlobalInvocationID.x;
	switch(pass_number)
	{
	case 0:
		first_pass_sum_normals(id);
		break;
	case 1:
		second_pass_normalize_normals(id);
		break;
	default:
		break;
	}
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ComputeSmoothMeshNormal::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	//str += vOffset + "<compute_mesh_normal>\n";

	//str += vOffset + "</compute_mesh_normal>\n";

	return str;
}

bool ComputeSmoothMeshNormal::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
{
	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strParentName == "compute_mesh_normal")
	{

	}

	return true;
}
