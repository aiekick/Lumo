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
#include <LumoBackend/Graph/Base/NodeSlotOutput.h>

class NodeSlotSceneAudiartOutput;
typedef std::weak_ptr<NodeSlotSceneAudiartOutput> NodeSlotSceneAudiartOutputWeak;
typedef std::shared_ptr<NodeSlotSceneAudiartOutput> NodeSlotSceneAudiartOutputPtr;

class NodeSlotSceneAudiartOutput : public NodeSlotOutput {
public:
    static NodeSlotSceneAudiartOutputPtr Create(NodeSlotSceneAudiartOutput vSlot);
    static NodeSlotSceneAudiartOutputPtr Create(const std::string& vName);
    static NodeSlotSceneAudiartOutputPtr Create(const std::string& vName, const bool& vHideName);
    static NodeSlotSceneAudiartOutputPtr Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget);

public:
    explicit NodeSlotSceneAudiartOutput();
    explicit NodeSlotSceneAudiartOutput(const std::string& vName);
    explicit NodeSlotSceneAudiartOutput(const std::string& vName, const bool& vHideName);
    explicit NodeSlotSceneAudiartOutput(const std::string& vName, const bool& vHideName, const bool& vShowWidget);
    ~NodeSlotSceneAudiartOutput();

    void Init();
    void Unit();

    void SendFrontNotification(const NotifyEvent& vEvent) override;

    void DrawDebugInfos();
};
