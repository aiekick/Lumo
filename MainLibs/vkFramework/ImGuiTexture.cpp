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

#include "ImGuiTexture.h"
#include <vkFramework/VulkanImGuiRenderer.h>
#include <vkFramework/VulkanFrameBufferAttachment.h>
#include <ctools/cTools.h>
#include <ctools/Logger.h>

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

ImGuiTexturePtr ImGuiTexture::Create()
{
	ImGuiTexturePtr res = std::make_shared<ImGuiTexture>();
	res->m_This = res;
	return res;
}

ImGuiTexture::ImGuiTexture() 
{
	ZoneScoped;
}

ImGuiTexture::~ImGuiTexture()
{
	ZoneScoped;
}

void ImGuiTexture::SetDescriptor(VulkanImGuiRenderer* vVulkanImGuiRenderer, vk::DescriptorImageInfo* vDescriptorImageInfo, float vRatio)
{
	ZoneScoped;

	assert(vVulkanImGuiRenderer);

	if (vDescriptorImageInfo)
	{
		if (IS_FLOAT_DIFFERENT(vRatio, 0.0f))
			ratio = vRatio;

		if (firstLoad)
		{
			descriptor = vVulkanImGuiRenderer->CreateImGuiTexture(
				(VkSampler)vDescriptorImageInfo->sampler,
				(VkImageView)vDescriptorImageInfo->imageView,
				(VkImageLayout)vDescriptorImageInfo->imageLayout);
			firstLoad = false;
		}
		else
		{
			descriptor = vVulkanImGuiRenderer->CreateImGuiTexture(
				(VkSampler)vDescriptorImageInfo->sampler,
				(VkImageView)vDescriptorImageInfo->imageView,
				(VkImageLayout)vDescriptorImageInfo->imageLayout,
				&descriptor);
		}

		canDisplayPreview = true;
	}
	else
	{
		ClearDescriptor();
	}
}

void ImGuiTexture::SetDescriptor(VulkanImGuiRenderer* vVulkanImGuiRenderer, vkApi::VulkanFrameBufferAttachment* vVulkanFrameBufferAttachment)
{
	ZoneScoped;

	assert(vVulkanImGuiRenderer);

	if (vVulkanFrameBufferAttachment)
	{
		ratio = (float)vVulkanFrameBufferAttachment->width / (float)vVulkanFrameBufferAttachment->height;

		if (firstLoad)
		{
			descriptor = vVulkanImGuiRenderer->CreateImGuiTexture(
				(VkSampler)vVulkanFrameBufferAttachment->attachmentDescriptorInfo.sampler,
				(VkImageView)vVulkanFrameBufferAttachment->attachmentDescriptorInfo.imageView,
				(VkImageLayout)vVulkanFrameBufferAttachment->attachmentDescriptorInfo.imageLayout);
			firstLoad = false;
		}
		else
		{
			descriptor = vVulkanImGuiRenderer->CreateImGuiTexture(
				(VkSampler)vVulkanFrameBufferAttachment->attachmentSampler,
				(VkImageView)vVulkanFrameBufferAttachment->attachmentView,
				(VkImageLayout)vVulkanFrameBufferAttachment->attachmentDescriptorInfo.imageLayout,
				&descriptor);
		}

		canDisplayPreview = true;
	}
	else
	{
		ClearDescriptor();
	}
}

void ImGuiTexture::ClearDescriptor()
{
	ZoneScoped;

	canDisplayPreview = false;
	firstLoad = true;
	descriptor = vk::DescriptorSet{};
}

void ImGuiTexture::DestroyDescriptor(VulkanImGuiRenderer* vVulkanImGuiRenderer)
{
	ZoneScoped;

	if (!destroyed)
	{
		if (vVulkanImGuiRenderer->DestroyImGuiTexture(&descriptor))
		{
			destroyed = true;
		}
	}
	else
	{
#if _DEBUG
		CTOOL_DEBUG_BREAK;
#endif
	}
}