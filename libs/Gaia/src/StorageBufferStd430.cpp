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

#include <gaia/StorageBufferStd430.h>

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

bool StorageBufferStd430::IsDirty()
{
	return isDirty;
}

bool StorageBufferStd430::IsOk()
{
	return firstUploadWasDone;
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
				VulkanRessource::upload(vVulkanCorePtr, bufferObjectPtr, datas.data(), datas.size());

				firstUploadWasDone = true;
				isDirty = false;
			}
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
			descriptorBufferInfo.buffer = bufferObjectPtr->buffer;
			descriptorBufferInfo.range = datas.size();
			descriptorBufferInfo.offset = 0;

			Upload(vVulkanCorePtr, false);

			return true;
		}
		else
		{
			DestroySBO();
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
	descriptorBufferInfo = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
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

bool StorageBufferStd430::RegisterByteSize(const std::string& vKey, uint32_t vSizeInBytes, uint32_t* vStartOffset)
{
	if (OffsetExist(vKey))
	{
		LogVarDebug("key %s is already defined in UniformBlockStd140. RegisterVar fail.", vKey.c_str());
	}
	else if (vSizeInBytes > 0)
	{
		uint32_t newSize = vSizeInBytes;
		uint32_t lastOffset = (uint32_t)datas.size();
		auto baseAlign = GetGoodAlignement(newSize);
		// il faut trouver le prochain offset qui est multiple de baseAlign
		auto startOffset = baseAlign * (uint32_t)std::ceil((double)lastOffset / (double)baseAlign);
		auto newSizeToAllocate = startOffset - lastOffset + newSize;
#ifdef PRINT_BLOCK_DATAS
		auto endOffset = startOffset + newSize;
		LogVarLightInfo("key %s, size %u, align %u, Offsets : %u => %u, size to alloc %u", vKey.c_str(), newSize, baseAlign, startOffset, endOffset, newSizeToAllocate);
#endif
		datas.resize(lastOffset + newSizeToAllocate);
		// on set de "lastOffset" à "lastOffset + newSizeToAllocate"
		memset(datas.data() + lastOffset, 0, newSizeToAllocate);
		AddOffsetForKey(vKey, startOffset);

		needRecreation = true;

		if (vStartOffset)
			*vStartOffset = startOffset;

		return true;
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
