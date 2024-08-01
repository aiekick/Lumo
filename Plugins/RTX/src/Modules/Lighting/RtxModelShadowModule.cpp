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

#include "RtxModelShadowModule.h"

#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImGuiPack.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Modules/Lighting/Pass/ModelShadow_Rtx_Pass.h>

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

std::shared_ptr<RtxModelShadowModule> RtxModelShadowModule::Create(GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res = std::make_shared<RtxModelShadowModule>(vVulkanCore);
    res->m_This = res;
    if (!res->Init()) {
        res.reset();
    }
    return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

RtxModelShadowModule::RtxModelShadowModule(GaiApi::VulkanCoreWeak vVulkanCore) : BaseRenderer(vVulkanCore) {
}

RtxModelShadowModule::~RtxModelShadowModule() {
    Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool RtxModelShadowModule::Init() {
    ZoneScoped;

    ct::uvec2 map_size = 512;

    m_Loaded = true;

    if (BaseRenderer::InitRtx(map_size)) {
        m_ModelShadow_Rtx_Pass_Ptr = std::make_shared<ModelShadow_Rtx_Pass>(m_VulkanCore);
        if (m_ModelShadow_Rtx_Pass_Ptr) {
            if (m_ModelShadow_Rtx_Pass_Ptr->InitRtx(map_size, 1U, false, vk::Format::eR32G32B32A32Sfloat)) {
                AddGenericPass(m_ModelShadow_Rtx_Pass_Ptr);
                m_Loaded = true;
            }
        }
    }
    return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool RtxModelShadowModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    ZoneScoped;

    BaseRenderer::Render("Rtx Model Shadow", vCmd);

    return true;
}

bool RtxModelShadowModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
        if (ImGui::CollapsingHeader_CheckBox("Rtx Model Shadow", -1.0f, true, true, &m_CanWeRender)) {
            if (m_ModelShadow_Rtx_Pass_Ptr) {
                return m_ModelShadow_Rtx_Pass_Ptr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
            }
        }
    }

    return false;
}

bool RtxModelShadowModule::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;
}

bool RtxModelShadowModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;
}

void RtxModelShadowModule::SetAccelStructure(SceneAccelStructureWeak vSceneAccelStructure) {
    if (m_ModelShadow_Rtx_Pass_Ptr) {
        m_ModelShadow_Rtx_Pass_Ptr->SetAccelStructure(vSceneAccelStructure);
    }
}

vk::DescriptorImageInfo* RtxModelShadowModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    if (m_ModelShadow_Rtx_Pass_Ptr) {
        return m_ModelShadow_Rtx_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize, vUserDatas);
    }

    return nullptr;
}

void RtxModelShadowModule::SetLightGroup(SceneLightGroupWeak vSceneLightGroup) {
    if (m_ModelShadow_Rtx_Pass_Ptr) {
        m_ModelShadow_Rtx_Pass_Ptr->SetLightGroup(vSceneLightGroup);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string RtxModelShadowModule::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;

    str += vOffset + "<rtx_model_shadow_module>\n";

    str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

    if (m_ModelShadow_Rtx_Pass_Ptr) {
        str += m_ModelShadow_Rtx_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
    }

    str += vOffset + "</rtx_model_shadow_module>\n";

    return str;
}

bool RtxModelShadowModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "rtx_model_shadow_module") {
        if (strName == "can_we_render")
            m_CanWeRender = ct::ivariant(strValue).GetB();
    }

    if (m_ModelShadow_Rtx_Pass_Ptr) {
        m_ModelShadow_Rtx_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
    }

    return true;
}
