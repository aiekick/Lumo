/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

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

#include <ctools/cTools.h>
#include <Gaia/gaia.h>
#include <LumoBackend/Base/BaseRenderer.h>
#include <Gaia/Core/VulkanCore.h>
#include <LumoBackend/Interfaces/TaskInterface.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

class LUMO_BACKEND_API TaskRenderer :
	public BaseRenderer,
	public TaskInterface
{
public:
	TaskRenderer(GaiApi::VulkanCoreWeak vVulkanCore);
	TaskRenderer(GaiApi::VulkanCoreWeak vVulkanCore, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool);
	virtual ~TaskRenderer() = default;

	bool IsTheGoodFrame(const uint32_t& vFrame);
};