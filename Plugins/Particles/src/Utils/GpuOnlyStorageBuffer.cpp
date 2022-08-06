#include "GpuOnlyStorageBuffer.h"

GpuOnlyStorageBufferPtr GpuOnlyStorageBuffer::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	if (vVulkanCorePtr)
	{
		auto res = std::make_shared<GpuOnlyStorageBuffer>(vVulkanCorePtr);
		return res;
	}

	return nullptr;
}

GpuOnlyStorageBuffer::GpuOnlyStorageBuffer(vkApi::VulkanCorePtr vVulkanCorePtr)
	: m_VulkanCorePtr(vVulkanCorePtr)
{

}

GpuOnlyStorageBuffer::~GpuOnlyStorageBuffer()
{
	DestroyBuffer();
}

bool GpuOnlyStorageBuffer::CreateBuffer(const uint32_t& vDatasSizeInBytes, const uint32_t& vDatasCount, const VmaMemoryUsage& vVmaMemoryUsage)
{
	if (vDatasCount && vDatasSizeInBytes && vDatasCount)
	{
		DestroyBuffer();

		auto sizeInBytes = vDatasCount * vDatasSizeInBytes;

		//gpu only since no udpate will be done on cpu side
		m_BufferObjectPtr = vkApi::VulkanRessource::createStorageBufferObject(
			m_VulkanCorePtr, sizeInBytes, vVmaMemoryUsage);
		if (m_BufferObjectPtr &&
			m_BufferObjectPtr->buffer)
		{
			m_BufferSize = vDatasCount;

			m_DescriptorBufferInfo.buffer = m_BufferObjectPtr->buffer;
			m_DescriptorBufferInfo.offset = 0U;
			m_DescriptorBufferInfo.range = sizeInBytes;

			return true;
		}
	}

	return false;
}

VulkanBufferObjectPtr GpuOnlyStorageBuffer::GetBufferObjectPtr()
{
	return m_BufferObjectPtr;
}

vk::DescriptorBufferInfo* GpuOnlyStorageBuffer::GetBufferInfo()
{
	return &m_DescriptorBufferInfo;
}

const uint32_t& GpuOnlyStorageBuffer::GetBufferSize()
{
	return m_BufferSize;
}

vk::Buffer* GpuOnlyStorageBuffer::GetVulkanBuffer()
{
	if (m_BufferObjectPtr)
	{
		return &m_BufferObjectPtr->buffer;
	}

	return nullptr;
}

void GpuOnlyStorageBuffer::DestroyBuffer()
{
	m_BufferObjectPtr.reset();
	m_DescriptorBufferInfo = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	m_BufferSize = 0U;
}
