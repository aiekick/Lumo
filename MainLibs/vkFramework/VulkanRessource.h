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

#include <vulkan/vulkan.hpp>
#include <vkFramework/VulkanCore.h>

#include <vector>

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
	vk::DescriptorBufferInfo bufferInfo;
};
typedef std::shared_ptr<VulkanBufferObject> VulkanBufferObjectPtr;

namespace vkApi {
class VulkanRessource
{
public: // image
	static void copy(VulkanCore* vVulkanCore, vk::Image dst, vk::Buffer src, const vk::BufferImageCopy& region, vk::ImageLayout layout = vk::ImageLayout::eTransferDstOptimal);
	static void copy(VulkanCore* vVulkanCore, vk::Image dst, vk::Buffer src, const std::vector<vk::BufferImageCopy>& regions, vk::ImageLayout layout = vk::ImageLayout::eTransferDstOptimal);
	static void copy(VulkanCore* vVulkanCore, vk::Buffer dst, vk::Image  src, const vk::BufferImageCopy& region, vk::ImageLayout layout = vk::ImageLayout::eTransferSrcOptimal);
	static void copy(VulkanCore* vVulkanCore, vk::Buffer dst, vk::Image  src, const std::vector<vk::BufferImageCopy>& regions, vk::ImageLayout layout = vk::ImageLayout::eTransferSrcOptimal);

	static VulkanRessourceObjectPtr createSharedImageObject(VulkanCore* vVulkanCore, const vk::ImageCreateInfo& image_info, const VmaAllocationCreateInfo& alloc_info);
	static VulkanRessourceObjectPtr createTextureImage2D(VulkanCore* vVulkanCore, uint32_t width, uint32_t height, uint32_t mipLevelCount, vk::Format format, void* hostdata_ptr);
	static VulkanRessourceObjectPtr createColorAttachment2D(VulkanCore* vVulkanCore, uint32_t width, uint32_t height, uint32_t mipLevelCount, vk::Format format, vk::SampleCountFlagBits vSampleCount);
	static VulkanRessourceObjectPtr createComputeTarget2D(VulkanCore* vVulkanCore, uint32_t width, uint32_t height, uint32_t mipLevelCount, vk::Format format, vk::SampleCountFlagBits vSampleCount);
	static VulkanRessourceObjectPtr createDepthAttachment(VulkanCore* vVulkanCore, uint32_t width, uint32_t height, vk::Format format, vk::SampleCountFlagBits vSampleCount);

	static void GenerateMipmaps(VulkanCore* vVulkanCore, vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	static void transitionImageLayout(VulkanCore* vVulkanCore, vk::Image image, vk::Format format, uint32_t mipLevel, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
	static void transitionImageLayout(VulkanCore* vVulkanCore, vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageSubresourceRange subresourceRange);

	static bool hasStencilComponent(vk::Format format);

	static void getDatasFromTextureImage2D(VulkanCore* vVulkanCore, uint32_t width, uint32_t height, vk::Format format, std::shared_ptr<VulkanRessourceObject> vImage, void* vDatas, uint32_t* vSize);

public: // buffers
	static void copy(VulkanCore* vVulkanCore, vk::Buffer dst, vk::Buffer src, const vk::BufferCopy& region, vk::CommandPool* vCommandPool = 0);
	static void copy(VulkanCore* vVulkanCore, vk::Buffer dst, vk::Buffer src, const std::vector<vk::BufferCopy>& regions, vk::CommandPool* vCommandPool = 0);
	static void upload(VulkanCore* vVulkanCore, VulkanBufferObject& dst_hostVisable, void* src_host, size_t size_bytes, size_t dst_offset = 0);

	static VulkanBufferObjectPtr createSharedBufferObject(VulkanCore* vVulkanCore, const vk::BufferCreateInfo& bufferinfo, const VmaAllocationCreateInfo& alloc_info);
	static VulkanBufferObjectPtr createUniformBufferObject(VulkanCore* vVulkanCore, uint64_t vSize);
	static VulkanBufferObjectPtr createStagingBufferObject(VulkanCore* vVulkanCore, uint64_t vSize);
	static VulkanBufferObjectPtr createStorageBufferObject(VulkanCore* vVulkanCore, uint64_t vSize, VmaMemoryUsage vMemoryUsage);
	static VulkanBufferObjectPtr createGPUOnlyStorageBufferObject(VulkanCore* vVulkanCore, void* vData, uint64_t vSize);

	template<class T> static VulkanBufferObjectPtr createVertexBufferObject(VulkanCore* vVulkanCore, const std::vector<T>& data, bool vUseSSBO = false, bool vUseTransformFeedback = false);
	template<class T> static VulkanBufferObjectPtr createIndexBufferObject(VulkanCore* vVulkanCore, const std::vector<T>& data, bool vUseSSBO = false, bool vUseTransformFeedback = false);
};

template<class T>
VulkanBufferObjectPtr VulkanRessource::createVertexBufferObject(VulkanCore* vVulkanCore, const std::vector<T>& data, bool vUseSSBO, bool vUseTransformFeedback)
{
	vk::BufferCreateInfo stagingBufferInfo = {};
	VmaAllocationCreateInfo stagingAllocInfo = {};
	stagingBufferInfo.size = data.size() * sizeof(T);
	stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
	stagingAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;
	auto stagebuffer = createSharedBufferObject(vVulkanCore, stagingBufferInfo, stagingAllocInfo);
	upload(vVulkanCore, *stagebuffer, (void*)data.data(), data.size() * sizeof(T));

	vk::BufferCreateInfo vboInfo = {};
	VmaAllocationCreateInfo vboAllocInfo = {};
	vboInfo.size = data.size() * sizeof(T);
	vboInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	if (vUseSSBO) vboInfo.usage = vboInfo.usage | vk::BufferUsageFlagBits::eStorageBuffer;
	if (vUseTransformFeedback) vboInfo.usage = vboInfo.usage | vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT;

	vboAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
	auto vbo = createSharedBufferObject(vVulkanCore, vboInfo, vboAllocInfo);

	vk::BufferCopy region = {};
	region.size = data.size() * sizeof(T);
	copy(vVulkanCore, vbo->buffer, stagebuffer->buffer, region);

	return vbo;
}

template<class T>
VulkanBufferObjectPtr VulkanRessource::createIndexBufferObject(VulkanCore* vVulkanCore, const std::vector<T>& data, bool vUseSSBO, bool vUseTransformFeedback)
{
	vk::BufferCreateInfo stagingBufferInfo = {};
	VmaAllocationCreateInfo stagingAllocInfo = {};
	stagingBufferInfo.size = data.size() * sizeof(T);
	stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
	stagingAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;
	auto stagebuffer = createSharedBufferObject(vVulkanCore, stagingBufferInfo, stagingAllocInfo);
	upload(vVulkanCore, *stagebuffer, (void*)data.data(), data.size() * sizeof(T));

	vk::BufferCreateInfo vboInfo = {};
	VmaAllocationCreateInfo vboAllocInfo = {};
	vboInfo.size = data.size() * sizeof(T);
	vboInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	if (vUseSSBO) vboInfo.usage = vboInfo.usage | vk::BufferUsageFlagBits::eStorageBuffer;
	if (vUseTransformFeedback) vboInfo.usage = vboInfo.usage | vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT;

	vboAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
	auto vbo = createSharedBufferObject(vVulkanCore, vboInfo, vboAllocInfo);

	vk::BufferCopy region = {};
	region.size = data.size() * sizeof(T);
	copy(vVulkanCore, vbo->buffer, stagebuffer->buffer, region);

	return vbo;
}
}