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

#include <imgui/imgui.h>
#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>
#include <Systems/FrameActionSystem.h>
#include <Graph/Graph.h>
#include <functional>
#include <string>
#include <vector>
#include <map>

struct GLFWwindow;
class MainFrame : public conf::ConfigAbstract
{
public:
	bool leftMouseClicked = false;
	bool leftMouseReleased = false;
	bool rightMouseClicked = false;
	ImVec2 m_DisplayPos = ImVec2(0, 0); // viewport
	ImVec2 m_DisplaySize = ImVec2(1280, 720);

	bool m_ShowImGui = false;				// show ImGui win
	bool m_ShowMetric = false;				// show metrics

private:
	bool m_ShowAboutDialog = false;			// show about dlg
	bool m_NeedToCloseApp = false;			// whenn app closing app is required
	bool m_SaveDialogIfRequired = false;	// open save options dialog (save / save as / continue without saving / cancel)
	bool m_SaveDialogActionWasDone = false;	// if action was done by save options dialog
	FrameActionSystem m_ActionSystem;

private:
	bool m_NeedToNewProject = false;
	bool m_NeedToLoadProject = false;
	bool m_NeedToCloseProject = false;
	std::string m_FilePathNameToLoad;

public:
	void Init();
	void Unit();

	void Display(const uint32_t& vCurrentFrame, ct::ivec4 vViewport);

	GLFWwindow* GetGLFWwindow() { return m_Window; }
	FrameActionSystem* GetActionSystem() { return &m_ActionSystem; }

private: // imgui pane / dialogs
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame);
	void DrawContent();

private:
	void SetAppTitle(const std::string& vFilePathName = std::string());

public: // configuration
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "");
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "");

public: // singleton
	static MainFrame* Instance(GLFWwindow* vWin = 0)
	{
		static MainFrame _instance(vWin);
		return &_instance;
	}

protected:
	GLFWwindow* m_Window = 0;
	MainFrame(GLFWwindow* vWin); // Prevent construction
	MainFrame(const MainFrame&) = default; // Prevent construction by copying
	MainFrame& operator =(const MainFrame&) { return *this; }; // Prevent assignment
	~MainFrame(); // Prevent unwanted destruction
};
