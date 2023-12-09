/*
Copyright 2022-2022 Stephane Cuillerdier (aka aiekick)

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

#include <LumoBackend/Graph/Graph.h>
#include <LumoBackend/Graph/Base/NodeSlotInput.h>

class NodeSlotAccelStructureInput;
typedef std::weak_ptr<NodeSlotAccelStructureInput> NodeSlotAccelStructureInputWeak;
typedef std::shared_ptr<NodeSlotAccelStructureInput> NodeSlotAccelStructureInputPtr;

class NodeSlotAccelStructureInput : public NodeSlotInput {
public:
    static NodeSlotAccelStructureInputPtr Create(NodeSlotAccelStructureInput vSlot);
    static NodeSlotAccelStructureInputPtr Create(const std::string& vName);
    static NodeSlotAccelStructureInputPtr Create(const std::string& vName, const bool& vHideName);
    static NodeSlotAccelStructureInputPtr Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget);

public:
    explicit NodeSlotAccelStructureInput();
    explicit NodeSlotAccelStructureInput(const std::string& vName);
    explicit NodeSlotAccelStructureInput(const std::string& vName, const bool& vHideName);
    explicit NodeSlotAccelStructureInput(const std::string& vName, const bool& vHideName, const bool& vShowWidget);
    ~NodeSlotAccelStructureInput();

    void Init();
    void Unit();

    void OnConnectEvent(NodeSlotWeak vOtherSlot) override;
    void OnDisConnectEvent(NodeSlotWeak vOtherSlot) override;

    void TreatNotification(NotifyEvent vEvent, const NodeSlotWeak& vEmitterSlot = NodeSlotWeak(), const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());

    void DrawDebugInfos();

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};