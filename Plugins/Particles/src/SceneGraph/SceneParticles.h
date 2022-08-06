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
the particles maagement is pretty simple
we build a storage buffer for retain all particles datas with a max particle count
we build a storage buffer of the same size with index datas for dead particle infos
we build a storage buffer of the same size with index datas for alive particle infos

for avoid unnecessary job, we need to dispatch only the alives particle count in a sim stage
so we need to consume the alive buffer during the sim stage and write a new "sorted" alive buffer

so need to have two alive buffers
the idea is consume the frist and write with atomic counter in the second. 
the atomic counter will be used for dispatch and will be reseted before each frame

so can be interesting to jsut dispatch the laive aprticles count only
dead and alive buffer are needed for know what particle is the particle storage buffer cna be used as a new particle available
when a particle just die : 
 - we append it in the dead buffer, with a atomic counter who give the last index of dead particle. if no particle will be -1
 - we remove it in the alive buffer, with a atomic counter who give the last index of alive particle. if no particle will be -1
when a particle just born :
 - we append if in the alive buffer, with a atomic counter who give the last index of alive particle. if no particle will be -1
 - we remove if in the dead buffer, with a atomic counter who give the last index of dead particle. if no particle will be -1

so for a particle system we have :
- particles buffer (complex struct) for n particles
- alive buffer pre sim of uint32_t x particles_count
- alive buffer post sim of uint32_t x particles_count
- dead buffer of uint32_t x particles_count
- a alive pre sim atomic counter
- a alive post sim atomic counter
- a dead atomic counter

job of a particle emitter :
- create particles

job of a particle simulator :
- move particles
- kill particles

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
		uint32_t alive_post_sim_counter = 0U;
		uint32_t alive_pre_sim_counter = 0U;
		uint32_t dead_m_counter = 0U;
	};

public:
	static SceneParticlesPtr Create(vkApi::VulkanCorePtr vVulkanCorePtr);
	static std::string GetParticlesDatasBufferHeader(const uint32_t& vStartBindingPoint);
	static std::string GetParticlesVertexInputBufferHeader();
	static std::string GetAliveParticlesPreSimBufferHeader(const uint32_t& vStartBindingPoint);
	static std::string GetAliveParticlesPostSimBufferHeader(const uint32_t& vStartBindingPoint);
	static std::string GetDeadParticlesBufferHeader(const uint32_t& vStartBindingPoint);
	static std::string GetCounterBufferHeader(const uint32_t& vStartBindingPoint);
	static VertexStruct::PipelineVertexInputState GetInputStateBeforePipelineCreation();

private:
	SceneParticlesWeak m_This;
	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;
	vk::DescriptorBufferInfo m_EmptyDescriptorBufferInfo = { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	CounterStruct m_Counters;

	GpuOnlyStorageBufferPtr m_ParticlesDatasBufferPtr = nullptr;
	GpuOnlyStorageBufferPtr m_AliveParticlesPreSimBufferPtr = nullptr;
	GpuOnlyStorageBufferPtr m_AliveParticlesPostSimBufferPtr = nullptr;
	GpuOnlyStorageBufferPtr m_DeadParticlesBufferPtr = nullptr;
	GpuOnlyStorageBufferPtr m_CountersBufferPtr = nullptr;

public:
	SceneParticles(vkApi::VulkanCorePtr vVulkanCorePtr);
	~SceneParticles();

	bool IsOk();

	bool Build(const uint32_t& vPaticlesMaxCount);

	void DestroyBuffers();

	GpuOnlyStorageBufferWeak GetParticlesDatasBuffer();
	GpuOnlyStorageBufferWeak GetAliveParticlesPreSimBuffer();
	GpuOnlyStorageBufferWeak GetAliveParticlesPostSimBuffer();
	GpuOnlyStorageBufferWeak GetDeadParticlesBuffer();
	GpuOnlyStorageBufferWeak GetCountersBuffer();

	vk::DescriptorBufferInfo* GetParticlesDatasBufferInfo();
	vk::DescriptorBufferInfo* GetAliveParticlesPreSimBufferInfo();
	vk::DescriptorBufferInfo* GetAliveParticlesPostSimBufferInfo();
	vk::DescriptorBufferInfo* GetDeadParticlesBufferInfo();
	vk::DescriptorBufferInfo* GetCountersBufferInfo();

	/// <summary>
	/// will return the buffer (compatible vertex input for render)
	/// and the particle count to render
	/// </summary>
	/// <param name="vOutParticleCountToRender"></param>
	/// <returns></returns>
	vk::Buffer* GetAliveParticlesPostSimVertexInputBuffer();

	/// <summary>
	/// will return counters from the gpu to cpu
	/// </summary>
	/// <returns></returns>
	CounterStruct* GetCountersFromGPU();

private:
	void Destroy();
};