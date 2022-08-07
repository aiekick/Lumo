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

#include <vector>
#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <vkFramework/VulkanRessource.h>
#include <Utils/GpuOnlyStorageBuffer.h>
#include <Utils/Mesh/VertexStruct.h>

class SceneParticles;
typedef std::shared_ptr<SceneParticles> SceneParticlesPtr;
typedef ct::cWeak<SceneParticles> SceneParticlesWeak;

/*
the particles management is pretty simple


[[Emission]] :

The dispatch count will be the size of the particles buffer, so the max count

Each spawn rate we emit a particle if the emission count is > to 0
The emission count will be atomicaly decreased

A particle si alive if the lifetime is > to 0

We put the index of alive particles in a alive pre sim index buffer
And we atomic add on the indirect drawing index

[[Simulation]] :

We advect aprticles

If lifetime is > to their max life time we kill them by jsut put the life time to -1

[[Reset]] :

Before the emission we must reset :
 - the buffer of pre sim index
 - the emission count counter to the emission count from the ubo

*/

class SceneParticles
{
public:
	struct ParticleStruct
	{
		ct::fvec4 pos3_mass1;		// xyz::pos, w:mass
		ct::fvec4 dir3_speed1;		// xyz:dirextion, w:speed
		ct::fvec4 color4;			// rgba:color
		ct::fvec2 life1_max_life1;	// x:life, y:max_life
	};

	struct CounterStruct
	{
		uint32_t alive_particles_count = 0U;
		uint32_t pending_emission_count = 0U;
	};

public:
	static SceneParticlesPtr Create(vkApi::VulkanCorePtr vVulkanCorePtr);
	static std::string GetParticlesDatasBufferHeader(const uint32_t& vStartBindingPoint);
	static std::string GetParticlesVertexInputBufferHeader();
	static std::string GetAliveParticlesIndexBufferHeader(const uint32_t& vStartBindingPoint);
	static std::string GetCounterBufferHeader(const uint32_t& vStartBindingPoint);
	static VertexStruct::PipelineVertexInputState GetInputStateBeforePipelineCreation();
	static std::string GetDrawIndirectCommandHeader(const uint32_t& vStartBindingPoint);

private:
	SceneParticlesWeak m_This;
	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;
	vk::DescriptorBufferInfo m_EmptyDescriptorBufferInfo = { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	CounterStruct m_Counters;

	GpuOnlyStorageBufferPtr m_ParticlesDatasBufferPtr = nullptr;
	GpuOnlyStorageBufferPtr m_AliveParticlesIndexBufferPtr = nullptr;
	GpuOnlyStorageBufferPtr m_CountersBufferPtr = nullptr;
	GpuOnlyStorageBufferPtr m_DrawArraysIndirectCommandBufferPtr = nullptr;

public:
	SceneParticles(vkApi::VulkanCorePtr vVulkanCorePtr);
	~SceneParticles();

	bool IsOk();

	bool Build(const uint32_t& vPaticlesMaxCount);

	void DestroyBuffers();

	vk::DescriptorBufferInfo* GetParticlesDatasBufferInfo();
	vk::DescriptorBufferInfo* GetAliveParticlesIndexBufferInfo();
	vk::DescriptorBufferInfo* GetCountersBufferInfo(); 
	vk::DescriptorBufferInfo* GetDrawIndirectCommandBufferInfo();

	vk::Buffer* GetDrawIndirectCommandBuffer();

	/// <summary>
	/// will return the buffer (compatible vertex input for render)
	/// and the particle count to render
	/// </summary>
	/// <returns></returns>
	vk::Buffer* GetParticlesVertexInputBuffer();

	/// <summary>
	/// will return the buffer (compatible index input for render)
	/// and the particle count to render
	/// </summary>
	/// <returns></returns>
	vk::Buffer* GetAliveParticlesIndexInputBuffer();

	/// <summary>
	/// will return counters from the gpu to cpu
	/// </summary>
	/// <returns></returns>
	CounterStruct* GetCountersFromGPU();

private:
	void Destroy();
};