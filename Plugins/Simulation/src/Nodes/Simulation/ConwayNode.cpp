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

#include "ConwayNode.h"
#include <Modules/Simulation/ConwayModule.h>
#include <LumoBackend/Graph/Slots/NodeSlotTexture2DInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTexture2DOutput.h>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

std::shared_ptr<ConwayNode> ConwayNode::Create(GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res = std::make_shared<ConwayNode>();
    res->m_This = res;
    if (!res->Init(vVulkanCore)) {
        res.reset();
    }
    return res;
}

ConwayNode::ConwayNode() : BaseNode() {
    m_NodeTypeString = "2D_SIMULATION_CONWAY";
}

ConwayNode::~ConwayNode() {
    Unit();
}

bool ConwayNode::Init(GaiApi::VulkanCoreWeak vVulkanCore) {
    name = "Conway";

    // on desctive pour le moment, le shader le gere, mais on s'en sert pas pour le moment
    AddInput(NodeSlotTexture2DInput::Create("Noise", 0U), true, false);
    AddOutput(NodeSlotTexture2DOutput::Create("Output", 0U), true, true);

    bool res = false;

    m_ConwayModulePtr = ConwayModule::Create(vVulkanCore);
    if (m_ConwayModulePtr) {
        res = true;
    }

    return res;
}

bool ConwayNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    bool res = false;

    BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

    // for update input texture buffer infos => avoid vk crash
    UpdateTexture2DInputDescriptorImageInfos(m_Inputs);

    if (m_ConwayModulePtr) {
        res = m_ConwayModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);

        // SendFrontNotification(TextureUpdateDone);
    }

    return res;
}

bool ConwayNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_ConwayModulePtr) {
        return m_ConwayModulePtr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
    }

    return false;
}

bool ConwayNode::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool ConwayNode::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_ConwayModulePtr) {
        return m_ConwayModulePtr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
    }

    return false;
}

void ConwayNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) {
    if (vBaseNodeState && vBaseNodeState->debug_mode) {
        auto drawList = nd::GetNodeBackgroundDrawList(nodeID);
        if (drawList) {
            char debugBuffer[255] = "\0";
            snprintf(debugBuffer, 254, "Used(%s)\nCell(%i, %i)", (used ? "true" : "false"), cell.x, cell.y);
            ImVec2 txtSize = ImGui::CalcTextSize(debugBuffer);
            drawList->AddText(pos - ImVec2(0, txtSize.y), ImGui::GetColorU32(ImGuiCol_Text), debugBuffer);
        }
    }
}

void ConwayNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) {
    if (m_ConwayModulePtr) {
        m_ConwayModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
    }

    // on fait ca apres
    BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void ConwayNode::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
    if (m_ConwayModulePtr) {
        m_ConwayModulePtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize, vUserDatas);
    }
}

vk::DescriptorImageInfo* ConwayNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    if (m_ConwayModulePtr) {
        return m_ConwayModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize, vUserDatas);
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string ConwayNode::getXml(const std::string& vOffset, const std::string& vUserDatas) {
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

        if (m_ConwayModulePtr) {
            res += m_ConwayModulePtr->getXml(vOffset + "\t", vUserDatas);
        }

        res += vOffset + "</node>\n";
    }

    return res;
}

bool ConwayNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (m_ConwayModulePtr) {
        m_ConwayModulePtr->setFromXml(vElem, vParent, vUserDatas);
    }

    return true;
}

void ConwayNode::UpdateShaders(const std::set<std::string>& vFiles) {
    if (m_ConwayModulePtr) {
        m_ConwayModulePtr->UpdateShaders(vFiles);
    }
}