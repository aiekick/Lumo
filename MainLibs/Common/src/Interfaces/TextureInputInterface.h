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
	std::array<vk::DescriptorImageInfo, size_of_array>  m_SamplerImageInfos;
	std::array<ImGuiTexture, size_of_array> m_ImGuiTextures;

protected: // internal use
	void DrawInputTexture(vkApi::VulkanCore* vVKCore, const char* vLabel, const uint32_t& vIdx, const float& vRatio);
};

template<size_t size_of_array>
void TextureInputInterface<size_of_array>::DrawInputTexture(vkApi::VulkanCore* vVKCore, const char* vLabel, const uint32_t& vIdx, const float& vRatio)
{
	if (vVKCore && vLabel && vIdx <= size_of_array) {
		auto imguiRendererPtr = vVKCore->GetVulkanImGuiRenderer().getValidShared();
		if (imguiRendererPtr) {
			if (ImGui::CollapsingHeader(vLabel)) {
				m_ImGuiTextures[vIdx].SetDescriptor(imguiRendererPtr.get(),
					&m_SamplerImageInfos[vIdx], vRatio);
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