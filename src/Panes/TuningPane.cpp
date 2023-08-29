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

#include <ImWidgets.h>
#include <ImGuiFileDialog.h>
#include <Gaia/Gui/VulkanImGuiRenderer.h>
#include <Project/ProjectFile.h>
#include <Frontend/MainFrontend.h>
#include <imgui_internal.h>
#include <LumoBackend/Graph/Base/BaseNode.h>

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>

#include <LumoBackend/Systems/CommonSystem.h>

#include <cinttypes> // printf zu

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

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

bool TuningPane::DrawPanes(const uint32_t& vCurrentFrame, PaneFlags& vInOutPaneShown, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	ZoneScoped;

	bool change = false;

	if (vInOutPaneShown & paneFlag)
	{
		static ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_MenuBar;
		if (ImGui::Begin<PaneFlags>(m_PaneName.c_str(),
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
				change = DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
			}
		}

		ImGui::End();
	}

	return change;
}

///////////////////////////////////////////////////////////////////////////////////
//// DIALOGS //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool TuningPane::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	ZoneScoped;

	return false;
}

bool TuningPane::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	bool change = false;

	change |= CommonSystem::Instance()->DrawImGui();

	auto ptr = GetParentNode().lock();
	if (ptr)
	{
		change |= ptr->DrawWidgets(vCurrentFrame, ImGui::GetCurrentContext(), vUserDatas);
	}

	return change;
}

bool TuningPane::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;
	UNUSED(vCurrentFrame);
	UNUSED(vRect);
	ImGui::SetCurrentContext(vContextPtr);
	UNUSED(vUserDatas);
	return false;
}

///////////////////////////////////////////////////////////////////////////////////
//// SELECTOR /////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void TuningPane::Select(BaseNodeWeak vObjet)
{
	SetParentNode(vObjet);
}