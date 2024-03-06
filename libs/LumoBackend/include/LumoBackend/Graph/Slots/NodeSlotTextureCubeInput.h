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

class NodeSlotTextureCubeInput;
typedef std::weak_ptr<NodeSlotTextureCubeInput> NodeSlotTextureCubeInputWeak;
typedef std::shared_ptr<NodeSlotTextureCubeInput> NodeSlotTextureCubeInputPtr;

class LUMO_BACKEND_API NodeSlotTextureCubeInput : public NodeSlotInput {
public:
    static NodeSlotTextureCubeInputPtr Create(NodeSlotTextureCubeInput vSlot);
    static NodeSlotTextureCubeInputPtr Create(const std::string& vName, const uint32_t& vBindingPoint);
    static NodeSlotTextureCubeInputPtr Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName);
    static NodeSlotTextureCubeInputPtr Create(
        const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget);

public:
    explicit NodeSlotTextureCubeInput();
    explicit NodeSlotTextureCubeInput(const std::string& vName, const uint32_t& vBindingPoint);
    explicit NodeSlotTextureCubeInput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName);
    explicit NodeSlotTextureCubeInput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget);
    virtual ~NodeSlotTextureCubeInput();

    void Init();
    void Unit();

    void OnConnectEvent(NodeSlotWeak vOtherSlot) override;
    void OnDisConnectEvent(NodeSlotWeak vOtherSlot) override;

    void TreatNotification(
        const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot = NodeSlotWeak(), const NodeSlotWeak& vReceiverSlot = NodeSlotWeak()) override;

    void DrawDebugInfos() override;
};