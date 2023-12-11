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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <LumoBackend/Graph/Slots/NodeSlotLightGroupInput.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Interfaces/LightGroupInputInterface.h>
#include <LumoBackend/Interfaces/LightGroupOutputInterface.h>

#include <utility>
static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotLightGroupInputPtr NodeSlotLightGroupInput::Create(NodeSlotLightGroupInput vSlot) {
    auto res = std::make_shared<NodeSlotLightGroupInput>(vSlot);
    res->m_This = res;
    return res;
}

NodeSlotLightGroupInputPtr NodeSlotLightGroupInput::Create(const std::string& vName) {
    auto res = std::make_shared<NodeSlotLightGroupInput>(vName);
    res->m_This = res;
    return res;
}

NodeSlotLightGroupInputPtr NodeSlotLightGroupInput::Create(const std::string& vName, const bool& vHideName) {
    auto res = std::make_shared<NodeSlotLightGroupInput>(vName, vHideName);
    res->m_This = res;
    return res;
}

NodeSlotLightGroupInputPtr NodeSlotLightGroupInput::Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget) {
    auto res = std::make_shared<NodeSlotLightGroupInput>(vName, vHideName, vShowWidget);
    res->m_This = res;
    return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// NODESLOT CLASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotLightGroupInput::NodeSlotLightGroupInput() : NodeSlotInput("", "LIGHT_GROUP") {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotLightGroupInput::NodeSlotLightGroupInput(const std::string& vName) : NodeSlotInput(vName, "LIGHT_GROUP") {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotLightGroupInput::NodeSlotLightGroupInput(const std::string& vName, const bool& vHideName) : NodeSlotInput(vName, "LIGHT_GROUP", vHideName) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotLightGroupInput::NodeSlotLightGroupInput(const std::string& vName, const bool& vHideName, const bool& vShowWidget)
    : NodeSlotInput(vName, "LIGHT_GROUP", vHideName, vShowWidget) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotLightGroupInput::~NodeSlotLightGroupInput() = default;

void NodeSlotLightGroupInput::Init() {
}

void NodeSlotLightGroupInput::Unit() {
    // ici pas besoin du assert sur le m_This
    // car NodeSlotLightGroupInput peut etre isntancié à l'ancienne en copie local donc sans shared_ptr
    // donc pour gagner du temps on va checker le this, si expiré on va pas plus loins
    if (!m_This.expired()) {
        if (!parentNode.expired()) {
            auto parentNodePtr = parentNode.lock();
            if (parentNodePtr) {
                auto graph = parentNodePtr->GetParentNode();
                if (!graph.expired()) {
                    auto graphPtr = graph.lock();
                    if (graphPtr) {
                        graphPtr->BreakAllLinksConnectedToSlot(m_This);
                    }
                }
            }
        }
    }
}

void NodeSlotLightGroupInput::OnConnectEvent(NodeSlotWeak vOtherSlot) {
    if (slotType == "LIGHT_GROUP") {
        auto endSlotPtr = vOtherSlot.lock();
        if (endSlotPtr) {
            auto parentLightGroupNodePtr = dynamic_pointer_cast<LightGroupInputInterface>(parentNode.lock());
            if (parentLightGroupNodePtr) {
                auto otherLightGroupNodePtr = dynamic_pointer_cast<LightGroupOutputInterface>(endSlotPtr->parentNode.lock());
                if (otherLightGroupNodePtr) {
                    parentLightGroupNodePtr->SetLightGroup(otherLightGroupNodePtr->GetLightGroup());
                }
            }
        }
    }
}

void NodeSlotLightGroupInput::OnDisConnectEvent(NodeSlotWeak vOtherSlot) {
    if (slotType == "LIGHT_GROUP") {
        auto endSlotPtr = vOtherSlot.lock();
        if (endSlotPtr) {
            auto parentNodePtr = dynamic_pointer_cast<LightGroupInputInterface>(parentNode.lock());
            if (parentNodePtr) {
                parentNodePtr->SetLightGroup(SceneLightGroupWeak());
            }
        }
    }
}

void NodeSlotLightGroupInput::TreatNotification(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& /*vReceiverSlot*/) {
    if (vEvent == LightGroupUpdateDone) {
        auto emiterSlotPtr = vEmitterSlot.lock();
        if (emiterSlotPtr) {
            if (emiterSlotPtr->IsAnOutput()) {
                auto parentLightGroupInputNodePtr = dynamic_pointer_cast<LightGroupInputInterface>(parentNode.lock());
                if (parentLightGroupInputNodePtr) {
                    auto otherLightGroupNodePtr = dynamic_pointer_cast<LightGroupOutputInterface>(emiterSlotPtr->parentNode.lock());
                    if (otherLightGroupNodePtr) {
                        parentLightGroupInputNodePtr->SetLightGroup(otherLightGroupNodePtr->GetLightGroup());
                    }
                }
            }
        }
    }
}

void NodeSlotLightGroupInput::DrawDebugInfos() {
    ImGui::Text("--------------------");
    ImGui::Text("Slot %s", name.c_str());
    ImGui::Text(IsAnInput() ? "Input" : "Output");
    ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}
