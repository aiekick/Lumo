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

#include "App.h"

#include <cstdio>			// printf, fprintf
#include <cstdlib>			// abort
#include <iostream>			// std::cout
#include <stdexcept>		// std::exception
#include <algorithm>		// std::min, std::max
#include <fstream>			// std::ifstream
#include <chrono>			// timer

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <ctools/FileHelper.h>
#include <ctools/cTools.h>

#include <ctools/Logger.h>

#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanWindow.h>
#include <vkFramework/VulkanImGuiOverlay.h>
#include <vkFramework/VulkanImGuiRenderer.h>
#include <vkFramework/Texture2D.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <ImWidgets/ImWidgets.h>

#include <Gui/MainFrame.h>

#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <Systems/CommonSystem.h>
#include <vkprofiler/Profiler.h>
#include <Profiler/vkProfiler.hpp>

#include <Headers/LumoCodeGeneratorBuild.h>

#include <Base/Base.h>

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

int App::Run(const std::string& vAppPath)
{
	ZoneScoped;

	printf("-----------\n");
	printf("[[ Lumo Code Generator Beta %s ]]\n", LumoCodeGenerator_BuildId);

	FileHelper::Instance()->SetAppPath(vAppPath);
	FileHelper::Instance()->SetCurDirectory(FileHelper::Instance()->GetAppPath());

#ifdef _DEBUG
	FileHelper::Instance()->CreateDirectoryIfNotExist("debug");
#endif

	FileHelper::Instance()->CreateDirectoryIfNotExist("plugins");
	FileHelper::Instance()->CreateDirectoryIfNotExist("shaders");
	FileHelper::Instance()->CreateDirectoryIfNotExist("projects");

	m_VulkanWindowPtr = vkApi::VulkanWindow::Create(WIDTH, HEIGHT, PROJECT_NAME " beta", false);
	if (m_VulkanWindowPtr)
	{
		const auto& main_window = m_VulkanWindowPtr->getWindowPtr();
		if (Init(main_window))
		{
			MainLoop(main_window);
			Unit(main_window);
		}

		m_VulkanWindowPtr->Unit();
		m_VulkanWindowPtr.reset();
	}

	return 0;
}

bool App::Init(GLFWwindow* vWindow)
{
	ZoneScoped;

	bool res = false;

	// Setup Vulkan
	if (glfwVulkanSupported())
	{
		// Core
		vkApi::VulkanCore::sVulkanShader = VulkanShader::Create();
		if (vkApi::VulkanCore::sVulkanShader)
		{
			m_VulkanCorePtr = vkApi::VulkanCore::Create(m_VulkanWindowPtr, "Lumo", 1, "Lumo Engine", 1, true, true);
			if (m_VulkanCorePtr)
			{
				// apres la creation du core
				CommonSystem::Instance()->CreateBufferObject(m_VulkanCorePtr);

				m_VulkanImGuiRendererPtr = std::make_shared<VulkanImGuiRenderer>();
				m_VulkanCorePtr->SetVulkanImGuiRenderer(m_VulkanImGuiRendererPtr);

				m_VulkanImGuiOverlayPtr = vkApi::VulkanImGuiOverlay::Create(
					m_VulkanCorePtr, m_VulkanImGuiRendererPtr, m_VulkanWindowPtr); // needed for alloc ImGui Textures

				ImGui::CustomStyle::Instance();

				// apres les autres, car on charge le fichier projet
				MainFrame::Instance(vWindow)->Init();

#ifdef USE_THUMBNAILS
				ImGuiFileDialog::Instance()->SetCreateThumbnailCallback([this](IGFD_Thumbnail_Info* vThumbnail_Info)
					{
						if (vThumbnail_Info &&
							vThumbnail_Info->isReadyToUpload &&
							vThumbnail_Info->textureFileDatas)
						{
							std::shared_ptr<FileDialogAsset> res = std::shared_ptr<FileDialogAsset>(new FileDialogAsset,
								[](FileDialogAsset* obj)
								{
									delete obj;
								}
							);

							res->tex = Texture2D::CreateFromMemory(
								m_VulkanCorePtr,
								vThumbnail_Info->textureFileDatas,
								vThumbnail_Info->textureWidth,
								vThumbnail_Info->textureHeight,
								vThumbnail_Info->textureChannels);
							res->set = m_VulkanImGuiRendererPtr->CreateImGuiTexture(
								(VkSampler)res->tex->m_DescriptorImageInfo.sampler,
								(VkImageView)res->tex->m_DescriptorImageInfo.imageView,
								(VkImageLayout)res->tex->m_DescriptorImageInfo.imageLayout);

							vThumbnail_Info->userDatas = (void*)res.get();

							m_FileDialogAssets.push_back(res);

							vThumbnail_Info->textureID = (ImTextureID)&res->set;

							delete[] vThumbnail_Info->textureFileDatas;
							vThumbnail_Info->textureFileDatas = nullptr;

							vThumbnail_Info->isReadyToUpload = false;
							vThumbnail_Info->isReadyToDisplay = true;

							m_VulkanCorePtr->getDevice().waitIdle();
						}
					});
				ImGuiFileDialog::Instance()->SetDestroyThumbnailCallback([this](IGFD_Thumbnail_Info* vThumbnail_Info)
					{
						if (vThumbnail_Info)
						{
							if (vThumbnail_Info->userDatas)
							{
								auto asset = (FileDialogAsset*)vThumbnail_Info->userDatas;
								asset->tex.reset();
								m_VulkanImGuiRendererPtr->DestroyImGuiTexture(&asset->set);
							}
						}
					});
#endif // USE_THUMBNAILS

				vkprof::vkProfiler::Instance()->Init(
					m_VulkanCorePtr->getPhysicalDevice(),
					m_VulkanCorePtr->getDevice());

				res = true;
			}
		} 
	}

	return res;
}

void App::MainLoop(GLFWwindow* vWindow)
{
	while (!glfwWindowShouldClose(vWindow))
	{
		ZoneScoped;

		// maintain active, prevent user change via imgui dialog
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Viewport

		glfwPollEvents();

		Update(); // to do absolutly beofre imgui rendering

		ct::ivec4 viewportRect = ct::ivec4(0, m_VulkanWindowPtr->getWindowResolution());

		ImGui::SetPUSHID(125);

		PrepareImGui(viewportRect);

		// Merged Rendering
		bool needResize = false;
		if (BeginRender(needResize))
		{
			m_VulkanImGuiOverlayPtr->render();
			EndRender();
		}

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

#ifdef USE_THUMBNAILS
		vkDeviceWaitIdle((VkDevice)m_VulkanCorePtr->getDevice());
		ImGuiFileDialog::Instance()->ManageGPUThumbnails();
#endif

		++m_CurrentFrame;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// RENDER ////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool App::BeginRender(bool& vNeedResize)
{
	ZoneScoped;

	if (m_VulkanCorePtr->AcquireNextImage(m_VulkanWindowPtr))
	{
		m_VulkanCorePtr->frameBegin();

		auto devicePtr = m_VulkanCorePtr->getFrameworkDevice().getValidShared();
		if (devicePtr)
		{
			auto cmd = m_VulkanCorePtr->getGraphicCommandBuffer();
			devicePtr->BeginDebugLabel(&cmd, "ImGui", IMGUI_RENDERER_DEBUG_COLOR);

			{
				TracyVkZone(m_VulkanCorePtr->getTracyContext(), cmd, "Record Renderer Command buffer");
			}

			m_VulkanCorePtr->beginMainRenderPass();

			return true;
		}
	}
	else // maybe a resize will fix
	{
		vNeedResize = true;
	}

	return false;
}

void App::EndRender()
{
	ZoneScoped;

	m_VulkanCorePtr->endMainRenderPass();

	auto cmd = m_VulkanCorePtr->getGraphicCommandBuffer();
	
	auto devicePtr = m_VulkanCorePtr->getFrameworkDevice().getValidShared();
	if (devicePtr)
	{
		devicePtr->EndDebugLabel(&cmd);
	}

	{
		TracyVkCollect(m_VulkanCorePtr->getTracyContext(), cmd);
	}

	{
		vkprof::vkProfiler::Instance()->Collect(cmd);
	}

	m_VulkanCorePtr->frameEnd();
	m_VulkanCorePtr->Present();
}

void App::PrepareImGui(ct::ivec4 vViewport)
{
	ZoneScoped;

	// ImGui Calc juste avant de rendre dnas la swapchain
	m_VulkanImGuiOverlayPtr->begin();

	auto io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		const auto viewport = ImGui::GetMainViewport();
		if (viewport)
		{
			const auto pos = viewport->WorkPos;
			const auto size = viewport->WorkSize;
			vViewport.x = (int)pos.x;
			vViewport.y = (int)pos.y;
			vViewport.z = (int)size.x;
			vViewport.w = (int)size.y;
		}
	}
	else
	{
		vViewport.x = 0;
		vViewport.y = 0;
	}

	MainFrame::Instance()->Display(m_CurrentFrame, vViewport);

	m_VulkanImGuiOverlayPtr->end();
}

void App::Update()
{
	ZoneScoped;

	m_VulkanCorePtr->GetDeltaTime(m_CurrentFrame);

	CommonSystem::Instance()->UploadBufferObjectIfDirty(m_VulkanCorePtr);
}

void App::IncFrame()
{
	++m_CurrentFrame;
}

vkApi::VulkanWindowPtr App::GetWindowPtr()
{
	return m_VulkanWindowPtr;
}

bool App::Unit(GLFWwindow* vWindow)
{
	ZoneScoped;

	UNUSED(vWindow);

	vkDeviceWaitIdle((VkDevice)m_VulkanCorePtr->getDevice());

	vkprof::vkProfiler::Instance()->Unit();

	m_FileDialogAssets.clear();

	MainFrame::Instance()->Unit(); // detruit tout les panes, dont les nodes

	m_VulkanImGuiOverlayPtr.reset();
	m_VulkanImGuiRendererPtr.reset();

	CommonSystem::Instance()->DestroyBufferObject();

	// fini la destruction de vulkan
	//vkApi::VulkanCore::sVulkanShader->Unit();
	vkApi::VulkanCore::sVulkanShader.reset();

	m_VulkanCorePtr->Unit();
	m_VulkanCorePtr.reset();

	return true;
}
