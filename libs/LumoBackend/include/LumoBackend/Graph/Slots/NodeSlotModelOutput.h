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

class NodeSlotModelOutput;
typedef std::weak_ptr<NodeSlotModelOutput> NodeSlotModelOutputWeak;
typedef std::shared_ptr<NodeSlotModelOutput> NodeSlotModelOutputPtr;

class LUMO_BACKEND_API NodeSlotModelOutput : public NodeSlotOutput {
public:
    static NodeSlotModelOutputPtr Create(NodeSlotModelOutput vSlot);
    static NodeSlotModelOutputPtr Create(const std::string& vName);
    static NodeSlotModelOutputPtr Create(const std::string& vName, const bool& vHideName);
    static NodeSlotModelOutputPtr Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget);

public:
    explicit NodeSlotModelOutput();
    explicit NodeSlotModelOutput(const std::string& vName);
    explicit NodeSlotModelOutput(const std::string& vName, const bool& vHideName);
    explicit NodeSlotModelOutput(const std::string& vName, const bool& vHideName, const bool& vShowWidget);
    ~NodeSlotModelOutput();

    void Init();
    void Unit();

    void SendFrontNotification(const NotifyEvent& vEvent) override;

    void DrawDebugInfos();
};