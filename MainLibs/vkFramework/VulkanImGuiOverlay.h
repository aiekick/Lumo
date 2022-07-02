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

#include <memory>

#include <vulkan/vulkan.hpp>
#include <imgui/imgui.h>

class VulkanImGuiRenderer;
namespace vkApi
{
	class VulkanCore;
	class VulkanWindow;
	class VulkanImGuiOverlay
	{
	public:
		VulkanImGuiOverlay(
			vkApi::VulkanCore* vVulkanCore, 
			VulkanImGuiRenderer *vVulkanImGuiRenderer, 
			vkApi::VulkanWindow *vVulkanWindow);
		~VulkanImGuiOverlay();

		void Destroy();

		void begin();
		void end();
		virtual bool render();

		void drawFPS();
		void drawDemo();

		ImGuiIO& imgui_io();

	private:
		vkApi::VulkanCore* m_VulkanCore = nullptr;
		VulkanImGuiRenderer* m_VulkanImGuiRenderer = nullptr;
		vkApi::VulkanWindow* m_VulkanWindow = nullptr;

	private:
		bool m_IsRecording = false;
		vk::PipelineCache m_PipelineCache = nullptr;
	};
}
