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

#include "SmoothNormalModule.h"

#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <cinttypes>
#include <Gaia/Buffer/FrameBuffer.h>

#include <Modules/Modifiers/Pass/SmoothNormalModule_Comp_Pass.h>

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

std::shared_ptr<SmoothNormalModule> SmoothNormalModule::Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode) {
    auto res = std::make_shared<SmoothNormalModule>(vVulkanCore);
    res->m_This = res;
    res->SetParentNode(vParentNode);
    if (!res->Init()) {
        res.reset();
    }
    return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

SmoothNormalModule::SmoothNormalModule(GaiApi::VulkanCoreWeak vVulkanCore) : BaseRenderer(vVulkanCore) {
}

SmoothNormalModule::~SmoothNormalModule() {
    Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool SmoothNormalModule::Init() {
    ZoneScoped;

    ct::uvec3 map_size = 1;

    m_Loaded = true;

    // one time update
    // will be executed when the mesh will be updated
    SetExecutionWhenNeededOnly(true);

    if (BaseRenderer::InitCompute3D(map_size)) {
        m_SmoothNormalModule_Comp_Pass_Ptr = std::make_shared<SmoothNormalModule_Comp_Pass>(m_VulkanCore);
        if (m_SmoothNormalModule_Comp_Pass_Ptr) {
            if (m_SmoothNormalModule_Comp_Pass_Ptr->InitCompute3D(map_size)) {
                AddGenericPass(m_SmoothNormalModule_Comp_Pass_Ptr);
                m_Loaded = true;
            }
        }
    }

    return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool SmoothNormalModule::ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    ZoneScoped;

    BaseRenderer::Render("Smooth Normal when mesh update", vCmd);

    // mesh was updated, we notify the parent here
    // becasue this execution is event based
    auto parentNodePtr = GetParentNode().lock();
    if (parentNodePtr) {
        parentNodePtr->SendFrontNotification(ModelUpdateDone);
    }

    m_LastExecutedFrame = vCurrentFrame;

    return true;
}

bool SmoothNormalModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
        if (ImGui::CollapsingHeader_CheckBox("Smooth Normal when mesh update", -1.0f, true, true, &m_CanWeRender)) {
            bool change = false;

            ImGui::Header("Computation");

            if (ImGui::ContrastedButton("Force Update")) {
                NeedNewExecution();
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

bool SmoothNormalModule::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;
}

bool SmoothNormalModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;
}

void SmoothNormalModule::SetModel(SceneModelWeak vSceneModel) {
    if (m_SmoothNormalModule_Comp_Pass_Ptr) {
        m_SmoothNormalModule_Comp_Pass_Ptr->SetModel(vSceneModel);

        NeedNewExecution();  // just updated so need a normal smoothing
    }
}

SceneModelWeak SmoothNormalModule::GetModel() {
    if (m_SmoothNormalModule_Comp_Pass_Ptr) {
        return m_SmoothNormalModule_Comp_Pass_Ptr->GetModel();
    }

    return SceneModelWeak();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string SmoothNormalModule::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;

    return str;
}

bool SmoothNormalModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    return true;
}