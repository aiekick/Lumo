// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include <SceneGraph/SceneParticles.h>
#include <vkFramework/VulkanCore.h>

///////////////////////////////////////////////////////
//// STATIC ///////////////////////////////////////////
///////////////////////////////////////////////////////

SceneParticlesPtr SceneParticles::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<SceneParticles>(vVulkanCorePtr);
	res->m_This = res;
	return res;
}

///////////////////////////////////////////////////////
//// STATIC : SBO /////////////////////////////////////
///////////////////////////////////////////////////////

std::string SceneParticles::GetParticlesDatasBufferHeader(const uint32_t& vStartBindingPoint)
{
	return ct::toStr(u8R"(
struct Particle
{
	vec4 pos3_mass1;		// xyz::pos, w:mass
	vec4 dir3_speed1;		// xyz:direction, w:speed
	vec4 color4;			// rgba:color
	vec2 life1_max_life1;	// x:life, y:max_life
};

layout(std430, binding = %u) buffer SBO_ParticleDatas
{
	Particle particleDatas[];
};
)", vStartBindingPoint);
}

std::string SceneParticles::GetParticlesVertexInputBufferHeader()
{
	return u8R"(
layout(location = 0) in vec4 particle_pos3_mass1;
layout(location = 1) in vec4 particle_dir3_speed1;
layout(location = 2) in vec4 particle_color4;
layout(location = 3) in vec2 particle_life1_max_life1;
)";
}

VertexStruct::PipelineVertexInputState SceneParticles::GetInputStateBeforePipelineCreation()
{
	ZoneScoped;

	VertexStruct::PipelineVertexInputState inputState = VertexStruct::PipelineVertexInputState{};

	inputState.binding.binding = 0;
	inputState.binding.stride = sizeof(ParticleStruct);
	inputState.binding.inputRate = vk::VertexInputRate::eVertex;

	uint32_t offset = 0;

	// xyz::pos, w:mass
	// xyz:direction, w:speed
	// rgba:color
	// x:life, y:max_life

	inputState.attributes.resize(4);

	{
		// xyz::pos, w:mass
		auto& attrib = inputState.attributes[0];
		attrib.binding = 0;
		attrib.location = 0;
		attrib.format = vk::Format::eR32G32B32A32Sfloat;
		attrib.offset = offsetof(ParticleStruct, pos3_mass1);
	}

	{
		// xyz:direction, w:speed
		auto& attrib = inputState.attributes[1];
		attrib.binding = 0;
		attrib.location = 1;
		attrib.format = vk::Format::eR32G32B32A32Sfloat;
		attrib.offset = offsetof(ParticleStruct, dir3_speed1);
	}

	{
		// rgba:color
		auto& attrib = inputState.attributes[2];
		attrib.binding = 0;
		attrib.location = 2;
		attrib.format = vk::Format::eR32G32B32A32Sfloat;
		attrib.offset = offsetof(ParticleStruct, color4);
	}

	{
		// x:life, y:max_life
		auto& attrib = inputState.attributes[3];
		attrib.binding = 0;
		attrib.location = 3;
		attrib.format = vk::Format::eR32G32Sfloat;
		attrib.offset = offsetof(ParticleStruct, life1_max_life1);
	}

	inputState.state = vk::PipelineVertexInputStateCreateInfo(
		vk::PipelineVertexInputStateCreateFlags(),
		1,
		&inputState.binding,
		static_cast<uint32_t>(inputState.attributes.size()),
		inputState.attributes.data()
	);

	return inputState;
}

std::string SceneParticles::GetAliveParticlesIndexBufferHeader(const uint32_t& vStartBindingPoint)
{
	return ct::toStr(u8R"(
layout(std430, binding = %u) buffer SBO_AlivePreSimParticleDatas
{
	uint alive_pre_sim_buffer[];
};
)", vStartBindingPoint);
}

std::string SceneParticles::GetCounterBufferHeader(const uint32_t& vStartBindingPoint)
{
	return ct::toStr(u8R"(
layout(std430, binding = %u) buffer SBO_AtomicCounters
{
	uint alive_particles_count;
	uint pending_emission_count;
};
)", vStartBindingPoint);
}

std::string SceneParticles::GetDrawIndirectCommandHeader(const uint32_t& vStartBindingPoint)
{
	return ct::toStr(u8R"(
layout(std430, binding = %u) buffer SBO_DrawIndexedIndirectCommand
{
	uint indexCount;
	uint instanceCount;
	uint firstIndex;
	int vertexOffset;
	uint firstInstance;
};
)", vStartBindingPoint);
}

///////////////////////////////////////////////////////
//// PUBLIC : CTOR / DTOR /////////////////////////////
///////////////////////////////////////////////////////

SceneParticles::SceneParticles(vkApi::VulkanCorePtr vVulkanCorePtr)
	: m_VulkanCorePtr(vVulkanCorePtr)
{
	m_ParticlesDatasBufferPtr = GpuOnlyStorageBuffer::Create(m_VulkanCorePtr);
	m_AliveParticlesIndexBufferPtr = GpuOnlyStorageBuffer::Create(m_VulkanCorePtr);
	m_CountersBufferPtr = GpuOnlyStorageBuffer::Create(m_VulkanCorePtr);
	m_DrawArraysIndirectCommandBufferPtr = GpuOnlyStorageBuffer::Create(m_VulkanCorePtr);
}

SceneParticles::~SceneParticles()
{
	Destroy();
}

///////////////////////////////////////////////////////
//// SBO //////////////////////////////////////////////
///////////////////////////////////////////////////////

bool SceneParticles::IsOk()
{
	return 
		(m_ParticlesDatasBufferPtr != nullptr) &&
		(m_AliveParticlesIndexBufferPtr != nullptr) &&
		(m_CountersBufferPtr != nullptr) &&
		(m_DrawArraysIndirectCommandBufferPtr != nullptr)
	;
}


bool SceneParticles::Build(const uint32_t& vPaticlesMaxCount)
{
	if (vPaticlesMaxCount)
	{
		if (m_ParticlesDatasBufferPtr &&
			m_ParticlesDatasBufferPtr->CreateBuffer(
				sizeof(ParticleStruct), vPaticlesMaxCount,
				VMA_MEMORY_USAGE_GPU_ONLY,
				vk::BufferUsageFlagBits::eVertexBuffer)) {
			if (m_AliveParticlesIndexBufferPtr &&
				m_AliveParticlesIndexBufferPtr->CreateBuffer(
				sizeof(uint32_t), vPaticlesMaxCount,
					VMA_MEMORY_USAGE_GPU_ONLY,
					vk::BufferUsageFlagBits::eIndexBuffer)) {
				// for this one we will need to get on cpu side the counters for render the next frame of the simulator
				// for limit the rendering to the alive particles only
				if (m_CountersBufferPtr &&
					m_CountersBufferPtr->CreateBuffer(
						sizeof(CounterStruct), 1U,
						VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_TO_CPU)) {
					if (m_DrawArraysIndirectCommandBufferPtr &&
						m_DrawArraysIndirectCommandBufferPtr->CreateBuffer(
							sizeof(VkDrawIndexedIndirectCommand), 1U,
							VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_TO_CPU,
							vk::BufferUsageFlagBits::eIndirectBuffer)) {
						return true;
					}
				}
			}
		}
	}

	DestroyBuffers();

	return false;
}

void SceneParticles::Destroy()
{
	m_ParticlesDatasBufferPtr.reset();
	m_AliveParticlesIndexBufferPtr.reset();
	m_CountersBufferPtr.reset();
	m_DrawArraysIndirectCommandBufferPtr.reset();
}

void SceneParticles::DestroyBuffers()
{
	if (m_ParticlesDatasBufferPtr)
		m_ParticlesDatasBufferPtr->DestroyBuffer();
	if (m_AliveParticlesIndexBufferPtr)
		m_AliveParticlesIndexBufferPtr->DestroyBuffer();
	if (m_CountersBufferPtr)
		m_CountersBufferPtr->DestroyBuffer();
	if (m_DrawArraysIndirectCommandBufferPtr)
		m_DrawArraysIndirectCommandBufferPtr->DestroyBuffer();
}

vk::DescriptorBufferInfo* SceneParticles::GetParticlesDatasBufferInfo()
{
	if (m_ParticlesDatasBufferPtr)
	{
		return m_ParticlesDatasBufferPtr->GetBufferInfo();
	}

	return &m_EmptyDescriptorBufferInfo;
}

vk::DescriptorBufferInfo* SceneParticles::GetAliveParticlesIndexBufferInfo()
{
	if (m_AliveParticlesIndexBufferPtr)
	{
		return m_AliveParticlesIndexBufferPtr->GetBufferInfo();
	}

	return &m_EmptyDescriptorBufferInfo;
}

vk::DescriptorBufferInfo* SceneParticles::GetCountersBufferInfo()
{
	if (m_CountersBufferPtr)
	{
		return m_CountersBufferPtr->GetBufferInfo();
	}

	return &m_EmptyDescriptorBufferInfo;
}

vk::DescriptorBufferInfo* SceneParticles::GetDrawIndirectCommandBufferInfo()
{
	if (m_DrawArraysIndirectCommandBufferPtr)
	{
		return m_DrawArraysIndirectCommandBufferPtr->GetBufferInfo();
	}

	return &m_EmptyDescriptorBufferInfo;
}

vk::Buffer* SceneParticles::GetParticlesVertexInputBuffer()
{
	if (m_ParticlesDatasBufferPtr)
	{
		return m_ParticlesDatasBufferPtr->GetVulkanBuffer();
	}

	return nullptr;
}

vk::Buffer* SceneParticles::GetAliveParticlesIndexInputBuffer()
{
	if (m_AliveParticlesIndexBufferPtr)
	{
		return m_AliveParticlesIndexBufferPtr->GetVulkanBuffer();
	}

	return nullptr;
}

vk::Buffer* SceneParticles::GetDrawIndirectCommandBuffer()
{
	if (m_DrawArraysIndirectCommandBufferPtr)
	{
		return m_DrawArraysIndirectCommandBufferPtr->GetVulkanBuffer();
	}

	return nullptr;
}

SceneParticles::CounterStruct* SceneParticles::GetCountersFromGPU()
{
	if (m_CountersBufferPtr)
	{
		auto sboPtr = m_CountersBufferPtr->GetBufferObjectPtr();
		if (sboPtr)
		{
			// we will get counter data from gpu
			void* mappedMemory = nullptr;
			if (vmaMapMemory(vkApi::VulkanCore::sAllocator, sboPtr->alloc_meta, &mappedMemory) == VK_SUCCESS)
			{
				memcpy(&m_Counters, mappedMemory, sizeof(CounterStruct));

				vmaUnmapMemory(vkApi::VulkanCore::sAllocator, sboPtr->alloc_meta);
			}

			return &m_Counters;
		}
	}

	return nullptr;
}

VkDrawIndexedIndirectCommand* SceneParticles::GetDrawIndirectCommandFromGPU()
{
	if (m_DrawArraysIndirectCommandBufferPtr)
	{
		auto sboPtr = m_DrawArraysIndirectCommandBufferPtr->GetBufferObjectPtr();
		if (sboPtr)
		{
			// we will get counter data from gpu
			void* mappedMemory = nullptr;
			if (vmaMapMemory(vkApi::VulkanCore::sAllocator, sboPtr->alloc_meta, &mappedMemory) == VK_SUCCESS)
			{
				memcpy(&m_IndexedIndirectCommand, mappedMemory, sizeof(VkDrawIndexedIndirectCommand));

				vmaUnmapMemory(vkApi::VulkanCore::sAllocator, sboPtr->alloc_meta);
			}

			return &m_IndexedIndirectCommand;
		}
	}

	return nullptr;
}