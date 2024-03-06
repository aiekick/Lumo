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

#include <LumoBackend/Graph/Slots/NodeSlotStorageBufferInput.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Interfaces/StorageBufferInputInterface.h>
#include <LumoBackend/Interfaces/StorageBufferOutputInterface.h>

#include <utility>


//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotStorageBufferInputPtr NodeSlotStorageBufferInput::Create(NodeSlotStorageBufferInput vSlot) {
    auto res = std::make_shared<NodeSlotStorageBufferInput>(vSlot);
    res->m_This = res;
    return res;
}

NodeSlotStorageBufferInputPtr NodeSlotStorageBufferInput::Create(const std::string& vName, const std::string& vType) {
    auto res = std::make_shared<NodeSlotStorageBufferInput>(vName, vType);
    res->m_This = res;
    return res;
}

NodeSlotStorageBufferInputPtr NodeSlotStorageBufferInput::Create(const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint) {
    auto res = std::make_shared<NodeSlotStorageBufferInput>(vName, vType, vBindingPoint);
    res->m_This = res;
    return res;
}

NodeSlotStorageBufferInputPtr NodeSlotStorageBufferInput::Create(
    const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint, const bool& vHideName) {
    auto res = std::make_shared<NodeSlotStorageBufferInput>(vName, vType, vBindingPoint, vHideName);
    res->m_This = res;
    return res;
}

NodeSlotStorageBufferInputPtr NodeSlotStorageBufferInput::Create(
    const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget) {
    auto res = std::make_shared<NodeSlotStorageBufferInput>(vName, vType, vBindingPoint, vHideName, vShowWidget);
    res->m_This = res;
    return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// NODESLOT CLASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotStorageBufferInput::NodeSlotStorageBufferInput() : NodeSlotInput("", "") {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotStorageBufferInput::NodeSlotStorageBufferInput(const std::string& vName, const std::string& vType) : NodeSlotInput(vName, vType) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotStorageBufferInput::NodeSlotStorageBufferInput(const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint)
    : NodeSlotInput(vName, vType) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
    descriptorBinding = vBindingPoint;
}

NodeSlotStorageBufferInput::NodeSlotStorageBufferInput(
    const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint, const bool& vHideName)
    : NodeSlotInput(vName, vType, vHideName) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
    descriptorBinding = vBindingPoint;
}

NodeSlotStorageBufferInput::NodeSlotStorageBufferInput(
    const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget)
    : NodeSlotInput(vName, vType, vHideName, vShowWidget) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
    descriptorBinding = vBindingPoint;
}

NodeSlotStorageBufferInput::~NodeSlotStorageBufferInput() = default;

void NodeSlotStorageBufferInput::Init() {
}

void NodeSlotStorageBufferInput::Unit() {
    // ici pas besoin du assert sur le m_This
    // car NodeSlotStorageBufferInput peut etre isntancié à l'ancienne en copie local donc sans shared_ptr
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

void NodeSlotStorageBufferInput::OnConnectEvent(NodeSlotWeak vOtherSlot) {
    auto endSlotPtr = vOtherSlot.lock();
    if (endSlotPtr) {
        if (endSlotPtr->slotType == slotType) {
            auto parentNodePtr = dynamic_pointer_cast<StorageBufferInputInterface<0u>>(parentNode.lock());
            if (parentNodePtr) {
                auto otherStorageBufferNodePtr = dynamic_pointer_cast<StorageBufferOutputInterface>(endSlotPtr->parentNode.lock());
                if (otherStorageBufferNodePtr) {
                    uint32_t storageBufferSize;
                    auto bufferViewPtr = otherStorageBufferNodePtr->GetStorageBuffer(endSlotPtr->descriptorBinding, &storageBufferSize);
                    parentNodePtr->SetStorageBuffer(descriptorBinding, bufferViewPtr, &storageBufferSize);
                }
            }
        }
    }
}

void NodeSlotStorageBufferInput::OnDisConnectEvent(NodeSlotWeak vOtherSlot) {
    auto endSlotPtr = vOtherSlot.lock();
    if (endSlotPtr) {
        if (endSlotPtr->slotType == slotType) {
            auto parentNodePtr = dynamic_pointer_cast<StorageBufferInputInterface<0u>>(parentNode.lock());
            if (parentNodePtr) {
                parentNodePtr->SetStorageBuffer(descriptorBinding, nullptr, nullptr);
            }
        }
    }
}

void NodeSlotStorageBufferInput::TreatNotification(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot) {
    if (vEvent == StorageBufferUpdateDone) {
        auto emiterSlotPtr = vEmitterSlot.lock();
        if (emiterSlotPtr) {
            if (emiterSlotPtr->IsAnOutput()) {
                auto parentNodePtr = parentNode.lock();
                if (parentNodePtr) {
                    auto parentStorageBufferInputNodePtr = dynamic_pointer_cast<StorageBufferInputInterface<0u>>(parentNodePtr);
                    if (parentStorageBufferInputNodePtr) {
                        auto otherStorageBufferNodePtr = dynamic_pointer_cast<StorageBufferOutputInterface>(emiterSlotPtr->parentNode.lock());
                        if (otherStorageBufferNodePtr) {
                            auto receiverSlotPtr = vReceiverSlot.lock();
                            if (receiverSlotPtr) {
                                uint32_t storageBufferSize;
                                auto bufferViewPtr =
                                    otherStorageBufferNodePtr->GetStorageBuffer(emiterSlotPtr->descriptorBinding, &storageBufferSize);
                                parentStorageBufferInputNodePtr->SetStorageBuffer(
                                    receiverSlotPtr->descriptorBinding, bufferViewPtr, &storageBufferSize);
                            }
                        }
                    }
                }
            }
        }
    }
}

void NodeSlotStorageBufferInput::DrawDebugInfos() {
    ImGui::Text("--------------------");
    ImGui::Text("Slot %s", name.c_str());
    ImGui::Text(IsAnInput() ? "Input" : "Output");
    ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}
