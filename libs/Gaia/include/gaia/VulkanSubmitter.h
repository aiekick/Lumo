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

// unique_lock::lock/unlock
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <mutex>          // std::mutex, std::unique_lock, std::defer_lock
#include <gaia/gaia.h>

namespace vkApi
{
	class VulkanCore;
	class GAIA_API VulkanSubmitter
	{
	public:
		static std::mutex criticalSectionMutex;

	public:
		VulkanSubmitter() = default;
		~VulkanSubmitter() = default;

	public:
		static bool Submit(vkApi::VulkanCorePtr vVulkanCorePtr, vk::QueueFlagBits vQueueType, vk::SubmitInfo vSubmitInfo, vk::Fence vWaitFence);
	};
}
