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

class NodeSlotLightGroupOutput;
typedef std::weak_ptr<NodeSlotLightGroupOutput> NodeSlotLightGroupOutputWeak;
typedef std::shared_ptr<NodeSlotLightGroupOutput> NodeSlotLightGroupOutputPtr;

class LUMO_BACKEND_API NodeSlotLightGroupOutput : public NodeSlotOutput {
public:
    static NodeSlotLightGroupOutputPtr Create(NodeSlotLightGroupOutput vSlot);
    static NodeSlotLightGroupOutputPtr Create(const std::string& vName);
    static NodeSlotLightGroupOutputPtr Create(const std::string& vName, const bool& vHideName);
    static NodeSlotLightGroupOutputPtr Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget);

public:
    explicit NodeSlotLightGroupOutput();
    explicit NodeSlotLightGroupOutput(const std::string& vName);
    explicit NodeSlotLightGroupOutput(const std::string& vName, const bool& vHideName);
    explicit NodeSlotLightGroupOutput(const std::string& vName, const bool& vHideName, const bool& vShowWidget);
    virtual ~NodeSlotLightGroupOutput();

    void Init();
    void Unit();

    void SendFrontNotification(const NotifyEvent& vEvent) override;

    void DrawDebugInfos() override;
};