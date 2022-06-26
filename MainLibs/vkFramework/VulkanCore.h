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
#include "vk_mem_alloc.h"
#include <glm/glm.hpp>

#include <ctools/cTools.h>

#include "VulkanWindow.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"

#include <string>
#include <functional>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <array>

#include <vkProfiler/Profiler.h> // for TracyVkCtx

class VulkanImGuiRenderer;
class VulkanShader;
struct GLFWwindow;
namespace vkApi
{
	class VulkanCore
	{
	public:
		static VmaAllocator sAllocator;
		static std::shared_ptr<VulkanShader> sVulkanShader;

	protected:
		VulkanSwapChain m_VulkanSwapChain;
		VulkanDevice m_VulkanDevice;
		TracyVkCtx m_TracyContext = nullptr;
		ct::cWeak<VulkanImGuiRenderer> m_VulkanImGuiRenderer;

	protected:
		std::vector<vk::CommandBuffer> m_CommandBuffers;
		std::vector<vk::Semaphore> m_ComputeCompleteSemaphores;
		std::vector<vk::Fence> m_ComputeWaitFences;
		std::vector<vk::CommandBuffer> m_ComputeCommandBuffers;
		vk::DescriptorPool m_DescriptorPool;
		vk::PipelineCache m_PipelineCache = nullptr;
		bool m_CreateSwapChain = false;

	public:
		static void check_error(vk::Result result);
		static void check_error(VkResult result);

	public:
		void Init(VulkanWindow* vVulkanWindow, const char* vAppName, const int& vAppVersion, const char* vEngineName, const int& vEngineVersion, bool vCreateSwapChain);
		void Unit();

	public: // get
		vk::Instance getInstance() const;
		vk::PhysicalDevice getPhysicalDevice() const;
		vk::Device getDevice() const;
		VulkanDevice* getFrameworkDevice();
		vk::DispatchLoaderDynamic GetDynamicLoader() const;
		vk::DescriptorPool getDescriptorPool() const;
		vk::RenderPass getMainRenderPass() const;
		vk::CommandBuffer getGraphicCommandBuffer() const;
		vk::CommandBuffer getComputeCommandBuffer() const;
		vk::SurfaceKHR getSurface() const;
		VulkanSwapChain getSwapchain() const;
		std::vector<vk::Semaphore> getPresentSemaphores();
		std::vector<vk::Semaphore> getRenderSemaphores();
		vk::Viewport getViewport() const;
		vk::Rect2D getRenderArea() const;
		VulkanQueue getQueue(vk::QueueFlagBits vQueueType);
		TracyVkCtx getTracyContext();
		vk::SampleCountFlagBits getSwapchainFrameBufferSampleCount() const;

		void SetVulkanImGuiRenderer(ct::cWeak<VulkanImGuiRenderer> vVulkanShader);
		ct::cWeak<VulkanImGuiRenderer> GetVulkanImGuiRenderer();

		// from device
		vk::SampleCountFlagBits GetMaxUsableSampleCount();

		void setupMemoryAllocator();
	public:
		void resize();

	public:// graphic
		bool frameBegin();
		void beginMainRenderPass();
		void endMainRenderPass();
		void frameEnd();

	public:// compute
		bool resetComputeFence();
		bool computeBegin();
		bool computeEnd();
		bool submitComputeCmd(vk::CommandBuffer vCmd);

	public: // KHR
		bool AcquireNextImage(VulkanWindow* vVulkanWindow);
		void Present();
		uint32_t getSwapchainFrameBuffers() const;
		bool justGainFocus();
		ct::frect* getDisplayRect();

	protected:

		void setupGraphicCommandsAndSynchronization();
		void destroyGraphicCommandsAndSynchronization();

		void setupComputeCommandsAndSynchronization();
		void destroyComputeCommandsAndSynchronization();

		void setupDescriptorPool();

	public:
		VulkanCore() {} // Prevent construction
		VulkanCore(const VulkanCore&) {}; // Prevent construction by copying
		VulkanCore& operator =(const VulkanCore&) { return *this; }; // Prevent assignment
		~VulkanCore() {} // Prevent unwanted destruction
	};
}
