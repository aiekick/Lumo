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

#include "DepthConvNode.h"
#include <Modules/Utils/DepthConvModule.h>
#include <LumoBackend/Graph/Slots/NodeSlotTexture2DInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTexture2DOutput.h>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

std::shared_ptr<DepthConvNode> DepthConvNode::Create(GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res = std::make_shared<DepthConvNode>();
    res->m_This = res;
    if (!res->Init(vVulkanCore)) {
        res.reset();
    }
    return res;
}

DepthConvNode::DepthConvNode() : BaseNode() {
    m_NodeTypeString = "DEPTH_CONVERSION";
}

DepthConvNode::~DepthConvNode() {
    Unit();
}

bool DepthConvNode::Init(GaiApi::VulkanCoreWeak vVulkanCore) {
    name = "Depth To Pos";

    AddInput(NodeSlotTexture2DInput::Create("Depth", 0U), true, true);
    AddOutput(NodeSlotTexture2DOutput::Create("Pos", 0U), true, true);
    AddOutput(NodeSlotTexture2DOutput::Create("Nor", 1U), true, true);

    bool res = false;

    m_DepthConvModulePtr = DepthConvModule::Create(vVulkanCore);
    if (m_DepthConvModulePtr) {
        res = true;
    }

    return res;
}

bool DepthConvNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

    // for update input texture buffer infos => avoid vk crash
    UpdateTexture2DInputDescriptorImageInfos(m_Inputs);

    if (m_DepthConvModulePtr) {
        return m_DepthConvModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
    }
    return false;
}

bool DepthConvNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_DepthConvModulePtr) {
        return m_DepthConvModulePtr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
    }

    return false;
}

bool DepthConvNode::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool DepthConvNode::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_DepthConvModulePtr) {
        return m_DepthConvModulePtr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
    }

    return false;
}

void DepthConvNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) {
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

void DepthConvNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) {
    if (m_DepthConvModulePtr) {
        m_DepthConvModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
    }

    // on fait ca apres
    BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void DepthConvNode::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
    if (m_DepthConvModulePtr) {
        m_DepthConvModulePtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize, vUserDatas);
    }
}

vk::DescriptorImageInfo* DepthConvNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    if (m_DepthConvModulePtr) {
        return m_DepthConvModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize, vUserDatas);
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string DepthConvNode::getXml(const std::string& vOffset, const std::string& vUserDatas) {
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

        if (m_DepthConvModulePtr) {
            res += m_DepthConvModulePtr->getXml(vOffset + "\t", vUserDatas);
        }

        res += vOffset + "</node>\n";
    }

    return res;
}

bool DepthConvNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (m_DepthConvModulePtr) {
        m_DepthConvModulePtr->setFromXml(vElem, vParent, vUserDatas);
    }

    return true;
}

void DepthConvNode::UpdateShaders(const std::set<std::string>& vFiles) {
    if (m_DepthConvModulePtr) {
        m_DepthConvModulePtr->UpdateShaders(vFiles);
    }
}