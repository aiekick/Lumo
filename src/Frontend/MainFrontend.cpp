/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

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

#include "MainFrontend.h"

#include <Headers/LumoBuild.h>

#include <Res/sdfmFont.h>
#include <Res/sdfmToolbarFont.cpp>

#include <ImGuiPack.h>

#include <Graph/Manager/NodeManager.h>
#include <Scene/Manager/SceneManager.h>

#include <Backend/MainBackend.h>
#include <Project/ProjectFile.h>
#include <Plugins/PluginManager.h>

#include <Gaia/gaia.h>

#include <Panes/DebugPane.h>
#include <Panes/GraphPane.h>
#include <Panes/View2DPane.h>
#include <Panes/View3DPane.h>
#include <Panes/TuningPane.h>
#include <Panes/ConsolePane.h>
#include <Panes/AnimatePane.h>
#include <Panes/ProfilerPane.h>
#include <Panes/SceneTreePane.h>
#include <Panes/SceneViewPane.h>

// panes
#define DEBUG_PANE_ICON ICON_SDFM_BUG
#define SCENE_PANE_ICON ICON_SDFM_FORMAT_LIST_BULLETED_TYPE
#define TUNING_PANE_ICON ICON_SDFM_TUNE
#define CONSOLE_PANE_ICON ICON_SDFMT_COMMENT_TEXT_MULTIPLE

// features
#define GRID_ICON ICON_SDFMT_GRID
#define MOUSE_ICON ICON_SDFMT_MOUSE
#define CAMERA_ICON ICON_SDFMT_CAMCORDER
#define GIZMO_ICON ICON_SDFMT_AXIS_ARROW

static const float& font_scale_ratio = 1.0f / 3.5f;

using namespace std::placeholders;

//////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

MainFrontendPtr MainFrontend::create() {
    auto res = std::make_shared<MainFrontend>();
    res->m_This = res;
    if (!res->init()) {
        res.reset();
    }
    return res;
}
bool MainFrontend::sCentralWindowHovered = false;

//////////////////////////////////////////////////////////////////////////////////
//// PUBLIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

MainFrontend::~MainFrontend() {
}

bool MainFrontend::init() {
    ImGui::CustomStyle::Init();

    m_build_themes();

    LayoutManager::Instance()->Init("Layouts", "Default Layout");

    LayoutManager::Instance()->SetPaneDisposalSize(PaneDisposal::LEFT, 300.0f);
    LayoutManager::Instance()->SetPaneDisposalSize(PaneDisposal::RIGHT, 300.0f);
    LayoutManager::Instance()->SetPaneDisposalSize(PaneDisposal::BOTTOM, 300.0f);

    LayoutManager::Instance()->AddPane(DebugPane::Instance(), "Debug Pane", "", PaneDisposal::CENTRAL, false, false);
    LayoutManager::Instance()->AddPane(SceneTreePane::Instance(), "Scene Tree Pane", "", PaneDisposal::RIGHT, false, false);
    LayoutManager::Instance()->AddPane(SceneViewPane::Instance(), "Scene View Pane", "", PaneDisposal::LEFT, true, true);
    LayoutManager::Instance()->AddPane(GraphPane::Instance(), "Graph Pane", "", PaneDisposal::CENTRAL, true, false);
    LayoutManager::Instance()->AddPane(TuningPane::Instance(), "Tuning Pane", "", PaneDisposal::RIGHT, true, false);
    LayoutManager::Instance()->AddPane(View3DPane::Instance(), "View3D Pane", "", PaneDisposal::LEFT, true, true);
    LayoutManager::Instance()->AddPane(View2DPane::Instance(), "View2D Pane", "", PaneDisposal::LEFT, false, false);
    LayoutManager::Instance()->AddPane(ConsolePane::Instance(), "Console Pane", "", PaneDisposal::BOTTOM, false, false);
    LayoutManager::Instance()->AddPane(ProfilerPane::Instance(), "Profiler Pane", "", PaneDisposal::BOTTOM, false, false);
    // LayoutManager::Instance()->AddPane(AnimatePane::Instance(), "Animate Pane", "", PaneDisposal::BOTTOM, false, false);

    // InitPänes is done in m_InitPanes, because a specific order is needed

    return m_build();
}

void MainFrontend::unit() {
    LayoutManager::Instance()->UnitPanes();
    auto pluginPanes = PluginManager::Instance()->GetPluginsPanes();
    for (auto& pluginPane : pluginPanes) {
        if (!pluginPane.paneWeak.expired()) {
            LayoutManager::Instance()->RemovePane(pluginPane.paneName);
        }
    }
}

bool MainFrontend::isValid() const {
    return false;
}

bool MainFrontend::isThereAnError() const {
    return false;
}

void MainFrontend::setBackend(const MainBackendWeak& vBackend) {
    m_MainBackend = vBackend;
}

void MainFrontend::Display(const uint32_t& vCurrentFrame) {
    const auto context_ptr = ImGui::GetCurrentContext();
    if (context_ptr != nullptr) {
        const auto& io = ImGui::GetIO();

        MainFrontend::sCentralWindowHovered = (ImGui::GetCurrentContext()->HoveredWindow == nullptr);

        // m_drawLeftButtonBar();
        m_drawMainMenuBar();
        m_drawMainStatusBar();

        if (LayoutManager::Instance()->BeginDockSpace(ImGuiDockNodeFlags_PassthruCentralNode)) {
            if (MainBackend::Instance()->GetBackendDatasRef().canWeTuneGizmo) {
                const auto viewport = ImGui::GetMainViewport();
                ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);
                ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);
                ImRect rc(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);
                DrawOverlays(vCurrentFrame, rc, context_ptr, {});
            }
            LayoutManager::Instance()->EndDockSpace();
        }

        if (LayoutManager::Instance()->DrawPanes(vCurrentFrame, context_ptr, {})) {
            ProjectFile::Instance()->SetProjectChange();
        }

        DrawDialogsAndPopups(vCurrentFrame, MainBackend::Instance()->GetDisplaySize(), context_ptr, {});

        ImGuiThemeHelper::Instance()->Draw();
        LayoutManager::Instance()->InitAfterFirstDisplay(io.DisplaySize);

        // on update la mouse apres l'affichage complet
        m_CanMouseAffectRendering();
    }
}

void MainFrontend::m_CanMouseAffectRendering() {
    const auto& CanUpdateMouse = MainFrontend::sCentralWindowHovered &&               // only update mouse when hovering the central node
                                 !ImGuizmo::IsUsing() &&                              // only update mouse if no gizmo is moving
                                 ImGui::GetTopMostAndVisiblePopupModal() == nullptr;  // only update mouse if no modal dialog is opened
    MainBackend::Instance()->UpdateMouseDatas(CanUpdateMouse);
}

void MainFrontend::setSize(const ct::ivec2& vSize) {
    m_size = vSize;
}

const ct::ivec2& MainFrontend::getSize() const {
    return m_size;
}

bool MainFrontend::resize(const ct::ivec2& /*vNewSize*/) {
    return false;
}

bool MainFrontend::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    bool res = false;
    return res;
}

bool MainFrontend::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    bool res = false;
    return res;
}

bool MainFrontend::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, void* vUserDatas) {
    m_ActionSystem.RunActions();
    LayoutManager::Instance()->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
    if (m_ShowImGui) {
        ImGui::ShowDemoWindow(&m_ShowImGui);
    }
    if (m_ShowMetric) {
        ImGui::ShowMetricsWindow(&m_ShowMetric);
    }
    return false;
}

void MainFrontend::OpenAboutDialog() {
    m_ShowAboutDialog = true;
}

void MainFrontend::m_drawMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu(" Project")) {
            if (ImGui::MenuItem(" New")) {
                Action_Menu_NewProject();
            }

            if (ImGui::MenuItem(" Open")) {
                Action_Menu_OpenProject();
            }

            if (ProjectFile::Instance()->IsProjectLoaded()) {
                ImGui::Separator();

                if (ImGui::MenuItem(" Re Open")) {
                    Action_Menu_ReOpenProject();
                }

                ImGui::Separator();

                if (ImGui::MenuItem(" Save")) {
                    Action_Menu_SaveProject();
                }

                if (ImGui::MenuItem(" Save As")) {
                    Action_Menu_SaveAsProject();
                }

                ImGui::Separator();

                if (ImGui::MenuItem(" Close")) {
                    Action_Menu_CloseProject();
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem(" About")) {
                OpenAboutDialog();
            }

            ImGui::EndMenu();
        }

        ImGui::Spacing();

        const auto& io = ImGui::GetIO();
        LayoutManager::Instance()->DisplayMenu(io.DisplaySize);

        ImGui::Spacing();

        if (ImGui::BeginMenu("Settings")) {
            if (ImGui::BeginMenu("Styles")) {
                ImGuiThemeHelper::Instance()->DrawMenu();

                ImGui::Separator();

                ImGui::MenuItem("Show ImGui", "", &m_ShowImGui);
                ImGui::MenuItem("Show ImGui Metric/Debug", "", &m_ShowMetric);

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ProjectFile::Instance()->IsThereAnyProjectChanges()) {
            ImGui::Spacing(200.0f);

            if (ImGui::MenuItem(" Save")) {
                Action_Menu_SaveProject();
            }
        }

        // ImGui Infos
        const auto label = ct::toStr("Dear ImGui %s (Docking)", ImGui::GetVersion());
        const auto size = ImGui::CalcTextSize(label.c_str());
        ImGui::Spacing(ImGui::GetContentRegionAvail().x - size.x - ImGui::GetStyle().FramePadding.x * 2.0f);
        ImGui::Text("%s", label.c_str());

        // MainFrontend::sAnyWindowsHovered |= ImGui::IsWindowHovered();

        ImGui::EndMainMenuBar();
    }
}

void MainFrontend::m_drawMainStatusBar() {
    if (ImGui::BeginMainStatusBar()) {
        Messaging::Instance()->DrawStatusBar();

        //  ImGui Infos
        const auto& io = ImGui::GetIO();
        const auto fps = ct::toStr("%.1f ms/frame (%.1f fps)", 1000.0f / io.Framerate, io.Framerate);
        const auto size = ImGui::CalcTextSize(fps.c_str());
        ImGui::Spacing(ImGui::GetContentRegionAvail().x - size.x - ImGui::GetStyle().FramePadding.x * 2.0f);
        ImGui::Text("%s", fps.c_str());

        // MainFrontend::sAnyWindowsHovered |= ImGui::IsWindowHovered();

        ImGui::EndMainStatusBar();
    }
}

void MainFrontend::m_drawLeftButtonBar() {
    // auto NeedOneFrameUpdate = false;

    const float& toolbar_width = 35.0f;

    static ImGui::ImCoolBarConfig _config;
    _config.normal_size = 30.0f;
    _config.hovered_size = 75.0f;
    _config.anchor = ImVec2(0.0f, 0.5f);
    _config.anim_step = 0.2f;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
    bool left_bar_opened = ImGui::BeginLeftToolBar(toolbar_width);
    ImGui::PopStyleVar();

    if (left_bar_opened) {
        ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 2));
        bool _opened = ImGui::BeginCoolBar("##coolbar", ImCoolBarFlags_Vertical, _config);
        auto window = ImGui::GetCurrentWindow();
        if (window) {
            // correct the rect of the window. maybe a bug on imgui..
            // the workrect can cause issue when click a timeline
            // channel close button when close to the toolbar
            // this thing correct the issue
            const auto& rc = window->Rect();
            window->WorkRect = rc;
            window->OuterRectClipped = rc;
            window->InnerRect = rc;
            window->InnerClipRect = rc;
            window->ParentWorkRect = rc;
            window->ClipRect = rc;
            window->ContentRegionRect = rc;
        }
        ImGui::PopStyleVar(2);

        if (_opened) {
#ifdef _DEBUG
            if (ImGui::CoolBarItem()) {  // Debug Pane
                const auto aw = ImGui::GetCoolBarItemWidth();
                m_ToolbarFontPtr->Scale = font_scale_ratio * ImGui::GetCoolBarItemScale();
                ImGui::RadioButtonLabeled_BitWize<PaneFlags>(ImVec2(aw, aw), DEBUG_PANE_ICON "##Debug", nullptr,
                    &LayoutManager::Instance()->pane_Shown, DebugPane::Instance()->paneFlag, false, true, 0, false, m_ToolbarFontPtr);
            }
#endif  // _DEBUG

            /*if (ImGui::CoolBarItem()) {  // show/hide Scene Pane
                const auto aw           = ImGui::GetCoolBarItemWidth();
                m_ToolbarFontPtr->Scale = font_scale_ratio * ImGui::GetCoolBarItemScale();
                ImGui::RadioButtonLabeled_BitWize<PaneFlags>(ImVec2(aw, aw),
                                                             SCENE_PANE_ICON "##Scene",
                                                             nullptr,
                                                             &LayoutManager::Instance()->pane_Shown,
                                                             ConfigPane::Instance()->paneFlag,
                                                             false,
                                                             true,
                                                             0,
                                                             false,
                                                             m_ToolbarFontPtr);
            }*/

            if (ImGui::CoolBarItem()) {  // show/hide Tuning Pane
                const auto aw = ImGui::GetCoolBarItemWidth();
                m_ToolbarFontPtr->Scale = font_scale_ratio * ImGui::GetCoolBarItemScale();
                ImGui::RadioButtonLabeled_BitWize<PaneFlags>(ImVec2(aw, aw), TUNING_PANE_ICON "##Tuning", nullptr,
                    &LayoutManager::Instance()->pane_Shown, TuningPane::Instance()->paneFlag, false, true, 0, false, m_ToolbarFontPtr);
            }

            if (ImGui::CoolBarItem()) {  // show/hide Grid Renderer
                const auto aw = ImGui::GetCoolBarItemWidth();
                m_ToolbarFontPtr->Scale = font_scale_ratio * ImGui::GetCoolBarItemScale();
                ImGui::RadioButtonLabeled(ImVec2(aw, aw), GRID_ICON "##Grid", nullptr, &MainBackend::Instance()->GetBackendDatasRef().canWeShowGrid,
                    false, m_ToolbarFontPtr);
            }

            if (ImGui::CoolBarItem()) {  // enable/disable Camera
                const auto aw = ImGui::GetCoolBarItemWidth();
                m_ToolbarFontPtr->Scale = font_scale_ratio * ImGui::GetCoolBarItemScale();
                ImGui::RadioButtonLabeled(ImVec2(aw, aw), CAMERA_ICON "##Camera", nullptr,
                    &MainBackend::Instance()->GetBackendDatasRef().canWeTuneCamera, false, m_ToolbarFontPtr);
            }

            if (ImGui::CoolBarItem()) {  // enable/disable Mouse
                const auto aw = ImGui::GetCoolBarItemWidth();
                m_ToolbarFontPtr->Scale = font_scale_ratio * ImGui::GetCoolBarItemScale();
                ImGui::RadioButtonLabeled(ImVec2(aw, aw), MOUSE_ICON "##Mouse", nullptr,
                    &MainBackend::Instance()->GetBackendDatasRef().canWeTuneMouse, false, m_ToolbarFontPtr);
            }

            if (ImGui::CoolBarItem()) {  // enable/disable Gizmo
                const auto aw = ImGui::GetCoolBarItemWidth();
                m_ToolbarFontPtr->Scale = font_scale_ratio * ImGui::GetCoolBarItemScale();
                ImGui::RadioButtonLabeled(ImVec2(aw, aw), GIZMO_ICON "##Gizmo", nullptr,
                    &MainBackend::Instance()->GetBackendDatasRef().canWeTuneGizmo, false, m_ToolbarFontPtr);
            }

            ImGui::EndCoolBar();
        }
        ImGui::EndLeftToolBar();
    }
}
///////////////////////////////////////////////////////
//// SAVE DIALOG WHEN UN SAVED CHANGES ////////////////
///////////////////////////////////////////////////////

void MainFrontend::OpenUnSavedDialog() {
    // force close dialog if any dialog is opened
    ImGuiFileDialog::Instance()->Close();

    m_SaveDialogIfRequired = true;
}
void MainFrontend::CloseUnSavedDialog() {
    m_SaveDialogIfRequired = false;
}

bool MainFrontend::ShowUnSavedDialog() {
    bool res = false;

    if (m_SaveDialogIfRequired) {
        if (ProjectFile::Instance()->IsProjectLoaded()) {
            if (ProjectFile::Instance()->IsThereAnyProjectChanges()) {
                /*
                Unsaved dialog behavior :
                -	save :
                    -	insert action : save project
                -	save as :
                    -	insert action : save as project
                -	continue without saving :
                    -	quit unsaved dialog
                -	cancel :
                    -	clear actions
                */

                ImGui::CloseCurrentPopup();
                ImGui::OpenPopup("Do you want to save before ?");
                if (ImGui::BeginPopupModal("Do you want to save before ?", (bool*)0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking)) {
                    if (ImGui::ContrastedButton("Save")) {
                        res = Action_UnSavedDialog_SaveProject();
                    }
                    ImGui::SameLine();
                    if (ImGui::ContrastedButton("Save As")) {
                        Action_UnSavedDialog_SaveAsProject();
                    }

                    if (ImGui::ContrastedButton("Continue without saving")) {
                        res = true;  // quit the action
                    }
                    ImGui::SameLine();
                    if (ImGui::ContrastedButton("Cancel")) {
                        Action_Cancel();
                    }

                    ImGui::EndPopup();
                }
            }
        }

        return res;  // quit if true, else continue on the next frame
    }

    return true;  // quit the action
}

///////////////////////////////////////////////////////
//// ACTIONS //////////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrontend::Action_Menu_NewProject() {
    /*
    new project :
    -	unsaved :
        -	add action : show unsaved dialog
        -	add action : open dialog for new project file name
    -	saved :
        -	add action : open dialog for new project file name
    */
    m_ActionSystem.Clear();
    Action_OpenUnSavedDialog_IfNeeded();
    m_ActionSystem.Add([this]() {
        CloseUnSavedDialog();
        ImGuiFileDialog::Instance()->OpenDialog("NewProjectDlg", "New Project File", ".lum", "", 1, nullptr, ImGuiFileDialogFlags_Modal);
        return true;
    });
    m_ActionSystem.Add([this]() { return Display_NewProjectDialog(); });
}

void MainFrontend::Action_Menu_OpenProject() {
    /*
    open project :
    -	unsaved :
        -	add action : show unsaved dialog
        -	add action : open project
    -	saved :
        -	add action : open project
    */
    m_ActionSystem.Clear();
    Action_OpenUnSavedDialog_IfNeeded();
    m_ActionSystem.Add([this]() {
        CloseUnSavedDialog();
        ImGuiFileDialog::Instance()->OpenDialog("OpenProjectDlg", "Open Project File", ".lum", "", 1, nullptr, ImGuiFileDialogFlags_Modal);
        return true;
    });
    m_ActionSystem.Add([this]() { return Display_OpenProjectDialog(); });
}

void MainFrontend::Action_Menu_ReOpenProject() {
    /*
    re open project :
    -	unsaved :
        -	add action : show unsaved dialog
        -	add action : re open project
    -	saved :
        -	add action : re open project
    */
    m_ActionSystem.Clear();
    Action_OpenUnSavedDialog_IfNeeded();
    m_ActionSystem.Add([this]() {
        MainBackend::Instance()->NeedToLoadProject(ProjectFile::Instance()->GetProjectFilepathName());
        return true;
    });
}

void MainFrontend::Action_Menu_SaveProject() {
    /*
    save project :
    -	never saved :
        -	add action : save as project
    -	saved in a file beofre :
        -	add action : save project
    */
    m_ActionSystem.Clear();
    m_ActionSystem.Add([this]() {
        if (!MainBackend::Instance()->SaveProject()) {
            CloseUnSavedDialog();
            ImGuiFileDialog::Instance()->OpenDialog("SaveProjectDlg", "Save Project File", ".lum", "", 1, nullptr, ImGuiFileDialogFlags_Modal);
        }
        return true;
    });
    m_ActionSystem.Add([this]() { return Display_SaveProjectDialog(); });
}

void MainFrontend::Action_Menu_SaveAsProject() {
    /*
    save as project :
    -	add action : save as project
    */
    m_ActionSystem.Clear();
    m_ActionSystem.Add([this]() {
        CloseUnSavedDialog();
        ImGuiFileDialog::Instance()->OpenDialog(
            "SaveProjectDlg", "Save Project File", ".lum", "", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal);
        return true;
    });
    m_ActionSystem.Add([this]() { return Display_SaveProjectDialog(); });
}

void MainFrontend::Action_Menu_CloseProject() {
    /*
    Close project :
    -	unsaved :
        -	add action : show unsaved dialog
        -	add action : Close project
    -	saved :
        -	add action : Close project
    */
    m_ActionSystem.Clear();
    Action_OpenUnSavedDialog_IfNeeded();
    m_ActionSystem.Add([this]() {
        MainBackend::Instance()->NeedToCloseProject();
        return true;
    });
}

void MainFrontend::Action_Window_CloseApp() {
    if (MainBackend::Instance()->IsNeedToCloseApp())
        return;  // block next call to close app when running
    /*
    Close app :
    -	unsaved :
        -	add action : show unsaved dialog
        -	add action : Close app
    -	saved :
        -	add action : Close app
    */

    m_ActionSystem.Clear();
    Action_OpenUnSavedDialog_IfNeeded();
    m_ActionSystem.Add([this]() {
        MainBackend::Instance()->CloseApp();
        return true;
    });
}

void MainFrontend::Action_OpenUnSavedDialog_IfNeeded() {
    if (ProjectFile::Instance()->IsProjectLoaded() && ProjectFile::Instance()->IsThereAnyProjectChanges()) {
        OpenUnSavedDialog();
        m_ActionSystem.Add([this]() { return ShowUnSavedDialog(); });
    }
}

void MainFrontend::Action_Cancel() {
    /*
    -	cancel :
        -	clear actions
    */
    CloseUnSavedDialog();
    m_ActionSystem.Clear();
    MainBackend::Instance()->NeedToCloseApp(false);
}

bool MainFrontend::Action_UnSavedDialog_SaveProject() {
    bool res = MainBackend::Instance()->SaveProject();
    if (!res) {
        m_ActionSystem.Insert([this]() { return Display_SaveProjectDialog(); });
        m_ActionSystem.Insert([this]() {
            CloseUnSavedDialog();
            ImGuiFileDialog::Instance()->OpenDialog(
                "SaveProjectDlg", "Save Project File", ".lum", ".", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal);
            return true;
        });
    }
    return res;
}

void MainFrontend::Action_UnSavedDialog_SaveAsProject() {
    m_ActionSystem.Insert([this]() { return Display_SaveProjectDialog(); });
    m_ActionSystem.Insert([this]() {
        CloseUnSavedDialog();
        ImGuiFileDialog::Instance()->OpenDialog(
            "SaveProjectDlg", "Save Project File", ".lum", ".", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal);
        return true;
    });
}

void MainFrontend::Action_UnSavedDialog_Cancel() {
    Action_Cancel();
}

///////////////////////////////////////////////////////
//// DIALOG FUNCS /////////////////////////////////////
///////////////////////////////////////////////////////

bool MainFrontend::Display_NewProjectDialog() {
    // need to return false to continue to be displayed next frame

    ImVec2 min = m_DisplaySize * 0.5f;
    ImVec2 max = m_DisplaySize;

    if (ImGuiFileDialog::Instance()->Display("NewProjectDlg", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            CloseUnSavedDialog();
            auto file = ImGuiFileDialog::Instance()->GetFilePathName();
            MainBackend::Instance()->NeedToNewProject(file);
        } else  // cancel
        {
            Action_Cancel();  // we interrupts all actions
        }

        ImGuiFileDialog::Instance()->Close();

        return true;
    }

    return false;
}

bool MainFrontend::Display_OpenProjectDialog() {
    // need to return false to continue to be displayed next frame

    ImVec2 min = m_DisplaySize * 0.5f;
    ImVec2 max = m_DisplaySize;

    if (ImGuiFileDialog::Instance()->Display("OpenProjectDlg", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            CloseUnSavedDialog();
            MainBackend::Instance()->NeedToLoadProject(ImGuiFileDialog::Instance()->GetFilePathName());
        } else  // cancel
        {
            Action_Cancel();  // we interrupts all actions
        }

        ImGuiFileDialog::Instance()->Close();

        return true;
    }

    return false;
}

bool MainFrontend::Display_SaveProjectDialog() {
    // need to return false to continue to be displayed next frame

    ImVec2 min = m_DisplaySize * 0.5f;
    ImVec2 max = m_DisplaySize;

    if (ImGuiFileDialog::Instance()->Display("SaveProjectDlg", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            CloseUnSavedDialog();
            MainBackend::Instance()->SaveAsProject(ImGuiFileDialog::Instance()->GetFilePathName());
        } else  // cancel
        {
            Action_Cancel();  // we interrupts all actions
        }

        ImGuiFileDialog::Instance()->Close();

        return true;
    }

    return false;
}

///////////////////////////////////////////////////////
//// APP CLOSING //////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrontend::IWantToCloseTheApp() {
    Action_Window_CloseApp();
}

///////////////////////////////////////////////////////
//// DROP /////////////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrontend::JustDropFiles(int count, const char** paths) {
    assert(0);

    std::map<std::string, std::string> dicoFont;
    std::string prj;

    for (int i = 0; i < count; ++i) {
        // file
        auto f = std::string(paths[i]);

        // lower case
        auto f_opt = f;
        for (auto& c : f_opt)
            c = (char)std::tolower((int)c);

        // well known extention
        if (f_opt.find(".ttf") != std::string::npos     // truetype (.ttf)
            || f_opt.find(".otf") != std::string::npos  // opentype (.otf)
            //||	f_opt.find(".ttc") != std::string::npos		// ttf/otf collection for futur (.ttc)
        ) {
            dicoFont[f] = f;
        }
        if (f_opt.find(".lum") != std::string::npos) {
            prj = f;
        }
    }

    // priority to project file
    if (!prj.empty()) {
        MainBackend::Instance()->NeedToLoadProject(prj);
    }
}

//////////////////////////////////////////////////////////////////////////////////
//// PRIVATE /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool MainFrontend::m_build() {
    // toolbar
    static ImFontConfig icons_config3;
    icons_config3.MergeMode = false;
    icons_config3.PixelSnapH = true;
    static const ImWchar icons_ranges3[] = {ICON_MIN_SDFMT, ICON_MAX_SDFMT, 0};
    const float& font_size = 20.0f / font_scale_ratio;
    m_ToolbarFontPtr =
        ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_SDFMT, font_size, &icons_config3, icons_ranges3);
    if (m_ToolbarFontPtr != nullptr) {
        m_ToolbarFontPtr->Scale = font_scale_ratio;
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string MainFrontend::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    std::string str;

    str += ImGuiThemeHelper::Instance()->getXml(vOffset);
    str += LayoutManager::Instance()->getXml(vOffset, "app");
    str += vOffset + "<bookmarks>" + ImGuiFileDialog::Instance()->SerializeBookmarks() + "</bookmarks>\n";
    str += vOffset + "<showaboutdialog>" + (m_ShowAboutDialog ? "true" : "false") + "</showaboutdialog>\n";
    str += vOffset + "<showimgui>" + (m_ShowImGui ? "true" : "false") + "</showimgui>\n";
    str += vOffset + "<showmetric>" + (m_ShowMetric ? "true" : "false") + "</showmetric>\n";

    return str;
}

bool MainFrontend::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != 0)
        strParentName = vParent->Value();

    ImGuiThemeHelper::Instance()->setFromXml(vElem, vParent);
    LayoutManager::Instance()->setFromXml(vElem, vParent, "app");

    if (strName == "bookmarks") {
        ImGuiFileDialog::Instance()->DeserializeBookmarks(strValue);
    } else if (strName == "showaboutdialog") {
        m_ShowAboutDialog = ct::ivariant(strValue).GetB();
    } else if (strName == "showimgui") {
        m_ShowImGui = ct::ivariant(strValue).GetB();
    } else if (strName == "showmetric") {
        m_ShowMetric = ct::ivariant(strValue).GetB();
    }

    return true;
}
