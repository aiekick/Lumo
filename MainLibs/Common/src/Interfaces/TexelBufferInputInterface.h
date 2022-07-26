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

#include <map>
#include <memory>
#include <Graph/Graph.h>
#include <ctools/cTools.h>
#include <vulkan/vulkan.hpp>
#include <ImWidgets/ImWidgets.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/vkFramework.h>

template<size_t size_of_array>
class TexelBufferInputInterface
{
protected:
	std::array<vk::BufferView, size_of_array> m_TexelBufferInfos;
	std::array<ct::fvec2, size_of_array> m_TexelBufferInfosSize;

public:
	virtual void SetTexelBuffer(const uint32_t& vBinding, vk::BufferView* vTexelBufferInfo, ct::fvec2* vTextureSize) = 0;
};