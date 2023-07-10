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

#include <map>
#include <memory>
#include <Graph/Graph.h>
#include <ctools/cTools.h>
#include <vulkan/vulkan.hpp>
#include <ImWidgets/ImWidgets.h>
#include <gaia/VulkanCore.h>
#include <gaia/ImGuiTexture.h>
#include <gaia/gaia.h>
#include <Common/Globals.h>

class COMMON_API TextureGroupInputFunctions
{
protected:
	void UpdateTextureGroupInputDescriptorImageInfos(const std::map<uint32_t, NodeSlotInputPtr>& vInputs);

public:
	virtual void SetTextures(const uint32_t& vBindingPoint, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes) = 0;
};

template<size_t size_of_array>
class COMMON_API TextureGroupInputInterface : public TextureGroupInputFunctions
{
protected:
	std::array<DescriptorImageInfoVector, size_of_array> m_ImageGroups;
	std::array<fvec2Vector, size_of_array> m_ImageGroupSizes;

	std::array<vk::DescriptorImageInfo, size_of_array> m_ImageGroupInfos;
	std::array<ImGuiTexture, size_of_array> m_ImGuiGroupTextures;
};