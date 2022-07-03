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
#include <vkFramework/vkFramework.h>

struct GLFWwindow;
namespace vkApi
{
	class VulkanWindow
	{
	public:
		static VulkanWindowPtr Create(const int& vWidth, const int& vHeight, const std::string& vName, const bool& vOffScreen);

	private:
		std::string m_Name;
		GLFWwindow* m_Window = nullptr;
		std::vector<const char*> m_VKInstanceExtension;

	public:
		bool Init(const int& vWidth, const int& vHeight, const std::string& vName, const bool& vOffScreen);
		void Unit();

		ct::ivec2 getFrameBufferResolution() const;
		ct::ivec2 getWindowResolution() const;

		bool IsMinimized();

		vk::SurfaceKHR createSurface(vk::Instance vkInstance);
		
		const std::string& name() const;
		const std::vector<const char*>& getVKInstanceExtensions() const;

		GLFWwindow* getWindowPtr() const;

	public:
		VulkanWindow() {} // Prevent construction
		VulkanWindow(const VulkanWindow&) {}; // Prevent construction by copying
		VulkanWindow& operator =(const VulkanWindow&) { return *this; }; // Prevent assignment
		~VulkanWindow() {} // Prevent unwanted destruction
	};
}
