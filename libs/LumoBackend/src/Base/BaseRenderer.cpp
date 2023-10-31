// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include <LumoBackend/Base/BaseRenderer.h>

#include <utility>
#include <functional>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>
#include <Gaia/Buffer/FrameBuffer.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <ctools/Logger.h>
#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>

#include <LumoBackend/Base/Base.h>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

#ifdef CUSTOM_LUMO_BACKEND_CONFIG
#include CUSTOM_LUMO_BACKEND_CONFIG
#else
#include <LumoBackend/Headers/LumoBackendConfigHeader.h>
#endif  // CUSTOM_LUMO_BACKEND_CONFIG

using namespace GaiApi;

// #define VERBOSE_DEBUG
// #define BLEND_ENABLED

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / CONSTRUCTOR //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

BaseRenderer::BaseRenderer(GaiApi::VulkanCorePtr vVulkanCorePtr) {
    ZoneScoped;

    m_VulkanCorePtr = vVulkanCorePtr;
    m_Device = m_VulkanCorePtr->getDevice();
}

BaseRenderer::BaseRenderer(GaiApi::VulkanCorePtr vVulkanCorePtr, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool) {
    ZoneScoped;

    m_VulkanCorePtr = vVulkanCorePtr;
    m_Device = m_VulkanCorePtr->getDevice();
    m_CommandPool = *vCommandPool;
    m_DescriptorPool = *vDescriptorPool;
}

BaseRenderer::~BaseRenderer() {
    ZoneScoped;

    Unit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / PASS /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BaseRenderer::AddGenericPass(ShaderPassWeak vPass) {
    if (!vPass.expired()) {
        m_ShaderPasses.push_back(vPass);
        return true;
    }
    return false;
}

ShaderPassWeak BaseRenderer::GetGenericPass(const uint32_t& vIdx) {
    if (m_ShaderPasses.size() > vIdx) {
        return m_ShaderPasses.at(vIdx);
    }

    return ShaderPassWeak();
}

void BaseRenderer::ClearGenericPasses() {
    m_ShaderPasses.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / DURING INIT //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BaseRenderer::ActionBeforeInit() {
}

void BaseRenderer::ActionAfterInitSucceed() {
}

void BaseRenderer::ActionAfterInitFail() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / INIT/UNIT ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BaseRenderer::InitPixel(const ct::uvec2& vSize) {
    ZoneScoped;

    ActionBeforeInit();

    m_Loaded = false;

    m_Device = m_VulkanCorePtr->getDevice();
    ct::uvec2 size = ct::clamp(vSize, 1u, 8192u);
    if (!size.emptyOR()) {
        m_Queue = m_VulkanCorePtr->getQueue(vk::QueueFlagBits::eGraphics);
        m_DescriptorPool = m_VulkanCorePtr->getDescriptorPool();
        m_CommandPool = m_Queue.cmdPools;

        m_OutputSize = ct::uvec3(size.x, size.y, 0);
        m_RenderArea = vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_OutputSize.x, m_OutputSize.y));
        m_Viewport = vk::Viewport(0.0f, 0.0f, static_cast<float>(m_OutputSize.x), static_cast<float>(m_OutputSize.y), 0, 1.0f);
        m_OutputRatio = ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y).ratioXY<float>();

        if (CreateCommanBuffer()) {
            if (CreateSyncObjects()) {
                m_Loaded = true;
            }
        }
    }

    if (m_Loaded) {
#ifdef PROFILER_INCLUDE
        m_TracyContext = TracyVkContext(m_VulkanCorePtr->getPhysicalDevice(), m_Device, m_Queue.vkQueue, m_CommandBuffers[0]);
#endif

        ActionAfterInitSucceed();
    } else {
        ActionAfterInitFail();
    }

    return m_Loaded;
}

bool BaseRenderer::InitCompute1D(const uint32_t& vSize) {
    ZoneScoped;

    ActionBeforeInit();

    m_Loaded = false;

    m_Device = m_VulkanCorePtr->getDevice();
    uint32_t size = ct::clamp(vSize, 1u, 8192u);
    if (vSize) {
        m_UniformSectionToShow = {"COMPUTE"};  // pour afficher les uniforms

        m_Queue = m_VulkanCorePtr->getQueue(vk::QueueFlagBits::eGraphics);
        m_DescriptorPool = m_VulkanCorePtr->getDescriptorPool();
        m_CommandPool = m_Queue.cmdPools;

        m_OutputSize = size;
        m_RenderArea = vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_OutputSize.x, m_OutputSize.y));
        m_Viewport = vk::Viewport(0.0f, 0.0f, static_cast<float>(m_OutputSize.x), static_cast<float>(m_OutputSize.y), 0, 1.0f);
        m_OutputRatio = ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y).ratioXY<float>();

        if (CreateCommanBuffer()) {
            if (CreateSyncObjects()) {
                m_Loaded = true;
            }
        }
    }

    if (m_Loaded) {
#ifdef PROFILER_INCLUDE
        m_TracyContext = TracyVkContext(m_VulkanCorePtr->getPhysicalDevice(), m_Device, m_Queue.vkQueue, m_CommandBuffers[0]);
#endif
        ActionAfterInitSucceed();
    } else {
        ActionAfterInitFail();
    }

    return m_Loaded;
}

bool BaseRenderer::InitCompute2D(const ct::uvec2& vSize) {
    ZoneScoped;

    ActionBeforeInit();

    m_Loaded = false;

    m_Device = m_VulkanCorePtr->getDevice();
    ct::uvec2 size = ct::clamp(vSize, 1u, 8192u);
    if (!size.emptyOR()) {
        m_UniformSectionToShow = {"COMPUTE"};  // pour afficher les uniforms

        m_Queue = m_VulkanCorePtr->getQueue(vk::QueueFlagBits::eGraphics);
        m_DescriptorPool = m_VulkanCorePtr->getDescriptorPool();
        m_CommandPool = m_Queue.cmdPools;

        m_OutputSize = ct::uvec3(size.x, size.y, 1);
        m_RenderArea = vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_OutputSize.x, m_OutputSize.y));
        m_Viewport = vk::Viewport(0.0f, 0.0f, static_cast<float>(m_OutputSize.x), static_cast<float>(m_OutputSize.y), 0, 1.0f);
        m_OutputRatio = ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y).ratioXY<float>();

        if (CreateCommanBuffer()) {
            if (CreateSyncObjects()) {
                m_Loaded = true;
            }
        }
    }

    if (m_Loaded) {
#ifdef PROFILER_INCLUDE
        m_TracyContext = TracyVkContext(m_VulkanCorePtr->getPhysicalDevice(), m_Device, m_Queue.vkQueue, m_CommandBuffers[0]);
#endif

        ActionAfterInitSucceed();
    } else {
        ActionAfterInitFail();
    }

    return m_Loaded;
}

bool BaseRenderer::InitCompute3D(const ct::uvec3& vSize) {
    ZoneScoped;

    ActionBeforeInit();

    m_Loaded = false;

    m_Device = m_VulkanCorePtr->getDevice();
    ct::uvec3 size = ct::clamp(vSize, 1u, 8192u);
    if (!size.emptyOR()) {
        m_UniformSectionToShow = {"COMPUTE"};  // pour afficher les uniforms

        m_Queue = m_VulkanCorePtr->getQueue(vk::QueueFlagBits::eGraphics);
        m_DescriptorPool = m_VulkanCorePtr->getDescriptorPool();
        m_CommandPool = m_Queue.cmdPools;

        m_OutputSize = size;
        m_RenderArea = vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_OutputSize.x, m_OutputSize.y));
        m_Viewport = vk::Viewport(0.0f, 0.0f, static_cast<float>(m_OutputSize.x), static_cast<float>(m_OutputSize.y), 0, 1.0f);
        m_OutputRatio = ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y).ratioXY<float>();

        if (CreateCommanBuffer()) {
            if (CreateSyncObjects()) {
                m_Loaded = true;
            }
        }
    }

    if (m_Loaded) {
#ifdef PROFILER_INCLUDE
        m_TracyContext = TracyVkContext(m_VulkanCorePtr->getPhysicalDevice(), m_Device, m_Queue.vkQueue, m_CommandBuffers[0]);
#endif
        ActionAfterInitSucceed();
    } else {
        ActionAfterInitFail();
    }

    return m_Loaded;
}

bool BaseRenderer::InitRtx(const ct::uvec2& vSize) {
    ZoneScoped;

    ActionBeforeInit();

    m_Loaded = false;

    m_Device = m_VulkanCorePtr->getDevice();
    ct::uvec2 size = ct::clamp(vSize, 1u, 8192u);
    if (!size.emptyOR()) {
        m_UniformSectionToShow = {"RTX"};  // pour afficher les uniforms

        m_Queue = m_VulkanCorePtr->getQueue(vk::QueueFlagBits::eGraphics);
        m_DescriptorPool = m_VulkanCorePtr->getDescriptorPool();
        m_CommandPool = m_Queue.cmdPools;

        m_OutputSize = ct::uvec3(size.x, size.y, 1);
        m_RenderArea = vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_OutputSize.x, m_OutputSize.y));
        m_Viewport = vk::Viewport(0.0f, 0.0f, static_cast<float>(m_OutputSize.x), static_cast<float>(m_OutputSize.y), 0, 1.0f);
        m_OutputRatio = ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y).ratioXY<float>();

        if (CreateCommanBuffer()) {
            if (CreateSyncObjects()) {
                m_Loaded = true;
            }
        }
    }

    if (m_Loaded) {
#ifdef PROFILER_INCLUDE
        m_TracyContext = TracyVkContext(m_VulkanCorePtr->getPhysicalDevice(), m_Device, m_Queue.vkQueue, m_CommandBuffers[0]);
#endif
        ActionAfterInitSucceed();
    } else {
        ActionAfterInitFail();
    }

    return m_Loaded;
}

void BaseRenderer::Unit() {
    ZoneScoped;

    m_Device.waitIdle();

    if (m_Loaded) {
        if (m_TracyContext) {
#ifdef PROFILER_INCLUDE
            TracyVkDestroy(m_TracyContext);
            m_TracyContext = nullptr;
#endif
        }
    }

    m_ShaderPasses.clear();
    DestroySyncObjects();
    DestroyCommanBuffer();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / GUI //////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BaseRenderer::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    bool change = false;
    for (auto pass : m_ShaderPasses) {
        auto pass_ptr = pass.lock();
        if (pass_ptr) {
            change |= pass_ptr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
        }
    }
    return change;
}

bool BaseRenderer::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    bool change = false;
    for (auto pass : m_ShaderPasses) {
        auto pass_ptr = pass.lock();
        if (pass_ptr) {
            change |= pass_ptr->DrawOverlays(vCurrentFrame, vRect, vContextPtr, vUserDatas);
        }
    }
    return change;
}

bool BaseRenderer::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    bool change = false;
    for (auto pass : m_ShaderPasses) {
        auto pass_ptr = pass.lock();
        if (pass_ptr) {
            change |= pass_ptr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
        }
    }
    return change;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / RESIZE ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BaseRenderer::NeedResizeByHand(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) {
    for (auto pass : m_ShaderPasses) {
        auto pass_ptr = pass.lock();
        if (pass_ptr) {
            pass_ptr->NeedResizeByHand(vNewSize, vCountColorBuffers);
        }
    }
}

void BaseRenderer::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) {
    for (auto pass : m_ShaderPasses) {
        auto pass_ptr = pass.lock();
        if (pass_ptr) {
            pass_ptr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / RENDER ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BaseRenderer::RenderShaderPasses(vk::CommandBuffer* vCmdBuffer) {
    if (m_MergedRendering) {
        vCmdBuffer->setViewport(0, 1, &m_Viewport);
        vCmdBuffer->setScissor(0, 1, &m_RenderArea);
    }
    for (auto pass : m_ShaderPasses) {
        auto pass_ptr = pass.lock();
        if (pass_ptr) {
            pass_ptr->DrawPass(vCmdBuffer);
        }
    }
}

void BaseRenderer::Render(const char* vSectionLabel, vk::CommandBuffer* /*vCmdBuffer*/) {
    ZoneScoped;

    if (m_CanWeRender || m_JustReseted) {
        auto cmd = GetCommandBuffer();
        if (cmd) {
            if (BeginRender(vSectionLabel)) {
                RenderShaderPasses(cmd);
                EndRender();
            }
        }
    }
}

void BaseRenderer::UpdateDescriptorsBeforeCommandBuffer() {
    for (auto pass : m_ShaderPasses) {
        auto pass_ptr = pass.lock();
        if (pass_ptr) {
            pass_ptr->UpdateRessourceDescriptor();
        }
    }
}

bool BaseRenderer::ResizeIfNeeded() {
    bool resized = false;

    if (!m_ShaderPasses.empty()) {
        for (auto pass : m_ShaderPasses) {
            auto pass_ptr = pass.lock();
            if (pass_ptr) {
                // only the last pass can be an output and must
                // be take into account for get
                // the renderarea and viewport
                resized = pass_ptr->ResizeIfNeeded();
            }
        }

        if (resized) {
            // only the last pass can be an output and must
            // be take into account for get
            // the renderarea and viewport
            auto pass_ptr = m_ShaderPasses.back().lock();
            if (pass_ptr) {
                auto fbo_ptr = pass_ptr->GetFrameBuffer().lock();
                if (fbo_ptr) {
                    m_RenderArea = fbo_ptr->GetRenderArea();
                    m_Viewport = fbo_ptr->GetViewport();
                    m_OutputRatio = fbo_ptr->GetOutputRatio();
                }
            }
        }
    }

    if (m_MergedRendering && m_VulkanCorePtr) {
        m_RenderArea = m_VulkanCorePtr->getRenderArea();
        m_Viewport = m_VulkanCorePtr->getViewport();
        m_OutputRatio = ct::fvec2((float)m_Viewport.width, (float)m_Viewport.height).ratioXY<float>();
    }

    return resized;
}

bool BaseRenderer::BeginRender(const char* vSectionLabel) {
    ZoneScoped;

    if (!m_Loaded)
        return false;

    ResizeIfNeeded();

    if (ResetFence()) {
        auto cmd = GetCommandBuffer();
        if (cmd) {
            BeginProfilerFrame("BaseRenderer");

            UpdateDescriptorsBeforeCommandBuffer();

            ResetCommandBuffer();
            BeginCommandBuffer(vSectionLabel);

            return true;
        }
    }

    return false;
}

void BaseRenderer::EndRender() {
    ZoneScoped;
    EndCommandBuffer();

    SubmitPixel();
    if (WaitFence()) {
        Swap();
    }
}

void BaseRenderer::SetMergedRendering(const bool& vMergedRendering) {
    ZoneScoped;
    m_MergedRendering = vMergedRendering;
    for (auto pass : m_ShaderPasses) {
        auto pass_ptr = pass.lock();
        if (pass_ptr) {
            pass_ptr->SetMergedRendering(vMergedRendering);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / RENDER ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BaseRenderer::ResetFence() {
    ZoneScoped;

    if (!m_Loaded)
        return false;

    return m_Device.resetFences(1, &m_WaitFences[m_CurrentFrame]) == vk::Result::eSuccess;
}

bool BaseRenderer::WaitFence() {
    ZoneScoped;

    if (!m_Loaded)
        return false;

    return m_Device.waitForFences(1, &m_WaitFences[m_CurrentFrame], VK_TRUE, UINT64_MAX) == vk::Result::eSuccess;
}

vk::CommandBuffer* BaseRenderer::GetCommandBuffer() {
    return &m_CommandBuffers[m_CurrentFrame];
}

void BaseRenderer::BeginProfilerFrame(const char* vFrameName) {
    // just for remove warning at compile time
#ifndef TRACY_ENABLE
    UNUSED(vFrameName);
#endif

#ifdef PROFILER_INCLUDE
    FrameMarkNamed(vFrameName);
#endif
}

void BaseRenderer::ResetCommandBuffer() {
    // todo : remove this func, because no more used.
    // now for better perf, we clear the command pool befaore rendering instead of command buffer per command buffer
    // auto cmd = GetCommandBuffer();
    // cmd->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
}

void BaseRenderer::BeginCommandBuffer(const char* vSectionLabel) {
    auto cmd = GetCommandBuffer();
    cmd->begin(vk::CommandBufferBeginInfo());

    if (vSectionLabel) {
        auto devicePtr = m_VulkanCorePtr->getFrameworkDevice().lock();
        if (devicePtr) {
            devicePtr->BeginDebugLabel(cmd, vSectionLabel, GENERIC_RENDERER_DEBUG_COLOR);
            m_DebugLabelWasUsed = true;
        }
    }

#ifdef PROFILER_INCLUDE
    {
        TracyVkZoneTransient(m_TracyContext, ___tracy_gpu_zone, *cmd, vSectionLabel, true);
        //TracyVkZone(m_TracyContext, *cmd, vSectionLabel); 
    }
#endif
}

void BaseRenderer::EndCommandBuffer() {
    auto cmd = GetCommandBuffer();
    if (cmd) {
        if (m_DebugLabelWasUsed) {
            auto devicePtr = m_VulkanCorePtr->getFrameworkDevice().lock();
            if (devicePtr) {
                devicePtr->EndDebugLabel(cmd);
                m_DebugLabelWasUsed = false;
            }
        }

        DoBeforeEndCommandBuffer(cmd);

#ifdef PROFILER_INCLUDE
        { TracyVkCollect(m_TracyContext, *cmd); }
#endif
        cmd->end();
    }
}

void BaseRenderer::SubmitPixel() {
    ZoneScoped;

    if (!m_Loaded)
        return;

    vk::SubmitInfo submitInfo;
    vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    submitInfo.setPWaitDstStageMask(&waitDstStageMask).setCommandBufferCount(1).setPCommandBuffers(&m_CommandBuffers[m_CurrentFrame]).setSignalSemaphoreCount(1).setPSignalSemaphores(&m_RenderCompleteSemaphores[0]);

    if (!m_FirstRender) {
        submitInfo.setWaitSemaphoreCount(1).setPWaitSemaphores(&m_RenderCompleteSemaphores[0]);
    } else {
        m_FirstRender = false;
    }

    m_FirstTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    VulkanSubmitter::Submit(m_VulkanCorePtr, vk::QueueFlagBits::eGraphics, submitInfo, m_WaitFences[m_CurrentFrame]);
}

void BaseRenderer::SubmitCompute() {
    ZoneScoped;

    if (!m_Loaded)
        return;

    vk::SubmitInfo submitInfo;
    vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eComputeShader;
    submitInfo.setPWaitDstStageMask(&waitDstStageMask).setCommandBufferCount(1).setPCommandBuffers(&m_CommandBuffers[m_CurrentFrame]).setSignalSemaphoreCount(1).setPSignalSemaphores(&m_RenderCompleteSemaphores[0]);

    if (!m_FirstRender) {
        submitInfo.setWaitSemaphoreCount(1).setPWaitSemaphores(&m_RenderCompleteSemaphores[0]);
    } else {
        m_FirstRender = false;
    }

    m_FirstTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    VulkanSubmitter::Submit(m_VulkanCorePtr, vk::QueueFlagBits::eCompute, submitInfo, m_WaitFences[m_CurrentFrame]);
}

void BaseRenderer::Swap() {
    ZoneScoped;

    if (!m_Loaded)
        return;

    m_CurrentFrame = 1 - m_CurrentFrame;

    m_SecondTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    static float deltaPrec = 1.0f / 1000.0f;  // on cree ca pour eviter de diviser, multiplier est plus low cost
    m_DeltaTime = (m_SecondTimeMark - m_FirstTimeMark) * deltaPrec;

    ++m_Frame;
    m_JustReseted = false;
}

void BaseRenderer::ResetFrame() {
    // ClearAttachments();
    m_Frame = 0;
    m_JustReseted = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / GET //////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

vk::Viewport BaseRenderer::GetViewport() const {
    ZoneScoped;

    return m_Viewport;
}

vk::Rect2D BaseRenderer::GetRenderArea() const {
    ZoneScoped;

    return m_RenderArea;
}

ct::fvec2 BaseRenderer::GetOutputSize() const {
    return ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y);
}

float BaseRenderer::GetOutputRatio() const {
    return m_OutputRatio;
}

void BaseRenderer::UpdateShaders(const std::set<std::string>& vFiles) {
    for (auto pass : m_ShaderPasses) {
        auto pass_ptr = pass.lock();
        if (pass_ptr) {
            pass_ptr->UpdateShaders(vFiles);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string BaseRenderer::getXml(const std::string& /*vOffset*/, const std::string& /*vUserDatas*/) {
    std::string str;

    // str += m_UniformWidgets.getXml(vOffset, vUserDatas);

    return str;
}

// return true for continue xml parsing of childs in this node or false for interupt the child exploration
bool BaseRenderer::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    // m_UniformWidgets.setFromXml(vElem, vParent, vUserDatas);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / COMMANDBUFFER ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BaseRenderer::DoBeforeEndCommandBuffer(vk::CommandBuffer* /*vCmdBuffer*/) {
}

bool BaseRenderer::CreateCommanBuffer() {
    ZoneScoped;

    m_CommandBuffers = m_Device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_CommandPool, vk::CommandBufferLevel::ePrimary, 2U));

    return true;
}

void BaseRenderer::DestroyCommanBuffer() {
    ZoneScoped;

    if (!m_CommandBuffers.empty()) {
        m_Device.freeCommandBuffers(m_CommandPool, m_CommandBuffers);
        m_CommandBuffers.clear();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / SYNC OBJECTS ////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BaseRenderer::CreateSyncObjects() {
    ZoneScoped;

    m_RenderCompleteSemaphores.resize(2U);
    m_WaitFences.resize(2U);
    for (size_t i = 0; i < 2U; ++i) {
        m_RenderCompleteSemaphores[i] = m_Device.createSemaphore(vk::SemaphoreCreateInfo());
        m_WaitFences[i] = m_Device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
    }

    return true;
}

void BaseRenderer::DestroySyncObjects() {
    ZoneScoped;

    for (size_t i = 0; i < 2U; ++i) {
        if (i < m_RenderCompleteSemaphores.size())
            m_Device.destroySemaphore(m_RenderCompleteSemaphores[i]);
        if (i < m_WaitFences.size())
            m_Device.destroyFence(m_WaitFences[i]);
    }

    m_RenderCompleteSemaphores.clear();
    m_WaitFences.clear();
}
