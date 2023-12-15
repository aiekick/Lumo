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

#include <LumoBackend/Graph/Slots/NodeSlotTexture2DInput.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Interfaces/Texture2DInputInterface.h>
#include <LumoBackend/Interfaces/Texture2DOutputInterface.h>

#include <utility>
static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotTexture2DInputPtr NodeSlotTexture2DInput::Create(NodeSlotTexture2DInput vSlot) {
    auto res = std::make_shared<NodeSlotTexture2DInput>(vSlot);
    res->m_This = res;
    return res;
}

NodeSlotTexture2DInputPtr NodeSlotTexture2DInput::Create(const std::string& vName, const uint32_t& vBindingPoint) {
    auto res = std::make_shared<NodeSlotTexture2DInput>(vName, vBindingPoint);
    res->m_This = res;
    return res;
}

NodeSlotTexture2DInputPtr NodeSlotTexture2DInput::Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName) {
    auto res = std::make_shared<NodeSlotTexture2DInput>(vName, vBindingPoint, vHideName);
    res->m_This = res;
    return res;
}

NodeSlotTexture2DInputPtr NodeSlotTexture2DInput::Create(
    const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget) {
    auto res = std::make_shared<NodeSlotTexture2DInput>(vName, vBindingPoint, vHideName, vShowWidget);
    res->m_This = res;
    return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// NODESLOT CLASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotTexture2DInput::NodeSlotTexture2DInput() : NodeSlotInput("", "TEXTURE_2D") {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotTexture2DInput::NodeSlotTexture2DInput(const std::string& vName, const uint32_t& vBindingPoint) : NodeSlotInput(vName, "TEXTURE_2D") {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
    descriptorBinding = vBindingPoint;
}

NodeSlotTexture2DInput::NodeSlotTexture2DInput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName)
    : NodeSlotInput(vName, "TEXTURE_2D", vHideName) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
    descriptorBinding = vBindingPoint;
}

NodeSlotTexture2DInput::NodeSlotTexture2DInput(
    const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget)
    : NodeSlotInput(vName, "TEXTURE_2D", vHideName, vShowWidget) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
    descriptorBinding = vBindingPoint;
}

NodeSlotTexture2DInput::~NodeSlotTexture2DInput() = default;

void NodeSlotTexture2DInput::Init() {
}

void NodeSlotTexture2DInput::Unit() {
    // ici pas besoin du assert sur le m_This
    // car NodeSlotTexture2DInput peut etre isntancié à l'ancienne en copie local donc sans shared_ptr
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

void NodeSlotTexture2DInput::OnConnectEvent(NodeSlotWeak vOtherSlot) {
    if (slotType == "TEXTURE_2D") {
        auto endSlotPtr = vOtherSlot.lock();
        if (endSlotPtr) {
            auto parentNodePtr = dynamic_pointer_cast<Texture2DInputInterface<0u>>(parentNode.lock());
            if (parentNodePtr) {
                auto otherTextureNodePtr = dynamic_pointer_cast<Texture2DOutputInterface>(endSlotPtr->parentNode.lock());
                if (otherTextureNodePtr) {
                    ct::fvec2 textureSize;
                    void* userDatasPtr = nullptr;
                    auto descPtr = otherTextureNodePtr->GetDescriptorImageInfo(endSlotPtr->descriptorBinding, &textureSize, userDatasPtr);
                    parentNodePtr->SetTexture(descriptorBinding, descPtr, &textureSize, userDatasPtr);
                } else {
                    CTOOL_DEBUG_BREAK;
                }
            } else {
                CTOOL_DEBUG_BREAK;
            }
        }
    }
}

void NodeSlotTexture2DInput::OnDisConnectEvent(NodeSlotWeak vOtherSlot) {
    if (slotType == "TEXTURE_2D") {
        auto endSlotPtr = vOtherSlot.lock();
        if (endSlotPtr) {
            auto parentNodePtr = dynamic_pointer_cast<Texture2DInputInterface<0u>>(parentNode.lock());
            if (parentNodePtr) {
                parentNodePtr->SetTexture(descriptorBinding, nullptr, nullptr, nullptr);
            } else {
                CTOOL_DEBUG_BREAK;
            }
        }
    }
}

void NodeSlotTexture2DInput::TreatNotification(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot) {
    if (vEvent == TextureUpdateDone) {
        auto emiterSlotPtr = vEmitterSlot.lock();
        if (emiterSlotPtr) {
            if (emiterSlotPtr->IsAnOutput()) {
                auto parentNodePtr = parentNode.lock();
                if (parentNodePtr) {
                    auto parentTexture2DInputNodePtr = dynamic_pointer_cast<Texture2DInputInterface<0u>>(parentNodePtr);
                    if (parentTexture2DInputNodePtr) {
                        auto otherNodePtr = dynamic_pointer_cast<Texture2DOutputInterface>(emiterSlotPtr->parentNode.lock());
                        if (otherNodePtr) {
                            auto receiverSlotPtr = vReceiverSlot.lock();
                            if (receiverSlotPtr) {
                                ct::fvec2 textureSize;
                                void* userDatasPtr = nullptr;
                                auto descPtr = otherNodePtr->GetDescriptorImageInfo(emiterSlotPtr->descriptorBinding, &textureSize, userDatasPtr);
                                parentTexture2DInputNodePtr->SetTexture(receiverSlotPtr->descriptorBinding, descPtr, &textureSize, userDatasPtr);
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

void NodeSlotTexture2DInput::MouseDoubleClickedOnSlot(const ImGuiMouseButton& vMouseButton) {
    if (!linkedSlots.empty()) {
        auto ptr = m_GetRootNode();
        if (ptr != nullptr) {
            ptr->SelectForGraphOutput_Callback(linkedSlots[0], vMouseButton);
        }
    }
}

void NodeSlotTexture2DInput::DrawDebugInfos() {
    ImGui::Text("--------------------");
    ImGui::Text("Slot %s", name.c_str());
    ImGui::Text(IsAnInput() ? "Input" : "Output");
    ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}
