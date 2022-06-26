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