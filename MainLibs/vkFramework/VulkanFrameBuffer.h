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
#include "VulkanFrameBufferAttachment.h"
#include <ctools/cTools.h>
#include <vkFramework/vkFramework.h>

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
		vk::Framebuffer framebuffer = nullptr;
		bool neverCleared = true;
		bool neverToClear = false;
		vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;
		uint32_t depthAttIndex = 0U;

	private:
		VulkanCorePtr m_VulkanCorePtr = nullptr;

	public:
		VulkanFrameBuffer();
		~VulkanFrameBuffer();

	public:
		bool Init(
			vkApi::VulkanCorePtr vVulkanCorePtr,
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