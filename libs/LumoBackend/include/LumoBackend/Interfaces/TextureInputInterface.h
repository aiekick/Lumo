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
#include <LumoBackend/Graph/Graph.h>
#include <ctools/cTools.h>
#include <Gaia/gaia.h>
#include <ImWidgets.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/gaia.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

class LUMO_BACKEND_API TextureInputFunctions
{
protected:
	void UpdateTextureInputDescriptorImageInfos(const std::map<uint32_t, NodeSlotInputPtr>& vInputs);

public:
	virtual void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) = 0;
};

template<size_t size_of_array>
class TextureInputInterface : public TextureInputFunctions
{
protected:
	std::array<vk::DescriptorImageInfo, size_of_array> m_ImageInfos;
	std::array<ct::fvec2, size_of_array> m_ImageInfosSize;
	std::array<ImGuiTexture, size_of_array> m_ImGuiTextures;

protected: // internal use
	void DrawInputTexture(GaiApi::VulkanCorePtr vVKCore, const char* vLabel, const uint32_t& vIdx, const float& vRatio);
};

template<size_t size_of_array>
void TextureInputInterface<size_of_array>::DrawInputTexture(GaiApi::VulkanCorePtr vVKCore, const char* vLabel, const uint32_t& vIdx, const float& vRatio)
{
	if (vVKCore && vLabel && vIdx <= (uint32_t)size_of_array) {
		auto imguiRendererPtr = vVKCore->GetVulkanImGuiRenderer().lock();
		if (imguiRendererPtr) {
			if (ImGui::CollapsingHeader(vLabel)) {
				m_ImGuiTextures[(size_t)vIdx].SetDescriptor(imguiRendererPtr,
					&m_ImageInfos[(size_t)vIdx], vRatio);
				if (m_ImGuiTextures[(size_t)vIdx].canDisplayPreview) {
					int w = (int)ImGui::GetContentRegionAvail().x;
					auto rect = ct::GetScreenRectWithRatio<int32_t>(m_ImGuiTextures[(size_t)vIdx].ratio, ct::ivec2(w, w), false);
					ImGui::ImageRect((ImTextureID)&m_ImGuiTextures[(size_t)vIdx].descriptor,
						ImVec2((float)rect.x, (float)rect.y),
						ImVec2((float)rect.w, (float)rect.h));
				}
			}
		}
	}
}