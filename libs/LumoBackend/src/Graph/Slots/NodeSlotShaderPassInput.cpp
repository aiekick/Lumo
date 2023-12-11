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

#include <LumoBackend/Graph/Slots/NodeSlotShaderPassInput.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Interfaces/ShaderPassInputInterface.h>
#include <LumoBackend/Interfaces/ShaderPassOutputInterface.h>

#include <utility>
static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotShaderPassInputPtr NodeSlotShaderPassInput::Create(NodeSlotShaderPassInput vSlot) {
    auto res = std::make_shared<NodeSlotShaderPassInput>(vSlot);
    res->m_This = res;
    return res;
}

NodeSlotShaderPassInputPtr NodeSlotShaderPassInput::Create(const std::string& vName) {
    auto res = std::make_shared<NodeSlotShaderPassInput>(vName);
    res->m_This = res;
    return res;
}

NodeSlotShaderPassInputPtr NodeSlotShaderPassInput::Create(const std::string& vName, const bool& vHideName) {
    auto res = std::make_shared<NodeSlotShaderPassInput>(vName, vHideName);
    res->m_This = res;
    return res;
}

NodeSlotShaderPassInputPtr NodeSlotShaderPassInput::Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget) {
    auto res = std::make_shared<NodeSlotShaderPassInput>(vName, vHideName, vShowWidget);
    res->m_This = res;
    return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// NODESLOT CLASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotShaderPassInput::NodeSlotShaderPassInput() : NodeSlotInput("", "SHADER_PASS") {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotShaderPassInput::NodeSlotShaderPassInput(const std::string& vName) : NodeSlotInput(vName, "SHADER_PASS") {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotShaderPassInput::NodeSlotShaderPassInput(const std::string& vName, const bool& vHideName) : NodeSlotInput(vName, "SHADER_PASS", vHideName) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotShaderPassInput::NodeSlotShaderPassInput(const std::string& vName, const bool& vHideName, const bool& vShowWidget)
    : NodeSlotInput(vName, "SHADER_PASS", vHideName, vShowWidget) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotShaderPassInput::~NodeSlotShaderPassInput() = default;

void NodeSlotShaderPassInput::Init() {
}

void NodeSlotShaderPassInput::Unit() {
    // ici pas besoin du assert sur le m_This
    // car NodeSlotShaderPassInput peut etre isntancié à l'ancienne en copie local donc sans shared_ptr
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

void NodeSlotShaderPassInput::OnConnectEvent(NodeSlotWeak vOtherSlot) {
    if (slotType == "SHADER_PASS") {
        auto endSlotPtr = vOtherSlot.lock();
        if (endSlotPtr) {
            auto parentNodePtr = dynamic_pointer_cast<ShaderPassInputInterface>(parentNode.lock());
            if (parentNodePtr) {
                auto otherPassNodePtr = dynamic_pointer_cast<ShaderPassOutputInterface>(endSlotPtr->parentNode.lock());
                if (otherPassNodePtr) {
                    auto passes = otherPassNodePtr->GetShaderPasses(endSlotPtr->GetSlotID());
                    parentNodePtr->SetShaderPasses(GetSlotID(), passes);
                } else {
                    CTOOL_DEBUG_BREAK;
                }
            } else {
                CTOOL_DEBUG_BREAK;
            }
        }
    }
}

void NodeSlotShaderPassInput::OnDisConnectEvent(NodeSlotWeak vOtherSlot) {
    if (slotType == "SHADER_PASS") {
        auto endSlotPtr = vOtherSlot.lock();
        if (endSlotPtr) {
            auto parentNodePtr = dynamic_pointer_cast<ShaderPassInputInterface>(parentNode.lock());
            if (parentNodePtr) {
                parentNodePtr->SetShaderPasses(GetSlotID(), SceneShaderPassWeak());
            } else {
                CTOOL_DEBUG_BREAK;
            }
        }
    }
}

void NodeSlotShaderPassInput::TreatNotification(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot) {
    if (vEvent == ShaderPassUpdateDone) {
        auto emiterSlotPtr = vEmitterSlot.lock();
        if (emiterSlotPtr) {
            if (emiterSlotPtr->IsAnOutput()) {
                auto parentNodePtr = parentNode.lock();
                if (parentNodePtr) {
                    auto parentPassInputNodePtr = dynamic_pointer_cast<ShaderPassInputInterface>(parentNodePtr);
                    if (parentPassInputNodePtr) {
                        auto otherNodePtr = dynamic_pointer_cast<ShaderPassOutputInterface>(emiterSlotPtr->parentNode.lock());
                        if (otherNodePtr) {
                            auto receiverSlotPtr = vReceiverSlot.lock();
                            if (receiverSlotPtr) {
                                auto passes = otherNodePtr->GetShaderPasses(emiterSlotPtr->GetSlotID());
                                parentPassInputNodePtr->SetShaderPasses(receiverSlotPtr->GetSlotID(), passes);
                            }
                        } else {
                            CTOOL_DEBUG_BREAK;
                        }
                    } else {
                        CTOOL_DEBUG_BREAK;
                    }
                }
            }
        }
    }
}

void NodeSlotShaderPassInput::MouseDoubleClickedOnSlot(const ImGuiMouseButton& vMouseButton) {
    if (!linkedSlots.empty()) {
        auto ptr = m_GetRootNode();
        if (ptr != nullptr) {
            ptr->SelectForGraphOutput_Callback(linkedSlots[0], vMouseButton);
        }
    }
}

void NodeSlotShaderPassInput::DrawDebugInfos() {
    ImGui::Text("--------------------");
    ImGui::Text("Slot %s", name.c_str());
    ImGui::Text(IsAnInput() ? "Input" : "Output");
    ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}
