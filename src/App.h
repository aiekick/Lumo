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

#include <glm/glm.hpp>

#include <vkFramework/VulkanImGuiOverlay.h>
#include <vkFramework/Texture2D.h>
#include <ctools/cTools.h>

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

namespace vkApi
{
	class VulkanImGuiOverlay;
	class VulkanWindow;
	class VulkanCore;
}
class VulkanShader;
const uint32_t WIDTH = 1700;
const uint32_t HEIGHT = 700;
struct GLFWwindow;
class App
{
private:
	std::unique_ptr<vkApi::VulkanImGuiOverlay> m_VulkanImGuiOverlay = nullptr;
	std::shared_ptr<VulkanImGuiRenderer> m_VulkanImGuiRendererPtr = nullptr;
	std::shared_ptr<vkApi::VulkanWindow> m_VulkanWindowPtr = nullptr;
	std::shared_ptr<vkApi::VulkanCore> m_VulkanCorePtr = nullptr;

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
	std::shared_ptr<vkApi::VulkanWindow> GetWindowPtr();
	bool Unit(GLFWwindow* vWindow);

private:
	bool Init(GLFWwindow* vWindow);
	void MainLoop(GLFWwindow* vWindow);
	bool BeginRender(bool& vNeedResize);
	void EndRender();
	void PrepareImGui(ct::ivec4 vViewport);
	void Update();
	void IncFrame();

private:
	std::function<void(std::set<std::string>)> m_ChangeFunc;
	std::set<std::string> m_PathsToTrack;
	void AddPathToTrack(std::string vPathToTrack, bool vCreateDirectoryIfNotExist);
	void InitFilesTracker(std::function<void(std::set<std::string>)> vChangeFunc, std::list<std::string> vPathsToTrack);
	void CheckIfTheseAreSomeFileChanges();
	void UpdateFiles(const std::set<std::string>& vFiles) const;

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
