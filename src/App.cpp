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

#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanWindow.h>
#include <ctools/Logger.h>
#include <vkFramework/VulkanImGuiRenderer.h>

#include <ImGuizmo/ImGuizmo.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <ImWidgets/ImWidgets.h>

#include <Gui/MainFrame.h>

#include <ImGuiFileDialog/ImGuiFileDialog.h>

#include <Graph/Manager/NodeManager.h>
#include <Systems/CommonSystem.h>
#include <Utils/Mesh/MeshLoader.h>
#include <vkProfiler/Profiler.h>
#include <Profiler/vkProfiler.hpp>

#include <Headers/Build.h>

#include <Plugins/PluginManager.h>
#include <Panes/View3DPane.h>

#include <Base/Base.h>

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

int App::Run(const std::string& vAppPath)
{
	ZoneScoped;

	LogVarLightInfo("[[ Lumo Beta %s ]]\n", Lumo_BuildId);

	FileHelper::Instance()->SetAppPath(vAppPath);
	FileHelper::Instance()->SetCurDirectory(FileHelper::Instance()->GetAppPath());

#ifdef _DEBUG
	FileHelper::Instance()->CreateDirectoryIfNotExist("debug");
	FileHelper::Instance()->CreateDirectoryIfNotExist("plugins");
	FileHelper::Instance()->CreateDirectoryIfNotExist("shaders");
	FileHelper::Instance()->CreateDirectoryIfNotExist("projects");
#endif

	m_VulkanWindowPtr = std::make_shared<vkApi::VulkanWindow>();
	m_VulkanWindowPtr->Init(WIDTH, HEIGHT, PROJECT_NAME " beta", false);

	const auto& main_window = m_VulkanWindowPtr->WinPtr();
	if (Init(main_window))
	{
		MainLoop(main_window);
		Unit(main_window);
	}

	m_VulkanWindowPtr->Unit();
	m_VulkanWindowPtr.reset();

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
		vkApi::VulkanCore::sVulkanShader = std::make_shared<VulkanShader>();
		vkApi::VulkanCore::sVulkanShader->Init();

		m_VulkanCorePtr = std::make_shared<vkApi::VulkanCore>();
		m_VulkanCorePtr->Init(m_VulkanWindowPtr.get(), "Lumo", 1, "1.0", 1, true);
		// apres la creation du core
		CommonSystem::Instance()->CreateBufferObject(m_VulkanCorePtr.get());

		m_VulkanImGuiRendererPtr = std::make_shared<VulkanImGuiRenderer>();
		m_VulkanCorePtr->SetVulkanImGuiRenderer(m_VulkanImGuiRendererPtr);

		m_VulkanImGuiOverlay = std::make_unique<vkApi::VulkanImGuiOverlay>(
			m_VulkanCorePtr.get(), m_VulkanImGuiRendererPtr.get(), m_VulkanWindowPtr.get()); // needed for alloc ImGui Textures
		
		View3DPane::Instance()->SetVulkanImGuiRenderer(m_VulkanImGuiRendererPtr.get());

		ImGui::CustomStyle::Instance();

		// on charge les plugins
		PluginManager::Instance()->LoadPlugins(m_VulkanCorePtr.get());

		NodeManager::Instance()->Init(m_VulkanCorePtr.get());

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
						[this](FileDialogAsset* obj)
						{
							delete obj;
						}
					);

					res->tex = Texture2D::CreateFromMemory(
						m_VulkanCorePtr.get(),
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

		ct::ivec4 viewportRect = ct::ivec4(0, m_VulkanWindowPtr->clentrez());

		ImGui::SetPUSHID(125);
		PluginManager::Instance()->ResetImGuiID(125);

		PrepareImGui(viewportRect);

		// Merged Rendering
		bool needResize = false;
		if (BeginRender(needResize))
		{
			m_VulkanImGuiOverlay->render();
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

		//even if without UI
		MeshLoader::Instance()->FinishIfRequired();

		// delete imgui nodes now
		// like that => no issue with imgui descriptors because after imgui render and before next node computing
		GraphPane::Instance()->DeleteNodesIfAnys();

		// mainframe post actions
		MainFrame::Instance()->PostRenderingActions();

		++m_CurrentFrame;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// RENDER ////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool App::BeginRender(bool& vNeedResize)
{
	ZoneScoped;

	if (m_VulkanCorePtr->AcquireNextImage(m_VulkanWindowPtr.get()))
	{
		m_VulkanCorePtr->frameBegin();

		m_VulkanCorePtr->getFrameworkDevice()->BeginDebugLabel(&m_VulkanCorePtr->getGraphicCommandBuffer(), "ImGui", IMGUI_RENDERER_DEBUG_COLOR);

		{
			TracyVkZone(m_VulkanCorePtr->getTracyContext(), m_VulkanCorePtr->getGraphicCommandBuffer(), "Record Renderer Command buffer");
		}
		m_VulkanCorePtr->beginMainRenderPass();

		return true;
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

	m_VulkanCorePtr->getFrameworkDevice()->EndDebugLabel(&m_VulkanCorePtr->getGraphicCommandBuffer());

	{
		TracyVkCollect(m_VulkanCorePtr->getTracyContext(), m_VulkanCorePtr->getGraphicCommandBuffer());
	}

	{
		vkprof::vkProfiler::Instance()->Collect(m_VulkanCorePtr->getGraphicCommandBuffer());
	}

	m_VulkanCorePtr->frameEnd();
	m_VulkanCorePtr->Present();
}

void App::PrepareImGui(ct::ivec4 vViewport)
{
	ZoneScoped;

	// ImGui Calc juste avant de rendre dnas la swapchain
	m_VulkanImGuiOverlay->begin();
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

	NodeManager::Instance()->DisplayDialogsAndPopups(m_CurrentFrame, MainFrame::Instance()->m_DisplaySize);

	m_VulkanImGuiOverlay->end();
}

void App::Update()
{
	ZoneScoped;

	CommonSystem::Instance()->UploadBufferObjectIfDirty(m_VulkanCorePtr.get());

	CheckIfTheseAreSomeFileChanges();

	NodeManager::Instance()->Execute(m_CurrentFrame);
}

void App::IncFrame()
{
	++m_CurrentFrame;
}

std::shared_ptr<vkApi::VulkanWindow> App::GetWindowPtr()
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

	ProjectFile::Instance()->Clear();
	NodeManager::Instance()->Unit();

	m_VulkanImGuiOverlay.reset();
	m_VulkanImGuiRendererPtr.reset();

	CommonSystem::Instance()->DestroyBufferObject();

	PluginManager::Instance()->Clear();

	// fini la destruction de vulkan
	vkApi::VulkanCore::sVulkanShader->Unit();
	vkApi::VulkanCore::sVulkanShader.reset();

	m_VulkanCorePtr->Unit();
	m_VulkanCorePtr.reset();

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
			file.find(".comp") != std::string::npos)
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