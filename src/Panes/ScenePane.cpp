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

#include "ScenePane.h"
#include <ImWidgets.h>
#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <Project/ProjectFile.h>

#include <cinttypes> // printf zu

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

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

bool ScenePane::DrawPanes(const uint32_t& vCurrentFrame, PaneFlags& vInOutPaneShown, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	ZoneScoped;

	if (vInOutPaneShown & paneFlag)
	{
		static ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_MenuBar;
		if (ImGui::Begin<PaneFlags>(paneName.c_str(),
			&vInOutPaneShown , paneFlag, flags))
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

	return false;
}

bool ScenePane::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas)
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

	return false;
}

bool ScenePane::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	return false;
}

bool ScenePane::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;
	UNUSED(vCurrentFrame);
	UNUSED(vRect);
	ImGui::SetCurrentContext(vContextPtr);
	UNUSED(vUserDatas);
	return false;
}
