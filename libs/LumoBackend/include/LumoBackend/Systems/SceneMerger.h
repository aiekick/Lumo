/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

#pragma once

#include <set>
#include <array>
#include <string>
#include <memory>

#include <Gaia/gaia.h>
#include <Gaia/Core/VulkanCore.h>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Base/BaseRenderer.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

#include <LumoBackend/Interfaces/TaskInterface.h>
#include <LumoBackend/Interfaces/ShaderPassInputInterface.h>
#include <LumoBackend/Interfaces/Texture2DOutputInterface.h>

class LUMO_BACKEND_API SceneMerger : public BaseRenderer,  //
                                     public TaskInterface  //
{                                                          //
private:
    FrameBufferPtr m_FrameBufferPtr = nullptr;
    //std::vector<ShaderPassWeak> m_tmpShaderPasses;

public:
    static SceneMerger* Instance(GaiApi::VulkanCoreWeak vVulkanCore = {}) {
        static SceneMerger _instance(vVulkanCore);
        return &_instance;
    }

public:
    SceneMerger(GaiApi::VulkanCoreWeak vVulkanCore);
    ~SceneMerger() = default;

    bool Init();
    void Unit();

    bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
    void RenderShaderPasses(vk::CommandBuffer* vCmdBufferPtr) override;

    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

    void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
    void NeedResizeByHand(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
    bool ResizeIfNeeded() override;

    vk::DescriptorImageInfo* GetDescriptorImageInfo(ct::fvec2* vOutSize);
};
