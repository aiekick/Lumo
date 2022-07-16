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

#include <vulkan/vulkan.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/vkFramework.h>

#include <vector>

//#ifdef USE_RTX
struct VulkanAccelStructObject
{
	vk::AccelerationStructureKHR handle;
	vk::Buffer buffer;
	VmaAllocation alloc_meta;
	VmaMemoryUsage alloc_usage;
};
typedef std::shared_ptr<VulkanAccelStructObject> VulkanAccelStructObjectPtr;
//#endif

struct VulkanRessourceObject
{
	vk::Image image;
	VmaAllocation alloc_meta;
};
typedef std::shared_ptr<VulkanRessourceObject> VulkanRessourceObjectPtr;

class VulkanBufferObject
{
public:
	vk::Buffer buffer;
	VmaAllocation alloc_meta;
	VmaMemoryUsage alloc_usage;
};
typedef std::shared_ptr<VulkanBufferObject> VulkanBufferObjectPtr;

namespace vkApi {
class VulkanRessource
{
public:
	static vk::DescriptorBufferInfo* getEmptyDescriptorBufferInfo() 
	{
		// is working only when nulldescriptor feature is enabled
		// and range must be VK_WHOLE_SIZE in this case
		static vk::DescriptorBufferInfo bufferInfo = { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
		return &bufferInfo;
	}

public: // image
	static void copy(VulkanCorePtr vVulkanCorePtr, vk::Image dst, vk::Buffer src, const vk::BufferImageCopy& region, vk::ImageLayout layout = vk::ImageLayout::eTransferDstOptimal);
	static void copy(VulkanCorePtr vVulkanCorePtr, vk::Image dst, vk::Buffer src, const std::vector<vk::BufferImageCopy>& regions, vk::ImageLayout layout = vk::ImageLayout::eTransferDstOptimal);
	static void copy(VulkanCorePtr vVulkanCorePtr, vk::Buffer dst, vk::Image  src, const vk::BufferImageCopy& region, vk::ImageLayout layout = vk::ImageLayout::eTransferSrcOptimal);
	static void copy(VulkanCorePtr vVulkanCorePtr, vk::Buffer dst, vk::Image  src, const std::vector<vk::BufferImageCopy>& regions, vk::ImageLayout layout = vk::ImageLayout::eTransferSrcOptimal);

	static VulkanRessourceObjectPtr createSharedImageObject(VulkanCorePtr vVulkanCorePtr, const vk::ImageCreateInfo& image_info, const VmaAllocationCreateInfo& alloc_info);
	static VulkanRessourceObjectPtr createTextureImage2D(VulkanCorePtr vVulkanCorePtr, uint32_t width, uint32_t height, uint32_t mipLevelCount, vk::Format format, void* hostdata_ptr);
	static VulkanRessourceObjectPtr createColorAttachment2D(VulkanCorePtr vVulkanCorePtr, uint32_t width, uint32_t height, uint32_t mipLevelCount, vk::Format format, vk::SampleCountFlagBits vSampleCount);
	static VulkanRessourceObjectPtr createComputeTarget2D(VulkanCorePtr vVulkanCorePtr, uint32_t width, uint32_t height, uint32_t mipLevelCount, vk::Format format, vk::SampleCountFlagBits vSampleCount);
	static VulkanRessourceObjectPtr createDepthAttachment(VulkanCorePtr vVulkanCorePtr, uint32_t width, uint32_t height, vk::Format format, vk::SampleCountFlagBits vSampleCount);

	static void GenerateMipmaps(VulkanCorePtr vVulkanCorePtr, vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	static void transitionImageLayout(VulkanCorePtr vVulkanCorePtr, vk::Image image, vk::Format format, uint32_t mipLevel, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
	static void transitionImageLayout(VulkanCorePtr vVulkanCorePtr, vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageSubresourceRange subresourceRange);

	static bool hasStencilComponent(vk::Format format);

	static void getDatasFromTextureImage2D(VulkanCorePtr vVulkanCorePtr, uint32_t width, uint32_t height, vk::Format format, std::shared_ptr<VulkanRessourceObject> vImage, void* vDatas, uint32_t* vSize);

public: // buffers
	static void copy(VulkanCorePtr vVulkanCorePtr, vk::Buffer dst, vk::Buffer src, const vk::BufferCopy& region, vk::CommandPool* vCommandPool = 0);
	static void copy(VulkanCorePtr vVulkanCorePtr, vk::Buffer dst, vk::Buffer src, const std::vector<vk::BufferCopy>& regions, vk::CommandPool* vCommandPool = 0);
	static void upload(VulkanCorePtr vVulkanCorePtr, VulkanBufferObject& dst_hostVisable, void* src_host, size_t size_bytes, size_t dst_offset = 0);

	static VulkanBufferObjectPtr createSharedBufferObject(VulkanCorePtr vVulkanCorePtr, const vk::BufferCreateInfo& bufferinfo, const VmaAllocationCreateInfo& alloc_info);
	static VulkanBufferObjectPtr createUniformBufferObject(VulkanCorePtr vVulkanCorePtr, uint64_t vSize);
	static VulkanBufferObjectPtr createStagingBufferObject(VulkanCorePtr vVulkanCorePtr, uint64_t vSize);
	static VulkanBufferObjectPtr createStorageBufferObject(VulkanCorePtr vVulkanCorePtr, uint64_t vSize, vk::BufferUsageFlags vBufferUsageFlags, VmaMemoryUsage vMemoryUsage);
	static VulkanBufferObjectPtr createStorageBufferObject(VulkanCorePtr vVulkanCorePtr, uint64_t vSize, VmaMemoryUsage vMemoryUsage);
	static VulkanBufferObjectPtr createGPUOnlyStorageBufferObject(VulkanCorePtr vVulkanCorePtr, void* vData, uint64_t vSize);

	template<class T> static VulkanBufferObjectPtr createVertexBufferObject(VulkanCorePtr vVulkanCorePtr, const std::vector<T>& data, bool vUseSSBO = false, bool vUseTransformFeedback = false, bool vUseRTX = false);
	template<class T> static VulkanBufferObjectPtr createIndexBufferObject(VulkanCorePtr vVulkanCorePtr, const std::vector<T>& data, bool vUseSSBO = false, bool vUseTransformFeedback = false, bool vUseRTX = false);

public: // RTX Accel Structure
	static VulkanAccelStructObjectPtr createAccelStructureBufferObject(VulkanCorePtr vVulkanCorePtr, uint64_t vSize, VmaMemoryUsage vMemoryUsage);
};

template<class T>
VulkanBufferObjectPtr VulkanRessource::createVertexBufferObject(VulkanCorePtr vVulkanCorePtr, const std::vector<T>& data, bool vUseSSBO, bool vUseTransformFeedback, bool vUseRTX)
{
	vk::BufferCreateInfo stagingBufferInfo = {};
	VmaAllocationCreateInfo stagingAllocInfo = {};
	stagingBufferInfo.size = data.size() * sizeof(T);
	stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
	stagingAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;
	auto stagebuffer = createSharedBufferObject(vVulkanCorePtr, stagingBufferInfo, stagingAllocInfo);
	upload(vVulkanCorePtr, *stagebuffer, (void*)data.data(), data.size() * sizeof(T));

	vk::BufferCreateInfo vboInfo = {};
	VmaAllocationCreateInfo vboAllocInfo = {};
	vboInfo.size = data.size() * sizeof(T);
	vboInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	if (vUseSSBO) vboInfo.usage = vboInfo.usage | 
		vk::BufferUsageFlagBits::eStorageBuffer;
	if (vUseTransformFeedback) vboInfo.usage = vboInfo.usage | 
		vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT;
	if (vUseRTX) vboInfo.usage = vboInfo.usage | 
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
		vk::BufferUsageFlagBits::eStorageBuffer |
		vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;

	vboAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
	auto vbo = createSharedBufferObject(vVulkanCorePtr, vboInfo, vboAllocInfo);

	vk::BufferCopy region = {};
	region.size = data.size() * sizeof(T);
	copy(vVulkanCorePtr, vbo->buffer, stagebuffer->buffer, region);

	return vbo;
}

template<class T>
VulkanBufferObjectPtr VulkanRessource::createIndexBufferObject(VulkanCorePtr vVulkanCorePtr, const std::vector<T>& data, bool vUseSSBO, bool vUseTransformFeedback, bool vUseRTX)
{
	vk::BufferCreateInfo stagingBufferInfo = {};
	VmaAllocationCreateInfo stagingAllocInfo = {};
	stagingBufferInfo.size = data.size() * sizeof(T);
	stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
	stagingAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;
	auto stagebuffer = createSharedBufferObject(vVulkanCorePtr, stagingBufferInfo, stagingAllocInfo);
	upload(vVulkanCorePtr, *stagebuffer, (void*)data.data(), data.size() * sizeof(T));

	vk::BufferCreateInfo vboInfo = {};
	VmaAllocationCreateInfo vboAllocInfo = {};
	vboInfo.size = data.size() * sizeof(T);
	vboInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	if (vUseSSBO) vboInfo.usage = vboInfo.usage | 
		vk::BufferUsageFlagBits::eStorageBuffer;
	if (vUseTransformFeedback) vboInfo.usage = vboInfo.usage | 
		vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT;
	if (vUseRTX) vboInfo.usage = vboInfo.usage |
		//vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | => not needed for index
		vk::BufferUsageFlagBits::eStorageBuffer |
		vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;

	vboAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
	auto vbo = createSharedBufferObject(vVulkanCorePtr, vboInfo, vboAllocInfo);

	vk::BufferCopy region = {};
	region.size = data.size() * sizeof(T);
	copy(vVulkanCorePtr, vbo->buffer, stagebuffer->buffer, region);

	return vbo;
}
}