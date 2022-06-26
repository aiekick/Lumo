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
#include <mutex>
#include <thread>

namespace vkApi
{
	class VulkanCore;
	class VulkanCommandBuffer
	{
	public:
		static vk::CommandBuffer beginSingleTimeCommands(vkApi::VulkanCore* vVulkanCore, bool begin, vk::CommandPool* vCommandPool = 0);
		static void flushSingleTimeCommands(vkApi::VulkanCore* vVulkanCore, vk::CommandBuffer& cmd, bool end, vk::CommandPool* vCommandPool = 0);
		static VulkanCommandBuffer CreateCommandBuffer(vkApi::VulkanCore* vVulkanCore, vk::QueueFlagBits vQueueType, vk::CommandPool* vCommandPool = 0);
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
		VulkanCore* m_VulkanCore = nullptr;

	public:
		void DestroyCommandBuffer();
		bool ResetFence();
		bool Begin();
		void End();
		bool SubmitCmd(vk::PipelineStageFlags vDstStage);
		bool SubmitCmd(vk::SubmitInfo vSubmitInfo);
	};
}