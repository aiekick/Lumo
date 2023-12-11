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

#include "ShadowMapNode.h"
#include <Modules/Lighting/ShadowMapModule.h>
#include <LumoBackend/Interfaces/ModelOutputInterface.h>
#include <LumoBackend/Graph/Slots/NodeSlotLightGroupInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotLightGroupOutput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTexture2DInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotModelInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureGroupInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureGroupOutput.h>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

std::shared_ptr<ShadowMapNode> ShadowMapNode::Create(GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res = std::make_shared<ShadowMapNode>();
    res->m_This = res;
    if (!res->Init(vVulkanCore)) {
        res.reset();
    }
    return res;
}

ShadowMapNode::ShadowMapNode() : BaseNode() {
    m_NodeTypeString = "SHADOW_MAPPING";
}

ShadowMapNode::~ShadowMapNode() {
    Unit();
}

bool ShadowMapNode::Init(GaiApi::VulkanCoreWeak vVulkanCore) {
    name = "Shadow Map";

    AddInput(NodeSlotModelInput::Create("Model"), true, false);
    AddInput(NodeSlotLightGroupInput::Create("Lights"), true, false);
    AddOutput(NodeSlotLightGroupOutput::Create("Lights"), true, false);
    AddOutput(NodeSlotTextureGroupOutput::Create("Outputs"), true, false);

    bool res = false;
    m_ShadowMapModulePtr = ShadowMapModule::Create(vVulkanCore);
    if (m_ShadowMapModulePtr) {
        res = true;
    }

    return res;
}

void ShadowMapNode::Unit() {
    m_ShadowMapModulePtr.reset();
}

bool ShadowMapNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

    if (m_ShadowMapModulePtr) {
        return m_ShadowMapModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
    }
    return false;
}

bool ShadowMapNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_ShadowMapModulePtr) {
        return m_ShadowMapModulePtr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
    }

    return false;
}

bool ShadowMapNode::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_ShadowMapModulePtr) {
        return m_ShadowMapModulePtr->DrawOverlays(vCurrentFrame, vRect, vContextPtr, vUserDatas);
    }
    return false;
}

bool ShadowMapNode::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_ShadowMapModulePtr) {
        return m_ShadowMapModulePtr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
    }
    return false;
}

void ShadowMapNode::SetModel(SceneModelWeak vSceneModel) {
    if (m_ShadowMapModulePtr) {
        m_ShadowMapModulePtr->SetModel(vSceneModel);
    }
}

void ShadowMapNode::SetLightGroup(SceneLightGroupWeak vSceneLightGroup) {
    if (m_ShadowMapModulePtr) {
        m_ShadowMapModulePtr->SetLightGroup(vSceneLightGroup);

        SendFrontNotification(LightGroupUpdateDone);
    }
}

SceneLightGroupWeak ShadowMapNode::GetLightGroup() {
    if (m_ShadowMapModulePtr) {
        return m_ShadowMapModulePtr->GetLightGroup();
    }

    return SceneLightGroupWeak();
}

DescriptorImageInfoVector* ShadowMapNode::GetDescriptorImageInfos(const uint32_t& vBindingPoint, fvec2Vector* vOutSizes) {
    if (m_ShadowMapModulePtr) {
        return m_ShadowMapModulePtr->GetDescriptorImageInfos(vBindingPoint, vOutSizes);
    }

    return nullptr;
}

void ShadowMapNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) {
    if (m_ShadowMapModulePtr) {
        m_ShadowMapModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
    }

    // on fait ca apres
    BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string ShadowMapNode::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string res;

    if (!m_ChildNodes.empty()) {
        res += BaseNode::getXml(vOffset, vUserDatas);
    } else {
        res += vOffset + ct::toStr("<node name=\"%s\" type=\"%s\" pos=\"%s\" id=\"%u\">\n", name.c_str(), m_NodeTypeString.c_str(),
                             ct::fvec2(pos.x, pos.y).string().c_str(), (uint32_t)GetNodeID());

        for (auto slot : m_Inputs) {
            res += slot.second->getXml(vOffset + "\t", vUserDatas);
        }

        for (auto slot : m_Outputs) {
            res += slot.second->getXml(vOffset + "\t", vUserDatas);
        }

        if (m_ShadowMapModulePtr) {
            res += m_ShadowMapModulePtr->getXml(vOffset + "\t", vUserDatas);
        }

        res += vOffset + "</node>\n";
    }

    return res;
}

bool ShadowMapNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    BaseNode::setFromXml(vElem, vParent, vUserDatas);

    if (m_ShadowMapModulePtr) {
        m_ShadowMapModulePtr->setFromXml(vElem, vParent, vUserDatas);
    }

    return true;
}

void ShadowMapNode::UpdateShaders(const std::set<std::string>& vFiles) {
    if (m_ShadowMapModulePtr) {
        m_ShadowMapModulePtr->UpdateShaders(vFiles);
    }
}