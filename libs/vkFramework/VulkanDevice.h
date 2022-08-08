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

#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include <ctools/cTools.h>
#include <vkFramework/vkFramework.h>

#define VULKAN_DEBUG 1
#define VULKAN_GPU_ID 0
#define VULKAN_DEBUG_FEATURES 1
#define VULKAN_DEBUG_SYNCHRONIZATION_FEATURES 1

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
		static VulkanDevicePtr Create(
			VulkanWindowWeak vVulkanWindow,
			const std::string& vAppName,
			const int& vAppVersion,
			const std::string& vEngineName,
			const int& vEngineVersion,
			const bool& vUseRTX);
	public:
		std::unordered_map<vk::QueueFlagBits, VulkanQueue> m_Queues;

	public:
		vk::Instance m_Instance;

		vk::DebugReportCallbackEXT m_DebugReport;
		vk::PhysicalDeviceRobustness2FeaturesEXT m_Robustness2Feature;
		vk::PhysicalDeviceAccelerationStructureFeaturesKHR m_AccelerationStructureFeature;
		vk::PhysicalDeviceRayTracingPipelineFeaturesKHR m_RayTracingPipelineFeature;
		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR m_RayTracingDeviceProperties;
		vk::PhysicalDeviceBufferDeviceAddressFeatures m_BufferDeviceAddress;
		vk::PhysicalDeviceFeatures m_PhysDeviceFeatures;
		vk::PhysicalDeviceFeatures2 m_PhysDeviceFeatures2;
		vk::PhysicalDevice m_PhysDevice;
		vk::Device m_LogDevice;
		uint32_t m_ApiVersion = VK_API_VERSION_1_0;

	public:
		bool m_Use_RTX = false;

	private: // debug extention must use dynamic loader m_Dldy ( not part of vulkan core), so we let it here
		vk::DebugUtilsLabelEXT markerInfo;		// marker info for vkCmdBeginDebugUtilsLabelEXT
		bool m_Debug_Utils_Supported = false;

	public:
		static void findBestExtensions(const char* vLabel, const std::vector<vk::ExtensionProperties>& installed, const std::vector<const char*>& wanted, std::vector<const char*>& out);
		static void findBestLayers(const std::vector<vk::LayerProperties>& installed, const std::vector<const char*>& wanted, std::vector<const char*>& out);
		static uint32_t getQueueIndex(vk::PhysicalDevice& physicalDevice, vk::QueueFlags flags, bool standalone);

	public:
		VulkanDevice();
		~VulkanDevice();

		bool Init(
			VulkanWindowWeak vVulkanWindow, 
			const std::string& vAppName, 
			const int& vAppVersion, 
			const std::string& vEngineName,
			const int& vEngineVersion,
			const bool& vUseRTX);
		void Unit();

		void WaitIdle();

		VulkanQueue getQueue(vk::QueueFlagBits vQueueType);

		// debug extention must use dynamic loader m_Dldy ( not part of vulkan core), so we let it here
		void BeginDebugLabel(vk::CommandBuffer *vCmd, const char* vLabel, ct::fvec4 vColor = 0.0f);
		void EndDebugLabel(vk::CommandBuffer *vCmd);

		void SetUseRTX(const bool& vFlag) { m_Use_RTX = vFlag; }

	private:
		bool CreateVulkanInstance(
			VulkanWindowWeak vVulkanWindow,
			const std::string& vAppName,
			const int& vAppVersion,
			const std::string& vEngineName,
			const int& vEngineVersion);
		void DestroyVulkanInstance();

		bool CreatePhysicalDevice();
		void DestroyPhysicalDevice();

		bool CreateLogicalDevice();
		void DestroyLogicalDevice();
	};
}
