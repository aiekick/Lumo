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

class NodeSlotShaderPassInput;
typedef std::weak_ptr<NodeSlotShaderPassInput> NodeSlotShaderPassInputWeak;
typedef std::shared_ptr<NodeSlotShaderPassInput> NodeSlotShaderPassInputPtr;

class LUMO_BACKEND_API NodeSlotShaderPassInput : public NodeSlotInput {
public:
    static NodeSlotShaderPassInputPtr Create(NodeSlotShaderPassInput vSlot);
    static NodeSlotShaderPassInputPtr Create(const std::string& vName);
    static NodeSlotShaderPassInputPtr Create(const std::string& vName, const bool& vHideName);
    static NodeSlotShaderPassInputPtr Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget);

public:
    explicit NodeSlotShaderPassInput();
    explicit NodeSlotShaderPassInput(const std::string& vName);
    explicit NodeSlotShaderPassInput(const std::string& vName, const bool& vHideName);
    explicit NodeSlotShaderPassInput(const std::string& vName, const bool& vHideName, const bool& vShowWidget);
    virtual ~NodeSlotShaderPassInput();

    void Init();
    void Unit();

    void OnConnectEvent(NodeSlotWeak vOtherSlot) override;
    void OnDisConnectEvent(NodeSlotWeak vOtherSlot) override;

    void TreatNotification(
        const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot = NodeSlotWeak(), const NodeSlotWeak& vReceiverSlot = NodeSlotWeak())override;

    void MouseDoubleClickedOnSlot(const ImGuiMouseButton& vMouseButton) override;

    void DrawDebugInfos() override;
};