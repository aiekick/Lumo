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

#include "PosToDepthModule.h"

#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Modules/Utils/Pass/PosToDepthModule_Quad_Pass.h>

using namespace GaiApi;

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<PosToDepthModule> PosToDepthModule::Create(GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res = std::make_shared<PosToDepthModule>(vVulkanCore);
    res->m_This = res;
    if (!res->Init()) {
        res.reset();
    }
    return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

PosToDepthModule::PosToDepthModule(GaiApi::VulkanCoreWeak vVulkanCore) : BaseRenderer(vVulkanCore) {
}

PosToDepthModule::~PosToDepthModule() {
    Unit();
}

//////////////////////////////////////////////////////////////
//// INIT ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool PosToDepthModule::Init() {
    ZoneScoped;

    ct::uvec2 map_size = 512;

    m_Loaded = true;

    if (BaseRenderer::InitPixel(map_size)) {
        m_PosToDepthModule_Quad_Pass_Ptr = std::make_shared<PosToDepthModule_Quad_Pass>(m_VulkanCore);
        if (m_PosToDepthModule_Quad_Pass_Ptr) {
            if (m_PosToDepthModule_Quad_Pass_Ptr->InitPixel(
                    map_size, 1U, true, true, 0.0f, false, false, vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1)) {
                AddGenericPass(m_PosToDepthModule_Quad_Pass_Ptr);
                m_Loaded = true;
            }
        }
    }

    return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool PosToDepthModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    ZoneScoped;

    BaseRenderer::Render("Pos To Depth Module", vCmd);

    return true;
}

bool PosToDepthModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
        if (ImGui::CollapsingHeader_CheckBox("Depth To Pos", -1.0f, true, true, &m_CanWeRender)) {
            if (m_PosToDepthModule_Quad_Pass_Ptr) {
                return m_PosToDepthModule_Quad_Pass_Ptr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
            }
        }
    }

    return false;
}

bool PosToDepthModule::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;
}

bool PosToDepthModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;
}

void PosToDepthModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) {
    BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void PosToDepthModule::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
    ZoneScoped;

    if (m_PosToDepthModule_Quad_Pass_Ptr) {
        m_PosToDepthModule_Quad_Pass_Ptr->SetTexture(vBindingPoint, vImageInfo, vTextureSize, vUserDatas);
    }
}

vk::DescriptorImageInfo* PosToDepthModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    ZoneScoped;

    if (m_PosToDepthModule_Quad_Pass_Ptr) {
        return m_PosToDepthModule_Quad_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize, vUserDatas);
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string PosToDepthModule::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;

    return str;
}

bool PosToDepthModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    return true;
}
