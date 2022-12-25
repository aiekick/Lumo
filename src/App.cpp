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

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Systems/FilesTrackerSystem.h>
#include <ctools/FileHelper.h>
#include <ctools/cTools.h>

#include <Panes/GraphPane.h>

#include <ctools/Logger.h>

#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanWindow.h>
#include <vkFramework/VulkanImGuiOverlay.h>
#include <vkFramework/VulkanImGuiRenderer.h>
#include <vkFramework/Texture2D.h>

#include <ImGuizmo/ImGuizmo.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <ImWidgets/ImWidgets.h>

#include <Gui/MainFrame.h>

#include <ImGuiFileDialog/ImGuiFileDialog.h>

#include <Graph/Manager/NodeManager.h>
#include <Systems/CommonSystem.h>
#include <vkProfiler/Profiler.h>
#include <Profiler/vkProfiler.hpp>

#include <Plugins/PluginManager.h>

#include <Headers/LumoBuild.h>

#include <Panes/View3DPane.h>
#include <Panes/View2DPane.h>

#include <Base/Base.h>

#include <Systems/RenderDocController.h>

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

int App::Run(const std::string& vAppPath)
{
	ZoneScoped;

	printf("-----------\n");
	printf("[[ Lumo Beta %s ]]\n", Lumo_BuildId);

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
		InitFilesTracker(
			std::bind(&App::UpdateFiles, this, std::placeholders::_1),
			std::list<std::string>{ "projects", "shaders" });

		// Core
		vkApi::VulkanCore::sVulkanShader = VulkanShader::Create();
		if (vkApi::VulkanCore::sVulkanShader)
		{
			bool _use_RTX = true;

			m_VulkanCorePtr = vkApi::VulkanCore::Create(m_VulkanWindowPtr, "Lumo", 1, "Lumo Engine", 1, true, _use_RTX);
			if (m_VulkanCorePtr)
			{
				// apres la creation du core
				CommonSystem::Instance()->CreateBufferObject(m_VulkanCorePtr);

				m_VulkanImGuiOverlayPtr = vkApi::VulkanImGuiOverlay::Create(
					m_VulkanCorePtr, m_VulkanWindowPtr); // needed for alloc ImGui Textures

				View3DPane::Instance()->SetVulkanImGuiRenderer(m_VulkanImGuiOverlayPtr->GetImGuiRenderer());
				View2DPane::Instance()->SetVulkanImGuiRenderer(m_VulkanImGuiOverlayPtr->GetImGuiRenderer());

				ImGui::CustomStyle::Instance();

				// on charge les plugins
				PluginManager::Instance()->LoadPlugins(m_VulkanCorePtr);

				NodeManager::Instance()->Init(m_VulkanCorePtr);

				// apres les autres, car on charge le fichier projet
				MainFrame::Instance(vWindow)->Init();

#ifdef USE_THUMBNAILS
				ImGuiFileDialog::Instance()->SetCreateThumbnailCallback([this](IGFD_Thumbnail_Info* vThumbnail_Info)
					{
						if (vThumbnail_Info &&
							vThumbnail_Info->isReadyToUpload &&
							vThumbnail_Info->textureFileDatas)
						{
							m_VulkanCorePtr->getDevice().waitIdle();

							std::shared_ptr<FileDialogAsset> resPtr = std::shared_ptr<FileDialogAsset>(new FileDialogAsset,
								[](FileDialogAsset* obj)
								{
									delete obj;
								}
							);

							if (resPtr)
							{
								resPtr->texturePtr = Texture2D::CreateFromMemory(
									m_VulkanCorePtr,
									vThumbnail_Info->textureFileDatas,
									vThumbnail_Info->textureWidth,
									vThumbnail_Info->textureHeight,
									vThumbnail_Info->textureChannels);

								if (resPtr->texturePtr)
								{
									auto imguiRendererPtr = m_VulkanImGuiOverlayPtr->GetImGuiRenderer().getValidShared();
									if (imguiRendererPtr)
									{
										resPtr->descriptorSet = imguiRendererPtr->CreateImGuiTexture(
											(VkSampler)resPtr->texturePtr->m_DescriptorImageInfo.sampler,
											(VkImageView)resPtr->texturePtr->m_DescriptorImageInfo.imageView,
											(VkImageLayout)resPtr->texturePtr->m_DescriptorImageInfo.imageLayout);

										vThumbnail_Info->userDatas = (void*)resPtr.get();

										m_FileDialogAssets.push_back(resPtr);

										vThumbnail_Info->textureID = (ImTextureID)&resPtr->descriptorSet;
									}

									delete[] vThumbnail_Info->textureFileDatas;
									vThumbnail_Info->textureFileDatas = nullptr;

									vThumbnail_Info->isReadyToUpload = false;
									vThumbnail_Info->isReadyToDisplay = true;

									m_VulkanCorePtr->getDevice().waitIdle();
								}
							}
						}
					});
				ImGuiFileDialog::Instance()->SetDestroyThumbnailCallback([this](IGFD_Thumbnail_Info* vThumbnail_Info)
					{
						if (vThumbnail_Info)
						{
							if (vThumbnail_Info->userDatas)
							{
								m_VulkanCorePtr->getDevice().waitIdle();

								auto assetPtr = (FileDialogAsset*)vThumbnail_Info->userDatas;
								if (assetPtr)
								{
									assetPtr->texturePtr.reset();
									assetPtr->descriptorSet = vk::DescriptorSet{};
									auto imguiRendererPtr = m_VulkanImGuiOverlayPtr->GetImGuiRenderer().getValidShared();
									if (imguiRendererPtr)
									{
										imguiRendererPtr->DestroyImGuiTexture(&assetPtr->descriptorSet);
									}
								}

								m_VulkanCorePtr->getDevice().waitIdle();
							}
						}
					});
#endif // USE_THUMBNAILS

				vkprof::vkProfiler::Instance()->Init(
					m_VulkanCorePtr->getPhysicalDevice(),
					m_VulkanCorePtr->getDevice());

				RenderDocController::Instance()->Init();
				
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

		RenderDocController::Instance()->StartCaptureIfResquested();

		// maintain active, prevent user change via imgui dialog
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable; // Disable Viewport

		glfwPollEvents();

		// to absolutly do beofre all vk rendering commands
		m_VulkanCorePtr->ResetCommandPools();

		Update(); // to do absolutly beofre imgui rendering

		PrepareImGui(ct::ivec4(0, m_VulkanWindowPtr->getWindowResolution()));

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

		// delete imgui nodes now
		// like that => no issue with imgui descriptors because after imgui render and before next node computing
		GraphPane::Instance()->DeleteNodesIfAnys();

		// mainframe post actions
		MainFrame::Instance()->PostRenderingActions();

		++m_CurrentFrame;

		//will pause the view until we move the mouse
		//glfwWaitEvents();

		RenderDocController::Instance()->EndCaptureIfResquested();
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

	ImGui::SetPUSHID(125);
	PluginManager::Instance()->ResetImGuiID(125);

	// ImGui Calc juste avant de rendre dnas la swapchain
	m_VulkanImGuiOverlayPtr->begin();
	ImGuizmo::BeginFrame();

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

	NodeManager::Instance()->DisplayDialogsAndPopups(m_CurrentFrame, MainFrame::Instance()->m_DisplaySize, ImGui::GetCurrentContext());

	m_VulkanImGuiOverlayPtr->end();
}

void App::Update()
{
	ZoneScoped;

	m_VulkanCorePtr->GetDeltaTime(m_CurrentFrame);

	CommonSystem::Instance()->UploadBufferObjectIfDirty(m_VulkanCorePtr);

	CheckIfTheseAreSomeFileChanges();

	NodeManager::Instance()->Execute(m_CurrentFrame);
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

	if (m_VulkanCorePtr)
	{
		vkDeviceWaitIdle((VkDevice)m_VulkanCorePtr->getDevice());
	}

	RenderDocController::Instance()->Unit();

	vkprof::vkProfiler::Instance()->Unit();

	m_FileDialogAssets.clear();

	MainFrame::Instance()->Unit(); // detruit tout les panes, dont les nodes

	ProjectFile::Instance()->Clear();
	NodeManager::Instance()->Unit();

	if (m_VulkanImGuiOverlayPtr)
	{
		m_VulkanImGuiOverlayPtr->Unit();
		m_VulkanImGuiOverlayPtr.reset();
	}

	CommonSystem::Instance()->DestroyBufferObject();

	PluginManager::Instance()->Clear();

	if (vkApi::VulkanCore::sVulkanShader)
	{
		vkApi::VulkanCore::sVulkanShader->Unit();
		vkApi::VulkanCore::sVulkanShader.reset();
	}

	if (m_VulkanCorePtr)
	{
		m_VulkanCorePtr->Unit();
		m_VulkanCorePtr.reset();
	}

	return true;
}

#define SHADER_PATH 0

void App::AddPathToTrack(std::string vPathToTrack, bool vCreateDirectoryIfNotExist)
{
	ZoneScoped;

	if (!vPathToTrack.empty())
	{
		if (vCreateDirectoryIfNotExist)
		{
			FileHelper::Instance()->CreateDirectoryIfNotExist(vPathToTrack);
		}

		if (m_PathsToTrack.find(vPathToTrack) == m_PathsToTrack.end()) // non trouv�
		{
			m_PathsToTrack.emplace(vPathToTrack);
			FileHelper::Instance()->puSearchPaths.push_back(vPathToTrack);
			FilesTrackerSystem::Instance()->addWatch(vPathToTrack);
		}
	}
}

void App::InitFilesTracker(std::function<void(std::set<std::string>)> vChangeFunc, std::list<std::string> vPathsToTrack)
{
	ZoneScoped;

	m_ChangeFunc = vChangeFunc;

	for (auto path : vPathsToTrack)
	{
		AddPathToTrack(path, true);
	}

	FilesTrackerSystem::Instance()->Changes = false;
}

void App::CheckIfTheseAreSomeFileChanges()
{
	ZoneScoped;

	FilesTrackerSystem::Instance()->update();

	if (FilesTrackerSystem::Instance()->Changes)
	{
		m_ChangeFunc(FilesTrackerSystem::Instance()->files);

		FilesTrackerSystem::Instance()->files.clear();

		FilesTrackerSystem::Instance()->Changes = false;
	}
}

void App::UpdateFiles(const std::set<std::string>& vFiles) const
{
	ZoneScoped;

	std::set<std::string> res;

	for (auto file : vFiles)
	{
		if (file.find(".vert") != std::string::npos ||
			file.find(".frag") != std::string::npos ||
			file.find(".tess") != std::string::npos ||
			file.find(".eval") != std::string::npos ||
			file.find(".glsl") != std::string::npos ||
			file.find(".geom") != std::string::npos ||
			file.find(".scen") != std::string::npos ||
			file.find(".blue") != std::string::npos ||
			file.find(".comp") != std::string::npos ||
			file.find(".rgen") != std::string::npos ||
			file.find(".rint") != std::string::npos ||
			file.find(".miss") != std::string::npos ||
			file.find(".ahit") != std::string::npos ||
			file.find(".chit") != std::string::npos)
		{
			ct::replaceString(file, "\\", "/");
			res.emplace(file);
		}

		if (file.find(".lum") != std::string::npos)
		{
		}
	}

	if (!res.empty())
	{
		NodeManager::Instance()->UpdateShaders(res);
	}
}