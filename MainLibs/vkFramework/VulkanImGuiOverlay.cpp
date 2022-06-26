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
	VulkanImGuiOverlay::VulkanImGuiOverlay(
		vkApi::VulkanCore* vVulkanCore,
		VulkanImGuiRenderer* vVulkanImGuiRenderer,
		vkApi::VulkanWindow* vVulkanWindow)
	{
		ZoneScoped;

		m_VulkanCore = vVulkanCore;
		m_VulkanImGuiRenderer = vVulkanImGuiRenderer;
		m_VulkanWindow = vVulkanWindow;

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable ViewPort
		io.FontAllowUserScaling = true; // activate zoom feature with ctrl + mousewheel
		io.ConfigWindowsMoveFromTitleBarOnly = true; // can move windows only with titlebar
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
		io.ConfigViewportsNoDecoration = false; // toujours mettre une frame au fenetre enfant
#endif

		// Setup Dear ImGui style
		//ImGui::StyleColorsClassic();
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		m_PipelineCache = m_VulkanCore->getDevice().createPipelineCache(vk::PipelineCacheCreateInfo());

		ImGui_ImplGlfw_InitForVulkan(m_VulkanWindow->WinPtr(), true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = (VkInstance)m_VulkanCore->getInstance();
		init_info.PhysicalDevice = (VkPhysicalDevice)m_VulkanCore->getPhysicalDevice();
		init_info.Device = (VkDevice)m_VulkanCore->getDevice();
		init_info.QueueFamily = m_VulkanCore->getQueue(vk::QueueFlagBits::eGraphics).familyQueueIndex;
		init_info.Queue = (VkQueue)m_VulkanCore->getQueue(vk::QueueFlagBits::eGraphics).vkQueue;
		init_info.PipelineCache = (VkPipelineCache)m_PipelineCache;
		init_info.DescriptorPool = (VkDescriptorPool)m_VulkanCore->getDescriptorPool();
		init_info.Allocator = nullptr;
		init_info.MinImageCount = m_VulkanCore->getSwapchainFrameBuffers();
		init_info.ImageCount = m_VulkanCore->getSwapchainFrameBuffers();
		init_info.MSAASamples = (VkSampleCountFlagBits)m_VulkanCore->getSwapchainFrameBufferSampleCount();
		init_info.CheckVkResultFn = vkApi::VulkanCore::check_error;

		m_VulkanImGuiRenderer->Init(&init_info, (VkRenderPass)m_VulkanCore->getMainRenderPass());

		// load memory font file
		ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_RM, 15.0f);
		static ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
		static ImWchar icons_ranges_NDP[] = { ICON_MIN_NDP, ICON_MAX_NDP, 0 };
		ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_NDP, 15.0f, &icons_config, icons_ranges_NDP);
		static ImWchar icons_ranges_NDP2[] = { ICON_MIN_NDP2, ICON_MAX_NDP2, 0 };
		ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_NDP2, 15.0f, &icons_config, icons_ranges_NDP2);

		m_VulkanImGuiRenderer->CreateFontsTexture(vVulkanCore);
	}

	VulkanImGuiOverlay::~VulkanImGuiOverlay()
	{
		Destroy(); // detuire les descripteur de imgui
	}

	void VulkanImGuiOverlay::Destroy()
	{
		ZoneScoped;

		m_VulkanCore->getDevice().waitIdle();

		if (m_PipelineCache != vk::PipelineCache(nullptr))
		{
			m_VulkanCore->getDevice().destroyPipelineCache(m_PipelineCache);
		}

		m_VulkanImGuiRenderer->Unit();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void VulkanImGuiOverlay::begin()
	{
		ZoneScoped;

		// Start the Dear ImGui frame
		m_VulkanImGuiRenderer->NewFrame();
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
			// Record Imgui Draw Data and draw funcs into command buffer
			m_VulkanImGuiRenderer->RenderDrawData(
				ImGui::GetDrawData(),
				(VkCommandBuffer)m_VulkanCore->getGraphicCommandBuffer());

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