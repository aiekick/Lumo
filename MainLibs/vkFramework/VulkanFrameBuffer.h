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
#include "VulkanFrameBufferAttachment.h"
#include <ctools/cTools.h>

namespace vkApi
{
	class VulkanCore;
	class VulkanFrameBuffer
	{
	public:
		std::vector<VulkanFrameBufferAttachment> attachments;
		std::vector<vk::ImageView> attachmentViews;
		std::vector<vk::ClearAttachment> attachmentClears;
		std::vector<vk::ClearValue> clearColorValues;
		std::vector<vk::ClearRect> rectClears;
		uint32_t mipLevelCount = 1u;
		uint32_t width = 0u;
		uint32_t height = 0u;
		vk::Format format = vk::Format::eR32G32B32A32Sfloat;
		float ratio = 0.0f;
		vk::Framebuffer framebuffer;
		bool neverCleared = true;
		bool neverToClear = false;
		vk::SampleCountFlagBits sampleCount;
		uint32_t depthAttIndex = 0U;

	private:
		VulkanCore* m_VulkanCore = nullptr;

	public:
		VulkanFrameBuffer();
		~VulkanFrameBuffer();

	public:
		bool Init(
			vkApi::VulkanCore* vVulkanCore,
			ct::uvec2 vSize,
			uint32_t vCount,
			vk::RenderPass& vRenderPass,
			bool vCreateRenderPass,
			bool vUseDepth = false,
			bool vNeedToClear = false,
			ct::fvec4 vClearColor = 0.0f,
			vk::Format vFormat = vk::Format::eR32G32B32A32Sfloat,
			vk::SampleCountFlagBits vSampleCount = vk::SampleCountFlagBits::e1);
		void Unit();

		VulkanFrameBufferAttachment* GetDepthAttachment();
	};
}