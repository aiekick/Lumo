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

#include "VulkanCore.h"

#include <ctools/Logger.h>
#include "VulkanSubmitter.h"

#define RECORD_VM_ALLOCATION

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <ctools/cTools.h>

#include <cstdio>			// printf, fprintf
#include <cstdlib>			// abort
#include <iostream>			// std::cout
#include <stdexcept>		// std::exception
#include <algorithm>		// std::min, std::max
#include <fstream>			// std::ifstream
#include <chrono>			// timer

#include <map>
#include <fstream>

#define VMA_IMPLEMENTATION
//#define VMA_DEBUG_LOG
//#define VMA_DEBUG_ALWAYS_DEDICATED_MEMORY (1)
//#define VMA_DEBUG_ALIGNMENT (1)
//#define VMA_DEBUG_MARGIN (4)
//#define VMA_DEBUG_INITIALIZE_ALLOCATIONS (1)
//#define VMA_DEBUG_DETECT_CORRUPTION (1)
//#define VMA_DEBUG_GLOBAL_MUTEX (1)
//#define VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY (1)
//#define VMA_SMALL_HEAP_MAX_SIZE (1)
//#define VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE (1)
//#ifdef RECORD_VM_ALLOCATION
//#define VMA_RECORDING_ENABLED 1
//#endif
#include "vk_mem_alloc.h"

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

static bool sGainFocus = false;

namespace vkApi
{
	VmaAllocator VulkanCore::sAllocator = nullptr;
	std::shared_ptr<VulkanShader> VulkanCore::sVulkanShader = nullptr;

	static void window_focus_callback(GLFWwindow* /*window*/, int focused)
	{
		ZoneScoped;

		if (focused)
		{
			sGainFocus = true;
		}
		else
		{
			sGainFocus = false;
		}
	}

	bool VulkanCore::justGainFocus()
	{
		ZoneScoped;

		bool res = sGainFocus;
		if (res)
			sGainFocus = false;
		return res;
	}

	void VulkanCore::check_error(vk::Result result)
	{
		ZoneScoped;

		if (result != vk::Result::eSuccess)
		{
			LogVarDebug("Debug : vulkan: error %s", vk::to_string(result).c_str());
		}
	}

	// MEMBERS
	void VulkanCore::Init(VulkanWindow* vVulkanWindow, const char* vAppName, const int& vAppVersion, const char* vEngineName, const int& vEngineVersion, bool vCreateSwapChain)
	{
		ZoneScoped;

		m_CreateSwapChain = vCreateSwapChain;

		glfwSetWindowFocusCallback(vVulkanWindow->WinPtr(), window_focus_callback);

		m_VulkanDevice.Init(vVulkanWindow, vAppName, vAppVersion, vEngineName, vEngineVersion);
		setupMemoryAllocator();
		if (m_CreateSwapChain)
		{
			m_VulkanSwapChain.Init(vVulkanWindow, this, std::bind(&VulkanCore::resize, this));
		}
		setupGraphicCommandsAndSynchronization();
		setupComputeCommandsAndSynchronization();
		setupDescriptorPool();

#ifdef TRACY_ENABLE
#ifdef ENABLE_CALIBRATED_CONTEXT
		m_TracyContext = TracyVkContextCalibrated(
			m_VulkanCore->getPhysicalDevice(),
			m_VulkanCore->getDevice(),
			m_VulkanCore->getQueue(vk::QueueFlagBits::eGraphics).vkQueue,
			m_VulkanCore->getGraphicCommandBuffer(),
			vkGetPhysicalDeviceCalibrateableTimeDomainsEXT,
			vkGetCalibratedTimestampsEXT);
#else
		m_TracyContext = TracyVkContext(
			m_VulkanCore->getPhysicalDevice(),
			m_VulkanCore->getDevice(),
			m_VulkanCore->getQueue(vk::QueueFlagBits::eGraphics).vkQueue,
			m_VulkanCore->getGraphicCommandBuffer());
#endif

		tracy::SetThreadName("Main");
#endif
	}

	void VulkanCore::Unit()
	{
		ZoneScoped;

		m_VulkanDevice.WaitIdle();

		TracyVkDestroy(m_TracyContext);

		m_VulkanDevice.m_LogDevice.destroyDescriptorPool(m_DescriptorPool);

		destroyComputeCommandsAndSynchronization();
		destroyGraphicCommandsAndSynchronization();

		if (m_CreateSwapChain)
		{
			m_VulkanSwapChain.Unit();
		}

		vmaDestroyAllocator(VulkanCore::sAllocator);

		m_VulkanDevice.Unit();
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//// PUBLIC // QUICK GET /////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	vk::RenderPass VulkanCore::getMainRenderPass() const { return m_VulkanSwapChain.m_RenderPass; }
	vk::CommandBuffer VulkanCore::getGraphicCommandBuffer() const { return m_CommandBuffers[m_VulkanSwapChain.m_FrameIndex]; }
	vk::SurfaceKHR VulkanCore::getSurface() const { return m_VulkanSwapChain.getSurface(); }
	VulkanSwapChain VulkanCore::getSwapchain() const { return m_VulkanSwapChain; }
	vk::Viewport VulkanCore::getViewport() const { return m_VulkanSwapChain.getViewport(); }
	vk::Rect2D VulkanCore::getRenderArea() const { return m_VulkanSwapChain.getRenderArea(); }
	std::vector<vk::Semaphore>VulkanCore::getPresentSemaphores() { return m_VulkanSwapChain.m_PresentCompleteSemaphores; }
	std::vector<vk::Semaphore> VulkanCore::getRenderSemaphores() { return m_VulkanSwapChain.m_PresentCompleteSemaphores; }
	vk::SampleCountFlagBits VulkanCore::getSwapchainFrameBufferSampleCount() const { return m_VulkanSwapChain.getSwapchainFrameBufferSampleCount(); }

	vk::Instance VulkanCore::getInstance() const { return m_VulkanDevice.m_Instance; }
	vk::PhysicalDevice VulkanCore::getPhysicalDevice() const { return m_VulkanDevice.m_PhysDevice; }
	vk::Device VulkanCore::getDevice() const { return m_VulkanDevice.m_LogDevice; }
	VulkanDevice* VulkanCore::getFrameworkDevice() { return &m_VulkanDevice; }
	vk::DispatchLoaderDynamic VulkanCore::GetDynamicLoader() const { return m_VulkanDevice.m_Dldy; }
	vk::DescriptorPool VulkanCore::getDescriptorPool() const { return m_DescriptorPool; }
	vk::CommandBuffer VulkanCore::getComputeCommandBuffer() const { return m_ComputeCommandBuffers[0]; }
	VulkanQueue VulkanCore::getQueue(vk::QueueFlagBits vQueueType) { return m_VulkanDevice.getQueue(vQueueType); }
	TracyVkCtx VulkanCore::getTracyContext() { return m_TracyContext; }

	void VulkanCore::SetVulkanImGuiRenderer(ct::cWeak<VulkanImGuiRenderer> vVulkanImGuiRenderer) { m_VulkanImGuiRenderer = vVulkanImGuiRenderer; }
	ct::cWeak<VulkanImGuiRenderer> VulkanCore::GetVulkanImGuiRenderer() { return m_VulkanImGuiRenderer; }
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//// PUBLIC // GET INFOS /////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	vk::SampleCountFlagBits VulkanCore::GetMaxUsableSampleCount()
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties((VkPhysicalDevice)m_VulkanDevice.m_PhysDevice, &physicalDeviceProperties);

		VkSampleCountFlags counts = 
			physicalDeviceProperties.limits.framebufferColorSampleCounts & 
			physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return vk::SampleCountFlagBits::e64; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return vk::SampleCountFlagBits::e32; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return vk::SampleCountFlagBits::e16; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return vk::SampleCountFlagBits::e8; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return vk::SampleCountFlagBits::e4; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return vk::SampleCountFlagBits::e2; }

		return vk::SampleCountFlagBits::e1;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//// PUBLIC //////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	void VulkanCore::check_error(VkResult result)
	{
		ZoneScoped;

		check_error(vk::Result(result));
	}

	void VulkanCore::resize()
	{
		ZoneScoped;

		m_VulkanDevice.WaitIdle();

		destroyGraphicCommandsAndSynchronization();

		if (m_CreateSwapChain)
		{
			m_VulkanSwapChain.Reload();
		}

		setupGraphicCommandsAndSynchronization();
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//// GRAPHIC //////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool VulkanCore::frameBegin()
	{
		ZoneScoped;

		FrameMark;

		if (m_CreateSwapChain)
		{
			if (m_VulkanDevice.m_LogDevice.waitForFences(1, 
				&m_VulkanSwapChain.m_WaitFences[m_VulkanSwapChain.m_FrameIndex], VK_TRUE, UINT64_MAX) == vk::Result::eSuccess)
			{
				if (m_VulkanDevice.m_LogDevice.resetFences(1, &m_VulkanSwapChain.m_WaitFences[m_VulkanSwapChain.m_FrameIndex]) == vk::Result::eSuccess)
				{
					m_CommandBuffers[m_VulkanSwapChain.m_FrameIndex].reset(vk::CommandBufferResetFlagBits::eReleaseResources);

					m_CommandBuffers[m_VulkanSwapChain.m_FrameIndex].begin(vk::CommandBufferBeginInfo());

					return true;
				}
			}
		}

		return false;
	}

	void VulkanCore::beginMainRenderPass()
	{
		ZoneScoped;

		if (m_CreateSwapChain)
		{
			m_CommandBuffers[m_VulkanSwapChain.m_FrameIndex].beginRenderPass(
				vk::RenderPassBeginInfo(
					m_VulkanSwapChain.m_RenderPass,
					m_VulkanSwapChain.m_SwapchainFrameBuffers[m_VulkanSwapChain.m_FrameIndex].frameBuffer,
					m_VulkanSwapChain.m_RenderArea,
					static_cast<uint32_t>(m_VulkanSwapChain.m_ClearValues.size()),
					m_VulkanSwapChain.m_ClearValues.data()),
				vk::SubpassContents::eInline);
		}
	}

	void VulkanCore::endMainRenderPass()
	{
		ZoneScoped;

		if (m_CreateSwapChain)
		{
			m_CommandBuffers[m_VulkanSwapChain.m_FrameIndex].endRenderPass();
		}
	}

	void VulkanCore::frameEnd()
	{
		ZoneScoped;

		if (m_CreateSwapChain)
		{
			m_CommandBuffers[m_VulkanSwapChain.m_FrameIndex].end();

			vk::SubmitInfo submitInfo;
			vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			submitInfo
				.setWaitSemaphoreCount(1)
				.setPWaitSemaphores(&m_VulkanSwapChain.m_PresentCompleteSemaphores[m_VulkanSwapChain.m_FrameIndex])
				.setPWaitDstStageMask(&waitDstStageMask)
				.setCommandBufferCount(1)
				.setPCommandBuffers(&m_CommandBuffers[m_VulkanSwapChain.m_FrameIndex])
				.setSignalSemaphoreCount(1)
				.setPSignalSemaphores(&m_VulkanSwapChain.m_RenderCompleteSemaphores[m_VulkanSwapChain.m_FrameIndex]);

			std::unique_lock<std::mutex> lck(VulkanSubmitter::criticalSectionMutex, std::defer_lock);
			lck.lock();
			auto result = getQueue(vk::QueueFlagBits::eGraphics).vkQueue.submit(1, &submitInfo, 
				m_VulkanSwapChain.m_WaitFences[m_VulkanSwapChain.m_FrameIndex]);
			if (result == vk::Result::eErrorDeviceLost)
			{
				// driver lost, we'll crash in this case:
				LogVarDebug("Debug : Driver Lost after submit");
			}
			lck.unlock();
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//// COMPUTE //////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool VulkanCore::resetComputeFence()
	{
		ZoneScoped;

		return (m_VulkanDevice.m_LogDevice.resetFences(1, 
			&m_ComputeWaitFences[0]) == vk::Result::eSuccess);
	}

	bool VulkanCore::computeBegin()
	{
		ZoneScoped;

		if (!m_ComputeCommandBuffers.empty())
		{
			auto cmd = m_ComputeCommandBuffers[0];

			resetComputeFence();

			cmd.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
			cmd.begin(vk::CommandBufferBeginInfo());

			return true;
		}

		return false;
	}

	bool VulkanCore::computeEnd()
	{
		ZoneScoped;

		if (!m_ComputeCommandBuffers.empty())
		{
			auto cmd = m_ComputeCommandBuffers[0];

			cmd.end();

			return submitComputeCmd(cmd);
		}

		return false;
	}

	bool VulkanCore::submitComputeCmd(vk::CommandBuffer vCmd)
	{
		ZoneScoped;

		if (vCmd)
		{
			vk::SubmitInfo submitInfo;
			vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eComputeShader;
			submitInfo
				//.setWaitSemaphoreCount(1)
				//.setPWaitSemaphores(&m_PresentCompleteSemaphores[0])
				.setPWaitDstStageMask(&waitDstStageMask)
				.setCommandBufferCount(1)
				.setPCommandBuffers(&vCmd)
				//.setSignalSemaphoreCount(1)
				//.setPSignalSemaphores(&m_PresentCompleteSemaphores[0])
				;

			if (VulkanSubmitter::Submit(this, vk::QueueFlagBits::eCompute, 
				submitInfo, m_ComputeWaitFences[0]))
			{
				return (m_VulkanDevice.m_LogDevice.waitForFences(1, 
					&m_ComputeWaitFences[0], VK_TRUE, UINT64_MAX) == vk::Result::eSuccess);
			}
		}

		return false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool VulkanCore::AcquireNextImage(VulkanWindow* vVulkanWindow)
	{
		ZoneScoped;

		assert(vVulkanWindow);

		if (!vVulkanWindow->IsMinimized())
		{
			if (m_CreateSwapChain)
			{
				return m_VulkanSwapChain.AcquireNextImage();
			}
		}

		return false;
	}

	void VulkanCore::Present()
	{
		ZoneScoped;

		if (m_CreateSwapChain)
		{
			if (m_VulkanDevice.m_LogDevice.waitForFences(1, 
				&m_VulkanSwapChain.m_WaitFences[m_VulkanSwapChain.m_FrameIndex], 
				VK_TRUE, UINT64_MAX) == vk::Result::eSuccess)
			{
				m_VulkanSwapChain.Present();
			}
		}
	}

	uint32_t VulkanCore::getSwapchainFrameBuffers() const
	{
		ZoneScoped;

		if (m_CreateSwapChain)
		{
			return m_VulkanSwapChain.getSwapchainFrameBuffers();
		}

		return 0U;
	}

	void VulkanCore::setupGraphicCommandsAndSynchronization()
	{
		ZoneScoped;

		// create draw commands
		m_CommandBuffers = m_VulkanDevice.m_LogDevice.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo(
				m_VulkanDevice.getQueue(vk::QueueFlagBits::eGraphics).cmdPools,
				vk::CommandBufferLevel::ePrimary,
				SWAPCHAIN_IMAGES_COUNT
			)
		);
	}

	void VulkanCore::setupComputeCommandsAndSynchronization()
	{
		ZoneScoped;

		// create command buffer for compute operation
		m_ComputeCommandBuffers = m_VulkanDevice.m_LogDevice.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo(
				m_VulkanDevice.getQueue(vk::QueueFlagBits::eCompute).cmdPools,
				vk::CommandBufferLevel::ePrimary,
				1
			)
		);
		// sync objects

		// Semaphore used to ensures that image presentation is complete before starting to submit again
		m_ComputeCompleteSemaphores.resize(1);
		m_ComputeCompleteSemaphores[0] = m_VulkanDevice.m_LogDevice.createSemaphore(vk::SemaphoreCreateInfo());
		// Fence for command buffer completion
		m_ComputeWaitFences.resize(1);
		m_ComputeWaitFences[0] = m_VulkanDevice.m_LogDevice.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
	}

	void VulkanCore::setupDescriptorPool()
	{
		ZoneScoped;

		//Descriptor Pool
		std::vector<vk::DescriptorPoolSize> descriptorPoolSizes =
		{
			vk::DescriptorPoolSize(vk::DescriptorType::eSampler, 1000),
			vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1000),
			//vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, 1000),
			vk::DescriptorPoolSize(vk::DescriptorType::eStorageImage, 1000),
			//vk::DescriptorPoolSize(vk::DescriptorType::eUniformTexelBuffer, 1000),
			//vk::DescriptorPoolSize(vk::DescriptorType::eStorageTexelBuffer, 1000),
			vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1000),
			vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 1000),
			//vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1000),
			//vk::DescriptorPoolSize(vk::DescriptorType::eStorageBufferDynamic, 1000),
			//vk::DescriptorPoolSize(vk::DescriptorType::eInputAttachment, 1000)
		};

		m_DescriptorPool = m_VulkanDevice.m_LogDevice.createDescriptorPool(
			vk::DescriptorPoolCreateInfo(
				vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				/*vk::DescriptorPoolCreateFlags(),*/
				static_cast<uint32_t>(1000 * descriptorPoolSizes.size()),
				static_cast<uint32_t>(descriptorPoolSizes.size()),
				descriptorPoolSizes.data()
			)
		);
	}

	void VulkanCore::destroyGraphicCommandsAndSynchronization()
	{
		ZoneScoped;

		m_VulkanDevice.m_LogDevice.freeCommandBuffers(m_VulkanDevice.getQueue(vk::QueueFlagBits::eGraphics).cmdPools, m_CommandBuffers);

		for (auto& queue : m_VulkanDevice.m_Queues)
		{
			m_VulkanDevice.m_LogDevice.resetCommandPool(queue.second.cmdPools, vk::CommandPoolResetFlagBits::eReleaseResources);
		}
	}

	void VulkanCore::destroyComputeCommandsAndSynchronization()
	{
		ZoneScoped;

		auto queue = m_VulkanDevice.getQueue(vk::QueueFlagBits::eCompute);
		m_VulkanDevice.m_LogDevice.freeCommandBuffers(queue.cmdPools, m_ComputeCommandBuffers);
		m_VulkanDevice.m_LogDevice.resetCommandPool(queue.cmdPools, vk::CommandPoolResetFlagBits::eReleaseResources);

		m_VulkanDevice.m_LogDevice.destroySemaphore(m_ComputeCompleteSemaphores[0]);
		m_ComputeCompleteSemaphores.clear();
		m_VulkanDevice.m_LogDevice.destroyFence(m_ComputeWaitFences[0]);
		m_ComputeWaitFences.clear();
	}

	void VulkanCore::setupMemoryAllocator()
	{
		ZoneScoped;

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = (VkPhysicalDevice)m_VulkanDevice.m_PhysDevice;
		allocatorInfo.device = (VkDevice)m_VulkanDevice.m_LogDevice;
		allocatorInfo.instance = (VkInstance)m_VulkanDevice.m_Instance;

#if defined(_DEBUG) && defined(RECORD_VM_ALLOCATION)
		//VmaRecordSettings vma_record_settings;
		//vma_record_settings.pFilePath = "vma_replay.log";
		//vma_record_settings.flags = VMA_RECORD_FLUSH_AFTER_CALL_BIT;
		//allocatorInfo.pRecordSettings = &vma_record_settings;
#endif
		vmaCreateAllocator(&allocatorInfo, &VulkanCore::sAllocator);
	}

	ct::frect* VulkanCore::getDisplayRect()
	{
		ZoneScoped;

		if (m_CreateSwapChain)
		{
			return &m_VulkanSwapChain.m_DisplayRect;
		}

		return nullptr;
	}
}