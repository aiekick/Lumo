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
#include "Texture2D.h"

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
#include <vkprofiler/Profiler.h>

static bool sGainFocus = false;

namespace vkApi
{
	VmaAllocator VulkanCore::sAllocator = nullptr;
	std::shared_ptr<VulkanShader> VulkanCore::sVulkanShader = nullptr;

	VulkanCorePtr VulkanCore::Create(
		VulkanWindowPtr vVulkanWindow,
		const std::string& vAppName,
		const int& vAppVersion,
		const std::string& vEngineName,
		const int& vEngineVersion,
		const bool& vCreateSwapChain,
		const bool& vUseRTX)
	{
		auto res = std::make_shared<VulkanCore>();
		res->m_This = res;
		if (!res->Init(vVulkanWindow, vAppName, vAppVersion, vEngineName, vEngineVersion, vCreateSwapChain, vUseRTX))
		{
			res.reset();
		}
		return res;
	}

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
	bool VulkanCore::Init(
		VulkanWindowPtr vVulkanWindow,
		const std::string& vAppName,
		const int& vAppVersion,
		const std::string& vEngineName,
		const int& vEngineVersion,
		const bool& vCreateSwapChain,
		const bool& vUseRTX)
	{
		ZoneScoped;

		m_CreateSwapChain = vCreateSwapChain;

		glfwSetWindowFocusCallback(vVulkanWindow->getWindowPtr(), window_focus_callback);

		m_VulkanDevicePtr = VulkanDevice::Create(vVulkanWindow, vAppName, vAppVersion, vEngineName, vEngineVersion, vUseRTX);
		if (m_VulkanDevicePtr)
		{
			setupMemoryAllocator();
			if (m_CreateSwapChain)
			{
				m_VulkanSwapChainPtr = VulkanSwapChain::Create(vVulkanWindow, m_This.getValidShared(), std::bind(&VulkanCore::resize, this));
			}
			setupGraphicCommandsAndSynchronization();
			setupComputeCommandsAndSynchronization();
			setupDescriptorPool();

#ifdef TRACY_ENABLE
#ifdef ENABLE_CALIBRATED_CONTEXT
			m_TracyContext = TracyVkContextCalibrated(
				m_VulkanCorePtr->getPhysicalDevice(),
				m_VulkanCorePtr->getDevice(),
				m_VulkanCorePtr->getQueue(vk::QueueFlagBits::eGraphics).vkQueue,
				m_VulkanCorePtr->getGraphicCommandBuffer(),
				vkGetPhysicalDeviceCalibrateableTimeDomainsEXT,
				vkGetCalibratedTimestampsEXT);
#else
			m_TracyContext = TracyVkContext(
				m_VulkanCorePtr->getPhysicalDevice(),
				m_VulkanCorePtr->getDevice(),
				m_VulkanCorePtr->getQueue(vk::QueueFlagBits::eGraphics).vkQueue,
				m_VulkanCorePtr->getGraphicCommandBuffer());
#endif

			tracy::SetThreadName("Main");
#endif
			m_EmptyTexturePtr = Texture2D::CreateEmptyTexture(m_This.getValidShared(), ct::uvec2(1, 1), vk::Format::eR8G8B8A8Unorm);

			return true;
		}

		return false;
	}

	void VulkanCore::Unit()
	{
		ZoneScoped;

		m_VulkanDevicePtr->WaitIdle();

		m_EmptyTexturePtr.reset();

		TracyVkDestroy(m_TracyContext);

		m_VulkanDevicePtr->m_LogDevice.destroyDescriptorPool(m_DescriptorPool);

		destroyComputeCommandsAndSynchronization();
		destroyGraphicCommandsAndSynchronization();

		if (m_CreateSwapChain)
		{
			m_VulkanSwapChainPtr->Unit();
		}

		vmaDestroyAllocator(VulkanCore::sAllocator);

		m_VulkanDevicePtr->Unit();
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//// PUBLIC // QUICK GET /////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	vk::RenderPass VulkanCore::getMainRenderPass() const { return m_VulkanSwapChainPtr->m_RenderPass; }
	vk::CommandBuffer VulkanCore::getGraphicCommandBuffer() const { return m_CommandBuffers[m_VulkanSwapChainPtr->m_FrameIndex]; }
	vk::SurfaceKHR VulkanCore::getSurface() const { return m_VulkanSwapChainPtr->getSurface(); }
	VulkanSwapChainWeak VulkanCore::getSwapchain() const { return m_VulkanSwapChainPtr; }
	vk::Viewport VulkanCore::getViewport() const { return m_VulkanSwapChainPtr->getViewport(); }
	vk::Rect2D VulkanCore::getRenderArea() const { return m_VulkanSwapChainPtr->getRenderArea(); }
	std::vector<vk::Semaphore>VulkanCore::getPresentSemaphores() { return m_VulkanSwapChainPtr->m_PresentCompleteSemaphores; }
	std::vector<vk::Semaphore> VulkanCore::getRenderSemaphores() { return m_VulkanSwapChainPtr->m_PresentCompleteSemaphores; }
	vk::SampleCountFlagBits VulkanCore::getSwapchainFrameBufferSampleCount() const { return m_VulkanSwapChainPtr->getSwapchainFrameBufferSampleCount(); }

	Texture2DWeak VulkanCore::getEmptyTexture() const { return m_EmptyTexturePtr; }
	vk::DescriptorImageInfo VulkanCore::getEmptyTextureDescriptorImageInfo() const { return m_EmptyTexturePtr->m_DescriptorImageInfo; }

	// when the NullDescriptor Feature is enabled
	vk::DescriptorBufferInfo* VulkanCore::getEmptyDescriptorBufferInfo() { return &m_EmptyDescriptorBufferInfo; }
	vk::BufferView* VulkanCore::getEmptyBufferView() { return &m_EmptyBufferView; }

	vk::Instance VulkanCore::getInstance() const { return m_VulkanDevicePtr->m_Instance; }
	vk::PhysicalDevice VulkanCore::getPhysicalDevice() const { return m_VulkanDevicePtr->m_PhysDevice; }
	vk::Device VulkanCore::getDevice() const { return m_VulkanDevicePtr->m_LogDevice; }
	VulkanDeviceWeak VulkanCore::getFrameworkDevice() { return m_VulkanDevicePtr; }
	vk::DescriptorPool VulkanCore::getDescriptorPool() const { return m_DescriptorPool; }
	vk::CommandBuffer VulkanCore::getComputeCommandBuffer() const { return m_ComputeCommandBuffers[0]; }
	VulkanQueue VulkanCore::getQueue(vk::QueueFlagBits vQueueType) { return m_VulkanDevicePtr->getQueue(vQueueType); }
	TracyVkCtx VulkanCore::getTracyContext() { return m_TracyContext; }

	void VulkanCore::SetVulkanImGuiRenderer(VulkanImGuiRendererWeak vVulkanImGuiRendererWeak) { m_VulkanImGuiRendererWeak = vVulkanImGuiRendererWeak; }
	VulkanImGuiRendererWeak VulkanCore::GetVulkanImGuiRenderer() { return m_VulkanImGuiRendererWeak; }

	float VulkanCore::GetDeltaTime(const uint32_t& vCurrentFrame)
	{
		static uint32_t current_frame = 0U;
		static float delta_time = 0.0f;
		if (vCurrentFrame != current_frame)
		{
			delta_time = ct::GetTimeInterval();
		}
		return delta_time;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//// PUBLIC // GET INFOS /////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	vk::SampleCountFlagBits VulkanCore::GetMaxUsableSampleCount()
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties((VkPhysicalDevice)m_VulkanDevicePtr->m_PhysDevice, &physicalDeviceProperties);

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

		m_VulkanDevicePtr->WaitIdle();

		destroyGraphicCommandsAndSynchronization();

		if (m_CreateSwapChain)
		{
			m_VulkanSwapChainPtr->Reload();
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
			if (m_VulkanDevicePtr->m_LogDevice.waitForFences(1, 
				&m_VulkanSwapChainPtr->m_WaitFences[m_VulkanSwapChainPtr->m_FrameIndex], VK_TRUE, UINT64_MAX) == vk::Result::eSuccess)
			{
				if (m_VulkanDevicePtr->m_LogDevice.resetFences(1, &m_VulkanSwapChainPtr->m_WaitFences[m_VulkanSwapChainPtr->m_FrameIndex]) == vk::Result::eSuccess)
				{
					//todo : reset pool instead ?
					m_CommandBuffers[m_VulkanSwapChainPtr->m_FrameIndex].reset(vk::CommandBufferResetFlagBits::eReleaseResources);

					m_CommandBuffers[m_VulkanSwapChainPtr->m_FrameIndex].begin(vk::CommandBufferBeginInfo());

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
			m_CommandBuffers[m_VulkanSwapChainPtr->m_FrameIndex].beginRenderPass(
				vk::RenderPassBeginInfo(
					m_VulkanSwapChainPtr->m_RenderPass,
					m_VulkanSwapChainPtr->m_SwapchainFrameBuffers[m_VulkanSwapChainPtr->m_FrameIndex].frameBuffer,
					m_VulkanSwapChainPtr->m_RenderArea,
					static_cast<uint32_t>(m_VulkanSwapChainPtr->m_ClearValues.size()),
					m_VulkanSwapChainPtr->m_ClearValues.data()),
				vk::SubpassContents::eInline);
		}
	}

	void VulkanCore::endMainRenderPass()
	{
		ZoneScoped;

		if (m_CreateSwapChain)
		{
			m_CommandBuffers[m_VulkanSwapChainPtr->m_FrameIndex].endRenderPass();
		}
	}

	void VulkanCore::frameEnd()
	{
		ZoneScoped;

		if (m_CreateSwapChain)
		{
			m_CommandBuffers[m_VulkanSwapChainPtr->m_FrameIndex].end();

			vk::SubmitInfo submitInfo;
			vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			submitInfo
				.setWaitSemaphoreCount(1)
				.setPWaitSemaphores(&m_VulkanSwapChainPtr->m_PresentCompleteSemaphores[m_VulkanSwapChainPtr->m_FrameIndex])
				.setPWaitDstStageMask(&waitDstStageMask)
				.setCommandBufferCount(1)
				.setPCommandBuffers(&m_CommandBuffers[m_VulkanSwapChainPtr->m_FrameIndex])
				.setSignalSemaphoreCount(1)
				.setPSignalSemaphores(&m_VulkanSwapChainPtr->m_RenderCompleteSemaphores[m_VulkanSwapChainPtr->m_FrameIndex]);

			VulkanSubmitter::Submit(m_This.getValidShared(), vk::QueueFlagBits::eGraphics,
				submitInfo, m_VulkanSwapChainPtr->m_WaitFences[m_VulkanSwapChainPtr->m_FrameIndex]);

			/*
			std::unique_lock<std::mutex> lck(VulkanSubmitter::criticalSectionMutex, std::defer_lock);
			lck.lock();
			auto result = getQueue(vk::QueueFlagBits::eGraphics).vkQueue.submit(1, &submitInfo, 
				m_VulkanSwapChainPtr->m_WaitFences[m_VulkanSwapChainPtr->m_FrameIndex]);
			if (result == vk::Result::eErrorDeviceLost)
			{
				// driver lost, we'll crash in this case:
				LogVarDebug("Debug : Driver Lost after submit");
			}
			lck.unlock();
			*/
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//// COMPUTE //////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool VulkanCore::resetComputeFence()
	{
		ZoneScoped;

		return (m_VulkanDevicePtr->m_LogDevice.resetFences(1, 
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

			if (VulkanSubmitter::Submit(m_This.getValidShared(), vk::QueueFlagBits::eCompute,
				submitInfo, m_ComputeWaitFences[0]))
			{
				return (m_VulkanDevicePtr->m_LogDevice.waitForFences(1, 
					&m_ComputeWaitFences[0], VK_TRUE, UINT64_MAX) == vk::Result::eSuccess);
			}
		}

		return false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool VulkanCore::AcquireNextImage(VulkanWindowPtr vVulkanWindow)
	{
		ZoneScoped;

		assert(vVulkanWindow);

		if (!vVulkanWindow->IsMinimized())
		{
			if (m_CreateSwapChain)
			{
				return m_VulkanSwapChainPtr->AcquireNextImage();
			}
		}

		return false;
	}

	void VulkanCore::Present()
	{
		ZoneScoped;

		if (m_CreateSwapChain)
		{
			if (m_VulkanDevicePtr->m_LogDevice.waitForFences(1, 
				&m_VulkanSwapChainPtr->m_WaitFences[m_VulkanSwapChainPtr->m_FrameIndex], 
				VK_TRUE, UINT64_MAX) == vk::Result::eSuccess)
			{
				m_VulkanSwapChainPtr->Present();
			}
		}
	}

	uint32_t VulkanCore::getSwapchainFrameBuffers() const
	{
		ZoneScoped;

		if (m_CreateSwapChain)
		{
			return m_VulkanSwapChainPtr->getSwapchainFrameBuffers();
		}

		return 0U;
	}

	void VulkanCore::setupGraphicCommandsAndSynchronization()
	{
		ZoneScoped;

		// create draw commands
		m_CommandBuffers = m_VulkanDevicePtr->m_LogDevice.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo(
				m_VulkanDevicePtr->getQueue(vk::QueueFlagBits::eGraphics).cmdPools,
				vk::CommandBufferLevel::ePrimary,
				SWAPCHAIN_IMAGES_COUNT
			)
		);
	}

	void VulkanCore::setupComputeCommandsAndSynchronization()
	{
		ZoneScoped;

		// create command buffer for compute operation
		m_ComputeCommandBuffers = m_VulkanDevicePtr->m_LogDevice.allocateCommandBuffers(
			vk::CommandBufferAllocateInfo(
				m_VulkanDevicePtr->getQueue(vk::QueueFlagBits::eCompute).cmdPools,
				vk::CommandBufferLevel::ePrimary,
				1
			)
		);
		// sync objects

		// Semaphore used to ensures that image presentation is complete before starting to submit again
		m_ComputeCompleteSemaphores.resize(1);
		m_ComputeCompleteSemaphores[0] = m_VulkanDevicePtr->m_LogDevice.createSemaphore(vk::SemaphoreCreateInfo());
		// Fence for command buffer completion
		m_ComputeWaitFences.resize(1);
		m_ComputeWaitFences[0] = m_VulkanDevicePtr->m_LogDevice.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
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
			vk::DescriptorPoolSize(vk::DescriptorType::eUniformTexelBuffer, 1000),
			vk::DescriptorPoolSize(vk::DescriptorType::eStorageTexelBuffer, 1000),
			vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1000),
			vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 1000),
			//vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1000),
			//vk::DescriptorPoolSize(vk::DescriptorType::eStorageBufferDynamic, 1000),
			//vk::DescriptorPoolSize(vk::DescriptorType::eInputAttachment, 1000)
		};

		if (m_VulkanDevicePtr->m_Use_RTX)
		{
			descriptorPoolSizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eAccelerationStructureKHR, 1000));
		}

		m_DescriptorPool = m_VulkanDevicePtr->m_LogDevice.createDescriptorPool(
			vk::DescriptorPoolCreateInfo(
				vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				/*vk::DescriptorPoolCreateFlags(),*/
				static_cast<uint32_t>(1000 * descriptorPoolSizes.size()),
				static_cast<uint32_t>(descriptorPoolSizes.size()),
				descriptorPoolSizes.data()
			)
		);
	}

	void VulkanCore::ResetCommandPools()
	{
		for (auto& queue : m_VulkanDevicePtr->m_Queues)
		{
			m_VulkanDevicePtr->m_LogDevice.resetCommandPool(queue.second.cmdPools, vk::CommandPoolResetFlagBits::eReleaseResources);
		}
	}

	void VulkanCore::destroyGraphicCommandsAndSynchronization()
	{
		ZoneScoped;

		m_VulkanDevicePtr->m_LogDevice.freeCommandBuffers(m_VulkanDevicePtr->getQueue(vk::QueueFlagBits::eGraphics).cmdPools, m_CommandBuffers);

		ResetCommandPools();
	}

	void VulkanCore::destroyComputeCommandsAndSynchronization()
	{
		ZoneScoped;

		auto queue = m_VulkanDevicePtr->getQueue(vk::QueueFlagBits::eCompute);
		m_VulkanDevicePtr->m_LogDevice.freeCommandBuffers(queue.cmdPools, m_ComputeCommandBuffers);
		m_VulkanDevicePtr->m_LogDevice.resetCommandPool(queue.cmdPools, vk::CommandPoolResetFlagBits::eReleaseResources);

		m_VulkanDevicePtr->m_LogDevice.destroySemaphore(m_ComputeCompleteSemaphores[0]);
		m_ComputeCompleteSemaphores.clear();
		m_VulkanDevicePtr->m_LogDevice.destroyFence(m_ComputeWaitFences[0]);
		m_ComputeWaitFences.clear();
	}

	void VulkanCore::setupMemoryAllocator()
	{
		ZoneScoped;

		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

		if (m_VulkanDevicePtr->m_Use_RTX)
		{
			allocatorInfo.flags |= 
				VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;  // VK_API_VERSION_1_2
		}

		allocatorInfo.physicalDevice = (VkPhysicalDevice)m_VulkanDevicePtr->m_PhysDevice;
		allocatorInfo.device = (VkDevice)m_VulkanDevicePtr->m_LogDevice;
		allocatorInfo.instance = (VkInstance)m_VulkanDevicePtr->m_Instance;

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
			return &m_VulkanSwapChainPtr->m_DisplayRect;
		}

		return nullptr;
	}
}