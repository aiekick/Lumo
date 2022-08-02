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
#include <mutex>
#include <thread>
#include <vkFramework/vkFramework.h>

namespace vkApi
{
	class VulkanCore;
	class VulkanCommandBuffer
	{
	public:
		static vk::CommandBuffer beginSingleTimeCommands(vkApi::VulkanCorePtr vVulkanCorePtr, bool begin, vk::CommandPool* vCommandPool = 0);
		static void flushSingleTimeCommands(vkApi::VulkanCorePtr vVulkanCorePtr, vk::CommandBuffer& cmd, bool end, vk::CommandPool* vCommandPool = 0);
		static VulkanCommandBuffer CreateCommandBuffer(vkApi::VulkanCorePtr vVulkanCorePtr, vk::QueueFlagBits vQueueType, vk::CommandPool* vCommandPool = 0);
		static std::mutex VulkanCommandBuffer_Mutex;

	public:
		vk::CommandBuffer cmd;
		vk::Fence fence;
		vk::QueueFlagBits type;
		vk::Queue queue;
		uint32_t familyQueueIndex = 0;
		vk::Device device;
		vk::CommandPool commandpool;

	private:
		VulkanCorePtr m_VulkanCorePtr = nullptr;

	public:
		void DestroyCommandBuffer();
		bool ResetFence();
		bool Begin();
		void End();
		bool SubmitCmd(vk::PipelineStageFlags vDstStage);
		bool SubmitCmd(vk::SubmitInfo vSubmitInfo);
	};
}