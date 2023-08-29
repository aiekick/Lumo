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

#include "MainFrontend.h"

#include <Headers/LumoBuild.h>

#include <Res/sdfmFont.h>
#include <Res/sdfmToolbarFont.cpp>

#include <imgui_internal.h>
#include <ImGuiFileDialog.h>

#include <Backend/MainBackend.h>
#include <Graph/Manager/NodeManager.h>
#include <Plugins/PluginManager.h>
#include <Project/ProjectFile.h>

#include <Gaia/gaia.h>

#include <Panes/DebugPane.h>
#include <Panes/View2DPane.h>
#include <Panes/View3DPane.h>
#include <Panes/ScenePane.h>
#include <Panes/TuningPane.h>
#include <Panes/GraphPane.h>
#include <Panes/ConsolePane.h>

#include <ImWidgets.h>

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

// messaging
#define MESSAGING_CODE_INFOS 0
#define MESSAGING_LABEL_INFOS ICON_SDFMT_INFORMATION
#define MESSAGING_CODE_WARNINGS 1
#define MESSAGING_LABEL_WARNINGS ICON_SDFMT_BELL_ALERT
#define MESSAGING_CODE_ERRORS 2
#define MESSAGING_LABEL_ERRORS ICON_SDFMT_CLOSE_CIRCLE
#define MESSAGING_LABEL_VKLAYER ICON_SDFMT_LAYERS
#define MESSAGING_LABEL_DEBUG ICON_SDFM_BUG

static const float& font_scale_ratio = 1.0f / 3.5f;

//////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

MainFrontendPtr MainFrontend::create() {
    auto res    = std::make_shared<MainFrontend>();
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

MainFrontend::~MainFrontend() { unit(); }

bool MainFrontend::init() {
    Messaging::Instance()->AddCategory(
        MESSAGING_CODE_INFOS, "Infos(s)", MESSAGING_LABEL_INFOS, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
    Messaging::Instance()->AddCategory(
        MESSAGING_CODE_WARNINGS, "Warnings(s)", MESSAGING_LABEL_WARNINGS, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));
    Messaging::Instance()->AddCategory(
        MESSAGING_CODE_ERRORS, "Errors(s)", MESSAGING_LABEL_ERRORS, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
    Messaging::Instance()->AddCategory(
        MESSAGING_TYPE_VKLAYER, "Vk Layer(s)", MESSAGING_LABEL_VKLAYER, ImVec4(0.8f, 0.0f, 0.4f, 1.0f));
    Messaging::Instance()->AddCategory(
        MESSAGING_TYPE_DEBUG, "Debug(s)", MESSAGING_LABEL_DEBUG, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));
    
    Logger::sStandardLogFunction = [this](const int& vType, const std::string& vMessage) {
        MessageData msg_datas;
        const auto& type = vType;
        Messaging::Instance()->AddMessage(vMessage, type, false, msg_datas, {});
    };

    ImGui::CustomStyle::Init();

    m_build_themes();

    LayoutManager::Instance()->Init("Layouts", "Default Layout");

    LayoutManager::Instance()->SetPaneDisposalSize(PaneDisposal::LEFT, 300.0f);
    LayoutManager::Instance()->SetPaneDisposalSize(PaneDisposal::RIGHT, 300.0f);
    LayoutManager::Instance()->SetPaneDisposalSize(PaneDisposal::BOTTOM, 300.0f);

    LayoutManager::Instance()->AddPane(DebugPane::Instance(), "Debug Pane", "", PaneDisposal::CENTRAL, false, false);
    LayoutManager::Instance()->AddPane(ScenePane::Instance(), "Scene Pane", "", PaneDisposal::RIGHT, false, false);
    LayoutManager::Instance()->AddPane(GraphPane::Instance(), "Graph Pane", "", PaneDisposal::CENTRAL, true, false);
    LayoutManager::Instance()->AddPane(TuningPane::Instance(), "Tuning Pane", "", PaneDisposal::RIGHT, true, false);
    LayoutManager::Instance()->AddPane(View3DPane::Instance(), "View3D Pane", "", PaneDisposal::LEFT, true, true);
    LayoutManager::Instance()->AddPane(View2DPane::Instance(), "View2D Pane", "", PaneDisposal::LEFT, false, false);
    LayoutManager::Instance()->AddPane(ConsolePane::Instance(), "Console Pane", "", PaneDisposal::BOTTOM, false, false);

    auto pluginPanes = PluginManager::Instance()->GetPluginsPanes();
    for (auto& pluginPane : pluginPanes) {
        if (!pluginPane.paneWeak.expired()) {
            LayoutManager::Instance()->AddPane(pluginPane.paneWeak, pluginPane.paneName, pluginPane.paneCategory,
                pluginPane.paneDisposal, pluginPane.isPaneOpenedDefault, pluginPane.isPaneFocusedDefault);
        }
    }

    LayoutManager::Instance()->InitPanes();

    //a faire apres InitPanes() sinon ConsolePane::Instance()->paneFlag vaudra 0 et changeras apres InitPanes()
    Messaging::sMessagePaneId = ConsolePane::Instance()->paneFlag;

    using namespace std::placeholders;
    BaseNode::sSelectCallback = std::bind(&MainFrontend::SelectNode, this, _1);
    BaseNode::sSelectForGraphOutputCallback = std::bind(&MainFrontend::SelectNodeForGraphOutput, this, _1, _2);
    NodeSlot::sSlotGraphOutputMouseLeftColor = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);
    NodeSlot::sSlotGraphOutputMouseMiddleColor = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);

    
#ifdef USE_THUMBNAILS
    ImGuiFileDialog::Instance()->SetCreateThumbnailCallback([this](IGFD_Thumbnail_Info* vThumbnail_Info) {
        if (vThumbnail_Info && vThumbnail_Info->isReadyToUpload && vThumbnail_Info->textureFileDatas) {
            m_VulkanCorePtr->getDevice().waitIdle();

            std::shared_ptr<FileDialogAsset> resPtr =
                std::shared_ptr<FileDialogAsset>(new FileDialogAsset, [](FileDialogAsset* obj) { delete obj; });

            if (resPtr) {
                resPtr->texturePtr = Texture2D::CreateFromMemory(m_VulkanCorePtr, vThumbnail_Info->textureFileDatas,
                    vThumbnail_Info->textureWidth, vThumbnail_Info->textureHeight, vThumbnail_Info->textureChannels);

                if (resPtr->texturePtr) {
                    auto imguiRendererPtr = m_VulkanImGuiOverlayPtr->GetImGuiRenderer().getValidShared();
                    if (imguiRendererPtr) {
                        resPtr->descriptorSet = imguiRendererPtr->CreateImGuiTexture(
                            (VkSampler)resPtr->texturePtr->m_DescriptorImageInfo.sampler,
                            (VkImageView)resPtr->texturePtr->m_DescriptorImageInfo.imageView,
                            (VkImageLayout)resPtr->texturePtr->m_DescriptorImageInfo.imageLayout);

                        vThumbnail_Info->userDatas = (void*)resPtr.get();

                        m_FileDialogAssets.push_back(resPtr);

                        vThumbnail_Info->textureID = (ImTextureID)&resPtr->descriptorSet;
                    }

                    delete[] vThumbnail_Info->textureFileDatas;
                    vThumbnail_Info->textureFileDatas = nullptr;

                    vThumbnail_Info->isReadyToUpload = false;
                    vThumbnail_Info->isReadyToDisplay = true;

                    m_VulkanCorePtr->getDevice().waitIdle();
                }
            }
        }
    });
    ImGuiFileDialog::Instance()->SetDestroyThumbnailCallback([this](IGFD_Thumbnail_Info* vThumbnail_Info) {
        if (vThumbnail_Info) {
            if (vThumbnail_Info->userDatas) {
                m_VulkanCorePtr->getDevice().waitIdle();

                auto assetPtr = (FileDialogAsset*)vThumbnail_Info->userDatas;
                if (assetPtr) {
                    assetPtr->texturePtr.reset();
                    assetPtr->descriptorSet = vk::DescriptorSet{};
                    auto imguiRendererPtr = m_VulkanImGuiOverlayPtr->GetImGuiRenderer().getValidShared();
                    if (imguiRendererPtr) {
                        imguiRendererPtr->DestroyImGuiTexture(&assetPtr->descriptorSet);
                    }
                }

                m_VulkanCorePtr->getDevice().waitIdle();
            }
        }
    });
#endif  // USE_THUMBNAILS

    return m_build();
}

void MainFrontend::unit() { LayoutManager::Instance()->UnitPanes(); }

void MainFrontend::SelectNode(const BaseNodeWeak& vNode)
{
    TuningPane::Instance()->Select(vNode);
    DebugPane::Instance()->Select(vNode);
}

void MainFrontend::SelectNodeForGraphOutput(const NodeSlotWeak& vSlot, const ImGuiMouseButton& vButton)
{
    if (NodeManager::Instance()->m_RootNodePtr)
    {
        if (vButton == ImGuiMouseButton_Left)
        {
            View3DPane::Instance()->SetOrUpdateOutput(vSlot);
        }
        else if (vButton == ImGuiMouseButton_Middle)
        {
            View2DPane::Instance()->SetOrUpdateOutput(vSlot);
        }
        else if (vButton == ImGuiMouseButton_Right)
        {

        }
    }
}

bool MainFrontend::isValid() const { return false; }

bool MainFrontend::isThereAnError() const { return false; }

void MainFrontend::setBackend(const MainBackendWeak& vBackend) { m_MainBackend = vBackend; }

void MainFrontend::Display(const uint32_t& vCurrentFrame) {
    const auto context_ptr = ImGui::GetCurrentContext();
    if (context_ptr != nullptr) {
        const auto& io = ImGui::GetIO();

        MainFrontend::sCentralWindowHovered = (ImGui::GetCurrentContext()->HoveredWindow == nullptr);

        m_drawLeftButtonBar();
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

        ImGui::SetPUSHID(LayoutManager::Instance()->DrawPanes(vCurrentFrame, context_ptr, {}));

        DrawDialogsAndPopups(vCurrentFrame, MainBackend::Instance()->GetDisplaySize(), context_ptr, {});

        ImGuiThemeHelper::Instance()->Draw();
        LayoutManager::Instance()->InitAfterFirstDisplay(io.DisplaySize);

        // on update la mouse apres l'affichage complet
        m_CanMouseAffectRendering();
    }
}

void MainFrontend::NeedToNewProject(const std::string& vFilePathName)
{
    m_NeedToNewProject = true;
    m_FilePathNameToLoad = vFilePathName;
}

void MainFrontend::NeedToLoadProject(const std::string& vFilePathName)
{
    m_NeedToLoadProject = true;
    m_FilePathNameToLoad = vFilePathName;
}

void MainFrontend::NeedToCloseProject()
{
    m_NeedToCloseProject = true;
}

bool MainFrontend::SaveProject()
{
    return ProjectFile::Instance()->Save();
}

void MainFrontend::SaveAsProject(const std::string& vFilePathName)
{
    ProjectFile::Instance()->SaveAs(vFilePathName);

    if (m_NeedToCloseApp)
    {
        //glfwSetWindowShouldClose(m_Window, 1); // close app
    }
}

// actions to do after rendering
void MainFrontend::PostRenderingActions()
{
    if (m_NeedToNewProject)
    {
        ProjectFile::Instance()->New(m_FilePathNameToLoad);
        //SetAppTitle(m_FilePathNameToLoad);

        m_FilePathNameToLoad.clear();
        m_NeedToNewProject = false;
    }

    if (m_NeedToLoadProject)
    {
        if (ProjectFile::Instance()->LoadAs(m_FilePathNameToLoad))
        {
            //SetAppTitle(m_FilePathNameToLoad);
            ProjectFile::Instance()->SetProjectChange(false);
        }
        else
        {
            LogVarError("Failed to load project %s", m_FilePathNameToLoad.c_str());
        }

        m_FilePathNameToLoad.clear();
        m_NeedToLoadProject = false;
    }

    if (m_NeedToCloseProject)
    {
        NodeManager::Instance()->Clear();
        ProjectFile::Instance()->Clear();
        m_NeedToCloseProject = false;
    }
}

void MainFrontend::m_CanMouseAffectRendering() {
    const auto& CanUpdateMouse = MainFrontend::sCentralWindowHovered &&             // only update mouse when hovering the central node
                                 !ImGuizmo::IsUsing() &&                            // only update mouse if no gizmo is moving
                                 ImGui::GetTopMostAndVisiblePopupModal() == nullptr;  // only update mouse if no modal dialog is opened
    MainBackend::Instance()->UpdateMouseDatas(CanUpdateMouse);
}

void MainFrontend::setSize(const ct::ivec2& vSize) { m_size = vSize; }

const ct::ivec2& MainFrontend::getSize() const { return m_size; }

bool MainFrontend::resize(const ct::ivec2& /*vNewSize*/) { return false; }

bool MainFrontend::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    bool res = false;
    return res;
}

bool MainFrontend::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    bool res = false;
    return res;
}

bool MainFrontend::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    bool change = false;
    m_ActionSystem.RunActions();
    if (m_ShowImGui) {
        ImGui::ShowDemoWindow(&m_ShowImGui);
    }
    if (m_ShowMetric) {
        ImGui::ShowMetricsWindow(&m_ShowMetric);
    }
    
    return false;
}

void MainFrontend::OpenAboutDialog()
{
    m_ShowAboutDialog = true;
}

void MainFrontend::m_drawMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu( " Project"))
        {
            if (ImGui::MenuItem( " New"))
            {
                Action_Menu_NewProject();
            }

            if (ImGui::MenuItem( " Open"))
            {
                Action_Menu_OpenProject();
            }

            if (ProjectFile::Instance()->IsLoaded())
            {
                ImGui::Separator();

                if (ImGui::MenuItem( " Re Open"))
                {
                    Action_Menu_ReOpenProject();
                }

                ImGui::Separator();

                if (ImGui::MenuItem( " Save"))
                {
                    Action_Menu_SaveProject();
                }

                if (ImGui::MenuItem( " Save As"))
                {
                    Action_Menu_SaveAsProject();
                }

                ImGui::Separator();

                if (ImGui::MenuItem( " Close"))
                {
                    Action_Menu_CloseProject();
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem( " About"))
            {
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

        if (ProjectFile::Instance()->IsThereAnyNotSavedChanged())
        {
            ImGui::Spacing(200.0f);

            if (ImGui::MenuItem(" Save"))
            {
                Action_Menu_SaveProject();
            }
        }

        // ImGui Infos
        const auto label = ct::toStr("Dear ImGui %s (Docking)", ImGui::GetVersion());
        const auto size  = ImGui::CalcTextSize(label.c_str());
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
        const auto& io   = ImGui::GetIO();
        const auto  fps  = ct::toStr("%.1f ms/frame (%.1f fps)", 1000.0f / io.Framerate, io.Framerate);
        const auto  size = ImGui::CalcTextSize(fps.c_str());
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
    _config.normal_size  = 30.0f;
    _config.hovered_size = 75.0f;
    _config.anchor       = ImVec2(0.0f, 0.5f);
    _config.anim_step    = 0.2f;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
    bool left_bar_opened = ImGui::BeginLeftToolBar(toolbar_width);
    ImGui::PopStyleVar();

    if (left_bar_opened) {
        ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 2));
        bool _opened = ImGui::BeginCoolBar("##coolbar", ImCoolBarFlags_Vertical, _config);
        auto window  = ImGui::GetCurrentWindow();
        if (window) {
            // correct the rect of the window. maybe a bug on imgui..
            // the workrect can cause issue when click a timeline
            // channel close button when close to the toolbar
            // this thing correct the issue
            const auto& rc            = window->Rect();
            window->WorkRect          = rc;
            window->OuterRectClipped  = rc;
            window->InnerRect         = rc;
            window->InnerClipRect     = rc;
            window->ParentWorkRect    = rc;
            window->ClipRect          = rc;
            window->ContentRegionRect = rc;
        }
        ImGui::PopStyleVar(2);

        if (_opened) {
#ifdef _DEBUG
            if (ImGui::CoolBarItem()) {  // Debug Pane
                const auto aw           = ImGui::GetCoolBarItemWidth();
                m_ToolbarFontPtr->Scale = font_scale_ratio * ImGui::GetCoolBarItemScale();
                ImGui::RadioButtonLabeled_BitWize<PaneFlags>(ImVec2(aw, aw),
                                                             DEBUG_PANE_ICON "##Debug",
                                                             nullptr,
                                                             &LayoutManager::Instance()->pane_Shown,
                                                             DebugPane::Instance()->paneFlag,
                                                             false,
                                                             true,
                                                             0,
                                                             false,
                                                             m_ToolbarFontPtr);
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
                const auto aw           = ImGui::GetCoolBarItemWidth();
                m_ToolbarFontPtr->Scale = font_scale_ratio * ImGui::GetCoolBarItemScale();
                ImGui::RadioButtonLabeled_BitWize<PaneFlags>(ImVec2(aw, aw),
                                                             TUNING_PANE_ICON "##Tuning",
                                                             nullptr,
                                                             &LayoutManager::Instance()->pane_Shown,
                                                             TuningPane::Instance()->paneFlag,
                                                             false,
                                                             true,
                                                             0,
                                                             false,
                                                             m_ToolbarFontPtr);
            }

            if (ImGui::CoolBarItem()) {  // show/hide Grid Renderer
                const auto aw           = ImGui::GetCoolBarItemWidth();
                m_ToolbarFontPtr->Scale = font_scale_ratio * ImGui::GetCoolBarItemScale();
                ImGui::RadioButtonLabeled(ImVec2(aw, aw), GRID_ICON "##Grid", nullptr, &MainBackend::Instance()->GetBackendDatasRef().canWeShowGrid, false, m_ToolbarFontPtr);
            }

            if (ImGui::CoolBarItem()) {  // enable/disable Camera
                const auto aw           = ImGui::GetCoolBarItemWidth();
                m_ToolbarFontPtr->Scale = font_scale_ratio * ImGui::GetCoolBarItemScale();
                ImGui::RadioButtonLabeled(ImVec2(aw, aw), CAMERA_ICON "##Camera", nullptr, &MainBackend::Instance()->GetBackendDatasRef().canWeTuneCamera, false, m_ToolbarFontPtr);
            }

            if (ImGui::CoolBarItem()) {  // enable/disable Mouse
                const auto aw           = ImGui::GetCoolBarItemWidth();
                m_ToolbarFontPtr->Scale = font_scale_ratio * ImGui::GetCoolBarItemScale();
                ImGui::RadioButtonLabeled(ImVec2(aw, aw), MOUSE_ICON "##Mouse", nullptr, &MainBackend::Instance()->GetBackendDatasRef().canWeTuneMouse, false, m_ToolbarFontPtr);
            }

            if (ImGui::CoolBarItem()) {  // enable/disable Gizmo
                const auto aw           = ImGui::GetCoolBarItemWidth();
                m_ToolbarFontPtr->Scale = font_scale_ratio * ImGui::GetCoolBarItemScale();
                ImGui::RadioButtonLabeled(ImVec2(aw, aw), GIZMO_ICON "##Gizmo", nullptr, &MainBackend::Instance()->GetBackendDatasRef().canWeTuneGizmo, false, m_ToolbarFontPtr);
            }

            ImGui::EndCoolBar();
        }
        ImGui::EndLeftToolBar();
    }
}
///////////////////////////////////////////////////////
//// SAVE DIALOG WHEN UN SAVED CHANGES ////////////////
///////////////////////////////////////////////////////

void MainFrontend::OpenUnSavedDialog()
{
    // force close dialog if any dialog is opened
    ImGuiFileDialog::Instance()->Close();

    m_SaveDialogIfRequired = true;
}
void MainFrontend::CloseUnSavedDialog()
{
    m_SaveDialogIfRequired = false;
}

bool MainFrontend::ShowUnSavedDialog()
{
    bool res = false;

    if (m_SaveDialogIfRequired)
    {
        if (ProjectFile::Instance()->IsLoaded())
        {
            if (ProjectFile::Instance()->IsThereAnyNotSavedChanged())
            {
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
                if (ImGui::BeginPopupModal("Do you want to save before ?", (bool*)0,
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking))
                {
                    if (ImGui::Button("Save"))
                    {
                        res = Action_UnSavedDialog_SaveProject();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Save As"))
                    {
                        Action_UnSavedDialog_SaveAsProject();
                    }

                    if (ImGui::Button("Continue without saving"))
                    {
                        res = true; // quit the action
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel"))
                    {
                        Action_Cancel();
                    }

                    ImGui::EndPopup();
                }
            }
        }

        return res; // quit if true, else continue on the next frame
    }

    return true; // quit the action
}

///////////////////////////////////////////////////////
//// ACTIONS //////////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrontend::Action_Menu_NewProject()
{
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
    m_ActionSystem.Add([this]()
        {
            CloseUnSavedDialog();
    ImGuiFileDialog::Instance()->OpenDialog(
        "NewProjectDlg", "New Project File", ".lum", "", 1, nullptr, ImGuiFileDialogFlags_Modal);
    return true;
        });
    m_ActionSystem.Add([this]()
        {
            return Display_NewProjectDialog();
        });
}

void MainFrontend::Action_Menu_OpenProject()
{
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
    m_ActionSystem.Add([this]()
        {
            CloseUnSavedDialog();
    ImGuiFileDialog::Instance()->OpenDialog(
        "OpenProjectDlg", "Open Project File", ".lum", "", 1, nullptr, ImGuiFileDialogFlags_Modal);
    return true;
        });
    m_ActionSystem.Add([this]()
        {
            return Display_OpenProjectDialog();
        });
}

void MainFrontend::Action_Menu_ReOpenProject()
{
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
    m_ActionSystem.Add([this]()
        {
            NeedToLoadProject(ProjectFile::Instance()->GetProjectFilepathName());
    return true;
        });
}

void MainFrontend::Action_Menu_SaveProject()
{
    /*
    save project :
    -	never saved :
        -	add action : save as project
    -	saved in a file beofre :
        -	add action : save project
    */
    m_ActionSystem.Clear();
    m_ActionSystem.Add([this]()
        {
            if (!SaveProject())
            {
                CloseUnSavedDialog();
                ImGuiFileDialog::Instance()->OpenDialog(
                    "SaveProjectDlg", "Save Project File", ".lum", "", 1, nullptr, ImGuiFileDialogFlags_Modal);
            }
    return true;
        });
    m_ActionSystem.Add([this]()
        {
            return Display_SaveProjectDialog();
        });
}

void MainFrontend::Action_Menu_SaveAsProject()
{
    /*
    save as project :
    -	add action : save as project
    */
    m_ActionSystem.Clear();
    m_ActionSystem.Add([this]()
        {
            CloseUnSavedDialog();
    ImGuiFileDialog::Instance()->OpenDialog(
        "SaveProjectDlg", "Save Project File", ".lum", "",
        1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal);
    return true;
        });
    m_ActionSystem.Add([this]()
        {
            return Display_SaveProjectDialog();
        });
}

void MainFrontend::Action_Menu_CloseProject()
{
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
    m_ActionSystem.Add([this]()
        {
            NeedToCloseProject();
    return true;
        });
}

void MainFrontend::Action_Window_CloseApp()
{
    if (m_NeedToCloseApp) return; // block next call to close app when running
    /*
    Close app :
    -	unsaved :
        -	add action : show unsaved dialog
        -	add action : Close app
    -	saved :
        -	add action : Close app
    */
    m_NeedToCloseApp = true;

    m_ActionSystem.Clear();
    Action_OpenUnSavedDialog_IfNeeded();
    m_ActionSystem.Add([this]()
        {
            //glfwSetWindowShouldClose(m_Window, 1); // close app
    return true;
        });
}

void MainFrontend::Action_OpenUnSavedDialog_IfNeeded()
{
    if (ProjectFile::Instance()->IsLoaded() &&
        ProjectFile::Instance()->IsThereAnyNotSavedChanged())
    {
        OpenUnSavedDialog();
        m_ActionSystem.Add([this]()
            {
                return ShowUnSavedDialog();
            });
    }
}

void MainFrontend::Action_Cancel()
{
    /*
    -	cancel :
        -	clear actions
    */
    CloseUnSavedDialog();
    m_ActionSystem.Clear();
    m_NeedToCloseApp = false;
}

bool MainFrontend::Action_UnSavedDialog_SaveProject()
{
    bool res = SaveProject();
    if (!res)
    {
        m_ActionSystem.Insert([this]()
            {
                return Display_SaveProjectDialog();
            });
        m_ActionSystem.Insert([this]()
            {
                CloseUnSavedDialog();
        ImGuiFileDialog::Instance()->OpenDialog(
            "SaveProjectDlg", "Save Project File", ".lum",
            ".", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal);
        return true;
            });
    }
    return res;
}

void MainFrontend::Action_UnSavedDialog_SaveAsProject()
{
    m_ActionSystem.Insert([this]()
        {
            return Display_SaveProjectDialog();
        });
    m_ActionSystem.Insert([this]()
        {
            CloseUnSavedDialog();
    ImGuiFileDialog::Instance()->OpenDialog(
        "SaveProjectDlg", "Save Project File", ".lum",
        ".", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal);
    return true;
        });
}

void MainFrontend::Action_UnSavedDialog_Cancel()
{
    Action_Cancel();
}

///////////////////////////////////////////////////////
//// DIALOG FUNCS /////////////////////////////////////
///////////////////////////////////////////////////////

bool MainFrontend::Display_NewProjectDialog()
{
    // need to return false to continue to be displayed next frame

    ImVec2 min = m_DisplaySize * 0.5f;
    ImVec2 max = m_DisplaySize;

    if (ImGuiFileDialog::Instance()->Display("NewProjectDlg",
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            CloseUnSavedDialog();
            auto file = ImGuiFileDialog::Instance()->GetFilePathName();
            NeedToNewProject(file);
        }
        else // cancel
        {
            Action_Cancel(); // we interrupts all actions
        }

        ImGuiFileDialog::Instance()->Close();

        return true;
    }

    return false;
}

bool MainFrontend::Display_OpenProjectDialog()
{
    // need to return false to continue to be displayed next frame

    ImVec2 min = m_DisplaySize * 0.5f;
    ImVec2 max = m_DisplaySize;

    if (ImGuiFileDialog::Instance()->Display("OpenProjectDlg",
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            CloseUnSavedDialog();
            NeedToLoadProject(ImGuiFileDialog::Instance()->GetFilePathName());
        }
        else // cancel
        {
            Action_Cancel(); // we interrupts all actions
        }

        ImGuiFileDialog::Instance()->Close();

        return true;
    }

    return false;
}

bool MainFrontend::Display_SaveProjectDialog()
{
    // need to return false to continue to be displayed next frame

    ImVec2 min = m_DisplaySize * 0.5f;
    ImVec2 max = m_DisplaySize;

    if (ImGuiFileDialog::Instance()->Display("SaveProjectDlg",
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            CloseUnSavedDialog();
            SaveAsProject(ImGuiFileDialog::Instance()->GetFilePathName());
        }
        else // cancel
        {
            Action_Cancel(); // we interrupts all actions
        }

        ImGuiFileDialog::Instance()->Close();

        return true;
    }

    return false;
}

///////////////////////////////////////////////////////
//// APP CLOSING //////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrontend::IWantToCloseTheApp()
{
    Action_Window_CloseApp();
}

///////////////////////////////////////////////////////
//// DROP /////////////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrontend::JustDropFiles(int count, const char** paths)
{
    assert(0);

    std::map<std::string, std::string> dicoFont;
    std::string prj;

    for (int i = 0; i < count; ++i)
    {
        // file
        auto f = std::string(paths[i]);

        // lower case
        auto f_opt = f;
        for (auto& c : f_opt)
            c = (char)std::tolower((int)c);

        // well known extention
        if (f_opt.find(".ttf") != std::string::npos			// truetype (.ttf)
            || f_opt.find(".otf") != std::string::npos			// opentype (.otf)
            //||	f_opt.find(".ttc") != std::string::npos		// ttf/otf collection for futur (.ttc)
            )
        {
            dicoFont[f] = f;
        }
        if (f_opt.find(".lum") != std::string::npos)
        {
            prj = f;
        }
    }

    // priority to project file
    if (!prj.empty())
    {
        NeedToLoadProject(prj);
    }
}

//////////////////////////////////////////////////////////////////////////////////
//// PRIVATE /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool MainFrontend::m_build() {
    // toolbar
    static ImFontConfig icons_config3;
    icons_config3.MergeMode              = false;
    icons_config3.PixelSnapH             = true;
    static const ImWchar icons_ranges3[] = {ICON_MIN_SDFMT, ICON_MAX_SDFMT, 0};
    const float&         font_size       = 20.0f / font_scale_ratio;
    m_ToolbarFontPtr                     = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_SDFMT, font_size, &icons_config3, icons_ranges3);
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
    str += vOffset + "<project>" + ProjectFile::Instance()->GetProjectFilepathName() + "</project>\n";

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
    }
    else if (strName == "project"){
        NeedToLoadProject(strValue);
    } else if (strName == "showaboutdialog") {
        m_ShowAboutDialog = ct::ivariant(strValue).GetB();
    } else if (strName == "showimgui") {
        m_ShowImGui = ct::ivariant(strValue).GetB();
    } else if (strName == "showmetric") {
        m_ShowMetric = ct::ivariant(strValue).GetB();
    }

    return true;
}
