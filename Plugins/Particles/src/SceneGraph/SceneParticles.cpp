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

layout(std430, binding = %u) readonly buffer SBO_ParticleDatas
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

std::string SceneParticles::GetAliveParticlesPreSimBufferHeader(const uint32_t& vStartBindingPoint)
{
	return ct::toStr(u8R"(
layout(std430, binding = %u) readonly buffer SBO_AlivePreSimParticleDatas
{
	uint alive_pre_sim_buffer[];
};
)", vStartBindingPoint);
}

std::string SceneParticles::GetAliveParticlesPostSimBufferHeader(const uint32_t& vStartBindingPoint)
{
	return ct::toStr(u8R"(
layout(std430, binding = %u) readonly buffer SBO_AlivePostSimParticleDatas
{
	uint alive_post_sim_buffer[];
};
)", vStartBindingPoint);
}

std::string SceneParticles::GetDeadParticlesBufferHeader(const uint32_t& vStartBindingPoint)
{
	return ct::toStr(u8R"(
layout(std430, binding = %u) readonly buffer SBO_DeadParticleDatas
{
	uint dead_buffer[];
};
)", vStartBindingPoint);
}

std::string SceneParticles::GetCounterBufferHeader(const uint32_t& vStartBindingPoint)
{
	return ct::toStr(u8R"(
layout(std430, binding = %u) readonly buffer SBO_AtomicCounters
{
	uint alive_post_sim_counter;
	uint alive_pre_sim_counter;
	uint dead_m_counter;
};
)", vStartBindingPoint);
}

VertexStruct::PipelineVertexInputState SceneParticles::GetInputStateBeforePipelineCreation()
{
	ZoneScoped;

	VertexStruct::PipelineVertexInputState inputState = VertexStruct::PipelineVertexInputState{};

	inputState.binding.binding = 0;
	inputState.binding.stride = sizeof(ct::fvec4) * 3U;;
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
		attrib.offset = 0;
		offset += sizeof(ct::fvec4);
	}

	{
		// xyz:direction, w:speed
		auto& attrib = inputState.attributes[1];
		attrib.binding = 0;
		attrib.location = 1;
		attrib.format = vk::Format::eR32G32B32A32Sfloat;
		attrib.offset = offset;
		offset += sizeof(ct::fvec4);
	}

	{
		// rgba:color
		auto& attrib = inputState.attributes[2];
		attrib.binding = 0;
		attrib.location = 2;
		attrib.format = vk::Format::eR32G32B32A32Sfloat;
		attrib.offset = offset;
		offset += sizeof(ct::fvec4);
	}

	{
		// x:life, y:max_life
		auto& attrib = inputState.attributes[3];
		attrib.binding = 0;
		attrib.location = 3;
		attrib.format = vk::Format::eR32G32Sfloat;
		attrib.offset = offset;
		offset += sizeof(ct::fvec2);
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

///////////////////////////////////////////////////////
//// PUBLIC : CTOR / DTOR /////////////////////////////
///////////////////////////////////////////////////////

SceneParticles::SceneParticles(vkApi::VulkanCorePtr vVulkanCorePtr)
	: m_VulkanCorePtr(vVulkanCorePtr)
{
	m_ParticlesDatasBufferPtr = GpuOnlyStorageBuffer::Create(m_VulkanCorePtr);
	m_AliveParticlesPreSimBufferPtr = GpuOnlyStorageBuffer::Create(m_VulkanCorePtr);
	m_AliveParticlesPostSimBufferPtr = GpuOnlyStorageBuffer::Create(m_VulkanCorePtr);
	m_DeadParticlesBufferPtr = GpuOnlyStorageBuffer::Create(m_VulkanCorePtr);
	m_CountersBufferPtr = GpuOnlyStorageBuffer::Create(m_VulkanCorePtr);
}

SceneParticles::~SceneParticles()
{
	m_ParticlesDatasBufferPtr.reset();
	m_AliveParticlesPreSimBufferPtr.reset();
	m_AliveParticlesPostSimBufferPtr.reset();
	m_DeadParticlesBufferPtr.reset();
	m_CountersBufferPtr.reset();
}

///////////////////////////////////////////////////////
//// SBO //////////////////////////////////////////////
///////////////////////////////////////////////////////

bool SceneParticles::IsOk()
{
	return 
		(m_ParticlesDatasBufferPtr != nullptr) &&
		(m_AliveParticlesPreSimBufferPtr != nullptr) &&
		(m_AliveParticlesPostSimBufferPtr != nullptr) &&
		(m_DeadParticlesBufferPtr != nullptr) &&
		(m_CountersBufferPtr != nullptr)
	;
}


bool SceneParticles::Build(const uint32_t& vPaticlesMaxCount)
{
	if (vPaticlesMaxCount)
	{
		if (m_ParticlesDatasBufferPtr && m_ParticlesDatasBufferPtr->CreateBuffer(sizeof(ParticleStruct), vPaticlesMaxCount)) {
			if (m_AliveParticlesPreSimBufferPtr && m_AliveParticlesPreSimBufferPtr->CreateBuffer(sizeof(uint32_t), vPaticlesMaxCount)) {
				if (m_AliveParticlesPostSimBufferPtr && m_AliveParticlesPostSimBufferPtr->CreateBuffer(sizeof(uint32_t), vPaticlesMaxCount)) {
					if (m_DeadParticlesBufferPtr && m_DeadParticlesBufferPtr->CreateBuffer(sizeof(uint32_t), vPaticlesMaxCount)) {
						// for this one we will need to get on cpu side the counters for render the next frame of the simulator
						// for limit the rendering to the alive particles only
						if (m_CountersBufferPtr && m_CountersBufferPtr->CreateBuffer(
							sizeof(CounterStruct), 1U, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_TO_CPU)) {
							return true;
						}
					}
				}
			}
		}
	}

	m_ParticlesDatasBufferPtr->DestroyBuffer();
	m_AliveParticlesPreSimBufferPtr->DestroyBuffer();
	m_AliveParticlesPostSimBufferPtr->DestroyBuffer();
	m_DeadParticlesBufferPtr->DestroyBuffer();
	m_CountersBufferPtr->DestroyBuffer();

	return false;
}

GpuOnlyStorageBufferWeak SceneParticles::GetParticlesDatasBuffer()
{
	return m_ParticlesDatasBufferPtr;
}

GpuOnlyStorageBufferWeak SceneParticles::GetAliveParticlesPreSimBuffer()
{
	return m_AliveParticlesPreSimBufferPtr;
}

GpuOnlyStorageBufferWeak SceneParticles::GetAliveParticlesPostSimBuffer()
{
	return m_AliveParticlesPostSimBufferPtr;
}

GpuOnlyStorageBufferWeak SceneParticles::GetDeadParticlesBuffer()
{
	return m_DeadParticlesBufferPtr;
}

GpuOnlyStorageBufferWeak SceneParticles::GetCountersBuffer()
{
	return m_CountersBufferPtr;
}

vk::DescriptorBufferInfo* SceneParticles::GetParticlesDatasBufferInfo()
{
	if (m_ParticlesDatasBufferPtr)
	{
		return m_ParticlesDatasBufferPtr->GetBufferInfo();
	}

	return &m_EmptyDescriptorBufferInfo;
}

vk::DescriptorBufferInfo* SceneParticles::GetAliveParticlesPreSimBufferInfo()
{
	if (m_AliveParticlesPreSimBufferPtr)
	{
		return m_AliveParticlesPreSimBufferPtr->GetBufferInfo();
	}

	return &m_EmptyDescriptorBufferInfo;
}

vk::DescriptorBufferInfo* SceneParticles::GetAliveParticlesPostSimBufferInfo()
{
	if (m_AliveParticlesPostSimBufferPtr)
	{
		return m_AliveParticlesPostSimBufferPtr->GetBufferInfo();
	}

	return &m_EmptyDescriptorBufferInfo;
}

vk::DescriptorBufferInfo* SceneParticles::GetDeadParticlesBufferInfo()
{
	if (m_DeadParticlesBufferPtr)
	{
		return m_DeadParticlesBufferPtr->GetBufferInfo();
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

vk::Buffer* SceneParticles::GetAliveParticlesPostSimVertexInputBuffer()
{
	if (m_AliveParticlesPostSimBufferPtr)
	{
		m_AliveParticlesPostSimBufferPtr->GetVulkanBuffer();
	}

	return nullptr;
}

SceneParticles::CounterStruct* SceneParticles::GetCountersFromGPU()
{
	if (m_CountersBufferPtr && 
		m_CountersBufferPtr->GetBufferInfo())
	{
		auto sboPtr = m_CountersBufferPtr->GetBufferObjectPtr();
		if (sboPtr)
		{
			/*
			struct CounterStruct
			{
				uint32_t alive_post_sim_counter = 0U;
				uint32_t alive_pre_sim_counter = 0U;
				uint32_t dead_m_counter = 0U;
			};
			*/

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