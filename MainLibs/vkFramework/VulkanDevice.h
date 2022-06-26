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

#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include <ctools/cTools.h>

#define VULKAN_DEBUG 1
#define VULKAN_DEBUG_FEATURES 0
#define VULKAN_GPU_ID 0

namespace vkApi
{
	struct VulkanQueue
	{
		uint32_t familyQueueIndex = 0U;
		vk::Queue vkQueue;
		vk::CommandPool cmdPools;
	};

	class VulkanWindow;
	class VulkanDevice
	{
	public:
		std::unordered_map<vk::QueueFlagBits, VulkanQueue> m_Queues;

	public:
		vk::Instance m_Instance;
		vk::DispatchLoaderDynamic m_Dldy;
		vk::DebugReportCallbackEXT m_DebugReport;
		vk::PhysicalDeviceFeatures m_PhysDeviceFeatures;
		vk::PhysicalDevice m_PhysDevice;
		vk::Device m_LogDevice;

	private: // debug extention must use dynamic loader m_Dldy ( not part of vulkan core), so we let it here
		vk::DebugUtilsLabelEXT markerInfo;		// marker info for vkCmdBeginDebugUtilsLabelEXT
		bool m_Debug_Utils_Supported = false;

	public:
		static void findBestExtensions(const std::vector<vk::ExtensionProperties>& installed, const std::vector<const char*>& wanted, std::vector<const char*>& out);
		static void findBestLayers(const std::vector<vk::LayerProperties>& installed, const std::vector<const char*>& wanted, std::vector<const char*>& out);
		static uint32_t getQueueIndex(vk::PhysicalDevice& physicalDevice, vk::QueueFlags flags, bool standalone);

	public:
		VulkanDevice();
		~VulkanDevice();

		bool Init(VulkanWindow* vVulkanWindow, const char* vAppName, const int& vAppVersion, const char* vEngineName, const int& vEngineVersion);
		void Unit();

		void WaitIdle();

		VulkanQueue getQueue(vk::QueueFlagBits vQueueType);

		// debug extention must use dynamic loader m_Dldy ( not part of vulkan core), so we let it here
		void BeginDebugLabel(vk::CommandBuffer *vCmd, const char* vLabel, ct::fvec4 vColor = 0.0f);
		void EndDebugLabel(vk::CommandBuffer *vCmd);

	private:
		void CreateVulkanInstance(VulkanWindow* vVulkanWindow, const char* vAppName, const int& vAppVersion, const char* vEngineName, const int& vEngineVersion);
		void DestroyVulkanInstance();

		void CreatePhysicalDevice();
		void DestroyPhysicalDevice();

		void CreateLogicalDevice();
		void DestroyLogicalDevice();
	};
}
