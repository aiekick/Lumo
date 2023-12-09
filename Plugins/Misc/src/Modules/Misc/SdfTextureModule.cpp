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

#include "SdfTextureModule.h"

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

#include <Modules/Misc/Pass/SdfTextureModule_Comp_Pass.h>

#include <LumoBackend/Helpers/RenderDocController.h>

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

std::shared_ptr<SdfTextureModule> SdfTextureModule::Create(GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res = std::make_shared<SdfTextureModule>(vVulkanCore);
    res->m_This = res;
    if (!res->Init()) {
        res.reset();
    }
    return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

SdfTextureModule::SdfTextureModule(GaiApi::VulkanCoreWeak vVulkanCore) : BaseRenderer(vVulkanCore) {
}

SdfTextureModule::~SdfTextureModule() {
    Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool SdfTextureModule::Init() {
    ZoneScoped;

    ct::uvec2 map_size = 512;

    m_Loaded = true;

    if (BaseRenderer::InitCompute2D(map_size)) {
        // one time update
        // will be executed when the mesh will be updated
        SetExecutionWhenNeededOnly(true);

        m_SdfTextureModule_Comp_Pass_Ptr = std::make_shared<SdfTextureModule_Comp_Pass>(m_VulkanCore);
        if (m_SdfTextureModule_Comp_Pass_Ptr) {
            if (m_SdfTextureModule_Comp_Pass_Ptr->InitCompute2D(map_size, 2U, false, vk::Format::eR32G32B32A32Sfloat)) {
                AddGenericPass(m_SdfTextureModule_Comp_Pass_Ptr);
                m_Loaded = true;
            }
        }
    }

    return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool SdfTextureModule::ExecuteAllTime(const uint32_t& /*vCurrentFrame*/, vk::CommandBuffer* vCmd, BaseNodeState* /*vBaseNodeState*/) {
    ZoneScoped;

    BaseRenderer::Render("SdfTexture", vCmd);

    return true;
}

bool SdfTextureModule::ExecuteWhenNeeded(const uint32_t& /*vCurrentFrame*/, vk::CommandBuffer* vCmd, BaseNodeState* /*vBaseNodeState*/) {
    ZoneScoped;

    BaseRenderer::Render("SdfTexture", vCmd);

    return true;
}

bool SdfTextureModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
        if (ImGui::CollapsingHeader_CheckBox("SdfTexture", -1.0f, true, true, &m_CanWeRender)) {
            bool change = false;

            if (ImGui::ContrastedButton("Upate")) {
                NeedNewExecution();
                RenderDocController::Instance()->RequestCapture();
            }

            for (auto pass : m_ShaderPasses) {
                auto passGuiPtr = dynamic_pointer_cast<GuiInterface>(pass.lock());
                if (passGuiPtr) {
                    change |= passGuiPtr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
                }
            }

            return change;
        }
    }

    return false;
}

bool SdfTextureModule::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;
}

bool SdfTextureModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
        if (RenderDocController::Instance()->IsCaptureStarted()) {
            NeedNewExecution();
        }
    }
    return false;
}

void SdfTextureModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) {
    BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void SdfTextureModule::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) {
    ZoneScoped;

    if (m_SdfTextureModule_Comp_Pass_Ptr) {
        m_SdfTextureModule_Comp_Pass_Ptr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
    }
}

vk::DescriptorImageInfo* SdfTextureModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize) {
    ZoneScoped;

    if (m_SdfTextureModule_Comp_Pass_Ptr) {
        return m_SdfTextureModule_Comp_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string SdfTextureModule::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;

    str += vOffset + "<blur_module>\n";

    str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

    if (m_SdfTextureModule_Comp_Pass_Ptr) {
        str += m_SdfTextureModule_Comp_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
    }

    str += vOffset + "</blur_module>\n";

    return str;
}

bool SdfTextureModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "blur_module") {
        if (strName == "can_we_render")
            m_CanWeRender = ct::ivariant(strValue).GetB();
    }

    if (m_SdfTextureModule_Comp_Pass_Ptr) {
        m_SdfTextureModule_Comp_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
    }

    return true;
}

void SdfTextureModule::AfterNodeXmlLoading() {
    if (m_SdfTextureModule_Comp_Pass_Ptr) {
        m_SdfTextureModule_Comp_Pass_Ptr->AfterNodeXmlLoading();
    }
}