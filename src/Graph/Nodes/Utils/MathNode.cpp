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

#include "MathNode.h"
#include <Graph/Modules/Utils/MathModule.h>
#include <LumoBackend/Graph/Slots/NodeSlotTexture2DInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTexture2DOutput.h>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

std::shared_ptr<MathNode> MathNode::Create(GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res = std::make_shared<MathNode>();
    res->m_This = res;
    if (!res->Init(vVulkanCore)) {
        res.reset();
    }
    return res;
}

MathNode::MathNode() : BaseNode() {
    m_NodeTypeString = "MATH";
}

MathNode::~MathNode() {
    Unit();
}

bool MathNode::Init(GaiApi::VulkanCoreWeak vVulkanCore) {
    name = "Math";

    for (uint32_t i = 0U; i < 3U; ++i) {
        auto slotPtr = NodeSlotTexture2DInput::Create("", i);
        if (slotPtr) {
            slotPtr->hidden = true;
            AddInput(slotPtr, true, false);
        }
    }

    auto slotPtr = NodeSlotTexture2DOutput::Create("Output", 0U);
    if (slotPtr) {
        slotPtr->showWidget = true;
        AddOutput(slotPtr, true, true);
    }

    bool res = false;

    m_MathModulePtr = MathModule::Create(vVulkanCore);
    if (m_MathModulePtr) {
        ReorganizeSlots();

        res = true;
    }

    return res;
}

bool MathNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

    // for update input texture buffer infos => avoid vk crash
    UpdateTexture2DInputDescriptorImageInfos(m_Inputs);

    bool res = false;

    if (m_MathModulePtr) {
        res = m_MathModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);

        SendFrontNotification(TextureUpdateDone);
    }

    return res;
}

bool MathNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    bool change = false;

    if (m_MathModulePtr) {
        change = m_MathModulePtr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
    }

    if (change) {
        ReorganizeSlots();
    }

    return change;
}

bool MathNode::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool MathNode::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (m_MathModulePtr) {
        return m_MathModulePtr->DrawDialogsAndPopups(vCurrentFrame, vMaxRect, vContextPtr, vUserDatas);
    }

    return false;
}

void MathNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) {
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

void MathNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) {
    if (m_MathModulePtr) {
        m_MathModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
    }

    // on fait ca apres
    BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void MathNode::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
    if (m_MathModulePtr) {
        m_MathModulePtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize, vUserDatas);
    }
}

vk::DescriptorImageInfo* MathNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    if (m_MathModulePtr) {
        return m_MathModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize, vUserDatas);
    }

    return nullptr;
}

void MathNode::TreatNotification(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot) {
    if (vEvent == GraphIsLoaded) {
        ReorganizeSlots();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string MathNode::getXml(const std::string& vOffset, const std::string& vUserDatas) {
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

        if (m_MathModulePtr) {
            res += m_MathModulePtr->getXml(vOffset + "\t", vUserDatas);
        }

        res += vOffset + "</node>\n";
    }

    return res;
}

bool MathNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (m_MathModulePtr) {
        m_MathModulePtr->setFromXml(vElem, vParent, vUserDatas);
    }

    return true;
}

void MathNode::UpdateShaders(const std::set<std::string>& vFiles) {
    if (m_MathModulePtr) {
        m_MathModulePtr->UpdateShaders(vFiles);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE /////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void MathNode::DrawOutputWidget(BaseNodeState* vBaseNodeState, NodeSlotWeak vSlot) {
    if (vBaseNodeState) {
        auto slotPtr = vSlot.lock();
        if (slotPtr) {
            if (m_MathModulePtr) {
                m_MathModulePtr->DrawNodeWidget(vBaseNodeState->m_CurrentFrame, ImGui::GetCurrentContext());
            }
        }
    }
}

void MathNode::ReorganizeSlots() {
    if (m_MathModulePtr) {
        auto count = m_MathModulePtr->GetComponentCount();

        // on va montrer que les slots utiles

        uint32_t idx = 0;
        for (auto& input : m_Inputs) {
            if (input.second) {
                if (idx >= count) {
                    input.second->hidden = true;
                    BreakAllLinksConnectedToSlot(input.second);
                } else {
                    input.second->hidden = false;
                    input.second->name = m_MathModulePtr->GetInputName(idx);
                }
            }
            ++idx;
        }
    }
}