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

#include <LumoBackend/Graph/Slots/NodeSlotTexture3DInput.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Interfaces/Texture3DInputInterface.h>
#include <LumoBackend/Interfaces/Texture3DOutputInterface.h>

#include <utility>
static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotTexture3DInputPtr NodeSlotTexture3DInput::Create(NodeSlotTexture3DInput vSlot) {
    auto res = std::make_shared<NodeSlotTexture3DInput>(vSlot);
    res->m_This = res;
    return res;
}

NodeSlotTexture3DInputPtr NodeSlotTexture3DInput::Create(const std::string& vName, const uint32_t& vBindingPoint) {
    auto res = std::make_shared<NodeSlotTexture3DInput>(vName, vBindingPoint);
    res->m_This = res;
    return res;
}

NodeSlotTexture3DInputPtr NodeSlotTexture3DInput::Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName) {
    auto res = std::make_shared<NodeSlotTexture3DInput>(vName, vBindingPoint, vHideName);
    res->m_This = res;
    return res;
}

NodeSlotTexture3DInputPtr NodeSlotTexture3DInput::Create(
    const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget) {
    auto res = std::make_shared<NodeSlotTexture3DInput>(vName, vBindingPoint, vHideName, vShowWidget);
    res->m_This = res;
    return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// NODESLOT CLASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotTexture3DInput::NodeSlotTexture3DInput() : NodeSlotInput("", "TEXTURE_3D") {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotTexture3DInput::NodeSlotTexture3DInput(const std::string& vName, const uint32_t& vBindingPoint) : NodeSlotInput(vName, "TEXTURE_3D") {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
    descriptorBinding = vBindingPoint;
}

NodeSlotTexture3DInput::NodeSlotTexture3DInput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName)
    : NodeSlotInput(vName, "TEXTURE_3D", vHideName) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
    descriptorBinding = vBindingPoint;
}

NodeSlotTexture3DInput::NodeSlotTexture3DInput(
    const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget)
    : NodeSlotInput(vName, "TEXTURE_3D", vHideName, vShowWidget) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
    descriptorBinding = vBindingPoint;
}

NodeSlotTexture3DInput::~NodeSlotTexture3DInput() = default;

void NodeSlotTexture3DInput::Init() {
}

void NodeSlotTexture3DInput::Unit() {
    // ici pas besoin du assert sur le m_This
    // car NodeSlotTexture3DInput peut etre isntancié à l'ancienne en copie local donc sans shared_ptr
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

void NodeSlotTexture3DInput::OnConnectEvent(NodeSlotWeak vOtherSlot) {
    if (slotType == "TEXTURE_3D") {
        auto endSlotPtr = vOtherSlot.lock();
        if (endSlotPtr) {
            auto parentNodePtr = dynamic_pointer_cast<Texture3DInputInterface<0u>>(parentNode.lock());
            if (parentNodePtr) {
                auto otherTextureNodePtr = dynamic_pointer_cast<Texture3DOutputInterface>(endSlotPtr->parentNode.lock());
                if (otherTextureNodePtr) {
                    ct::fvec2 textureSize;
                    auto descPtr = otherTextureNodePtr->GetDescriptorImageInfo(endSlotPtr->descriptorBinding, &textureSize);
                    parentNodePtr->SetTexture(descriptorBinding, descPtr, &textureSize);
                } else {
                    CTOOL_DEBUG_BREAK;
                }
            } else {
                CTOOL_DEBUG_BREAK;
            }
        }
    }
}

void NodeSlotTexture3DInput::OnDisConnectEvent(NodeSlotWeak vOtherSlot) {
    if (slotType == "TEXTURE_3D") {
        auto endSlotPtr = vOtherSlot.lock();
        if (endSlotPtr) {
            auto parentNodePtr = dynamic_pointer_cast<Texture3DInputInterface<0u>>(parentNode.lock());
            if (parentNodePtr) {
                parentNodePtr->SetTexture(descriptorBinding, nullptr, nullptr);
            } else {
                CTOOL_DEBUG_BREAK;
            }
        }
    }
}

void NodeSlotTexture3DInput::TreatNotification(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot) {
    if (vEvent == TextureUpdateDone) {
        auto emiterSlotPtr = vEmitterSlot.lock();
        if (emiterSlotPtr) {
            if (emiterSlotPtr->IsAnOutput()) {
                auto parentNodePtr = parentNode.lock();
                if (parentNodePtr) {
                    auto parentTexture3DInputNodePtr = dynamic_pointer_cast<Texture3DInputInterface<0u>>(parentNodePtr);
                    if (parentTexture3DInputNodePtr) {
                        auto otherNodePtr = dynamic_pointer_cast<Texture3DOutputInterface>(emiterSlotPtr->parentNode.lock());
                        if (otherNodePtr) {
                            auto receiverSlotPtr = vReceiverSlot.lock();
                            if (receiverSlotPtr) {
                                ct::fvec2 textureSize;
                                auto descPtr = otherNodePtr->GetDescriptorImageInfo(emiterSlotPtr->descriptorBinding, &textureSize);
                                parentTexture3DInputNodePtr->SetTexture(receiverSlotPtr->descriptorBinding, descPtr, &textureSize);
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

void NodeSlotTexture3DInput::MouseDoubleClickedOnSlot(const ImGuiMouseButton& vMouseButton) {
    if (!linkedSlots.empty()) {
        auto ptr = m_GetRootNode();
        if (ptr != nullptr) {
            ptr->SelectForGraphOutput_Callback(linkedSlots[0], vMouseButton);
        }
    }
}

void NodeSlotTexture3DInput::DrawDebugInfos() {
    ImGui::Text("--------------------");
    ImGui::Text("Slot %s", name.c_str());
    ImGui::Text(IsAnInput() ? "Input" : "Output");
    ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}
