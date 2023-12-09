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

#include "NodeSlotSceneAudiArtInput.h"

#include <utility>
#include <SceneGraph/SceneAudiArt.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <Interfaces/SceneAudiArtInputInterface.h>
#include <Interfaces/SceneAudiArtOutputInterface.h>

static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotSceneAudiArtInputPtr NodeSlotSceneAudiArtInput::Create(NodeSlotSceneAudiArtInput vSlot) {
    auto res = std::make_shared<NodeSlotSceneAudiArtInput>(vSlot);
    res->m_This = res;
    return res;
}

NodeSlotSceneAudiArtInputPtr NodeSlotSceneAudiArtInput::Create(const std::string& vName) {
    auto res = std::make_shared<NodeSlotSceneAudiArtInput>(vName);
    res->m_This = res;
    return res;
}

NodeSlotSceneAudiArtInputPtr NodeSlotSceneAudiArtInput::Create(const std::string& vName, const bool& vHideName) {
    auto res = std::make_shared<NodeSlotSceneAudiArtInput>(vName, vHideName);
    res->m_This = res;
    return res;
}

NodeSlotSceneAudiArtInputPtr NodeSlotSceneAudiArtInput::Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget) {
    auto res = std::make_shared<NodeSlotSceneAudiArtInput>(vName, vHideName, vShowWidget);
    res->m_This = res;
    return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC CLASS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotSceneAudiArtInput::NodeSlotSceneAudiArtInput() : NodeSlotInput("", "SCENEAUDIART") {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotSceneAudiArtInput::NodeSlotSceneAudiArtInput(const std::string& vName) : NodeSlotInput(vName, "SCENEAUDIART") {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotSceneAudiArtInput::NodeSlotSceneAudiArtInput(const std::string& vName, const bool& vHideName)
    : NodeSlotInput(vName, "SCENEAUDIART", vHideName) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotSceneAudiArtInput::NodeSlotSceneAudiArtInput(const std::string& vName, const bool& vHideName, const bool& vShowWidget)
    : NodeSlotInput(vName, "SCENEAUDIART", vHideName, vShowWidget) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotSceneAudiArtInput::~NodeSlotSceneAudiArtInput() = default;

void NodeSlotSceneAudiArtInput::Init() {
}

void NodeSlotSceneAudiArtInput::Unit() {
    // ici pas besoin du assert sur le m_This
    // car NodeSlotSceneAudiArtInput peut etre isntancié à l'ancienne en copie local donc sans shared_ptr
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

void NodeSlotSceneAudiArtInput::OnConnectEvent(NodeSlotWeak vOtherSlot) {
    auto endSlotPtr = vOtherSlot.lock();
    if (endSlotPtr) {
        auto parentNodePtr = dynamic_pointer_cast<SceneAudiArtInputInterface>(parentNode.lock());
        if (parentNodePtr) {
            auto otherCodeNodePtr = dynamic_pointer_cast<SceneAudiArtOutputInterface>(endSlotPtr->parentNode.lock());
            if (otherCodeNodePtr) {
                parentNodePtr->SetSceneAudiArt(name, otherCodeNodePtr->GetSceneAudiArt(endSlotPtr->name));
            }
        }
    }
}

void NodeSlotSceneAudiArtInput::OnDisConnectEvent(NodeSlotWeak vOtherSlot) {
    auto endSlotPtr = vOtherSlot.lock();
    if (endSlotPtr) {
        auto parentNodePtr = dynamic_pointer_cast<SceneAudiArtInputInterface>(parentNode.lock());
        if (parentNodePtr) {
            parentNodePtr->SetSceneAudiArt(name, SceneAudiArtWeak());
        }
    }
}

void NodeSlotSceneAudiArtInput::TreatNotification(NotifyEvent vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot) {
    if (vEvent == SceneAudiArtUpdateDone) {
        auto emiterSlotPtr = vEmitterSlot.lock();
        if (emiterSlotPtr) {
            if (emiterSlotPtr->IsAnOutput()) {
                auto parentCodeInputNodePtr = dynamic_pointer_cast<SceneAudiArtInputInterface>(parentNode.lock());
                if (parentCodeInputNodePtr) {
                    auto otherNodePtr = dynamic_pointer_cast<SceneAudiArtOutputInterface>(emiterSlotPtr->parentNode.lock());
                    if (otherNodePtr) {
                        auto receiverSlotPtr = vReceiverSlot.lock();
                        if (receiverSlotPtr) {
                            parentCodeInputNodePtr->SetSceneAudiArt(receiverSlotPtr->name, otherNodePtr->GetSceneAudiArt(emiterSlotPtr->name));
                        }
                    }
                }
            }
        }
    }
}

void NodeSlotSceneAudiArtInput::DrawDebugInfos() {
    ImGui::Text("--------------------");
    ImGui::Text("Slot SceneAudiArt", name.c_str());
    ImGui::Text(IsAnInput() ? "Input" : "Output");
    ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}
