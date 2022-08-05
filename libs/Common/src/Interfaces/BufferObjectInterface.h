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

#include <string>
#include <memory>
#include <ctools/cTools.h>
#include <vulkan/vulkan.hpp>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/vkFramework.h>

#ifdef BUILD_SHARED
#ifdef _WIN32
#define COMMON_API   __declspec( dllimport )
#endif // _WIN32
#endif // BUILD_SHARED
#ifndef COMMON_API
#define COMMON_API
#endif // COMMON_API

class COMMON_API BufferObjectInterface
{
protected:
	VulkanBufferObjectPtr m_BufferObjectPtr = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo = { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	uint32_t m_BufferSize = 0U;

public:
	bool m_BufferObjectIsDirty = false;

public:
	virtual void UploadBufferObjectIfDirty(vkApi::VulkanCorePtr vVulkanCorePtr) = 0;
	virtual bool CreateBufferObject(vkApi::VulkanCorePtr vVulkanCorePtr) = 0;
	virtual void DestroyBufferObject()
	{
		m_BufferObjectPtr.reset();
		m_DescriptorBufferInfo = vk::DescriptorBufferInfo { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	}

	//virtual std::string GetBufferObjectStructureHeader(const uint32_t& vBindingPoint) = 0;
	virtual vk::DescriptorBufferInfo* GetBufferInfo()
	{
		return &m_DescriptorBufferInfo;
	}

	// we reimplement it because vk::DescriptorBufferInfo.range 
	// can be not the full size becasue related to vk::DescriptorBufferInfo.offset
	virtual const uint32_t& GetBufferSIze()
	{
		return m_BufferSize;
	}
};
