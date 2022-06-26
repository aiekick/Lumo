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
#include <vkFramework/VulkanRessource.h>
#include <ctools/cTools.h>

namespace vkApi
{
	class VulkanFrameBufferAttachment
	{
	public:
		std::shared_ptr<VulkanRessourceObject> attachment = nullptr;
		vk::ImageView attachmentView = {};
		vk::Sampler attachmentSampler = {};
		vk::DescriptorImageInfo attachmentDescriptorInfo = {};
		vk::AttachmentDescription attachmentDescription = {}; // pour al renderpass
		uint32_t mipLevelCount = 1U;
		uint32_t width = 0u;
		uint32_t height = 0u;
		vk::Format format = vk::Format::eR32G32B32A32Sfloat;
		float ratio = 0.0f;
		vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;

	private:
		vkApi::VulkanCore* m_VulkanCore = nullptr;

	public:
		VulkanFrameBufferAttachment() = default;
		~VulkanFrameBufferAttachment();

	public:
		bool InitColor2D(vkApi::VulkanCore* vVulkanCore, ct::uvec2 vSize, vk::Format vFormat, uint32_t vMipLevelCount, bool vNeedToClear, vk::SampleCountFlagBits vSampleCount);
		bool InitDepth(vkApi::VulkanCore* vVulkanCore, ct::uvec2 vSize, vk::Format vFormat, vk::SampleCountFlagBits vSampleCount);
		void Unit();
	};
}