// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "UniformBlockStd140.h"

#include <vkProfiler/Profiler.h>

using namespace vkApi;

UniformBlockStd140::~UniformBlockStd140()
{
	ZoneScoped;

	Unit();
}

bool UniformBlockStd140::Build()
{
	ZoneScoped;

	return true;
}

void UniformBlockStd140::Unit()
{
	ZoneScoped;

	DestroyUBO();
	Clear();
}

void UniformBlockStd140::Clear()
{
	ZoneScoped;

	descriptorBufferInfo = vk::DescriptorBufferInfo {VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
	datas.clear();
	offsets.clear();
	isDirty = false;
}

void UniformBlockStd140::SetDirty()
{
	ZoneScoped;

	isDirty = true;
}

void UniformBlockStd140::Upload(vkApi::VulkanCorePtr vVulkanCorePtr, bool vOnlyIfDirty)
{
	ZoneScoped;

	if (!vOnlyIfDirty || (vOnlyIfDirty && isDirty))
	{
		if (bufferObjectPtr)
		{
			VulkanRessource::upload(vVulkanCorePtr , bufferObjectPtr, datas.data(), datas.size());
		}

		isDirty = false;
	}
}

bool UniformBlockStd140::CreateUBO(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	bool res = false;

	if (!datas.empty())
	{
		bufferObjectPtr = VulkanRessource::createUniformBufferObject(vVulkanCorePtr, datas.size());
		if (bufferObjectPtr)
		{
			descriptorBufferInfo.buffer = bufferObjectPtr->buffer;
			descriptorBufferInfo.range = datas.size();
			descriptorBufferInfo.offset = 0;
			res = true;
		}
	}
	else
	{
		LogVarDebug("Debug : CreateUBO() Fail, datas is empty, nothing to upload !");
	}

	return res;
}

void UniformBlockStd140::DestroyUBO()
{
	ZoneScoped;

	bufferObjectPtr.reset();
}

bool UniformBlockStd140::RecreateUBO(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	bool res = false;

	if (bufferObjectPtr)
	{
		DestroyUBO();
		CreateUBO(vVulkanCorePtr);

		res = true;
	}

	return res;
}

void UniformBlockStd140::AddOffsetForKey(const std::string& vKey, uint32_t vOffset)
{
	ZoneScoped;

	offsets[vKey] = vOffset;
}

uint32_t UniformBlockStd140::GetGoodAlignement(uint32_t vSize)
{
	ZoneScoped;

	uint32_t goodAlign = (uint32_t)std::pow(2, std::ceil(log(vSize) / log(2)));
	return std::min(goodAlign, 16u);
}