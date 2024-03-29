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

#pragma once
#pragma warning(disable : 4251)

#include <LumoBackend/Graph/Graph.h>
#include <LumoBackend/Graph/Base/NodeSlotInput.h>

class NodeSlotVariableInput;
typedef std::weak_ptr<NodeSlotVariableInput> NodeSlotVariableInputWeak;
typedef std::shared_ptr<NodeSlotVariableInput> NodeSlotVariableInputPtr;

class LUMO_BACKEND_API NodeSlotVariableInput : public NodeSlotInput {
public:
    static NodeSlotVariableInputPtr Create(NodeSlotVariableInput vSlot);
    static NodeSlotVariableInputPtr Create(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex);
    static NodeSlotVariableInputPtr Create(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName);
    static NodeSlotVariableInputPtr Create(
        const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName, const bool& vShowWidget);

public:
    explicit NodeSlotVariableInput();
    explicit NodeSlotVariableInput(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex);
    explicit NodeSlotVariableInput(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName);
    explicit NodeSlotVariableInput(
        const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName, const bool& vShowWidget);
    virtual ~NodeSlotVariableInput();

    void Init();
    void Unit();

    void OnConnectEvent(NodeSlotWeak vOtherSlot) override;
    void OnDisConnectEvent(NodeSlotWeak vOtherSlot) override;

    void TreatNotification(
        const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot = NodeSlotWeak(), const NodeSlotWeak& vReceiverSlot = NodeSlotWeak()) override;

    void DrawDebugInfos() override;
};