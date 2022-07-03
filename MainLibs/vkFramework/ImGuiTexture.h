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

#include <ctools/cTools.h>
#include <vulkan/vulkan.hpp>
#include <vkFramework/vkFramework.h>

class ImGuiTexture;
typedef std::shared_ptr<ImGuiTexture> ImGuiTexturePtr;
typedef ct::cWeak<ImGuiTexture> ImGuiTextureWeak;

namespace vkApi { class VulkanFrameBufferAttachment; }
class VulkanImGuiRenderer;
class ImGuiTexture
{
public:
	static ImGuiTexturePtr Create();

public:
	ImGuiTextureWeak m_This;
	vk::DescriptorSet descriptor = {};
	float ratio = 0.0f;
	bool canDisplayPreview = false;
	bool firstLoad = true;
	bool destroyed = false;

public:
	ImGuiTexture();
	~ImGuiTexture();
	void SetDescriptor(VulkanImGuiRendererWeak vVulkanImGuiRenderer, vk::DescriptorImageInfo* vDescriptorImageInfo, float vRatio = 1.0f);
	void SetDescriptor(VulkanImGuiRendererWeak vVulkanImGuiRenderer, vkApi::VulkanFrameBufferAttachment* vVulkanFrameBufferAttachment);
	void ClearDescriptor();
	void DestroyDescriptor(VulkanImGuiRendererWeak vVulkanImGuiRenderer);
};