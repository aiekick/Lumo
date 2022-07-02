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

class TaskInterface
{
protected:
	// to compare to current frame
	// to know is the execution was already done
	uint32_t m_LastExecutedFrame = 0U;

public:
	virtual bool Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer *vCmd = nullptr) = 0;
};