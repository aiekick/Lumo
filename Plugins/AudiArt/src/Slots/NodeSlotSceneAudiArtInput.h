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

class NodeSlotSceneAudiArtInput;
typedef std::weak_ptr<NodeSlotSceneAudiArtInput> NodeSlotSceneAudiArtInputWeak;
typedef std::shared_ptr<NodeSlotSceneAudiArtInput> NodeSlotSceneAudiArtInputPtr;

class NodeSlotSceneAudiArtInput : public NodeSlotInput {
public:
    static NodeSlotSceneAudiArtInputPtr Create(NodeSlotSceneAudiArtInput vSlot);
    static NodeSlotSceneAudiArtInputPtr Create(const std::string& vName);
    static NodeSlotSceneAudiArtInputPtr Create(const std::string& vName, const bool& vHideName);
    static NodeSlotSceneAudiArtInputPtr Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget);

public:
    explicit NodeSlotSceneAudiArtInput();
    explicit NodeSlotSceneAudiArtInput(const std::string& vName);
    explicit NodeSlotSceneAudiArtInput(const std::string& vName, const bool& vHideName);
    explicit NodeSlotSceneAudiArtInput(const std::string& vName, const bool& vHideName, const bool& vShowWidget);
    ~NodeSlotSceneAudiArtInput();

    void Init();
    void Unit();

    void OnConnectEvent(NodeSlotWeak vOtherSlot) override;
    void OnDisConnectEvent(NodeSlotWeak vOtherSlot) override;

    void TreatNotification(NotifyEvent vEvent, const NodeSlotWeak& vEmitterSlot = NodeSlotWeak(), const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());

    void DrawDebugInfos();
};
