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

#include "BreakTexturesGroupNode.h"
#include <LumoBackend/Interfaces/LightGroupOutputInterface.h>
#include <LumoBackend/Interfaces/TextureGroupOutputInterface.h>
#include <LumoBackend/Graph/Slots/NodeSlotTexture2DInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTexture2DOutput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureGroupInput.h>

/*
// tofix : the select of output crash Lumo...
*/

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

std::shared_ptr<BreakTexturesGroupNode> BreakTexturesGroupNode::Create(GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res = std::make_shared<BreakTexturesGroupNode>();
    res->m_This = res;
    if (!res->Init(vVulkanCore)) {
        res.reset();
    }
    return res;
}

BreakTexturesGroupNode::BreakTexturesGroupNode() : BaseNode() {
    m_NodeTypeString = "BREAK_TEXTURE_2D_GROUP";

    // variable slot count only for inputs
    // important for xml loading
    m_OutputSlotsInternalMode = NODE_INTERNAL_MODE_Enum::NODE_INTERNAL_MODE_DYNAMIC;
}

BreakTexturesGroupNode::~BreakTexturesGroupNode() {
    Unit();
}

bool BreakTexturesGroupNode::Init(GaiApi::VulkanCoreWeak vVulkanCore) {
    name = "Break Tex2D Group";

    AddInput(NodeSlotTextureGroupInput::Create("Textures"), true, false);

    return true;
}

void BreakTexturesGroupNode::Unit() {
}

bool BreakTexturesGroupNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

    // for update input texture buffer infos => avoid vk crash
    UpdateTextureGroupInputDescriptorImageInfos(m_Inputs);

    return true;
}

bool BreakTexturesGroupNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool BreakTexturesGroupNode::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool BreakTexturesGroupNode::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void BreakTexturesGroupNode::TreatNotification(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot) {
    if (vEvent == GraphIsLoaded || vEvent == TextureGroupUpdateDone) {
        ReorganizeSlots();
    }
}

void BreakTexturesGroupNode::SetTextures(const uint32_t& vBindingPoint, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes) {
    m_Textures.clear();

    if (vImageInfos) {
        for (auto& info : *vImageInfos) {
            m_Textures.push_back(info);
        }
    }

    ReorganizeSlots();
}

vk::DescriptorImageInfo* BreakTexturesGroupNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    if (vBindingPoint < (uint32_t)m_Textures.size()) {
        return &m_Textures[vBindingPoint];
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string BreakTexturesGroupNode::getXml(const std::string& vOffset, const std::string& vUserDatas) {
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

        res += vOffset + "</node>\n";
    }

    return res;
}

bool BreakTexturesGroupNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    return true;
}

NodeSlotWeak BreakTexturesGroupNode::AddPreDefinedOutput(const NodeSlot& vNodeSlot) {
    // tofix : to test xml loading (normally ok like for SceneMergerNode)
    CTOOL_DEBUG_BREAK;

    if (vNodeSlot.slotType == "TEXTURE_2D") {
        auto slot_ptr = NodeSlotTexture2DOutput::Create(ct::toStr("Output %u", vNodeSlot.index), vNodeSlot.index);
        if (slot_ptr) {
            slot_ptr->parentNode = m_This;
            slot_ptr->slotPlace = NodeSlot::PlaceEnum::OUTPUT;
            slot_ptr->hideName = true;
            slot_ptr->type = Lumo::uType::uTypeEnum::U_FLOW;
            slot_ptr->index = vNodeSlot.index;
            slot_ptr->pinID = vNodeSlot.pinID;
            const auto& slotID = vNodeSlot.GetSlotID();

            // pour eviter que des slots aient le meme id qu'un nodePtr
            BaseNode::freeNodeId = ct::maxi<uint32_t>(BaseNode::freeNodeId, slotID) + 1U;

            m_Outputs[slotID] = slot_ptr;
            return m_Outputs.at(slotID);
        }
    } else {
        CTOOL_DEBUG_BREAK;
        LogVarError("node slot is of type %s.. must be of type TEXTURE_2D", vNodeSlot.slotType.c_str());
    }

    return NodeSlotWeak();
}

void BreakTexturesGroupNode::ReorganizeSlots() {
    if (m_Textures.size() != m_Outputs.size()) {
        m_Outputs.clear();

        for (uint32_t idx = 0U; idx < (uint32_t)m_Textures.size(); ++idx) {
            AddOutput(NodeSlotTexture2DOutput::Create(ct::toStr("Output %u", idx), idx), true, true);
        }
    }
}
