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

class NodeSlotTexture2DOutput;
typedef std::weak_ptr<NodeSlotTexture2DOutput> NodeSlotTexture2DOutputWeak;
typedef std::shared_ptr<NodeSlotTexture2DOutput> NodeSlotTexture2DOutputPtr;

class LUMO_BACKEND_API NodeSlotTexture2DOutput : public NodeSlotOutput {
public:
    static NodeSlotTexture2DOutputPtr Create(NodeSlotTexture2DOutput vSlot);
    static NodeSlotTexture2DOutputPtr Create(const std::string& vName, const uint32_t& vBindingPoint);
    static NodeSlotTexture2DOutputPtr Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName);
    static NodeSlotTexture2DOutputPtr Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget);

public:
    explicit NodeSlotTexture2DOutput();
    explicit NodeSlotTexture2DOutput(const std::string& vName, const uint32_t& vBindingPoint);
    explicit NodeSlotTexture2DOutput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName);
    explicit NodeSlotTexture2DOutput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget);
    ~NodeSlotTexture2DOutput();

    void Init();
    void Unit();

    void SendFrontNotification(const NotifyEvent& vEvent) override;

    void MouseDoubleClickedOnSlot(const ImGuiMouseButton& vMouseButton) override;

    void DrawDebugInfos();
};