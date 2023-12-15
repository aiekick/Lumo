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

#include "SmoothNormalNode.h"
#include <Modules/Modifiers/SmoothNormalModule.h>
#include <LumoBackend/Graph/Slots/NodeSlotModelInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotModelOutput.h>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

std::shared_ptr<SmoothNormalNode> SmoothNormalNode::Create(GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res = std::make_shared<SmoothNormalNode>();
    res->m_This = res;
    if (!res->Init(vVulkanCore)) {
        res.reset();
    }
    return res;
}

SmoothNormalNode::SmoothNormalNode() : BaseNode() {
    m_NodeTypeString = "SMOOTH_NORMAL";
}

SmoothNormalNode::~SmoothNormalNode() {
}

bool SmoothNormalNode::Init(GaiApi::VulkanCoreWeak vVulkanCore) {
    name = "Smooth Normal";

    AddInput(NodeSlotModelInput::Create("Model"), true, true);
    AddOutput(NodeSlotModelOutput::Create("Mesh"), true, true);

    // we keep this node in ExecuteAllTime, because we need to propagate to inputs for each frames

    m_SmoothNormalModulePtr = SmoothNormalModule::Create(vVulkanCore, m_This);
    if (m_SmoothNormalModulePtr) {
        return true;
    }

    return false;
}

bool SmoothNormalNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

    if (m_SmoothNormalModulePtr) {
        return m_SmoothNormalModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
    }
    return false;
}

bool SmoothNormalNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_SmoothNormalModulePtr) {
        return m_SmoothNormalModulePtr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
    }

    return false;
}

bool SmoothNormalNode::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool SmoothNormalNode::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_SmoothNormalModulePtr) {
        return m_SmoothNormalModulePtr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
    }
    return false;
}

void SmoothNormalNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) {
    if (vBaseNodeState && vBaseNodeState->debug_mode) {
        auto drawList = nd::GetNodeBackgroundDrawList(nodeID);
        if (drawList) {
            char debugBuffer[255] = "\0";
            snprintf(debugBuffer, 254, "Used(%s)\nCell(%i, %i)" /*\nPos(%.1f, %.1f)\nSize(%.1f, %.1f)*/, (used ? "true" : "false"), cell.x,
                cell.y /*, pos.x, pos.y, size.x, size.y*/);
            ImVec2 txtSize = ImGui::CalcTextSize(debugBuffer);
            drawList->AddText(pos - ImVec2(0, txtSize.y), ImGui::GetColorU32(ImGuiCol_Text), debugBuffer);
        }
    }
}

void SmoothNormalNode::SetModel(SceneModelWeak vSceneModel) {
    if (m_SmoothNormalModulePtr) {
        m_SmoothNormalModulePtr->SetModel(vSceneModel);
    }
}

SceneModelWeak SmoothNormalNode::GetModel() {
    if (m_SmoothNormalModulePtr) {
        return m_SmoothNormalModulePtr->GetModel();
    }

    return SceneModelWeak();
}

void SmoothNormalNode::DrawOutputWidget(BaseNodeState* vBaseNodeState, NodeSlotWeak vSlot) {
    // one output only
    // if (m_SmoothNormalModulePtr)
    {
        // ImGui::Text("%s", m_SmoothNormal->GetFileName().c_str());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string SmoothNormalNode::getXml(const std::string& vOffset, const std::string& vUserDatas) {
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

        if (m_SmoothNormalModulePtr) {
            res += m_SmoothNormalModulePtr->getXml(vOffset + "\t", vUserDatas);
        }

        res += vOffset + "</node>\n";
    }

    return res;
}

bool SmoothNormalNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (m_SmoothNormalModulePtr) {
        m_SmoothNormalModulePtr->setFromXml(vElem, vParent, vUserDatas);
    }

    return true;
}