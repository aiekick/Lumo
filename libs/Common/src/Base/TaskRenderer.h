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

#include <ctools/cTools.h>
#include <vulkan/vulkan.hpp>
#include <Base/BaseRenderer.h>
#include <vkFramework/VulkanCore.h>
#include <Interfaces/TaskInterface.h>

class TaskRenderer :
	public BaseRenderer,
	public TaskInterface
{
public:
	TaskRenderer(vkApi::VulkanCorePtr vVulkanCorePtr);
	TaskRenderer(vkApi::VulkanCorePtr vVulkanCorePtr, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool);
	virtual ~TaskRenderer() = default;

	bool IsTheGoodFrame(const uint32_t& vFrame);
};