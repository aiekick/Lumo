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

#include <map>
#include <memory>
#include <LumoBackend/Graph/Graph.h>
#include <ctools/cTools.h>
#include <Gaia/gaia.h>
#include <ImWidgets.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/gaia.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

template<size_t size_of_array>
class LUMO_BACKEND_API TexelBufferInputInterface
{
protected:
	std::array<vk::Buffer, size_of_array> m_TexelBuffers;
	std::array<vk::BufferView, size_of_array> m_TexelBufferViews;
	std::array<ct::uvec2, size_of_array> m_TexelBufferViewsSize;

public:
	virtual void SetTexelBuffer(const uint32_t& vBindingPoint, vk::Buffer* vTexelBuffer, ct::uvec2* vTexelBufferSize) = 0;
	virtual void SetTexelBufferView(const uint32_t& vBindingPoint, vk::BufferView* vTexelBufferView, ct::uvec2* vTexelBufferSize) = 0;
};