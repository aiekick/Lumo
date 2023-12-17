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

#include <LumoBackend/Graph/Graph.h>

#include <ctools/cTools.h>

#include <ImGuiPack.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/TaskInterface.h>

class SceneManager : public GuiInterface, public TaskInterface {
public:
    // init / unit
    bool Init(GaiApi::VulkanCoreWeak vVulkanCore);
    void Unit();

    bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;

    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, void* vUserDatas) override;

    vk::DescriptorImageInfo* GetDescriptorImageInfo(ct::fvec2* vOutSize);

    void NeedResizeByResizeEvent(ct::ivec2* vNewSize);

public:  // singleton
    static SceneManager* Instance() {
        static SceneManager _instance;
        return &_instance;
    }

protected:
    SceneManager() = default;
    ~SceneManager() = default;
};
