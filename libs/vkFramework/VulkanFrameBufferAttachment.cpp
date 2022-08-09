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

#include "VulkanFrameBufferAttachment.h"
#include "VulkanCore.h"

#define TRACE_MEMORY
#include <vkprofiler/Profiler.h>

namespace vkApi
{
	VulkanFrameBufferAttachment::~VulkanFrameBufferAttachment()
	{
		ZoneScoped;

		Unit();
	}

	bool VulkanFrameBufferAttachment::InitColor2D(
		vkApi::VulkanCorePtr vVulkanCorePtr,
		ct::uvec2 vSize,
		vk::Format vFormat,
		uint32_t vMipLevelCount,
		bool vNeedToClear,
		vk::SampleCountFlagBits vSampleCount)
	{
		ZoneScoped;

		m_VulkanCorePtr = vVulkanCorePtr;

		bool res = false;

		ct::uvec2 size = ct::clamp(vSize, 1u, 8192u);
		if (!size.emptyOR())
		{
			mipLevelCount = vMipLevelCount;
			width = size.x;
			height = size.y;
			format = vFormat;
			ratio = (float)height / (float)width;
			sampleCount = vSampleCount;

			attachment = VulkanRessource::createColorAttachment2D(m_VulkanCorePtr, width, height, mipLevelCount, format, sampleCount);

			vk::ImageViewCreateInfo imViewInfo = {};
			imViewInfo.flags = vk::ImageViewCreateFlags();
			imViewInfo.image = attachment->image;
			imViewInfo.viewType = vk::ImageViewType::e2D;
			imViewInfo.format = format;
			imViewInfo.components = vk::ComponentMapping();
			imViewInfo.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, mipLevelCount, 0, 1);
			attachmentView = m_VulkanCorePtr->getDevice().createImageView(imViewInfo);

			vk::SamplerCreateInfo samplerInfo = {};
			samplerInfo.flags = vk::SamplerCreateFlags();
			samplerInfo.magFilter = vk::Filter::eLinear;
			samplerInfo.minFilter = vk::Filter::eLinear;
			samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
			samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge; // U
			samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge; // V
			samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge; // W
			//samplerInfo.mipLodBias = 0.0f;
			//samplerInfo.anisotropyEnable = false;
			//samplerInfo.maxAnisotropy = 0.0f;
			//samplerInfo.compareEnable = false;
			//samplerInfo.compareOp = vk::CompareOp::eAlways;
			//samplerInfo.minLod = 0.0f;
			//samplerInfo.maxLod = static_cast<float>(m_MipLevelCount);
			//samplerInfo.unnormalizedCoordinates = false;
			attachmentSampler = m_VulkanCorePtr->getDevice().createSampler(samplerInfo);

			attachmentDescriptorInfo.sampler = attachmentSampler;
			attachmentDescriptorInfo.imageView = attachmentView;
			attachmentDescriptorInfo.imageLayout = vk::ImageLayout::eAttachmentOptimal;

			attachmentDescription.flags = vk::AttachmentDescriptionFlags();
			attachmentDescription.format = format;
			attachmentDescription.samples = vSampleCount;

			if (vNeedToClear)
			{
				attachmentDescription.loadOp = vk::AttachmentLoadOp::eClear;
			}
			else
			{
				if (sampleCount != vk::SampleCountFlagBits::e1)
				{
					attachmentDescription.loadOp = vk::AttachmentLoadOp::eDontCare;
				}
				else
				{
					attachmentDescription.loadOp = vk::AttachmentLoadOp::eLoad;
				}
			}

			if (sampleCount != vk::SampleCountFlagBits::e1)
			{
				attachmentDescription.storeOp = vk::AttachmentStoreOp::eDontCare;
			}
			else
			{
				attachmentDescription.storeOp = vk::AttachmentStoreOp::eStore;
			}

			attachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			attachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
			attachmentDescription.finalLayout = vk::ImageLayout::eAttachmentOptimal;

			res = true;
		}

		return res;
	}

	bool VulkanFrameBufferAttachment::InitDepth(
		vkApi::VulkanCorePtr vVulkanCorePtr, 
		ct::uvec2 vSize, 
		vk::Format vFormat, 
		vk::SampleCountFlagBits vSampleCount)
	{
		ZoneScoped;

		m_VulkanCorePtr = vVulkanCorePtr;

		bool res = false;

		ct::uvec2 size = ct::clamp(vSize, 1u, 8192u);
		if (!size.emptyOR())
		{
			mipLevelCount = 1U;
			width = size.x;
			height = size.y;
			format = vFormat;
			ratio = (float)height / (float)width;
			sampleCount = vSampleCount;

			attachment = VulkanRessource::createDepthAttachment(m_VulkanCorePtr, width, height, format, sampleCount);

			vk::ImageViewCreateInfo imViewInfo = {};
			imViewInfo.flags = vk::ImageViewCreateFlags();
			imViewInfo.image = attachment->image;
			imViewInfo.viewType = vk::ImageViewType::e2D;
			imViewInfo.format = format;
			imViewInfo.components = vk::ComponentMapping();
			imViewInfo.subresourceRange = vk::ImageSubresourceRange(
				vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
				0,
				1,
				0,
				1);
			attachmentView = m_VulkanCorePtr->getDevice().createImageView(imViewInfo);

			vk::SamplerCreateInfo samplerInfo = {};
			samplerInfo.flags = vk::SamplerCreateFlags();
			samplerInfo.magFilter = vk::Filter::eLinear;
			samplerInfo.minFilter = vk::Filter::eLinear;
			samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
			samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge; // U
			samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge; // V
			samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge; // W
			//samplerInfo.mipLodBias = 0.0f;
			//samplerInfo.anisotropyEnable = false;
			//samplerInfo.maxAnisotropy = 0.0f;
			//samplerInfo.compareEnable = false;
			//samplerInfo.compareOp = vk::CompareOp::eAlways;
			//samplerInfo.minLod = 0.0f;
			//samplerInfo.maxLod = static_cast<float>(m_MipLevelCount);
			//samplerInfo.unnormalizedCoordinates = false;
			attachmentSampler = m_VulkanCorePtr->getDevice().createSampler(samplerInfo);

			attachmentDescriptorInfo.sampler = attachmentSampler;
			attachmentDescriptorInfo.imageView = attachmentView;
			attachmentDescriptorInfo.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

			attachmentDescription.flags = vk::AttachmentDescriptionFlags();
			attachmentDescription.format = format;
			attachmentDescription.samples = sampleCount;
			attachmentDescription.loadOp = vk::AttachmentLoadOp::eClear;
			attachmentDescription.storeOp = vk::AttachmentStoreOp::eStore;
			attachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eClear;
			attachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			attachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
			attachmentDescription.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

			res = true;
		}

		return res;
	}

	void VulkanFrameBufferAttachment::Unit()
	{
		ZoneScoped;

		attachment.reset();
		m_VulkanCorePtr->getDevice().destroyImageView(attachmentView);
		m_VulkanCorePtr->getDevice().destroySampler(attachmentSampler);
	}
}