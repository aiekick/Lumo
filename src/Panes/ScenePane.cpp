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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "ScenePane.h"

#include <Gui/MainFrame.h>

#include <Panes/Manager/LayoutManager.h>
#include <ImWidgets/ImWidgets.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>

#include <imgui/imgui_internal.h>

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>

#include <cinttypes> // printf zu

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

static int SourcePane_WidgetId = 0;

ScenePane::ScenePane() = default;
ScenePane::~ScenePane() = default;

bool ScenePane::Init()
{
	ZoneScoped;

	return true;
}

void ScenePane::Unit()
{
	ZoneScoped;
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

int ScenePane::DrawPanes(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas)
{
	ZoneScoped;

	SourcePane_WidgetId = vWidgetId;

	if (LayoutManager::Instance()->m_Pane_Shown & m_PaneFlag)
	{
		static ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_MenuBar;
		if (ImGui::Begin<PaneFlags>(m_PaneName,
			&LayoutManager::Instance()->m_Pane_Shown, m_PaneFlag, flags))
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
				//SourcePane_WidgetId = SceneManager::Instance()->DrawSceneTree(SourcePane_WidgetId);
			}
		}

		ImGui::End();
	}

	return SourcePane_WidgetId;
}

void ScenePane::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, std::string vUserDatas)
{
	ZoneScoped;

	if (ProjectFile::Instance()->IsLoaded())
	{
		/*
		ImVec2 maxSize = MainFrame::Instance()->m_DisplaySize;
		ImVec2 minSize = maxSize * 0.5f;
		if (ImGuiFileDialog::Instance()->Display("OpenShaderCode",
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking,
			minSize, maxSize))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
				auto code = FileHelper::Instance()->LoadFileToString(filePathName);
				SetCode(code);
			}

			ImGuiFileDialog::Instance()->Close();
		}
		*/
	}
}

int ScenePane::DrawWidgets(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas)
{
	return vWidgetId;
}