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

class NodeSlotTexture3DInput;
typedef std::weak_ptr<NodeSlotTexture3DInput> NodeSlotTexture3DInputWeak;
typedef std::shared_ptr<NodeSlotTexture3DInput> NodeSlotTexture3DInputPtr;

class LUMO_BACKEND_API NodeSlotTexture3DInput : public NodeSlotInput {
public:
    static NodeSlotTexture3DInputPtr Create(NodeSlotTexture3DInput vSlot);
    static NodeSlotTexture3DInputPtr Create(const std::string& vName, const uint32_t& vBindingPoint);
    static NodeSlotTexture3DInputPtr Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName);
    static NodeSlotTexture3DInputPtr Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget);

public:
    explicit NodeSlotTexture3DInput();
    explicit NodeSlotTexture3DInput(const std::string& vName, const uint32_t& vBindingPoint);
    explicit NodeSlotTexture3DInput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName);
    explicit NodeSlotTexture3DInput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget);
    ~NodeSlotTexture3DInput();

    void Init();
    void Unit();

    void OnConnectEvent(NodeSlotWeak vOtherSlot) override;
    void OnDisConnectEvent(NodeSlotWeak vOtherSlot) override;

    void TreatNotification(
        const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot = NodeSlotWeak(), const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());

    void MouseDoubleClickedOnSlot(const ImGuiMouseButton& vMouseButton) override;

    void DrawDebugInfos();
};