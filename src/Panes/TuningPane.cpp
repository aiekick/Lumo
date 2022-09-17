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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "TuningPane.h"

#include <Panes/Manager/LayoutManager.h>
#include <ImWidgets/ImWidgets.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <vkFramework/VulkanImGuiRenderer.h>
#include <Project/ProjectFile.h>
#include <Gui/MainFrame.h>
#include <imgui/imgui_internal.h>
#include <Graph/Base/BaseNode.h>

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>

#include <Systems/CommonSystem.h>

#include <cinttypes> // printf zu

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

static int SourcePane_WidgetId = 0;

///////////////////////////////////////////////////////////////////////////////////
//// CONSTRUCTORS /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

TuningPane::TuningPane()
{

}

TuningPane::~TuningPane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// INIT/UNIT ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool TuningPane::Init()
{
	ZoneScoped;

	return true;
}

void TuningPane::Unit()
{
	ZoneScoped;

	SetParentNode();
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

int TuningPane::DrawPanes(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas, PaneFlags& vInOutPaneShown)
{
	ZoneScoped;

	SourcePane_WidgetId = vWidgetId;

	if (vInOutPaneShown & m_PaneFlag)
	{
		static ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_MenuBar;
		if (ImGui::Begin<PaneFlags>(m_PaneName,
			&vInOutPaneShown , m_PaneFlag, flags))
		{
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
			auto win = ImGui::GetCurrentWindowRead();
			if (win->Viewport->Idx != 0)
				flags |= ImGuiWindowFlags_NoResize;// | ImGuiWindowFlags_NoTitleBar;
			else
				flags = ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoBringToFrontOnFocus |
				ImGuiWindowFlags_MenuBar;
#endif
			if (ProjectFile::Instance()->IsLoaded())
			{
				SourcePane_WidgetId = DrawWidgets(vCurrentFrame, SourcePane_WidgetId, vUserDatas);
			}
		}

		ImGui::End();
	}

	return SourcePane_WidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// DIALOGS //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void TuningPane::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, std::string vUserDatas)
{
	ZoneScoped;
}

int TuningPane::DrawWidgets(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas)
{
	CommonSystem::Instance()->DrawImGui();

	auto ptr = GetParentNode().getValidShared();
	if (ptr)
	{
		ptr->DrawWidgets(vCurrentFrame, ImGui::GetCurrentContext());
	}

	return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// SELECTOR /////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void TuningPane::Select(BaseNodeWeak vObjet)
{
	SetParentNode(vObjet);
}