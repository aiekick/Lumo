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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "VulkanImGuiOverlay.h"
#include <assert.h>

#include "VulkanCore.h"
#include "VulkanWindow.h"
#include "VulkanCommandBuffer.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <vkFramework/VulkanImGuiRenderer.h>
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

#include <GLFW/glfw3.h>

#include <FontIcons/CustomFont.cpp>
#include <FontIcons/CustomFont2.cpp>
#include <FontIcons/Roboto_Medium.cpp>
#include <ctools/FileHelper.h>

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

namespace vkApi
{
	VulkanImGuiOverlayPtr VulkanImGuiOverlay::Create(
		vkApi::VulkanCoreWeak vVulkanCoreWeak,
		VulkanImGuiRendererWeak vVulkanImGuiRendererWeak,
		vkApi::VulkanWindowWeak vVulkanWindowWeak)
	{
		auto res = std::make_shared<VulkanImGuiOverlay>();
		if (!res->Init(vVulkanCoreWeak, vVulkanImGuiRendererWeak, vVulkanWindowWeak))
		{
			res.reset();
		}
		return res;
	}

	VulkanImGuiOverlay::VulkanImGuiOverlay() = default;

	VulkanImGuiOverlay::~VulkanImGuiOverlay()
	{
		Unit(); // detuit les descripteur de imgui
	}

	bool VulkanImGuiOverlay::Init(
		vkApi::VulkanCoreWeak vVulkanCoreWeak,
		VulkanImGuiRendererWeak vVulkanImGuiRendererWeak,
		vkApi::VulkanWindowWeak vVulkanWindowWeak)
	{
		ZoneScoped;

		m_VulkanCoreWeak = vVulkanCoreWeak;
		m_VulkanImGuiRendererWeak = vVulkanImGuiRendererWeak;
		m_VulkanWindowWeak = vVulkanWindowWeak;

		auto corePtr = m_VulkanCoreWeak.getValidShared();
		if (corePtr)
		{
			// Setup Dear ImGui context
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
			//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable ViewPort
			io.FontAllowUserScaling = true; // activate zoom feature with ctrl + mousewheel
			io.ConfigWindowsMoveFromTitleBarOnly = true; // can move windows only with titlebar
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
			io.ConfigViewportsNoDecoration = false; // toujours mettre une frame au fenetre enfant
#endif

			ImGui::StyleColorsDark();

			m_PipelineCache = corePtr->getDevice().createPipelineCache(vk::PipelineCacheCreateInfo());

			auto winPtr = m_VulkanWindowWeak.getValidShared();
			if (winPtr)
			{
				ImGui_ImplGlfw_InitForVulkan(winPtr->getWindowPtr(), true);
				ImGui_ImplVulkan_InitInfo init_info = {};
				init_info.Instance = (VkInstance)corePtr->getInstance();
				init_info.PhysicalDevice = (VkPhysicalDevice)corePtr->getPhysicalDevice();
				init_info.Device = (VkDevice)corePtr->getDevice();
				init_info.QueueFamily = corePtr->getQueue(vk::QueueFlagBits::eGraphics).familyQueueIndex;
				init_info.Queue = (VkQueue)corePtr->getQueue(vk::QueueFlagBits::eGraphics).vkQueue;
				init_info.PipelineCache = (VkPipelineCache)m_PipelineCache;
				init_info.DescriptorPool = (VkDescriptorPool)corePtr->getDescriptorPool();
				init_info.Allocator = nullptr;
				init_info.MinImageCount = corePtr->getSwapchainFrameBuffers();
				init_info.ImageCount = corePtr->getSwapchainFrameBuffers();
				init_info.MSAASamples = (VkSampleCountFlagBits)corePtr->getSwapchainFrameBufferSampleCount();
				init_info.CheckVkResultFn = vkApi::VulkanCore::check_error;

				auto imguiRendPtr = m_VulkanImGuiRendererWeak.getValidShared();
				if (imguiRendPtr)
				{
					imguiRendPtr->Init(&init_info, (VkRenderPass)corePtr->getMainRenderPass());

					// load memory font file
					ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_RM, 15.0f);
					static ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
					static ImWchar icons_ranges_NDP[] = { ICON_MIN_NDP, ICON_MAX_NDP, 0 };
					ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_NDP, 15.0f, &icons_config, icons_ranges_NDP);
					static ImWchar icons_ranges_NDP2[] = { ICON_MIN_NDP2, ICON_MAX_NDP2, 0 };
					ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_NDP2, 15.0f, &icons_config, icons_ranges_NDP2);
					
					imguiRendPtr->CreateFontsTexture(corePtr);

					return true;
				}
			}
		}

		return false;
	}

	void VulkanImGuiOverlay::Unit()
	{
		ZoneScoped;

		auto corePtr = m_VulkanCoreWeak.getValidShared();
		if (corePtr)
		{
			corePtr->getDevice().waitIdle();

			if (m_PipelineCache != vk::PipelineCache(nullptr))
			{
				corePtr->getDevice().destroyPipelineCache(m_PipelineCache);
			}
		}

		auto imguiRendPtr = m_VulkanImGuiRendererWeak.getValidShared();
		if (imguiRendPtr)
		{
			imguiRendPtr->Unit();
		}

		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void VulkanImGuiOverlay::begin()
	{
		ZoneScoped;

		auto imguiRendPtr = m_VulkanImGuiRendererWeak.getValidShared();
		if (imguiRendPtr)
		{
			imguiRendPtr->NewFrame();
		}
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void VulkanImGuiOverlay::end()
	{
		ZoneScoped;

		ImGui::Render();
	}

	bool VulkanImGuiOverlay::render()
	{
		ZoneScoped;

		auto main_draw_datas = ImGui::GetDrawData();
		const bool main_is_minimized = (main_draw_datas->DisplaySize.x <= 0.0f || main_draw_datas->DisplaySize.y <= 0.0f);
		if (!main_is_minimized)
		{
			auto imguiRendPtr = m_VulkanImGuiRendererWeak.getValidShared();
			auto corePtr = m_VulkanCoreWeak.getValidShared();
			if (corePtr && imguiRendPtr)
			{
				imguiRendPtr->RenderDrawData(
					ImGui::GetDrawData(),
					(VkCommandBuffer)corePtr->getGraphicCommandBuffer());
			}

			return true;
		}

		return false;
	}

	void VulkanImGuiOverlay::drawFPS()
	{
		ZoneScoped;

		const ImGuiWindowFlags fpsWindowFlags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoScrollbar;

		ImGui::Begin("fps", 0, fpsWindowFlags);
		ImGui::SetWindowPos(ImVec2(0, 0));
		ImGui::Text("GUI: Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	void VulkanImGuiOverlay::drawDemo()
	{
		ZoneScoped;

		ImGui::ShowDemoWindow();
	}

	ImGuiIO& VulkanImGuiOverlay::imgui_io()
	{
		ZoneScoped;

		return ImGui::GetIO();
	}
}