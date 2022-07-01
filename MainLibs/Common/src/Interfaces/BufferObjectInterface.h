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
