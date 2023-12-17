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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <LumoBackend/Systems/SceneMerger.h>

#include <ctools/Logger.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Buffer/FrameBuffer.h>

using namespace GaiApi;

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

SceneMerger::SceneMerger(GaiApi::VulkanCoreWeak vVulkanCore) : BaseRenderer(vVulkanCore) {
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool SceneMerger::Init() {
    ZoneScoped;
    m_Loaded = false;
    ct::uvec2 map_size = 512;
    if (!m_VulkanCore.expired() && BaseRenderer::InitPixel(map_size)) {
        AllowResizeOnResizeEvents(true);
        AllowResizeByHandOrByInputs(true);
        m_FrameBufferPtr = FrameBuffer::Create(m_VulkanCore);
        if (m_FrameBufferPtr &&
            m_FrameBufferPtr->Init(map_size, 1U, true, true, 0.0f, false, vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e2)) {
            m_Loaded = true;
        }
    }
    return m_Loaded;
}

void SceneMerger::Unit() {
    BaseRenderer::Unit();
    m_FrameBufferPtr.reset();
    m_VulkanCore.reset();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool SceneMerger::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    ZoneScoped;
    BaseRenderer::Render("Scene Merger", vCmd);
    return true;
}

void SceneMerger::RenderShaderPasses(vk::CommandBuffer* vCmdBufferPtr) {
    if (m_FrameBufferPtr && m_FrameBufferPtr->Begin(vCmdBufferPtr)) {
        m_FrameBufferPtr->ClearAttachmentsIfNeeded(vCmdBufferPtr);
        //tofix : we need to remove expired shaderpasses maybe
        //        during evaluation, by building a new 
        //        valid vector then swap them after
        for (auto pass : m_ShaderPasses) {
            auto pass_ptr = pass.lock();
            if (pass_ptr && pass_ptr->StartDrawPass(vCmdBufferPtr)) {
                pass_ptr->SetLastExecutedFrame(m_LastExecutedFrame);  // for have widgets use
                pass_ptr->DrawModel(vCmdBufferPtr, 1U);
                pass_ptr->EndDrawPass(vCmdBufferPtr);
            } 
        }
        m_FrameBufferPtr->End(vCmdBufferPtr);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMerger::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;
}

bool SceneMerger::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;
}

bool SceneMerger::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / RESIZE ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMerger::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) {
    ZoneScoped;
    if (IsResizeableByResizeEvent()) {
        if (m_FrameBufferPtr) {
            m_FrameBufferPtr->NeedResize(vNewSize, vCountColorBuffers);
        }
    }
}

void SceneMerger::NeedResizeByHand(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) {
    ZoneScoped;
    if (IsResizeableByHand()) {
        if (m_FrameBufferPtr) {
            m_FrameBufferPtr->NeedResize(vNewSize, vCountColorBuffers);
        }
    }
}

bool SceneMerger::ResizeIfNeeded() {
    ZoneScoped;
    if (IsResizeableByHand() || IsResizeableByResizeEvent()) {
        if (m_FrameBufferPtr && m_FrameBufferPtr->ResizeIfNeeded()) {
            auto output_size = m_FrameBufferPtr->GetOutputSize();
            for (auto pass : m_ShaderPasses) {
                auto pass_ptr = pass.lock();
                if (pass_ptr) {
                    pass_ptr->UpdatePixel2DViewportSize(output_size);
                }
            }
            return true;
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* SceneMerger::GetDescriptorImageInfo(ct::fvec2* vOutSize) {
    ZoneScoped;
    if (m_FrameBufferPtr) {
        if (vOutSize) {
            *vOutSize = m_FrameBufferPtr->GetOutputSize();
        }
        return m_FrameBufferPtr->GetFrontDescriptorImageInfo(0U);
    }
    return nullptr;
}
