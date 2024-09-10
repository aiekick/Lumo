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

#include "NodeSlotSceneAudiartInput.h"

#include <utility>
#include <SceneGraph/SceneAudiart.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <Interfaces/SceneAudiartInputInterface.h>
#include <Interfaces/SceneAudiartOutputInterface.h>

static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotSceneAudiartInputPtr NodeSlotSceneAudiartInput::Create(NodeSlotSceneAudiartInput vSlot) {
    auto res = std::make_shared<NodeSlotSceneAudiartInput>(vSlot);
    res->m_This = res;
    return res;
}

NodeSlotSceneAudiartInputPtr NodeSlotSceneAudiartInput::Create(const std::string& vName) {
    auto res = std::make_shared<NodeSlotSceneAudiartInput>(vName);
    res->m_This = res;
    return res;
}

NodeSlotSceneAudiartInputPtr NodeSlotSceneAudiartInput::Create(const std::string& vName, const bool& vHideName) {
    auto res = std::make_shared<NodeSlotSceneAudiartInput>(vName, vHideName);
    res->m_This = res;
    return res;
}

NodeSlotSceneAudiartInputPtr NodeSlotSceneAudiartInput::Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget) {
    auto res = std::make_shared<NodeSlotSceneAudiartInput>(vName, vHideName, vShowWidget);
    res->m_This = res;
    return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC CLASS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotSceneAudiartInput::NodeSlotSceneAudiartInput() : NodeSlotInput("", "SCENEAUDIART") {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotSceneAudiartInput::NodeSlotSceneAudiartInput(const std::string& vName) : NodeSlotInput(vName, "SCENEAUDIART") {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotSceneAudiartInput::NodeSlotSceneAudiartInput(const std::string& vName, const bool& vHideName)
    : NodeSlotInput(vName, "SCENEAUDIART", vHideName) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotSceneAudiartInput::NodeSlotSceneAudiartInput(const std::string& vName, const bool& vHideName, const bool& vShowWidget)
    : NodeSlotInput(vName, "SCENEAUDIART", vHideName, vShowWidget) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotSceneAudiartInput::~NodeSlotSceneAudiartInput() = default;

void NodeSlotSceneAudiartInput::Init() {
}

void NodeSlotSceneAudiartInput::Unit() {
    // ici pas besoin du assert sur le m_This
    // car NodeSlotSceneAudiartInput peut etre isntancié à l'ancienne en copie local donc sans shared_ptr
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

void NodeSlotSceneAudiartInput::OnConnectEvent(NodeSlotWeak vOtherSlot) {
    auto endSlotPtr = vOtherSlot.lock();
    if (endSlotPtr) {
        auto parentNodePtr = dynamic_pointer_cast<SceneAudiartInputInterface>(parentNode.lock());
        if (parentNodePtr) {
            auto otherCodeNodePtr = dynamic_pointer_cast<SceneAudiartOutputInterface>(endSlotPtr->parentNode.lock());
            if (otherCodeNodePtr) {
                parentNodePtr->SetSceneAudiart(name, otherCodeNodePtr->GetSceneAudiart(endSlotPtr->name));
            }
        }
    }
}

void NodeSlotSceneAudiartInput::OnDisConnectEvent(NodeSlotWeak vOtherSlot) {
    auto endSlotPtr = vOtherSlot.lock();
    if (endSlotPtr) {
        auto parentNodePtr = dynamic_pointer_cast<SceneAudiartInputInterface>(parentNode.lock());
        if (parentNodePtr) {
            parentNodePtr->SetSceneAudiart(name, SceneAudiartWeak());
        }
    }
}

void NodeSlotSceneAudiartInput::TreatNotification(NotifyEvent vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot) {
    if (vEvent == SceneAudiartUpdateDone) {
        auto emiterSlotPtr = vEmitterSlot.lock();
        if (emiterSlotPtr) {
            if (emiterSlotPtr->IsAnOutput()) {
                auto parentCodeInputNodePtr = dynamic_pointer_cast<SceneAudiartInputInterface>(parentNode.lock());
                if (parentCodeInputNodePtr) {
                    auto otherNodePtr = dynamic_pointer_cast<SceneAudiartOutputInterface>(emiterSlotPtr->parentNode.lock());
                    if (otherNodePtr) {
                        auto receiverSlotPtr = vReceiverSlot.lock();
                        if (receiverSlotPtr) {
                            parentCodeInputNodePtr->SetSceneAudiart(receiverSlotPtr->name, otherNodePtr->GetSceneAudiart(emiterSlotPtr->name));
                        }
                    }
                }
            }
        }
    }
}

void NodeSlotSceneAudiartInput::DrawDebugInfos() {
    ImGui::Text("--------------------");
    ImGui::Text("Slot SceneAudiart", name.c_str());
    ImGui::Text(IsAnInput() ? "Input" : "Output");
    ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}
