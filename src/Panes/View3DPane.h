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

#pragma once

#include <Gaia/Resources/VulkanFrameBufferAttachment.h>
#include <Gaia/Gui/VulkanImGuiRenderer.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <LumoBackend/Graph/Graph.h>
#include <AbstractPane.h>
#include <stdint.h>
#include <string>
#include <map>

class ProjectFile;
class VulkanImGuiRenderer;
class View3DPane : public AbstractPane {
private:
    NodeSlotWeak m_Texture2DOutputSlot;
    ImGuiTexture m_ImGuiTexture;
    ct::irect m_PreviewRect;
    VulkanImGuiRendererWeak m_VulkanImGuiRenderer;

    //uint32_t m_PreviewBufferId = 0;
    bool m_CanWeTuneCamera = true;
    bool m_CanWeTuneMouse = true;
    bool m_DisplayPictureByRatio = true;
    bool m_CanWeTuneGizmo = false;
    float m_DisplayQuality = 1.0f;
    ct::fvec2 m_CurrNormalizedMousePos;
    ct::fvec2 m_LastNormalizedMousePos;
    bool m_MouseDrag = false;
    bool m_UINeedRefresh = false;
    //uint32_t m_MaxBuffers = 0;

    // for send the resize event to nodes
    ct::ivec2 m_PaneSize;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(
        const uint32_t& vCurrentFrame, PaneFlags& vInOutPaneShown, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    void Select(BaseNodeWeak vObjet);

    ct::fvec2 SetOrUpdateOutput(NodeSlotWeak vTexture2DOutputSlot);
    void SetVulkanImGuiRenderer(VulkanImGuiRendererWeak vVulkanImGuiRenderer);

    bool UpdateCameraIfNeeded();

private:
    void m_SetDescriptor(GaiApi::VulkanFrameBufferAttachment* vVulkanFrameBufferAttachment);
    bool m_CanUpdateMouse(bool vWithMouseDown, int vMouseButton);
    void m_UpdateCamera(ImVec2 vOrg, ImVec2 vSize);

public:  // singleton
    static std::shared_ptr<View3DPane> Instance() {
        static std::shared_ptr<View3DPane> _instance = std::make_shared<View3DPane>();
        return _instance;
    }

public:
    View3DPane();                             // Prevent construction
    View3DPane(const View3DPane&) = default;  // Prevent construction by copying
    View3DPane& operator=(const View3DPane&) {
        return *this;
    };              // Prevent assignment
    virtual ~View3DPane();  // Prevent unwanted destruction;
};
