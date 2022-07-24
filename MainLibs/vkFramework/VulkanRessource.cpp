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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "VulkanRessource.h"

#include "VulkanCore.h"
#include "VulkanCommandBuffer.h"
#include "vk_mem_alloc.h"

#include <ctools/Logger.h>

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

namespace vkApi {

//////////////////////////////////////////////////////////////////////////////////
//// IMAGE ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void VulkanRessource::copy(vkApi::VulkanCorePtr vVulkanCorePtr, vk::Image dst, vk::Buffer src, const vk::BufferImageCopy& region, vk::ImageLayout layout)
{
	ZoneScoped;

	auto cmd = VulkanCommandBuffer::beginSingleTimeCommands(vVulkanCorePtr, true);
	cmd.copyBufferToImage(src, dst, layout, region);
	VulkanCommandBuffer::flushSingleTimeCommands(vVulkanCorePtr, cmd, true);
}

void VulkanRessource::copy(vkApi::VulkanCorePtr vVulkanCorePtr, vk::Buffer dst, vk::Image src, const vk::BufferImageCopy& region, vk::ImageLayout layout)
{
	ZoneScoped;

	auto cmd = VulkanCommandBuffer::beginSingleTimeCommands(vVulkanCorePtr, true);
	cmd.copyImageToBuffer(src, layout, dst, region);
	VulkanCommandBuffer::flushSingleTimeCommands(vVulkanCorePtr, cmd, true);
}

void VulkanRessource::copy(vkApi::VulkanCorePtr vVulkanCorePtr, vk::Image dst, vk::Buffer src, const std::vector<vk::BufferImageCopy>& regions, vk::ImageLayout layout)
{
	ZoneScoped;

	auto cmd = VulkanCommandBuffer::beginSingleTimeCommands(vVulkanCorePtr, true);
	cmd.copyBufferToImage(src, dst, layout, regions);
	VulkanCommandBuffer::flushSingleTimeCommands(vVulkanCorePtr, cmd, true);
}

void VulkanRessource::copy(vkApi::VulkanCorePtr vVulkanCorePtr, vk::Buffer dst, vk::Image src, const std::vector<vk::BufferImageCopy>& regions, vk::ImageLayout layout)
{
	ZoneScoped;

	auto cmd = VulkanCommandBuffer::beginSingleTimeCommands(vVulkanCorePtr, true);
	cmd.copyImageToBuffer(src, layout, dst, regions);
	VulkanCommandBuffer::flushSingleTimeCommands(vVulkanCorePtr, cmd, true);
}

bool VulkanRessource::hasStencilComponent(vk::Format format)
{
	ZoneScoped;

	return
		format == vk::Format::eD16UnormS8Uint ||
		format == vk::Format::eD24UnormS8Uint ||
		format == vk::Format::eD32SfloatS8Uint;
}

VulkanRessourceObjectPtr VulkanRessource::createSharedImageObject(vkApi::VulkanCorePtr vVulkanCorePtr, const vk::ImageCreateInfo& image_info, const VmaAllocationCreateInfo& alloc_info)
{
	ZoneScoped;

	auto ret = VulkanRessourceObjectPtr(new VulkanRessourceObject, [](VulkanRessourceObject* obj)
		{
			vmaDestroyImage(vkApi::VulkanCore::sAllocator, (VkImage)obj->image, obj->alloc_meta);
		});

	VulkanCore::check_error(vmaCreateImage(vkApi::VulkanCore::sAllocator, (VkImageCreateInfo*)&image_info, &alloc_info, (VkImage*)&ret->image, &ret->alloc_meta, nullptr));
	return ret;
}

VulkanRessourceObjectPtr VulkanRessource::createTextureImage2D(vkApi::VulkanCorePtr vVulkanCorePtr, uint32_t width, uint32_t height, uint32_t mipLevelCount, vk::Format format, void* hostdata_ptr)
{
	ZoneScoped;

	uint32_t channels = 0;
	uint32_t elem_size = 0;

	mipLevelCount = ct::maxi(mipLevelCount, 1u);

	switch (format)
	{
	case vk::Format::eB8G8R8A8Unorm:
	case vk::Format::eR8G8B8A8Unorm:
		channels = 4;
		elem_size = 8 / 8;
		break;
	case vk::Format::eB8G8R8Unorm:
	case vk::Format::eR8G8B8Unorm:
		channels = 3;
		elem_size = 8 / 8;
		break;
	case vk::Format::eR8Unorm:
		channels = 1;
		elem_size = 8 / 8;
		break;
	case vk::Format::eD16Unorm:
		channels = 1;
		elem_size = 16 / 8;
		break;
	case vk::Format::eR32G32B32A32Sfloat:
		channels = 4;
		elem_size = 32 / 8;
		break;
	case vk::Format::eR32G32B32Sfloat:
		channels = 3;
		elem_size = 32 / 8;
		break;
	case vk::Format::eR32Sfloat:
		channels = 1;
		elem_size = 32 / 8;
		break;
	default:
		LogVarError("unsupported type: %s", vk::to_string(format).c_str());
		throw std::invalid_argument("unsupported fomat type!");
	}

	vk::BufferCreateInfo stagingBufferInfo = {};
	VmaAllocationCreateInfo stagingAllocInfo = {};
	stagingBufferInfo.size = width * height * channels * elem_size;
	stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
	stagingAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;
	auto stagebuffer = createSharedBufferObject(vVulkanCorePtr, stagingBufferInfo, stagingAllocInfo);
	upload(vVulkanCorePtr, *stagebuffer, hostdata_ptr, width * height * channels * elem_size);

	VmaAllocationCreateInfo image_alloc_info = {};
	image_alloc_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
	auto familyQueueIndex = vVulkanCorePtr->getQueue(vk::QueueFlagBits::eGraphics).familyQueueIndex;
	auto texture = createSharedImageObject(vVulkanCorePtr, vk::ImageCreateInfo(
		vk::ImageCreateFlags(),
		vk::ImageType::e2D,
		format,
		vk::Extent3D(vk::Extent2D(width, height), 1),
		mipLevelCount, 1u,
		vk::SampleCountFlagBits::e1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::SharingMode::eExclusive,
		1,
		&familyQueueIndex,
		vk::ImageLayout::eUndefined),
		image_alloc_info);

	vk::BufferImageCopy copyParams(
		0u, 0u, 0u,
		vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0u, 0u, 1),
		vk::Offset3D(0, 0, 0),
		vk::Extent3D(width, height, 1));

	// on va copier que le mip level 0, on fera les autre dans GenerateMipmaps juste apres ce block
	transitionImageLayout(vVulkanCorePtr, texture->image, format, mipLevelCount, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	copy(vVulkanCorePtr, texture->image, stagebuffer->buffer, copyParams);
	if (mipLevelCount > 1)
	{
		GenerateMipmaps(vVulkanCorePtr, texture->image, format, width, height, mipLevelCount);
	}
	else
	{
		transitionImageLayout(vVulkanCorePtr, texture->image, format, mipLevelCount, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	}

	return texture;
}

void VulkanRessource::getDatasFromTextureImage2D(
	VulkanCorePtr vVulkanCorePtr, uint32_t width, uint32_t height, 
	vk::Format format, std::shared_ptr<VulkanRessourceObject> vImage, 
	void* vDatas, uint32_t* vSize)
{
	UNUSED(width);
	UNUSED(height);
	UNUSED(format);
	UNUSED(vImage);
	UNUSED(vDatas);
	UNUSED(vSize);

	ZoneScoped;
}

VulkanRessourceObjectPtr VulkanRessource::createColorAttachment2D(vkApi::VulkanCorePtr vVulkanCorePtr, uint32_t width, uint32_t height, uint32_t mipLevelCount, vk::Format format, vk::SampleCountFlagBits vSampleCount)
{
	ZoneScoped;

	mipLevelCount = ct::maxi(mipLevelCount, 1u);

	auto graphicQueue = vVulkanCorePtr->getQueue(vk::QueueFlagBits::eGraphics);
	auto computeQueue = vVulkanCorePtr->getQueue(vk::QueueFlagBits::eCompute);
	std::vector<uint32_t> familyIndices = { graphicQueue.familyQueueIndex };
	if (computeQueue.familyQueueIndex != graphicQueue.familyQueueIndex)
		familyIndices.push_back(computeQueue.familyQueueIndex);

	vk::ImageCreateInfo imageInfo = {};
	imageInfo.flags = vk::ImageCreateFlags();
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.format = format;
	imageInfo.extent = vk::Extent3D(width, height, 1);
	imageInfo.mipLevels = mipLevelCount;
	imageInfo.arrayLayers = 1U;
	imageInfo.samples = vSampleCount;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;// | vk::ImageUsageFlagBits::eStorage;
	if (familyIndices.size() > 1)
		imageInfo.sharingMode = vk::SharingMode::eConcurrent;
	else
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(familyIndices.size());
	imageInfo.pQueueFamilyIndices = familyIndices.data();
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;

	VmaAllocationCreateInfo image_alloc_info = {};
	image_alloc_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;

	auto vkoPtr = VulkanRessource::createSharedImageObject(vVulkanCorePtr, imageInfo, image_alloc_info);
	if (vkoPtr)
	{
		VulkanRessource::transitionImageLayout(vVulkanCorePtr, vkoPtr->image, format, mipLevelCount, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal);
	}

	return vkoPtr;
}

VulkanRessourceObjectPtr VulkanRessource::createComputeTarget2D(vkApi::VulkanCorePtr vVulkanCorePtr, uint32_t width, uint32_t height, uint32_t mipLevelCount, vk::Format format, vk::SampleCountFlagBits vSampleCount)
{
	ZoneScoped;

	mipLevelCount = ct::maxi(mipLevelCount, 1u);

	auto graphicQueue = vVulkanCorePtr->getQueue(vk::QueueFlagBits::eGraphics);
	auto computeQueue = vVulkanCorePtr->getQueue(vk::QueueFlagBits::eCompute);
	std::vector<uint32_t> familyIndices = { graphicQueue.familyQueueIndex };
	if (computeQueue.familyQueueIndex != graphicQueue.familyQueueIndex)
		familyIndices.push_back(computeQueue.familyQueueIndex);

	vk::ImageCreateInfo imageInfo = {};
	imageInfo.flags = vk::ImageCreateFlags();
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.format = format;
	imageInfo.extent = vk::Extent3D(width, height, 1);
	imageInfo.mipLevels = mipLevelCount;
	imageInfo.arrayLayers = 1U;
	imageInfo.samples = vSampleCount;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	imageInfo.usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
	if (familyIndices.size() > 1)
		imageInfo.sharingMode = vk::SharingMode::eConcurrent;
	else
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(familyIndices.size());
	imageInfo.pQueueFamilyIndices = familyIndices.data();
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;

	VmaAllocationCreateInfo image_alloc_info = {};
	image_alloc_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;

	auto vkoPtr = VulkanRessource::createSharedImageObject(vVulkanCorePtr, imageInfo, image_alloc_info);
	if (vkoPtr)
	{
		VulkanRessource::transitionImageLayout(vVulkanCorePtr, vkoPtr->image, format, mipLevelCount, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
	}

	return vkoPtr;
}

VulkanRessourceObjectPtr VulkanRessource::createDepthAttachment(vkApi::VulkanCorePtr vVulkanCorePtr, uint32_t width, uint32_t height, vk::Format format, vk::SampleCountFlagBits vSampleCount)
{
	ZoneScoped;

	auto graphicQueue = vVulkanCorePtr->getQueue(vk::QueueFlagBits::eGraphics);
	auto computeQueue = vVulkanCorePtr->getQueue(vk::QueueFlagBits::eCompute);
	std::vector<uint32_t> familyIndices = { graphicQueue.familyQueueIndex };
	if (computeQueue.familyQueueIndex != graphicQueue.familyQueueIndex)
		familyIndices.push_back(computeQueue.familyQueueIndex);

	vk::ImageCreateInfo imageInfo = {};
	imageInfo.flags = vk::ImageCreateFlags();
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.format = format;
	imageInfo.extent = vk::Extent3D(width, height, 1);
	imageInfo.mipLevels = 1U;
	imageInfo.arrayLayers = 1U;
	imageInfo.samples = vSampleCount;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(familyIndices.size());
	imageInfo.pQueueFamilyIndices = familyIndices.data();
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;

	VmaAllocationCreateInfo image_alloc_info = {};
	image_alloc_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;

	return VulkanRessource::createSharedImageObject(vVulkanCorePtr, imageInfo, image_alloc_info);
}

void VulkanRessource::GenerateMipmaps(vkApi::VulkanCorePtr vVulkanCorePtr, vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
	ZoneScoped;

	if (mipLevels > 1)
	{
		auto physDevice = vVulkanCorePtr->getPhysicalDevice();

		// Check if image format supports linear blitting
		vk::FormatProperties formatProperties = physDevice.getFormatProperties(imageFormat);

		if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
		{
			throw std::runtime_error("texture image format does not support linear blitting!");
		}

		vk::CommandBuffer commandBuffer = VulkanCommandBuffer::beginSingleTimeCommands(vVulkanCorePtr, true);

		vk::ImageMemoryBarrier barrier;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; ++i)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

			commandBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eTransfer,
				vk::DependencyFlags(),
				{},
				{},
				barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage((VkCommandBuffer)commandBuffer,
				(VkImage)image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				(VkImage)image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			commandBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eFragmentShader,
				vk::DependencyFlags(),
				{},
				{},
				barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		commandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlags(),
			{},
			{},
			barrier);

		VulkanCommandBuffer::flushSingleTimeCommands(vVulkanCorePtr, commandBuffer, true);
	}
}

void VulkanRessource::transitionImageLayout(vkApi::VulkanCorePtr vVulkanCorePtr, vk::Image image, vk::Format format, uint32_t mipLevel, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	ZoneScoped;

	vk::ImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = mipLevel;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;

	VulkanRessource::transitionImageLayout(vVulkanCorePtr, image, format, oldLayout, newLayout, subresourceRange);
}

void VulkanRessource::transitionImageLayout(vkApi::VulkanCorePtr vVulkanCorePtr, vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageSubresourceRange subresourceRange)
{
	ZoneScoped;

	auto cmd = VulkanCommandBuffer::beginSingleTimeCommands(vVulkanCorePtr, true);
	vk::ImageMemoryBarrier barrier = {};
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = image;
	barrier.subresourceRange = subresourceRange;
	barrier.srcAccessMask = vk::AccessFlags();  //
	barrier.dstAccessMask = vk::AccessFlags();  //

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	// TODO:
	if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		if (VulkanRessource::hasStencilComponent(format))
		{
			barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	}

	bool wasFound = true;
	if (oldLayout == vk::ImageLayout::eUndefined)
	{
		if (newLayout == vk::ImageLayout::eTransferDstOptimal ||
			newLayout == vk::ImageLayout::eTransferSrcOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlags();
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;

			wasFound = true;
		}
		else if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlags();
			barrier.dstAccessMask =
				vk::AccessFlagBits::eDepthStencilAttachmentRead |
				vk::AccessFlagBits::eDepthStencilAttachmentWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;

			wasFound = true;
		}
		else if (newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlags();
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;

			wasFound = true;
		}
		else if (newLayout == vk::ImageLayout::eGeneral)
		{
			barrier.srcAccessMask = vk::AccessFlags();
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eComputeShader;

			wasFound = true;
		}
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		if (newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;

			wasFound = true;
		}
	}

	if (!wasFound)
	{
		LogVarDebug("Debug : unsupported layouts: %s, %s",
			vk::to_string(oldLayout).c_str(),
			vk::to_string(newLayout).c_str());
		throw std::invalid_argument("unsupported layout transition!");
	}

	cmd.pipelineBarrier(sourceStage, destinationStage,
		vk::DependencyFlags(),
		{},
		{},
		barrier);

	VulkanCommandBuffer::flushSingleTimeCommands(vVulkanCorePtr, cmd, true);
}

//////////////////////////////////////////////////////////////////////////////////
//// BUFFER //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void VulkanRessource::copy(vkApi::VulkanCorePtr vVulkanCorePtr, vk::Buffer dst, vk::Buffer src, const vk::BufferCopy& region, vk::CommandPool* vCommandPool)
{
	ZoneScoped;

	auto cmd = VulkanCommandBuffer::beginSingleTimeCommands(vVulkanCorePtr, true, vCommandPool);
	cmd.copyBuffer(src, dst, region);
	VulkanCommandBuffer::flushSingleTimeCommands(vVulkanCorePtr, cmd, true, vCommandPool);
}

void VulkanRessource::upload(vkApi::VulkanCorePtr vVulkanCorePtr, VulkanBufferObject& dst_hostVisable, void* src_host, size_t size_bytes, size_t dst_offset)
{
	ZoneScoped;

	if (dst_hostVisable.alloc_usage == VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY)
	{
		LogVarDebug("Debug : upload not done because it is VMA_MEMORY_USAGE_GPU_ONLY");
		return;
	}
	void* dst = nullptr;
	VulkanCore::check_error(vmaMapMemory(vkApi::VulkanCore::sAllocator, dst_hostVisable.alloc_meta, &dst));
	memcpy((uint8_t*)dst + dst_offset, src_host, size_bytes);
	vmaUnmapMemory(vkApi::VulkanCore::sAllocator, dst_hostVisable.alloc_meta);
}

void VulkanRessource::copy(vkApi::VulkanCorePtr vVulkanCorePtr, vk::Buffer dst, vk::Buffer src, const std::vector<vk::BufferCopy>& regions, vk::CommandPool* vCommandPool)
{
	ZoneScoped;

	auto cmd = VulkanCommandBuffer::beginSingleTimeCommands(vVulkanCorePtr, true, vCommandPool);
	cmd.copyBuffer(src, dst, regions);
	VulkanCommandBuffer::flushSingleTimeCommands(vVulkanCorePtr, cmd, true, vCommandPool);
}

void VulkanRessource::SetDeviceAddress(const vk::Device& vDevice, VulkanBufferObjectPtr vVulkanBufferObjectPtr)
{
	if (vDevice &&
		vVulkanBufferObjectPtr &&
		(vVulkanBufferObjectPtr->buffer_usage & vk::BufferUsageFlagBits::eShaderDeviceAddress))
	{
		vk::BufferDeviceAddressInfoKHR bufferDeviceAddressInfo{};
		bufferDeviceAddressInfo.buffer = vVulkanBufferObjectPtr->buffer;
		vVulkanBufferObjectPtr->device_address = vDevice.getBufferAddressKHR(&bufferDeviceAddressInfo);
	}
}

VulkanBufferObjectPtr VulkanRessource::createSharedBufferObject(vkApi::VulkanCorePtr vVulkanCorePtr, const vk::BufferCreateInfo& bufferinfo, const VmaAllocationCreateInfo& alloc_info)
{
	ZoneScoped;

	auto dataPtr = VulkanBufferObjectPtr(new VulkanBufferObject, [](VulkanBufferObject* obj)
		{
			vmaDestroyBuffer(vkApi::VulkanCore::sAllocator, (VkBuffer)obj->buffer, obj->alloc_meta);
			//obj->bufferInfo = vk::DescriptorBufferInfo{};
		});
	dataPtr->alloc_usage = alloc_info.usage;
	dataPtr->buffer_usage = bufferinfo.usage;
	VulkanCore::check_error(vmaCreateBuffer(vkApi::VulkanCore::sAllocator, (VkBufferCreateInfo*)&bufferinfo, &alloc_info,
		(VkBuffer*)&dataPtr->buffer, &dataPtr->alloc_meta, nullptr));

	if (dataPtr && dataPtr->buffer)
	{
		SetDeviceAddress(vVulkanCorePtr->getDevice(), dataPtr);
	}
	else
	{
		dataPtr.reset();
	}

	return dataPtr;
}

VulkanBufferObjectPtr VulkanRessource::createUniformBufferObject(vkApi::VulkanCorePtr vVulkanCorePtr, uint64_t vSize)
{
	ZoneScoped;

	vk::BufferCreateInfo sbo_create_info = {};
	VmaAllocationCreateInfo sbo_alloc_info = {};
	sbo_create_info.size = vSize;
	sbo_create_info.usage = vk::BufferUsageFlagBits::eUniformBuffer;
	sbo_alloc_info.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;

	return createSharedBufferObject(vVulkanCorePtr, sbo_create_info, sbo_alloc_info);
}

VulkanBufferObjectPtr VulkanRessource::createStagingBufferObject(vkApi::VulkanCorePtr vVulkanCorePtr, uint64_t vSize)
{
	ZoneScoped;

	vk::BufferCreateInfo stagingBufferInfo = {};
	VmaAllocationCreateInfo stagingAllocInfo = {};
	stagingBufferInfo.size = vSize;
	stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
	stagingAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;

	return createSharedBufferObject(vVulkanCorePtr, stagingBufferInfo, stagingAllocInfo);
}


VulkanBufferObjectPtr VulkanRessource::createStorageBufferObject(
	vkApi::VulkanCorePtr vVulkanCorePtr, 
	uint64_t vSize, 
	vk::BufferUsageFlags vBufferUsageFlags, 
	VmaMemoryUsage vMemoryUsage)
{
	ZoneScoped;

	vk::BufferCreateInfo storageBufferInfo = {};
	storageBufferInfo.size = vSize;
	storageBufferInfo.sharingMode = vk::SharingMode::eExclusive;
	storageBufferInfo.usage = vBufferUsageFlags;

	VmaAllocationCreateInfo storageAllocInfo = {};
	storageAllocInfo.usage = vMemoryUsage;

	return createSharedBufferObject(vVulkanCorePtr, storageBufferInfo, storageAllocInfo);
}

VulkanBufferObjectPtr VulkanRessource::createStorageBufferObject(vkApi::VulkanCorePtr vVulkanCorePtr, uint64_t vSize, VmaMemoryUsage vMemoryUsage)
{
	ZoneScoped;

	vk::BufferCreateInfo storageBufferInfo = {};
	VmaAllocationCreateInfo storageAllocInfo = {};
	storageBufferInfo.size = vSize;
	storageBufferInfo.sharingMode = vk::SharingMode::eExclusive;
	storageAllocInfo.usage = vMemoryUsage;

	if (vMemoryUsage == VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU)
	{
		storageBufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst;
		return createSharedBufferObject(vVulkanCorePtr, storageBufferInfo, storageAllocInfo);
	}
	else if (vMemoryUsage == VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_TO_CPU)
	{
		storageBufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc;
		return createSharedBufferObject(vVulkanCorePtr, storageBufferInfo, storageAllocInfo);
	}
	else if (vMemoryUsage == VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY)
	{
		storageBufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer;
		return createSharedBufferObject(vVulkanCorePtr, storageBufferInfo, storageAllocInfo);
	}

	return nullptr;
}

VulkanBufferObjectPtr VulkanRessource::createGPUOnlyStorageBufferObject(vkApi::VulkanCorePtr vVulkanCorePtr, void* vData, uint64_t vSize)
{
	if (vData && vSize)
	{
		vk::BufferCreateInfo stagingBufferInfo = {};
		stagingBufferInfo.size = vSize;
		stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		VmaAllocationCreateInfo stagingAllocInfo = {};
		stagingAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;
		auto stagebuffer = createSharedBufferObject(vVulkanCorePtr, stagingBufferInfo, stagingAllocInfo);
		upload(vVulkanCorePtr, *stagebuffer, vData, vSize);

		vk::BufferCreateInfo storageBufferInfo = {};
		storageBufferInfo.size = vSize;
		storageBufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst;
		storageBufferInfo.sharingMode = vk::SharingMode::eExclusive;
		VmaAllocationCreateInfo vboAllocInfo = {};
		vboAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
		auto vboPtr = createSharedBufferObject(vVulkanCorePtr, storageBufferInfo, vboAllocInfo);

		vk::BufferCopy region = {};
		region.size = vSize;
		copy(vVulkanCorePtr, vboPtr->buffer, stagebuffer->buffer, region);

		return vboPtr;
	}

	return nullptr;
}

VulkanBufferObjectPtr VulkanRessource::createStorageTexelBuffer(
	vkApi::VulkanCorePtr vVulkanCorePtr,
	void* vData, uint64_t vSize,
	VmaMemoryUsage vMemoryUsage)
{
	if (vData && vSize)
	{
		vk::BufferCreateInfo stagingBufferInfo = {};
		stagingBufferInfo.size = vSize;
		stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		VmaAllocationCreateInfo stagingAllocInfo = {};
		stagingAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;
		auto stagebuffer = createSharedBufferObject(vVulkanCorePtr, stagingBufferInfo, stagingAllocInfo);
		upload(vVulkanCorePtr, *stagebuffer, vData, vSize);

		vk::BufferCreateInfo storageBufferInfo = {};
		storageBufferInfo.size = vSize;
		storageBufferInfo.usage = vk::BufferUsageFlagBits::eStorageTexelBuffer | vk::BufferUsageFlagBits::eTransferDst;
		storageBufferInfo.sharingMode = vk::SharingMode::eExclusive;
		VmaAllocationCreateInfo vboAllocInfo = {};
		vboAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
		auto vboPtr = createSharedBufferObject(vVulkanCorePtr, storageBufferInfo, vboAllocInfo);

		vk::BufferCopy region = {};
		region.size = vSize;
		copy(vVulkanCorePtr, vboPtr->buffer, stagebuffer->buffer, region);

		return vboPtr;
	}

	return nullptr;
}

VulkanBufferObjectPtr VulkanRessource::createUniformTexelBuffer(
	vkApi::VulkanCorePtr vVulkanCorePtr,
	void* vData, uint64_t vSize,
	VmaMemoryUsage vMemoryUsage)
{
	if (vData && vSize)
	{
		vk::BufferCreateInfo stagingBufferInfo = {};
		stagingBufferInfo.size = vSize;
		stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		VmaAllocationCreateInfo stagingAllocInfo = {};
		stagingAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;
		auto stagebuffer = createSharedBufferObject(vVulkanCorePtr, stagingBufferInfo, stagingAllocInfo);
		upload(vVulkanCorePtr, *stagebuffer, vData, vSize);

		vk::BufferCreateInfo storageBufferInfo = {};
		storageBufferInfo.size = vSize;
		storageBufferInfo.usage = vk::BufferUsageFlagBits::eUniformTexelBuffer | vk::BufferUsageFlagBits::eTransferDst;
		storageBufferInfo.sharingMode = vk::SharingMode::eExclusive;
		VmaAllocationCreateInfo vboAllocInfo = {};
		vboAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
		auto vboPtr = createSharedBufferObject(vVulkanCorePtr, storageBufferInfo, vboAllocInfo);

		vk::BufferCopy region = {};
		region.size = vSize;
		copy(vVulkanCorePtr, vboPtr->buffer, stagebuffer->buffer, region);

		return vboPtr;
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////
//// RTX / ACCEL STRUCTURE ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void VulkanRessource::SetDeviceAddress(const vk::Device& vDevice, VulkanAccelStructObjectPtr vVulkanAccelStructObjectPtr)
{
	if (vDevice && 
		vVulkanAccelStructObjectPtr && 
		(vVulkanAccelStructObjectPtr->buffer_usage & vk::BufferUsageFlagBits::eShaderDeviceAddress))
	{
		vk::BufferDeviceAddressInfoKHR bufferDeviceAddressInfo{};
		bufferDeviceAddressInfo.buffer = vVulkanAccelStructObjectPtr->buffer;
		vVulkanAccelStructObjectPtr->device_address = vDevice.getBufferAddressKHR(&bufferDeviceAddressInfo);
	}
}

VulkanAccelStructObjectPtr VulkanRessource::createAccelStructureBufferObject(VulkanCorePtr vVulkanCorePtr, uint64_t vSize, VmaMemoryUsage vMemoryUsage)
{
	ZoneScoped;

	vk::BufferCreateInfo storageBufferInfo = {};
	storageBufferInfo.size = vSize;
	storageBufferInfo.sharingMode = vk::SharingMode::eExclusive;

	if (vMemoryUsage == VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY)
	{
		storageBufferInfo.usage = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR;
	}
	
	VmaAllocationCreateInfo storageAllocInfo = {};
	storageAllocInfo.usage = vMemoryUsage;

	auto dataPtr = VulkanAccelStructObjectPtr(new VulkanAccelStructObject, [](VulkanAccelStructObject* obj)
	{
		vmaDestroyBuffer(vkApi::VulkanCore::sAllocator, (VkBuffer)obj->buffer, obj->alloc_meta);
	});
	dataPtr->alloc_usage = storageAllocInfo.usage;
	dataPtr->buffer_usage = storageBufferInfo.usage;

	VulkanCore::check_error(vmaCreateBuffer(vkApi::VulkanCore::sAllocator, (VkBufferCreateInfo*)&storageBufferInfo, &storageAllocInfo,
		(VkBuffer*)&dataPtr->buffer, &dataPtr->alloc_meta, nullptr));

	if (dataPtr && dataPtr->buffer)
	{
		SetDeviceAddress(vVulkanCorePtr->getDevice(), dataPtr);
	}
	else
	{
		dataPtr.reset();
	}

	return dataPtr;
}
}