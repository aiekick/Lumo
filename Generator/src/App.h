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

#include <glm/glm.hpp>

#include <vkFramework/vkFramework.h>

#include <string>
#include <functional>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <array>
#include <memory>

struct FileDialogAsset
{
	Texture2DPtr tex = nullptr;
	vk::DescriptorSet set;
};

const uint32_t WIDTH = 1700;
const uint32_t HEIGHT = 700;
struct GLFWwindow;
class App
{
private:
	vkApi::VulkanImGuiOverlayPtr m_VulkanImGuiOverlayPtr = nullptr;
	VulkanImGuiRendererPtr m_VulkanImGuiRendererPtr = nullptr;
	vkApi::VulkanWindowPtr m_VulkanWindowPtr = nullptr;
	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;

public:
	std::vector<std::shared_ptr<FileDialogAsset>> m_FileDialogAssets;

	ct::fvec2 m_NormalizedMousePos;
	ct::fvec2 m_LastNormalizedMousePos;
	bool m_MouseDrag = false;
	bool m_UINeedRefresh = false;
	bool m_CanWeTuneCamera = true;
	bool m_CanWeCatchMouse = true;
	float m_DisplayQuality = 1.0f;

	uint32_t m_CurrentFrame = 0U;

public:
	int Run(const std::string& vAppPath);
	vkApi::VulkanWindowPtr GetWindowPtr();
	bool Unit(GLFWwindow* vWindow);

private:
	bool Init(GLFWwindow* vWindow);
	void MainLoop(GLFWwindow* vWindow);
	bool BeginRender(bool& vNeedResize);
	void EndRender();
	void PrepareImGui(ct::ivec4 vViewport);
	void Update();
	void IncFrame();

public: // singleton
	static App* Instance()
	{
		static App _instance;
		return &_instance;
	}

protected:
	App() = default; // Prevent construction
	App(const App&) = default; // Prevent construction by copying
	App& operator =(const App&) { return *this; }; // Prevent assignment
	~App() = default; // Prevent unwanted destruction
};
