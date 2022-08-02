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

#include <ctools/cTools.h>

#include <vkFramework/VulkanFrameBuffer.h>

#include <string>
#include <memory>

class ShaderInterface
{
public:
	ct::uvec3 m_OutputSize;			// size of the output
	uint32_t m_CurrentFrame = 0;	// current frame of double buffering
	ct::uvec2 m_ScreenSize;			// utile pour le widget mouse

public: // virtual pure
	virtual vk::DescriptorImageInfo GetBackImageInfo(uint32_t vIndex) = 0;
	virtual vk::DescriptorImageInfo GetFrontImageInfo(uint32_t vIndex) = 0;
	virtual vkApi::VulkanFrameBuffer* GetBackFbo() = 0;
	virtual vkApi::VulkanFrameBuffer* GetFrontFbo() = 0;
};