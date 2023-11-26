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

#include "PostProcessingModule.h"

#include <cinttypes>
#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <ImGuiPack.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Gaia/Buffer/FrameBuffer.h>
#include <LumoBackend/Interfaces/EffectInterface.h>

#include <Modules/PostPro/Effects/Pass/BloomModule_Comp_2D_Pass.h>
#include <Modules/PostPro/Effects/Pass/BlurModule_Comp_2D_Pass.h>
#include <Modules/PostPro/Effects/Pass/ChromaticAberrationsModule_Comp_2D_Pass.h>
#include <Modules/PostPro/Effects/Pass/DilationModule_Comp_2D_Pass.h>
#include <Modules/PostPro/Effects/Pass/SharpnessModule_Comp_2D_Pass.h>
#include <Modules/PostPro/Effects/Pass/ToneMapModule_Comp_2D_Pass.h>
#include <Modules/PostPro/Effects/Pass/VignetteModule_Comp_2D_Pass.h>

using namespace GaiApi;

#ifdef PROFILER_INCLUDE
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<PostProcessingModule> PostProcessingModule::Create(GaiApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode) {
    ZoneScoped;
    if (!vVulkanCorePtr) {
        return nullptr;
    }
    auto res = std::make_shared<PostProcessingModule>(vVulkanCorePtr);
    res->SetParentNode(vParentNode);
    res->m_This = res;
    if (!res->Init()) {
        res.reset();
    }
    return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

PostProcessingModule::PostProcessingModule(GaiApi::VulkanCorePtr vVulkanCorePtr) : BaseRenderer(vVulkanCorePtr) {
    ZoneScoped;
}

PostProcessingModule::~PostProcessingModule() {
    ZoneScoped;
    Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool PostProcessingModule::Init() {
    ZoneScoped;
    ct::uvec2 map_size = 512;
    if (BaseRenderer::InitCompute2D(map_size)) {
        m_BloomModule_Comp_2D_Pass_Ptr = BloomModule_Comp_2D_Pass::Create(map_size, m_VulkanCorePtr);
        if (m_BloomModule_Comp_2D_Pass_Ptr) {
            m_BloomModule_Comp_2D_Pass_Ptr->EnableEffect(false);
            m_BlurModule_Comp_2D_Pass_Ptr = BlurModule_Comp_2D_Pass::Create(map_size, m_VulkanCorePtr);
            if (m_BlurModule_Comp_2D_Pass_Ptr) {
                m_BlurModule_Comp_2D_Pass_Ptr->EnableEffect(false);
                m_SharpnessModule_Comp_2D_Pass_Ptr = SharpnessModule_Comp_2D_Pass::Create(map_size, m_VulkanCorePtr);
                if (m_SharpnessModule_Comp_2D_Pass_Ptr) {
                    m_SharpnessModule_Comp_2D_Pass_Ptr->EnableEffect(false);
                    m_ChromaticAberrationsModule_Comp_2D_Pass_Ptr = ChromaticAberrationsModule_Comp_2D_Pass::Create(map_size, m_VulkanCorePtr);
                    if (m_ChromaticAberrationsModule_Comp_2D_Pass_Ptr) {
                        m_ChromaticAberrationsModule_Comp_2D_Pass_Ptr->EnableEffect(false);
                        m_DilationModule_Comp_2D_Pass_Ptr = DilationModule_Comp_2D_Pass::Create(map_size, m_VulkanCorePtr);
                        if (m_DilationModule_Comp_2D_Pass_Ptr) {
                            m_DilationModule_Comp_2D_Pass_Ptr->EnableEffect(false);
                            m_ToneMapModule_Comp_2D_Pass_Ptr = ToneMapModule_Comp_2D_Pass::Create(map_size, m_VulkanCorePtr);
                            if (m_ToneMapModule_Comp_2D_Pass_Ptr) {
                                m_ToneMapModule_Comp_2D_Pass_Ptr->EnableEffect(false);
                                m_VignetteModule_Comp_2D_Pass_Ptr = VignetteModule_Comp_2D_Pass::Create(map_size, m_VulkanCorePtr);
                                if (m_VignetteModule_Comp_2D_Pass_Ptr) {
                                    m_VignetteModule_Comp_2D_Pass_Ptr->EnableEffect(false);
                                    AddGenericPass(m_BloomModule_Comp_2D_Pass_Ptr);                 // 1) BLOOM
                                    AddGenericPass(m_BlurModule_Comp_2D_Pass_Ptr);                  // 2) BLUR
                                    AddGenericPass(m_SharpnessModule_Comp_2D_Pass_Ptr);             // 3) SHARPNESS
                                    AddGenericPass(m_ChromaticAberrationsModule_Comp_2D_Pass_Ptr);  // 4) CHROMATIC ABERRATION
                                    AddGenericPass(m_DilationModule_Comp_2D_Pass_Ptr);              // 5) DILATION
                                    AddGenericPass(m_ToneMapModule_Comp_2D_Pass_Ptr);               // 6) TONE MAPPING
                                    AddGenericPass(m_VignetteModule_Comp_2D_Pass_Ptr);              // 7) VIGNETTE
                                    m_Loaded = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool PostProcessingModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    ZoneScoped;
    BaseRenderer::Render("Post Processing", vCmd);
    return true;
}

bool PostProcessingModule::ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    ZoneScoped;
    BaseRenderer::Render("Post Processing", vCmd);
    return true;
}

void PostProcessingModule::UpdateDescriptorsBeforeCommandBuffer() {
    // will connect active effects is there is at least two active effects 
    m_FirstPassPtr = nullptr;
    m_LastPassPtr = nullptr;
    std::shared_ptr<TextureOutputInterface> _lastActivePassPtr = nullptr;
    for (auto pass : m_ShaderPasses) {
        auto pass_ptr = pass.lock();
        if (pass_ptr != nullptr) {
            auto effect_ptr = std::dynamic_pointer_cast<EffectInterface>(pass_ptr);
            if (effect_ptr != nullptr && effect_ptr->IsEffectEnabled() != nullptr && *effect_ptr->IsEffectEnabled()) {
                auto texture_input_ptr = std::dynamic_pointer_cast<TextureInputFunctions>(pass_ptr);
                if (texture_input_ptr != nullptr) {
                    if (_lastActivePassPtr != nullptr) {
                        ct::fvec2 outSize;  // must be empty if not will create a resize on the ouput of the next pass
                        auto outputTexture = _lastActivePassPtr->GetDescriptorImageInfo(0U, &outSize);
                        texture_input_ptr->SetTexture(0U, outputTexture, &outSize);
                    }
                }
                _lastActivePassPtr = std::dynamic_pointer_cast<TextureOutputInterface>(pass_ptr);
                if (m_FirstPassPtr == nullptr) {
                    m_FirstPassPtr = texture_input_ptr;
                }
                m_LastPassPtr = _lastActivePassPtr;
            }
        }
    }

    BaseRenderer::UpdateDescriptorsBeforeCommandBuffer();
}

void PostProcessingModule::RenderShaderPasses(vk::CommandBuffer* vCmdBufferPtr) {
    // 1) BLOOM
    if (m_BloomModule_Comp_2D_Pass_Ptr->IsEffectEnabled() && //
        *m_BloomModule_Comp_2D_Pass_Ptr->IsEffectEnabled()) {
        m_BloomModule_Comp_2D_Pass_Ptr->DrawPass(vCmdBufferPtr);
        vCmdBufferPtr->pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(),
            vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead), nullptr, nullptr);
    }

    // 2) BLUR
    if (m_BlurModule_Comp_2D_Pass_Ptr->IsEffectEnabled() && //
        *m_BlurModule_Comp_2D_Pass_Ptr->IsEffectEnabled()) {
        m_BlurModule_Comp_2D_Pass_Ptr->DrawPass(vCmdBufferPtr);
        vCmdBufferPtr->pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(),
            vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead), nullptr, nullptr);
    }

    // 3) SHARPNESS
    if (m_SharpnessModule_Comp_2D_Pass_Ptr->IsEffectEnabled() && //
        *m_SharpnessModule_Comp_2D_Pass_Ptr->IsEffectEnabled()) {
        m_SharpnessModule_Comp_2D_Pass_Ptr->DrawPass(vCmdBufferPtr);
        vCmdBufferPtr->pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(),
            vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead), nullptr, nullptr);
    }

    // 4) CHROMATIC ABERRATION
    if (m_ChromaticAberrationsModule_Comp_2D_Pass_Ptr->IsEffectEnabled() && //
        *m_ChromaticAberrationsModule_Comp_2D_Pass_Ptr->IsEffectEnabled()) {
        m_ChromaticAberrationsModule_Comp_2D_Pass_Ptr->DrawPass(vCmdBufferPtr);
        vCmdBufferPtr->pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(),
            vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead), nullptr, nullptr);
    }

    // 5) DILATION
    if (m_DilationModule_Comp_2D_Pass_Ptr->IsEffectEnabled() && //
        *m_DilationModule_Comp_2D_Pass_Ptr->IsEffectEnabled()) {
        m_DilationModule_Comp_2D_Pass_Ptr->DrawPass(vCmdBufferPtr);
        vCmdBufferPtr->pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(),
            vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead), nullptr, nullptr);
    }

    // 6) TONE MAPPING
    if (m_ToneMapModule_Comp_2D_Pass_Ptr->IsEffectEnabled() && //
        *m_ToneMapModule_Comp_2D_Pass_Ptr->IsEffectEnabled()) {
        m_ToneMapModule_Comp_2D_Pass_Ptr->DrawPass(vCmdBufferPtr);
        vCmdBufferPtr->pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(),
            vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead), nullptr, nullptr);
    }

    // 7) VIGNETTE
    if (m_VignetteModule_Comp_2D_Pass_Ptr->IsEffectEnabled() && //
        *m_VignetteModule_Comp_2D_Pass_Ptr->IsEffectEnabled()) {
        m_VignetteModule_Comp_2D_Pass_Ptr->DrawPass(vCmdBufferPtr);
        vCmdBufferPtr->pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(),
            vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead), nullptr, nullptr);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool PostProcessingModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    if (m_LastExecutedFrame == vCurrentFrame) {
        if (ImGui::CollapsingHeader_CheckBox("Post Processing##PostProcessingModule", -1.0f, true, true, &m_CanWeRender)) {
            bool change = false;
            for (auto pass : m_ShaderPasses) {
                auto ptr = pass.lock();
                if (ptr) {
                    change |= ptr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
                }
            }
            return change;
        }
    }
    return false;
}

bool PostProcessingModule::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    if (m_LastExecutedFrame == vCurrentFrame) {
        bool change = false;
        for (auto pass : m_ShaderPasses) {
            auto ptr = pass.lock();
            if (ptr) {
                change |= ptr->DrawOverlays(vCurrentFrame, vRect, vContextPtr, vUserDatas);
            }
        }
        return change;
    }
    return false;
}

bool PostProcessingModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    if (m_LastExecutedFrame == vCurrentFrame) {
        bool change = false;
        for (auto pass : m_ShaderPasses) {
            auto ptr = pass.lock();
            if (ptr) {
                change |= ptr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
            }
        }
        return change;
    }
    return false;
}

void PostProcessingModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) {
    ZoneScoped;
    BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void PostProcessingModule::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) {
    ZoneScoped;
    if (vBindingPoint == 0U) {  // color
        // will set the color to the input of the first active effect
        if (m_FirstPassPtr != nullptr) {
            m_FirstPassPtr->SetTexture(0U, vImageInfo, vTextureSize);
        } 
        // in case there is no enabled effects
        if (vBindingPoint == 0 && vImageInfo != nullptr) {
            m_ImageInfos[0] = *vImageInfo;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* PostProcessingModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize) {
    ZoneScoped;

    // will get the output of the last active effect
    if (m_LastPassPtr != nullptr) {
        return m_LastPassPtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
    } else if (vBindingPoint == 0) { // no enabled effect, this module is passing
        return &m_ImageInfos[0];
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string PostProcessingModule::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;
    std::string str;
    str += vOffset + "<post_processing_module>\n";
    str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";
    for (auto pass : m_ShaderPasses) {
        auto ptr = pass.lock();
        if (ptr) {
            str += ptr->getXml(vOffset + "\t", vUserDatas);
        }
    }
    str += vOffset + "</post_processing_module>\n";
    return str;
}

bool PostProcessingModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    ZoneScoped;
    std::string strName;
    std::string strValue;
    std::string strParentName;
    strName = vElem->Value();
    if (vElem->GetText()) {
        strValue = vElem->GetText();
    }
    if (vParent != nullptr) {
        strParentName = vParent->Value();
    }
    if (strParentName == "post_processing_module") {
        if (strName == "can_we_render") {
            m_CanWeRender = ct::ivariant(strValue).GetB();
        }
    }
    for (auto pass : m_ShaderPasses) {
        auto ptr = pass.lock();
        if (ptr) {
            ptr->setFromXml(vElem, vParent, vUserDatas);
        }
    }
    return true;
}

void PostProcessingModule::AfterNodeXmlLoading() {
    ZoneScoped;
    m_BloomModule_Comp_2D_Pass_Ptr->AfterNodeXmlLoading();
    m_BlurModule_Comp_2D_Pass_Ptr->AfterNodeXmlLoading();
    m_DilationModule_Comp_2D_Pass_Ptr->AfterNodeXmlLoading();
    m_ChromaticAberrationsModule_Comp_2D_Pass_Ptr->AfterNodeXmlLoading();
    m_SharpnessModule_Comp_2D_Pass_Ptr->AfterNodeXmlLoading();
    m_ToneMapModule_Comp_2D_Pass_Ptr->AfterNodeXmlLoading();
    m_VignetteModule_Comp_2D_Pass_Ptr->AfterNodeXmlLoading();
}
