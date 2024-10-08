// NoodlesPlate Copyright (C) 2017-2023 Stephane Cuillerdier aka Aiekick
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "ProfilerPane.h"

#include "AnimatePane.h"
#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <imgui_internal.h>
#include <Frontend/MainFrontend.h>
#include <cinttypes>  // printf zu

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

ProfilerPane::ProfilerPane() = default;
ProfilerPane::~ProfilerPane() {
    Unit();
}

bool ProfilerPane::Init() {
    /*iagp::InAppGpuProfiler::Instance()->SetImGuiBeginFunctor([this](const char* vLabel, bool* pOpen, ImGuiWindowFlags vFlags) -> bool {
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;
        if (ImGui::Begin<PaneFlags>(paneName.c_str(), &m_InOutPaneShown, paneFlag, flags, pOpen)) {
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
            auto win = ImGui::GetCurrentWindowRead();
            if (win->Viewport->Idx != 0)
                flags |= ImGuiWindowFlags_NoResize;  // | ImGuiWindowFlags_NoTitleBar;
            else
                flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;
#endif
            return true;
        }
        return false;
    });*/
    return true;
}

void ProfilerPane::Unit() {
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool ProfilerPane::DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    UNUSED(vCurrentFrame);
    ImGui::SetCurrentContext(vContextPtr);
    UNUSED(vUserDatas);
    bool change = false;

    GaiApi::vkProfiler::Instance()->isActiveRef() = false;

    if (vOpened && *vOpened) {
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;
        if (ImGui::Begin(GetName().c_str(), vOpened, flags)) {
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
            auto win = ImGui::GetCurrentWindowRead();
            if (win->Viewport->Idx != 0)
                flags |= ImGuiWindowFlags_NoResize;  // | ImGuiWindowFlags_NoTitleBar;
            else
                flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;
#endif
            // draw iagp frame
            GaiApi::vkProfiler::Instance()->isActiveRef() = true;
            GaiApi::vkProfiler::Instance()->DrawFlamGraphNoWin();
        }

        // MainFrame::sAnyWindowsHovered |= ImGui::IsWindowHovered();

        ImGui::End();

        GaiApi::vkProfiler::Instance()->DrawFlamGraphChilds();
        GaiApi::vkProfiler::Instance()->DrawDetails();
    }

    return change;
}

bool ProfilerPane::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    UNUSED(vCurrentFrame);
    UNUSED(vRect);
    ImGui::SetCurrentContext(vContextPtr);
    UNUSED(vUserDatas);
    return false;
}

bool ProfilerPane::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    UNUSED(vCurrentFrame);
    UNUSED(vMaxRect);
    ImGui::SetCurrentContext(vContextPtr);
    UNUSED(vUserDatas);
    return false;
}

bool ProfilerPane::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    UNUSED(vCurrentFrame);
    ImGui::SetCurrentContext(vContextPtr);
    UNUSED(vUserDatas);
    return false;
}
