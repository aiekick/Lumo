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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "ImGuiOverlay.h"
#include <assert.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Gui/VulkanWindow.h>
#include <Gaia/Core/VulkanCommandBuffer.h>

#include <ctools/FileHelper.h>
#include <Backend/MainBackend.h>
#include <Frontend/MainFrontend.h>

#include <imgui.h>
#include <Gaia/Gui/VulkanImGuiRenderer.h>
#include <stdio.h>   // printf, fprintf
#include <stdlib.h>  // abort

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

#include <GLFW/glfw3.h>

#include <Res/sdfmFont.cpp>
#include <Res/Roboto_Medium.cpp>
#include <Res/FireCode.cpp>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

ImGuiOverlayPtr ImGuiOverlay::Create(GaiApi::VulkanCoreWeak vVulkanCoreWeak, GaiApi::VulkanWindowWeak vVulkanWindowWeak) {
    auto res = std::make_shared<ImGuiOverlay>();
    if (!res->Init(vVulkanCoreWeak, vVulkanWindowWeak)) {
        res.reset();
    }
    return res;
}

ImGuiOverlay::ImGuiOverlay() = default;
ImGuiOverlay::~ImGuiOverlay() = default;

bool ImGuiOverlay::Init(GaiApi::VulkanCoreWeak vVulkanCoreWeak, GaiApi::VulkanWindowWeak vVulkanWindowWeak) {
    ZoneScoped;

    m_VulkanCoreWeak   = vVulkanCoreWeak;
    m_VulkanWindowWeak = vVulkanWindowWeak;

    auto corePtr = m_VulkanCoreWeak.lock();
    if (corePtr) {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Enable Docking
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable ViewPort
        io.FontAllowUserScaling              = true;  // activate zoom feature with ctrl + mousewheel
        io.ConfigWindowsMoveFromTitleBarOnly = true;  // can move windows only with titlebar
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
        io.ConfigViewportsNoDecoration = false;  // toujours mettre une frame au fenetre enfant
#endif

        ImGui::StyleColorsDark();

        auto winPtr = m_VulkanWindowWeak.lock();
        if (winPtr) {
            m_VulkanImGuiRendererPtr = VulkanImGuiRenderer::Create(m_VulkanCoreWeak, m_VulkanWindowWeak);
            if (m_VulkanImGuiRendererPtr) {
                corePtr->SetVulkanImGuiRenderer(m_VulkanImGuiRendererPtr);

                // MAIN FONT
                auto mainFontPtr = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_RM, 60.0f);
                if (mainFontPtr) {
                    mainFontPtr->Scale = 0.25f;
                }

                // ICON FONT
                static ImFontConfig icons_config_SDFM;
                icons_config_SDFM.MergeMode = true;
                icons_config_SDFM.PixelSnapH = true;
                static ImWchar icons_ranges_SDFM[] = {ICON_MIN_SDFM, ICON_MAX_SDFM, 0};
                auto iconFontPtr = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(
                    FONT_ICON_BUFFER_NAME_SDFM, 60.0f, &icons_config_SDFM, icons_ranges_SDFM);

                // CODE FONT
                auto codeFontPtr = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_FCR, 60.0f);
                if (codeFontPtr) {
                    codeFontPtr->Scale = 0.25f;
                }

                m_MainFrontendPtr = MainFrontend::create();
                if (m_MainFrontendPtr != nullptr) {
                    MainBackend::Instance()->setFrontend(m_MainFrontendPtr);
                    m_MainFrontendPtr->setBackend(MainBackend::Instance()->GetWeak());
                    return m_VulkanImGuiRendererPtr->CreateFontsTexture();
                }
            }
        }
    }

    return false;
}

void ImGuiOverlay::Unit() {
    ZoneScoped;

    if (m_MainFrontendPtr) {
        m_MainFrontendPtr->unit();
        m_MainFrontendPtr.reset();
    }

    if (m_VulkanImGuiRendererPtr) {
        m_VulkanImGuiRendererPtr->Unit();
        m_VulkanImGuiRendererPtr.reset();
    }

    ImGui::DestroyContext();
}

void ImGuiOverlay::begin() {
    ZoneScoped;

    if (m_VulkanImGuiRendererPtr) {
        m_VulkanImGuiRendererPtr->NewFrame();
    }

    ImGui::NewFrame();
}

void ImGuiOverlay::end() {
    ZoneScoped;

    ImGui::Render();
}

bool ImGuiOverlay::render() {
    ZoneScoped;

    bool res     = false;

    auto corePtr   = m_VulkanCoreWeak.lock();
    if (corePtr) {
        auto devicePtr = corePtr->getFrameworkDevice().lock();
        if (devicePtr) {
            auto cmd = corePtr->getGraphicCommandBuffer();
            devicePtr->BeginDebugLabel(&cmd, "ImGui", IMGUI_RENDERER_DEBUG_COLOR);

            {
                auto main_draw_datas         = ImGui::GetDrawData();
                const bool main_is_minimized = (main_draw_datas->DisplaySize.x <= 0.0f || main_draw_datas->DisplaySize.y <= 0.0f);
                if (!main_is_minimized) {
                    if (corePtr && m_VulkanImGuiRendererPtr) {
                        m_VulkanImGuiRendererPtr->RenderDrawData(ImGui::GetDrawData(), (VkCommandBuffer)corePtr->getGraphicCommandBuffer());
                        res = true;
                    }
                }
            }

            devicePtr->EndDebugLabel(&cmd);
        }
    }

    return res;
}

void ImGuiOverlay::drawFPS() {
    ZoneScoped;

    const ImGuiWindowFlags fpsWindowFlags = ImGuiWindowFlags_NoTitleBar |    //
                                            ImGuiWindowFlags_NoMove |        //
                                            ImGuiWindowFlags_NoCollapse |    //
                                            ImGuiWindowFlags_NoBackground |  //
                                            ImGuiWindowFlags_NoResize |      //
                                            ImGuiWindowFlags_NoScrollbar;    //

    ImGui::Begin("fps", 0, fpsWindowFlags);
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::Text("GUI: Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

void ImGuiOverlay::drawDemo() {
    ZoneScoped;

    ImGui::ShowDemoWindow();
}

MainFrontendPtr ImGuiOverlay::getFrontend() {
    return m_MainFrontendPtr;
}

ImGuiIO& ImGuiOverlay::imgui_io() {
    ZoneScoped;

    return ImGui::GetIO();
}

VulkanImGuiRendererWeak ImGuiOverlay::GetImGuiRenderer() {
    return m_VulkanImGuiRendererPtr;
}
