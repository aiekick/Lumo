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

class COMMON_API TextureCubeInputFunctions
{
protected:
	void UpdateTextureCubeInputDescriptorImageInfos(const std::map<uint32_t, NodeSlotInputPtr>& vInputs);

public:
	virtual void SetTextureCube(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureCubeSize) = 0;
};

template<size_t size_of_array>
class COMMON_API TextureCubeInputInterface : public TextureCubeInputFunctions
{
protected:
	std::array<vk::DescriptorImageInfo, size_of_array> m_ImageCubeInfos;
	std::array<ct::fvec2, size_of_array> m_ImageCubeInfosSize;
	std::array<std::array<ImGuiTexture, 6U>, size_of_array> m_ImGuiTextureCubes;

protected: // internal use
	void DrawInputTextureCube(vkApi::VulkanCorePtr vVKCore, const char* vLabel, const uint32_t& vIdx, const float& vRatio);
};

template<size_t size_of_array>
void COMMON_API TextureCubeInputInterface<size_of_array>::DrawInputTextureCube(vkApi::VulkanCorePtr vVKCore, const char* vLabel, const uint32_t& vIdx, const float& vRatio)
{
	if (vVKCore && vLabel && vIdx <= (uint32_t)size_of_array) {
		auto imguiRendererPtr = vVKCore->GetVulkanImGuiRenderer().getValidShared();
		if (imguiRendererPtr) {
			if (ImGui::CollapsingHeader(vLabel)) {
				m_ImGuiTextureCubes[(size_t)vIdx].SetDescriptor(imguiRendererPtr,
					&m_ImageCubeInfos[(size_t)vIdx], vRatio);
				if (m_ImGuiTextureCubes[(size_t)vIdx].canDisplayPreview) {
					int w = (int)ImGui::GetContentRegionAvail().x;
					auto rect = ct::GetScreenRectWithRatio<int32_t>(m_ImGuiTextureCubes[(size_t)vIdx].ratio, ct::ivec2(w, w), false);
					ImGui::ImageRect((ImTextureCubeID)&m_ImGuiTextureCubes[(size_t)vIdx].descriptor,
						ImVec2((float)rect.x, (float)rect.y),
						ImVec2((float)rect.w, (float)rect.h));
				}
			}
		}
	}
}