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

#pragma once

#include <LumoBackend/Graph/Graph.h>
#include <LumoBackend/Graph/Base/NodeSlotInput.h>

class NodeSlotSceneAudiartInput;
typedef std::weak_ptr<NodeSlotSceneAudiartInput> NodeSlotSceneAudiartInputWeak;
typedef std::shared_ptr<NodeSlotSceneAudiartInput> NodeSlotSceneAudiartInputPtr;

class NodeSlotSceneAudiartInput : public NodeSlotInput {
public:
    static NodeSlotSceneAudiartInputPtr Create(NodeSlotSceneAudiartInput vSlot);
    static NodeSlotSceneAudiartInputPtr Create(const std::string& vName);
    static NodeSlotSceneAudiartInputPtr Create(const std::string& vName, const bool& vHideName);
    static NodeSlotSceneAudiartInputPtr Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget);

public:
    explicit NodeSlotSceneAudiartInput();
    explicit NodeSlotSceneAudiartInput(const std::string& vName);
    explicit NodeSlotSceneAudiartInput(const std::string& vName, const bool& vHideName);
    explicit NodeSlotSceneAudiartInput(const std::string& vName, const bool& vHideName, const bool& vShowWidget);
    ~NodeSlotSceneAudiartInput();

    void Init();
    void Unit();

    void OnConnectEvent(NodeSlotWeak vOtherSlot) override;
    void OnDisConnectEvent(NodeSlotWeak vOtherSlot) override;

    void TreatNotification(NotifyEvent vEvent, const NodeSlotWeak& vEmitterSlot = NodeSlotWeak(), const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());

    void DrawDebugInfos();
};