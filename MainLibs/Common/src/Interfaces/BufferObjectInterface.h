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

#ifdef BUILD_SHARED
#ifdef _WIN32
#define COMMON_API   __declspec( dllimport )
#endif // _WIN32
#endif // BUILD_SHARED
#ifndef COMMON_API
#define COMMON_API
#endif // COMMON_API

namespace vkApi { class VulkanCore; }
class COMMON_API BufferObjectInterface
{
public:
	VulkanBufferObjectPtr m_BufferObject = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo;
	bool m_BufferObjectIsDirty = false;

public:
	virtual void UploadBufferObjectIfDirty(vkApi::VulkanCore* vVulkanCore) = 0;
	virtual bool CreateBufferObject(vkApi::VulkanCore* vVulkanCore) = 0;
	virtual void DestroyBufferObject() = 0;
	//virtual std::string GetBufferObjectStructureHeader(const uint32_t& vBinding) = 0;
	virtual vk::DescriptorBufferInfo* GetBufferInfo() = 0;
};
