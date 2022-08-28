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
#include "vk_mem_alloc.h"
#include <vkFramework/vkFramework.h>

#include <ctools/cTools.h>

#define USE_VSYNC 1
#define SWAPCHAIN_IMAGES_COUNT 3 // tripple buffering is interesting when vsync is enabled

// the deph here is not compatible wiht imgui viewport who is not suning depth is the wiewport swapchain
// this will create many validation layering issues
//#define SWAPCHAIN_USE_DEPTH

#include <string>
#include <functional>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <array>
namespace vkApi
{
	class VulkanWindow;
	class VulkanCore;
	class VulkanSwapChain
	{
	public:
		enum FrameType
		{
			COLOR = 0,
#ifdef SWAPCHAIN_USE_DEPTH
			DEPTH,
#endif
			FrameTypeSize
		};
		struct SwapChainFrameBuffer
		{
			std::array<vk::ImageView, FrameTypeSize> views;
			vk::Framebuffer frameBuffer;
		};
		struct DepthImageMem
		{
			vk::Image image;
			VmaAllocation meta;
			vk::ImageView view;
		};

	public:
		static VulkanSwapChainPtr Create(VulkanWindowPtr vVulkanWindow, VulkanCorePtr vVulkanCorePtr, std::function<void()> vResizeFunc);

	private:
		std::function<void()> m_ResizeFunction = nullptr;

	public:
		// swapchain
		vk::SurfaceKHR m_Surface;
		vk::SwapchainKHR m_Swapchain;
		vk::Rect2D m_RenderArea;
		vk::Viewport m_Viewport;
		vk::Extent2D m_OutputSize;
		vk::Format m_SurfaceColorFormat = vk::Format::eB8G8R8A8Unorm;
		vk::ColorSpaceKHR m_SurfaceColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
#ifdef SWAPCHAIN_USE_DEPTH
		vk::Format m_SurfaceDepthFormat = vk::Format::eD32SfloatS8Uint;
		DepthImageMem m_Depth;
#endif
		vk::SampleCountFlagBits m_SampleCount = vk::SampleCountFlagBits::e1;
		ct::frect m_DisplayRect;
		std::array<SwapChainFrameBuffer, SWAPCHAIN_IMAGES_COUNT> m_SwapchainFrameBuffers;
		uint32_t m_FrameIndex = 0;

		// sync objects : graphic
		std::array<vk::Semaphore, SWAPCHAIN_IMAGES_COUNT> m_PresentCompleteSemaphores;
		std::array<vk::Semaphore, SWAPCHAIN_IMAGES_COUNT> m_RenderCompleteSemaphores;
		std::array<vk::Fence, SWAPCHAIN_IMAGES_COUNT> m_WaitFences;

		// render pass
		vk::RenderPass m_RenderPass;
#ifdef SWAPCHAIN_USE_DEPTH
		std::array<vk::ClearValue, 2> m_ClearValues;
#else
		std::array<vk::ClearValue, 1> m_ClearValues;
#endif

	private:
		VulkanWindowPtr m_VulkanWindowPtr = nullptr;
		VulkanCorePtr m_VulkanCorePtr = nullptr;

	public:
		VulkanSwapChain() = default;
		~VulkanSwapChain() = default;

	public:
		bool Init(VulkanWindowPtr vVulkanWindow, VulkanCorePtr vVulkanCorePtr, std::function<void()> vResizeFunc);
		bool Load();
		bool Reload();
		void Unit();

	public: // get
		vk::SurfaceKHR			getSurface() const;
		vk::SwapchainKHR		getSwapchain() const;
		vk::Viewport			getViewport() const;
		vk::Rect2D				getRenderArea() const;
		uint32_t				getSwapchainFrameBuffers() const;
		vk::SampleCountFlagBits getSwapchainFrameBufferSampleCount() const;

	public: // Presentation
		bool AcquireNextImage();
		void Present();

	private:
		void CheckSurfaceFormat();
		void Resize();

		bool CreateSurface();
		bool CreateSyncObjects();
		bool CreateFrameBuffers();
		bool CreateRenderPass();

		void DestroySurface();
		void DestroySyncObjects();
		void DestroyFrameBuffers();
		void DestroyRenderPass();
	};
}
