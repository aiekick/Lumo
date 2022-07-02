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

#include <ctools/ConfigAbstract.h>
#include <Panes/Abstract/AbstractPane.h>
#include <imgui/imgui.h>
#include <array>

class ProjectFile;
class LayoutManager : public conf::ConfigAbstract
{
private:
	ImGuiID m_DockSpaceID = 0;
	bool m_FirstLayout = false;
	bool m_FirstStart = true;
	char m_MenuLabel[PANE_NAME_BUFFER_SIZE + 1] = "";
	char m_DefaultMenuLabel[PANE_NAME_BUFFER_SIZE + 1] = "";
	std::array<float, (size_t)PaneDisposal::Count> m_PaneDisposalSizes = 
	{	0.0f, // central size is ignored because depednant of others
		200.0f, // left size
		200.0f, // right size
		200.0f, // bottom size
		200.0f // top size
	};

protected:
	std::map<PaneDisposal, AbstractPane*> m_PanesByDisposal;
	std::map<const char*, AbstractPane*> m_PanesByName;
	std::vector<AbstractPane*> m_PanesInDisplayOrder;
	std::map<PaneFlags, AbstractPane*> m_PanesByFlag;
	
public:
	PaneFlags m_Pane_Focused_Default = 0;
	PaneFlags m_Pane_Opened_Default = 0;
	PaneFlags m_Pane_Shown = 0;
	PaneFlags m_Pane_Focused = 0;
	PaneFlags m_Pane_Hovered = 0;
	PaneFlags m_Pane_LastHovered = 0;
	ImVec2 m_LastSize;

public:
	void AddPane(
		AbstractPane*vPane,
		const char* vName,
		PaneFlags vFlag, 
		PaneDisposal vPaneDisposal, 
		bool vIsOpenedDefault, 
		bool vIsFocusedDefault);
	void SetPaneDisposalSize(const PaneDisposal& vPaneDisposal, float vSize);

public:
	void Init(const char* vMenuLabel, const char* vDefautlMenuLabel);
	void Unit();
	void InitAfterFirstDisplay(ImVec2 vSize);
	bool BeginDockSpace(ImGuiDockNodeFlags vFlags);
	void EndDockSpace();
	bool IsDockSpaceHoleHovered();

	void ApplyInitialDockingLayout(ImVec2 vSize = ImVec2(0, 0));

	virtual void DisplayMenu(ImVec2 vSize);
	virtual int DisplayPanes(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas = "");
	virtual void DrawDialogsAndPopups(const uint32_t& vCurrentFrame, std::string vUserDatas = "");
	virtual int DrawWidgets(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas = "");

public:
	void ShowSpecificPane(PaneFlags vPane);
	void HideSpecificPane(PaneFlags vPane);
	void FocusSpecificPane(PaneFlags vPane);
	void FocusSpecificPane(const char* vlabel);
	void ShowAndFocusSpecificPane(PaneFlags vPane);
	bool IsSpecificPaneFocused(PaneFlags vPane);
	bool IsSpecificPaneFocused(const char* vlabel);
	void AddSpecificPaneToExisting(const char* vNewPane, const char* vExistingPane);

private: // configuration
	PaneFlags Internal_GetFocusedPanes();
	void Internal_SetFocusedPanes(PaneFlags vActivePanes);

public: // configuration
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "");
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "");

public: // singleton
	static LayoutManager *Instance()
	{
		static LayoutManager _instance;
		return &_instance;
	}

protected:
	LayoutManager(); // Prevent construction
	LayoutManager(const LayoutManager&) = delete;
	LayoutManager& operator =(const LayoutManager&) = delete;
	virtual ~LayoutManager(); // Prevent unwanted destruction
};

