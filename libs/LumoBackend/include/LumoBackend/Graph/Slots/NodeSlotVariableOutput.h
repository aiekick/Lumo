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
#include <LumoBackend/Graph/Base/NodeSlotOutput.h>

class NodeSlotVariableOutput;
typedef std::weak_ptr<NodeSlotVariableOutput> NodeSlotVariableOutputWeak;
typedef std::shared_ptr<NodeSlotVariableOutput> NodeSlotVariableOutputPtr;

class LUMO_BACKEND_API NodeSlotVariableOutput : public NodeSlotOutput {
public:
    static NodeSlotVariableOutputPtr Create(NodeSlotVariableOutput vSlot);
    static NodeSlotVariableOutputPtr Create(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex);
    static NodeSlotVariableOutputPtr Create(
        const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName);
    static NodeSlotVariableOutputPtr Create(
        const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName, const bool& vShowWidget);

public:
    explicit NodeSlotVariableOutput();
    explicit NodeSlotVariableOutput(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex);
    explicit NodeSlotVariableOutput(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName);
    explicit NodeSlotVariableOutput(
        const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName, const bool& vShowWidget);
    ~NodeSlotVariableOutput();

    void Init();
    void Unit();

    void SendFrontNotification(const NotifyEvent& vEvent) override;

    void DrawDebugInfos();
};