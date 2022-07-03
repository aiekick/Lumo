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
#include <vkFramework/ImGuiTexture.h>
#include <vkFramework/vkFramework.h>

namespace vkApi { class VulkanCore; }

class TextureInputFunctions
{
protected:
	void UpdateInputDescriptorImageInfos(const std::map<uint32_t, NodeSlotPtr>& vInputs);

public:
	virtual void SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo) = 0;
};

template<size_t size_of_array>
class TextureInputInterface : public TextureInputFunctions
{
protected:
	bool m_NeedSamplerUpdate = false; 
	Texture2DPtr m_EmptyTexturePtr = nullptr;
	std::array<vk::DescriptorImageInfo, size_of_array> m_ImageInfos;
	std::array<ImGuiTexture, size_of_array> m_ImGuiTextures;

protected: // internal use
	void DrawInputTexture(vkApi::VulkanCorePtr vVKCore, const char* vLabel, const uint32_t& vIdx, const float& vRatio);
};

template<size_t size_of_array>
void TextureInputInterface<size_of_array>::DrawInputTexture(vkApi::VulkanCorePtr vVKCore, const char* vLabel, const uint32_t& vIdx, const float& vRatio)
{
	if (vVKCore && vLabel && vIdx <= size_of_array) {
		auto imguiRendererPtr = vVKCore->GetVulkanImGuiRenderer().getValidShared();
		if (imguiRendererPtr) {
			if (ImGui::CollapsingHeader(vLabel)) {
				m_ImGuiTextures[vIdx].SetDescriptor(imguiRendererPtr,
					&m_ImageInfos[vIdx], vRatio);
				if (m_ImGuiTextures[vIdx].canDisplayPreview) {
					int w = (int)ImGui::GetContentRegionAvail().x;
					auto rect = ct::GetScreenRectWithRatio<int32_t>(m_ImGuiTextures[vIdx].ratio, ct::ivec2(w, w), false);
					ImGui::ImageRect((ImTextureID)&m_ImGuiTextures[vIdx].descriptor,
						ImVec2((float)rect.x, (float)rect.y),
						ImVec2((float)rect.w, (float)rect.h));
				}
			}
		}
	}
}