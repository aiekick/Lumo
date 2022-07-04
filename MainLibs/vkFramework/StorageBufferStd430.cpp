/*
Copyright 2018-2022 Stephane Cuillerdier (aka aiekick)

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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "StorageBufferStd430.h"

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

using namespace vkApi;

StorageBufferStd430::~StorageBufferStd430()
{
	ZoneScoped;

	Unit();
}

bool StorageBufferStd430::Build()
{
	ZoneScoped;

	return true;
}

void StorageBufferStd430::Unit()
{
	ZoneScoped;

	DestroySBO();
	Clear();
}

void StorageBufferStd430::Clear()
{
	ZoneScoped;

	datas.clear();
	offsets.clear();
	isDirty = false;
}

void StorageBufferStd430::SetDirty()
{
	ZoneScoped;

	isDirty = true;
}

void StorageBufferStd430::Upload(vkApi::VulkanCorePtr vVulkanCorePtr, bool vOnlyIfDirty)
{
	ZoneScoped;

	if (vVulkanCorePtr)
	{
		RecreateSBO(vVulkanCorePtr);
		
		if (!vOnlyIfDirty || (vOnlyIfDirty && isDirty))
		{
			if (bufferObjectPtr)
			{
				VulkanRessource::upload(vVulkanCorePtr, *bufferObjectPtr, datas.data(), datas.size());
			}

			isDirty = false;
		}
	}
}

bool StorageBufferStd430::CreateSBO(vkApi::VulkanCorePtr vVulkanCorePtr, VmaMemoryUsage vVmaMemoryUsage)
{
	ZoneScoped;

	if (!datas.empty())
	{
		needRecreation = false;

		bufferObjectPtr = VulkanRessource::createStorageBufferObject(vVulkanCorePtr, datas.size(), vVmaMemoryUsage);
		if (bufferObjectPtr && bufferObjectPtr->buffer)
		{
			bufferObjectPtr->bufferInfo.buffer = bufferObjectPtr->buffer;
			bufferObjectPtr->bufferInfo.range = datas.size();
			bufferObjectPtr->bufferInfo.offset = 0;

			Upload(vVulkanCorePtr, false);

			return true;
		}
		else
		{
			bufferObjectPtr.reset();
		}
	}
	else
	{
		LogVarDebug("CreateSBO() Fail, datas is empty, nothing to upload !");
	}

	return false;
}

void StorageBufferStd430::DestroySBO()
{
	ZoneScoped;

	bufferObjectPtr.reset();
}

bool StorageBufferStd430::RecreateSBO(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	if (needRecreation)
	{
		if (vVulkanCorePtr && bufferObjectPtr)
		{
			VmaMemoryUsage usage = bufferObjectPtr->alloc_usage;
			DestroySBO();
			CreateSBO(vVulkanCorePtr, usage);

			isDirty = true;

			return true;
		}
	}
	
	return false;
}

void StorageBufferStd430::AddOffsetForKey(const std::string& vKey, uint32_t vOffset)
{
	ZoneScoped;

	offsets[vKey] = vOffset;
}

uint32_t StorageBufferStd430::GetGoodAlignement(uint32_t vSize)
{
	ZoneScoped;

	uint32_t goodAlign = (uint32_t)std::pow(2, std::ceil(log(vSize) / log(2)));
	return std::min(goodAlign, 16u);
}
