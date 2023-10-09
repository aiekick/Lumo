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

#pragma once
#pragma warning(disable : 4251)

#include <memory>

#include <Gaia/gaia.h>
#include <ImGuiPack.h>
#include <Gaia/gaia.h>
#include <Headers/Globals.h>

class VulkanImGuiRenderer;

class VulkanCore;
class VulkanWindow;
class ImGuiOverlay {
public:
    static ImGuiOverlayPtr Create(GaiApi::VulkanCoreWeak vVulkanCoreWeak, GaiApi::VulkanWindowWeak vVulkanWindowWeak);

private:
    MainFrontendPtr m_MainFrontendPtr = nullptr;
    GaiApi::VulkanCoreWeak m_VulkanCoreWeak;
    VulkanImGuiRendererPtr m_VulkanImGuiRendererPtr = nullptr;
    GaiApi::VulkanWindowWeak m_VulkanWindowWeak;
    bool m_IsRecording = false;

public:
    ImGuiOverlay();
    ~ImGuiOverlay();

    bool Init(GaiApi::VulkanCoreWeak vVulkanCoreWeak, GaiApi::VulkanWindowWeak vVulkanWindowWeak);
    void Unit();

    void begin();
    void end();
    virtual bool render();

    void drawFPS();
    void drawDemo();

    MainFrontendPtr getFrontend();

    ImGuiIO& imgui_io();

    VulkanImGuiRendererWeak GetImGuiRenderer();
};
