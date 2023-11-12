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

#include "VariableNode.h"
#include <Graph/Modules/Widgets/VariableModule.h>
#include <LumoBackend/Graph/Slots/NodeSlotVariableOutput.h>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

std::shared_ptr<VariableNode> VariableNode::Create(GaiApi::VulkanCorePtr vVulkanCorePtr, const std::string& vNodeType) {
    auto res = std::make_shared<VariableNode>(vNodeType);
    res->m_This = res;
    if (!res->Init(vVulkanCorePtr)) {
        res.reset();
    }
    return res;
}

VariableNode::VariableNode(const std::string& vNodeType) : BaseNode() {
    m_NodeTypeString = vNodeType;
}

VariableNode::~VariableNode() {
    Unit();
}

bool VariableNode::Init(GaiApi::VulkanCorePtr vVulkanCorePtr) {
    name = "Widget Bool";

    std::string slot_type;
    if (m_NodeTypeString == "WIDGET_BOOLEAN")
        slot_type = "WIDGET_BOOLEAN";
    else if (m_NodeTypeString == "WIDGET_FLOAT")
        slot_type = "WIDGET_FLOAT";
    else if (m_NodeTypeString == "WIDGET_INT")
        slot_type = "WIDGET_INT";
    else if (m_NodeTypeString == "WIDGET_UINT")
        slot_type = "WIDGET_UINT";

    auto slotPtr = NodeSlotVariableOutput::Create("Output", slot_type, 0U);
    if (slotPtr) {
        slotPtr->showWidget = true;
        AddOutput(slotPtr, true, true);
    }

    m_VariableModulePtr = VariableModule::Create(m_NodeTypeString, m_This);
    return (m_VariableModulePtr != nullptr);
}

bool VariableNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

    if (m_VariableModulePtr) {
        return m_VariableModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
    }

    return false;
}

bool VariableNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_VariableModulePtr) {
        return m_VariableModulePtr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
    }

    return false;
}

bool VariableNode::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool VariableNode::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_VariableModulePtr) {
        return m_VariableModulePtr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
    }

    return false;
}

void VariableNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) {
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

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string VariableNode::getXml(const std::string& vOffset, const std::string& vUserDatas) {
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

        if (m_VariableModulePtr) {
            res += m_VariableModulePtr->getXml(vOffset + "\t", vUserDatas);
        }

        res += vOffset + "</node>\n";
    }

    return res;
}

bool VariableNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (m_VariableModulePtr) {
        m_VariableModulePtr->setFromXml(vElem, vParent, vUserDatas);
    }

    return true;
}

void VariableNode::DrawOutputWidget(BaseNodeState* vBaseNodeState, NodeSlotWeak vSlot) {
    if (vBaseNodeState) {
        auto slotPtr = vSlot.lock();
        if (slotPtr) {
            if (m_VariableModulePtr) {
                m_VariableModulePtr->DrawNodeWidget(0U, ImGui::GetCurrentContext());
            }
        }
    }
}

SceneVariableWeak VariableNode::GetVariable(const uint32_t& vVariableIndex) {
    if (m_VariableModulePtr) {
        return m_VariableModulePtr->GetVariable(vVariableIndex);
    }

    return SceneVariableWeak();
}