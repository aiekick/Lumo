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
