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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "SceneManager.h"

#include <Frontend/MainFrontend.h>
#include <ctools/FileHelper.h>
#include <ImGuiPack.h>

#include <LumoBackend/Systems/CommonSystem.h>
#include <LumoBackend/Systems/SceneMerger.h>

#include <Gaia/Core/VulkanCore.h>
#include <LumoBackend/Interfaces/CameraInterface.h>
#include <LumoBackend/Interfaces/ShaderUpdateInterface.h>
#include <LumoBackend/Systems/GizmoSystem.h>

#include <Gaia/Gui/VulkanProfiler.h>

using namespace GaiApi;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneManager::Init(GaiApi::VulkanCoreWeak vVulkanCore) {
    ZoneScoped;
    return SceneMerger::Instance(vVulkanCore)->Init();
}

void SceneManager::Unit() {
    ZoneScoped;
    SceneMerger::Instance()->Unit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// TASK //////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneManager::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    bool res = false;

    {
        vkProfScopedNoCmd("Scene", "%s", "Scene");
        res |= SceneMerger::Instance()->Execute(vCurrentFrame, vCmd);
    }

    return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// IMGUI /////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneManager::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    bool change = false;
    change |= CommonSystem::Instance()->DrawImGui();
    change |= SceneMerger::Instance()->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
    return change;
}

bool SceneManager::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);
    ImGuizmo::SetRect(vRect.Min.x, vRect.Min.y, vRect.GetWidth(), vRect.GetHeight());
    return SceneMerger::Instance()->DrawOverlays(vCurrentFrame, vRect, vContextPtr, vUserDatas);
}

bool SceneManager::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return SceneMerger::Instance()->DrawDialogsAndPopups(vCurrentFrame, vMaxRect, vContextPtr, vUserDatas);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE OUTPUT ////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* SceneManager::GetDescriptorImageInfo(ct::fvec2* vOutSize) {
    return SceneMerger::Instance()->GetDescriptorImageInfo(vOutSize);
}

void SceneManager::NeedResizeByResizeEvent(ct::ivec2* vNewSize) {
    SceneMerger::Instance()->NeedResizeByResizeEvent(vNewSize, nullptr);
}
