#pragma once

#include <Headers/Globals.h>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Headers/LumoBackendDefs.h>
#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/MouseInterface.h>

#include <Rendering/DisplaySizeQuadRenderer.h>

#include <Frontend/ImGuiOverlay.h>

#include <Gaia/gaia.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Gui/VulkanWindow.h>
#include <Gaia/Buffer/FrameBuffer.h>
#include <Gaia/Interfaces/iService.h>
#include <Gaia/Interfaces/iSurface.h>

#include <string>
#include <memory>
#include <array>
#include <vector>
#include <unordered_map>

struct FileDialogAsset {
    Texture2DPtr texturePtr = nullptr;
    vk::DescriptorSet descriptorSet = vk::DescriptorSet{};
};

struct BackendDatas {
    bool canWeTuneCamera = false;  // enable/disable Camera
    bool canWeTuneMouse = false;   // enable/disable Mouse
    bool canWeTuneGizmo = false;   // enable/disable Gizmo
    bool canWeShowGrid = false;    // show/hide Grid Renderer
};

class MainBackend : public gaia::iService, public gaia::iSurface<ct::ivec2>, public conf::ConfigAbstract {
public:
    static MainBackendPtr Create();

private:
    MainBackendWeak m_This;
    MainFrontendWeak m_Frontend;

    GaiApi::VulkanWindowPtr m_VulkanWindowPtr = nullptr;
    ImGuiOverlayPtr m_ImGuiOverlayPtr = nullptr;
    GaiApi::VulkanCorePtr m_VulkanCorePtr = nullptr;

    std::vector<std::shared_ptr<FileDialogAsset>> m_FileDialogAssets;

    ct::ivec2 m_DisplayPos;
    ct::ivec2 m_DisplaySize;

    // mouse
    MouseInterface m_MouseInterface;
    ct::fvec4 m_MouseFrameSize;
    ct::fvec2 m_MousePos;
    ct::fvec2 m_LastNormalizedMousePos;
    ct::fvec2 m_NormalizedMousePos;

    bool m_MouseDrag = false;
    bool m_NeedRefresh = true;
    bool m_ConsoleVisiblity = false;
    BackendDatas m_BackendDatas;
    float m_DisplayQuality = 1.0f;
    uint32_t m_CurrentFrame = 0U;

    bool m_NeedToCloseApp = false;  // when app closing app is required

    bool m_NeedToNewProject = false;
    bool m_NeedToLoadProject = false;
    bool m_NeedToCloseProject = false;
    std::string m_ProjectFileToLoad;

    std::function<void(std::set<std::string>)> m_ChangeFunc;
    std::set<std::string> m_PathsToTrack;

    DisplaySizeQuadRendererPtr m_DisplaySizeQuadRendererPtr = nullptr;  // will render in swapchain the offscreen textures of m_FrameBufferMergerPtr

public:  // getters
    BackendDatas& GetBackendDatasRef() {
        return m_BackendDatas;
    }
    ImVec2 GetDisplayPos() {
        return ImVec2((float)m_DisplayPos.x, (float)m_DisplayPos.y);
    }
    ImVec2 GetDisplaySize() {
        return ImVec2((float)m_DisplaySize.x, (float)m_DisplaySize.y);
    }
    ImRect GetDisplayRect() {
        const auto& p = GetDisplayPos();
        return ImRect(p, p + GetDisplaySize());
    }

public:
    virtual ~MainBackend();

    void run();

    bool init() override;
    void unit() override;

    MainBackendWeak GetWeak() {
        return m_This;
    }
    MainFrontendWeak GetFrontend() {
        return m_Frontend;
    }
    GaiApi::VulkanCoreWeak GetVulkanCore() {
        return m_VulkanCorePtr;
    }
    ImGuiOverlayWeak GetOverlay() {
        return m_ImGuiOverlayPtr;
    }

    GaiApi::VulkanWindowWeak getWindow() {
        return m_VulkanWindowPtr;
    }

    bool isValid() const override;
    bool isThereAnError() const override;

    void NeedToNewProject(const std::string& vFilePathName);
    void NeedToLoadProject(const std::string& vFilePathName);
    void NeedToCloseProject();

    bool SaveProject();
    void SaveAsProject(const std::string& vFilePathName);

    void PostRenderingActions();

    bool IsNeedToCloseApp();
    void NeedToCloseApp(const bool& vFlag = true);
    void CloseApp();

    void setSize(const ct::ivec2& vSize) override;
    const ct::ivec2& getSize() const override;
    bool resize(const ct::ivec2& vNewSize) override;

    void setAppTitle(const std::string& vFilePathName = {});
    void setFrontend(const MainFrontendWeak& vFontend);

    ct::dvec2 GetMousePos();
    int GetMouseButton(int vButton);

    std::string getAppRelativeFilePathName(const std::string& vFilePathName);

    void UpdateMouseDatas(bool vCanUseMouse);

    void NeedRefresh(const bool& vFlag = true);

public:  // configuration
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

    void SetConsoleVisibility(const bool& vFlag);
    void SwitchConsoleVisibility();
    bool GetConsoleVisibility();

private:
    bool m_build() override;
    void m_RenderOffScreen();

    bool m_CreateVulkanWindow();
    void m_InitFilesTracker();
    bool m_CreateVulkanCore();
    void m_InitScenes();
    void m_InitNodes();
    void m_InitPlugins();
    void m_InitSystems();
    void m_InitPanes();
    bool m_CreateImGuiOverlay();
    bool m_CreateRenderers();

    void m_DestroyRenderers();
    void m_DestroyImGuiOverlay();
    void m_UnitScenes();
    void m_UnitNodes();
    void m_UnitPlugins();
    void m_UnitSystems();
    void m_DestroyVulkanWindow();
    void m_DestroyVulkanCore();
    void m_UnitFilesTracker();

    void m_DeleteNodesIfAnys();

    void m_MainLoop();
    bool m_BeginRender(bool& vNeedResize);
    void m_EndRender();
    void m_PrepareImGui(ImRect vViewport);
    void m_Update();
    void m_IncFrame();
    void m_Resize();

    void m_AddPathToTrack(std::string vPathToTrack, bool vCreateDirectoryIfNotExist);
    void m_InitFilesTracker(std::function<void(std::set<std::string>)> vChangeFunc, std::list<std::string> vPathsToTrack);
    void m_CheckIfTheseAreSomeFileChanges();
    void m_UpdateFiles(const std::set<std::string>& vFiles);

    void m_UpdateMouse();
    void m_UpdateCamera(const bool& vForce = false);
    void m_UpdateSound();

    void m_UpdateCameraAndMouse();

public:  // singleton
    static MainBackend* Instance() {
        static MainBackendPtr _instance_ptr = MainBackend::Create();
        return _instance_ptr.get();
    };
};
