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

#pragma once

#include <vulkan/vulkan.hpp>
#include <ctools/cTools.h>

struct GLFWwindow;
namespace vkApi
{
	class VulkanWindow
	{
	public:
		void Init(int width, int height, const std::string& name, bool offscreen);
		void Unit();

		ct::ivec2 pixelrez() const;
		ct::ivec2 clentrez() const;

		bool IsMinimized();

		const std::string& name() const;
		vk::SurfaceKHR createSurface(vk::Instance vkInstance);
		const std::vector<const char*>& vkInstanceExtensions() const;

		GLFWwindow* WinPtr() const;

	private:
		std::string d_name;
		GLFWwindow* m_Window = nullptr;
		std::vector<const char*> d_vkInstanceExtension;

	public:
		VulkanWindow() {} // Prevent construction
		VulkanWindow(const VulkanWindow&) {}; // Prevent construction by copying
		VulkanWindow& operator =(const VulkanWindow&) { return *this; }; // Prevent assignment
		~VulkanWindow() {} // Prevent unwanted destruction
	};
}
