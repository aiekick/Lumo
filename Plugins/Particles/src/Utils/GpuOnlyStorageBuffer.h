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
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanRessource.h>

class GpuOnlyStorageBuffer;
typedef std::shared_ptr<GpuOnlyStorageBuffer> GpuOnlyStorageBufferPtr;
typedef ct::cWeak<GpuOnlyStorageBuffer> GpuOnlyStorageBufferWeak;

class GpuOnlyStorageBuffer
{
public:
	static GpuOnlyStorageBufferPtr Create(vkApi::VulkanCorePtr vVulkanCorePtr);

private:
	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;
	VulkanBufferObjectPtr m_BufferObjectPtr = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo = { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	uint32_t m_BufferSize = 0U;

public:
	GpuOnlyStorageBuffer(vkApi::VulkanCorePtr vVulkanCorePtr);
	~GpuOnlyStorageBuffer();
	bool CreateBuffer(
		const uint32_t& vDatasSizeInBytes, 
		const uint32_t& vDatasCount, 
		const VmaMemoryUsage& vVmaMemoryUsage = VMA_MEMORY_USAGE_GPU_ONLY);

	VulkanBufferObjectPtr GetBufferObjectPtr();
	vk::DescriptorBufferInfo* GetBufferInfo();
	const uint32_t& GetBufferSize();
	vk::Buffer* GetVulkanBuffer();
	void DestroyBuffer();
};