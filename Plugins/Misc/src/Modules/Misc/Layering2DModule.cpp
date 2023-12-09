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

#include "Layering2DModule.h"

#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <cinttypes>
#include <Gaia/Buffer/FrameBuffer.h>

#include <Modules/Misc/Pass/Layering2DModule_Comp_Pass.h>

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

std::shared_ptr<Layering2DModule> Layering2DModule::Create(GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res = std::make_shared<Layering2DModule>(vVulkanCore);
    res->m_This = res;
    if (!res->Init()) {
        res.reset();
    }
    return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

Layering2DModule::Layering2DModule(GaiApi::VulkanCoreWeak vVulkanCore) : BaseRenderer(vVulkanCore) {
}

Layering2DModule::~Layering2DModule() {
    Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool Layering2DModule::Init() {
    ZoneScoped;

    ct::uvec2 map_size = 512;

    m_Loaded = false;

    if (BaseRenderer::InitCompute2D(map_size)) {
        m_Layering2DModule_Comp_Pass_Ptr = std::make_shared<Layering2DModule_Comp_Pass>(m_VulkanCore);
        if (m_Layering2DModule_Comp_Pass_Ptr) {
            // will be resized ot input size
            m_Layering2DModule_Comp_Pass_Ptr->AllowResizeOnResizeEvents(false);
            m_Layering2DModule_Comp_Pass_Ptr->AllowResizeByHandOrByInputs(true);

            if (m_Layering2DModule_Comp_Pass_Ptr->InitCompute2D(map_size, 1U, false, vk::Format::eR32G32B32A32Sfloat)) {
                AddGenericPass(m_Layering2DModule_Comp_Pass_Ptr);
                m_Loaded = true;
            }
        }
    }

    return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool Layering2DModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    ZoneScoped;

    BaseRenderer::Render("2D Layering", vCmd);

    return true;
}

bool Layering2DModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
        if (ImGui::CollapsingHeader_CheckBox("2D Layering", -1.0f, true, true, &m_CanWeRender)) {
            bool change = false;

            if (m_Layering2DModule_Comp_Pass_Ptr) {
                change |= m_Layering2DModule_Comp_Pass_Ptr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
            }

            return change;
        }
    }

    return false;
}

bool Layering2DModule::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;
}

bool Layering2DModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;
}

void Layering2DModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) {
    BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void Layering2DModule::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) {
    ZoneScoped;

    if (m_Layering2DModule_Comp_Pass_Ptr) {
        m_Layering2DModule_Comp_Pass_Ptr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
    }
}

vk::DescriptorImageInfo* Layering2DModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize) {
    ZoneScoped;

    if (m_Layering2DModule_Comp_Pass_Ptr) {
        return m_Layering2DModule_Comp_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string Layering2DModule::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;

    str += vOffset + "<layering_2d_module>\n";

    str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

    if (m_Layering2DModule_Comp_Pass_Ptr) {
        str += m_Layering2DModule_Comp_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
    }

    str += vOffset + "</layering_2d_module>\n";

    return str;
}

bool Layering2DModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "layering_2d_module") {
        if (strName == "can_we_render")
            m_CanWeRender = ct::ivariant(strValue).GetB();
    }

    if (m_Layering2DModule_Comp_Pass_Ptr) {
        m_Layering2DModule_Comp_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
    }

    return true;
}