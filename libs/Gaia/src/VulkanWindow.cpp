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

#include <gaia/VulkanWindow.h>
#include <ctools/Logger.h>
#include <GLFW/glfw3.h>

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

static void glfw_error_callback(int error, const char* description)
{
	ZoneScoped;

	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
	LogVarError("Glfw Error %d: %s", error, description);
}

static std::vector<const char*> getRequiredExtensions()
{
	ZoneScoped;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	//if (enableValidationLayers)
	{
		extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}
namespace vkApi
{

	VulkanWindowPtr VulkanWindow::Create(const int& vWidth, const int& vHeight, const std::string& vName, const bool& vOffScreen, const bool& vDecorated)
	{
		auto res = std::make_shared<VulkanWindow>();
		if (!res->Init(vWidth, vHeight, vName, vOffScreen, vDecorated))
		{
			res.reset();
		}
		return res;
	}

	bool VulkanWindow::Init(const int& vWidth, const int& vHeight, const std::string& vName, const bool& vOffScreen, const bool& vDecorated)
	{
		ZoneScoped;

		m_Name = vName;

		glfwSetErrorCallback(glfw_error_callback);
		if (!glfwInit())
		{
			return false;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		if (vOffScreen)
		{
			glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
		}
		else if (!vDecorated)
		{
			glfwWindowHint(GLFW_DECORATED, GL_FALSE);
		}

		m_Window = glfwCreateWindow(vWidth, vHeight, vName.c_str(), NULL, NULL);
		if (!m_Window)
		{
			return false;
		}

		m_VKInstanceExtension = getRequiredExtensions();

		return true;
	}

	void VulkanWindow::Unit()
	{
		ZoneScoped;

		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	vk::SurfaceKHR VulkanWindow::createSurface(vk::Instance vkInstance)
	{
		ZoneScoped;

		vk::SurfaceKHR surface;
		VkResult err = glfwCreateWindowSurface((VkInstance)vkInstance, m_Window, nullptr, (VkSurfaceKHR*)&surface);
		if (err != VK_SUCCESS)
		{
			exit(EXIT_FAILURE);
		}
		return surface;
	}

	const std::vector<const char*>& VulkanWindow::getVKInstanceExtensions() const
	{
		ZoneScoped;

		return m_VKInstanceExtension;
	}

	GLFWwindow* VulkanWindow::getWindowPtr() const
	{
		ZoneScoped;

		return m_Window;
	}

	ct::ivec2 VulkanWindow::getFrameBufferResolution() const
	{
		ZoneScoped;

		ct::ivec2 res;
		glfwGetFramebufferSize(m_Window, &res.x, &res.y);
		return res;
	}

	ct::ivec2 VulkanWindow::getWindowResolution() const
	{
		ZoneScoped;

		ct::ivec2 res;
		glfwGetWindowSize(m_Window, &res.x, &res.y);

		return res;
	}

	bool VulkanWindow::IsMinimized()
	{
		ZoneScoped;

		// pause if minimized
		auto size = getFrameBufferResolution();
		if (size.x == 0 || size.y == 0)
			return true;
		return false;
	}

	const std::string& VulkanWindow::name() const
	{
		ZoneScoped;

		return m_Name;
	}
}