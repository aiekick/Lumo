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

#include "SSReflectionModule.h"

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

#include <Modules/ScreenSpace/Effects/Pass/SSReflectionModule_Comp_2D_Pass.h>

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

std::shared_ptr<SSReflectionModule> SSReflectionModule::Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode) {
    ZoneScoped;
    auto res = std::make_shared<SSReflectionModule>(vVulkanCore);
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

SSReflectionModule::SSReflectionModule(GaiApi::VulkanCoreWeak vVulkanCore) : BaseRenderer(vVulkanCore) {
    ZoneScoped;
}

SSReflectionModule::~SSReflectionModule() {
    ZoneScoped;

    Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool SSReflectionModule::Init() {
    ZoneScoped;

    m_Loaded = false;

    ct::uvec2 map_size = 512;
    if (BaseRenderer::InitCompute2D(map_size)) {
        // SetExecutionWhenNeededOnly(true);
        m_SSReflectionModule_Comp_2D_Pass_Ptr = SSReflectionModule_Comp_2D_Pass::Create(map_size, m_VulkanCore);
        if (m_SSReflectionModule_Comp_2D_Pass_Ptr) {
            // by default but can be changed via widget
            m_SSReflectionModule_Comp_2D_Pass_Ptr->AllowResizeOnResizeEvents(true);
            m_SSReflectionModule_Comp_2D_Pass_Ptr->AllowResizeByHandOrByInputs(false);
            AddGenericPass(m_SSReflectionModule_Comp_2D_Pass_Ptr);
            m_Loaded = true;
        }
    }

    return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool SSReflectionModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    ZoneScoped;
    BaseRenderer::Render("SS Reflection", vCmd);
    return true;
}

bool SSReflectionModule::ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    ZoneScoped;
    BaseRenderer::Render("SS Reflection", vCmd);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool SSReflectionModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
        if (m_SSReflectionModule_Comp_2D_Pass_Ptr) {
            return m_SSReflectionModule_Comp_2D_Pass_Ptr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
        }
    }

    return false;
}

bool SSReflectionModule::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    if (m_LastExecutedFrame == vCurrentFrame) {
        if (m_SSReflectionModule_Comp_2D_Pass_Ptr) {
            return m_SSReflectionModule_Comp_2D_Pass_Ptr->DrawOverlays(vCurrentFrame, vRect, vContextPtr, vUserDatas);
        }
    }

    return false;
}

bool SSReflectionModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    if (m_LastExecutedFrame == vCurrentFrame) {
        if (m_SSReflectionModule_Comp_2D_Pass_Ptr) {
            return m_SSReflectionModule_Comp_2D_Pass_Ptr->DrawDialogsAndPopups(vCurrentFrame, vMaxRect, vContextPtr, vUserDatas);
        }
    }

    return false;
}

void SSReflectionModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) {
    ZoneScoped;

    // do some code

    BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void SSReflectionModule::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
    ZoneScoped;

    if (m_SSReflectionModule_Comp_2D_Pass_Ptr) {
        m_SSReflectionModule_Comp_2D_Pass_Ptr->SetTexture(vBindingPoint, vImageInfo, vTextureSize, vUserDatas);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* SSReflectionModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    ZoneScoped;

    if (m_SSReflectionModule_Comp_2D_Pass_Ptr) {
        return m_SSReflectionModule_Comp_2D_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize, vUserDatas);
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string SSReflectionModule::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;
    std::string str;
    str += vOffset + "<ss_reflection_module>\n";
    str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";
    if (m_SSReflectionModule_Comp_2D_Pass_Ptr) {
        str += m_SSReflectionModule_Comp_2D_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
    }
    str += vOffset + "</ss_reflection_module>\n";
    return str;
}

bool SSReflectionModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    ZoneScoped;

    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "ss_reflection_module") {
        if (strName == "can_we_render") {
            m_CanWeRender = ct::ivariant(strValue).GetB();
        }
    }

    if (m_SSReflectionModule_Comp_2D_Pass_Ptr) {
        m_SSReflectionModule_Comp_2D_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
    }

    return true;
}

void SSReflectionModule::AfterNodeXmlLoading() {
    ZoneScoped;
    if (m_SSReflectionModule_Comp_2D_Pass_Ptr) {
        m_SSReflectionModule_Comp_2D_Pass_Ptr->AfterNodeXmlLoading();
    }
}
