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

#include "VulkanWindow.h"
#include <ctools/Logger.h>
#include <GLFW/glfw3.h>

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

static void glfw_error_callback(int error, const char* description)
{
	ZoneScoped;

	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
	LogVarError("Glfw Error %d: %s\n", error, description);
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
	void VulkanWindow::Init(int width, int height, const std::string& name, bool offscreen)
	{
		ZoneScoped;

		d_name = name;

		glfwSetErrorCallback(glfw_error_callback);
		if (!glfwInit())
		{
			exit(EXIT_FAILURE);
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		if (offscreen)
		{
			glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
		}
		m_Window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
		if (!m_Window)
		{
			exit(EXIT_FAILURE);
		}

		d_vkInstanceExtension = getRequiredExtensions();
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

	const std::vector<const char*>& VulkanWindow::vkInstanceExtensions() const
	{
		ZoneScoped;

		return d_vkInstanceExtension;
	}

	GLFWwindow* VulkanWindow::WinPtr() const
	{
		ZoneScoped;

		return m_Window;
	}

	ct::ivec2 VulkanWindow::pixelrez() const
	{
		ZoneScoped;

		ct::ivec2 res;
		glfwGetFramebufferSize(m_Window, &res.x, &res.y);
		return res;
	}

	ct::ivec2 VulkanWindow::clentrez() const
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
		auto size = pixelrez();
		if (size.x == 0 || size.y == 0)
			return true;
		return false;
	}

	const std::string& VulkanWindow::name() const
	{
		ZoneScoped;

		return d_name;
	}
}