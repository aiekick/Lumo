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
#pragma warning(disable : 4251)

#include <vulkan/vulkan.hpp>
#include <gaia/VulkanRessource.h>
#include <ctools/cTools.h>
#include <gaia/gaia.h>

namespace vkApi
{
	class GAIA_API VulkanFrameBufferAttachment
	{
	public:
		std::shared_ptr<VulkanImageObject> attachment = nullptr;
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
		vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;

	public:
		VulkanFrameBufferAttachment() = default;
		~VulkanFrameBufferAttachment();

	public:
		bool InitColor2D(vkApi::VulkanCorePtr vVulkanCorePtr, ct::uvec2 vSize, vk::Format vFormat, uint32_t vMipLevelCount, bool vNeedToClear, vk::SampleCountFlagBits vSampleCount);
		bool InitDepth(vkApi::VulkanCorePtr vVulkanCorePtr, ct::uvec2 vSize, vk::Format vFormat, vk::SampleCountFlagBits vSampleCount);
		void Unit();
	};
}