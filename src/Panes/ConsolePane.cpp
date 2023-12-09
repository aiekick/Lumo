// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "ConsolePane.h"
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

static int SourcePane_WidgetId = 0;

ConsolePane::ConsolePane() = default;
ConsolePane::~ConsolePane() {
    Unit();
}

bool ConsolePane::Init() {
    return true;
}

void ConsolePane::Unit() {
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool ConsolePane::DrawPanes(const uint32_t& vCurrentFrame, PaneFlags& vInOutPaneShown, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    UNUSED(vCurrentFrame);
    ImGui::SetCurrentContext(vContextPtr);
    UNUSED(vUserDatas);
    bool change = false;

    if (vInOutPaneShown & paneFlag) {
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;
        if (ImGui::Begin<PaneFlags>(paneName.c_str(), &vInOutPaneShown, paneFlag, flags)) {
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
            auto win = ImGui::GetCurrentWindowRead();
            if (win->Viewport->Idx != 0)
                flags |= ImGuiWindowFlags_NoResize;  // | ImGuiWindowFlags_NoTitleBar;
            else
                flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;
#endif
            if (ImGui::BeginMenuBar()) {
#ifdef WIN32
                if (ImGui::MenuItem("Console", "", MainBackend::Instance()->GetConsoleVisibility())) {
                    MainBackend::Instance()->SwitchConsoleVisibility();
                }
#endif
                ImGui::EndMenuBar();
            }

            Messaging::Instance()->DrawConsolePane();
        }

        ImGui::End();
    }

    return change;
}

bool ConsolePane::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    UNUSED(vCurrentFrame);
    UNUSED(vRect);
    ImGui::SetCurrentContext(vContextPtr);
    UNUSED(vUserDatas);
    return false;
}

bool ConsolePane::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    UNUSED(vCurrentFrame);
    UNUSED(vMaxSize);
    ImGui::SetCurrentContext(vContextPtr);
    UNUSED(vUserDatas);
    return false;
}

bool ConsolePane::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    UNUSED(vCurrentFrame);
    ImGui::SetCurrentContext(vContextPtr);
    UNUSED(vUserDatas);
    return false;
}
