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

#pragma once

#include <vulkan/vulkan.hpp>
#include <vkFramework/VulkanRessource.h>
#include <ctools/Logger.h>

#include <map>
#include <string>
#include <vector>
#include <unordered_map>

//#define PRINT_BLOCK_DATAS

/*
manage in auto the alignement of Buffer for the standard std430 only
*/

#define OffsetExist(key) offsets.find(vKey) != offsets.end()

class StorageBufferStd430
{
private:
	// key = uniform name, offset in buffer for have op on uniform data
	std::unordered_map<std::string, uint32_t> offsets;
	// uniforms datas buffer
	std::vector<uint8_t> datas;

	// if he is dirty, value has been changed and mut be uploaded in gpu memory
	// dirty at first for init in gpu memory
	bool isDirty = true;

	// if new var as added, need recreation, because there is a new size
	bool needRecreation = false;

public: // vulkan object to share
	VulkanBufferObjectPtr bufferObjectPtr = nullptr;
	vk::DescriptorBufferInfo descriptorBufferInfo = { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

public:
	~StorageBufferStd430();

	// init / unit
	bool Build();
	void Unit();
	void Clear();
	void SetDirty();

	// upload to gpu memory
	void Upload(vkApi::VulkanCorePtr vVulkanCorePtr, bool vOnlyIfDirty);

	// create/destory ubo
	bool CreateSBO(vkApi::VulkanCorePtr vVulkanCorePtr, VmaMemoryUsage vVmaMemoryUsage);
	void DestroySBO();
	bool RecreateSBO(vkApi::VulkanCorePtr vVulkanCorePtr);

	// add size to uniform block, return startOffset
	bool RegisterByteSize(const std::string& vKey, uint32_t vSizeInBytes, uint32_t *vStartOffset = 0)
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
			printf("key %s, size %u, align %u, Offsets : %u => %u, size to alloc %u\n", vKey.c_str(), newSize, baseAlign, startOffset, endOffset, newSizeToAllocate);
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

	// templates (defined under class)
	// add a variable
	template<typename T> void RegisterVar(const std::string& vKey, T vValue); // add var to uniform block
	template<typename T> void RegisterVar(const std::string& vKey, T *vValue, uint32_t vSizeInBytes); // add var to uniform block

	// Get / set + op on variables
	template<typename T> bool GetVar(const std::string& vKey, T& vValue); // Get
	template<typename T> bool SetVar(const std::string& vKey, T vValue); // set
	template<typename T> bool SetVar(const std::string& vKey, T *vValue, uint32_t vSizeInBytes); // set
	template<typename T> bool SetAddVar(const std::string&, T vValue); // add and set like +=

private:
	// templates (defined under class)
	uint32_t GetGoodAlignement(uint32_t vSize);

	// non templates (define in class implementation)
	void AddOffsetForKey(const std::string& vKey, uint32_t vOffset);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC TEMPLATES //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
void StorageBufferStd430::RegisterVar(const std::string& vKey, T *vValue, uint32_t vSizeInBytes)
{
	uint32_t startOffset;
	if (RegisterByteSize(vKey, vSizeInBytes, &startOffset))
	{
		// on copy de "startOffset" à "startOffset + vSizeInBytes"
		memcpy(datas.data() + startOffset, vValue, vSizeInBytes);
	}
}

template<typename T>
void StorageBufferStd430::RegisterVar(const std::string& vKey, T vValue)
{
	RegisterVar(vKey, &vValue, sizeof(vValue));
}

template<typename T>
bool StorageBufferStd430::GetVar(const std::string& vKey, T& vValue)
{
	if (OffsetExist(vKey))
	{
		uint32_t offset = offsets[vKey];
		uint32_t size = sizeof(vValue);
		memcpy(&vValue, datas.data() + offset, size);
		return true;
	}
	LogVarDebug("key %s not exist in UniformBlockStd140. GetVar fail.", vKey.c_str());
	return false;
}

template<typename T>
bool StorageBufferStd430::SetVar(const std::string& vKey, T *vValue, uint32_t vSizeInBytes)
{
	if (OffsetExist(vKey) && vSizeInBytes > 0)
	{
		uint32_t newSize = vSizeInBytes;
		uint32_t offset = offsets[vKey];
		memcpy(datas.data() + offset, vValue, newSize);
		isDirty = true;
		return true;
	}
	LogVarDebug("key %s not exist in UniformBlockStd140. SetVar fail.", vKey.c_str());
	return false;
}

template<typename T>
bool StorageBufferStd430::SetVar(const std::string& vKey, T vValue)
{
	return SetVar(vKey, &vValue, sizeof(vValue));
}

template<typename T>
bool StorageBufferStd430::SetAddVar(const std::string& vKey, T vValue)
{
	T v;
	if (GetVar(vKey, v))
	{
		v += vValue;
		return SetVar(vKey, v);
	}
	LogVarDebug("key %s not exist in UniformBlockStd140. SetAddVar fail.", vKey.c_str());
	return false;
}