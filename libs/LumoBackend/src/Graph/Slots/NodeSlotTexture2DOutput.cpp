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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <LumoBackend/Graph/Slots/NodeSlotTexture2DOutput.h>

#include <utility>
#include <LumoBackend/Graph/Base/BaseNode.h>



//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotTexture2DOutputPtr NodeSlotTexture2DOutput::Create(NodeSlotTexture2DOutput vSlot) {
    auto res = std::make_shared<NodeSlotTexture2DOutput>(vSlot);
    res->m_This = res;
    return res;
}

NodeSlotTexture2DOutputPtr NodeSlotTexture2DOutput::Create(const std::string& vName, const uint32_t& vBindingPoint) {
    auto res = std::make_shared<NodeSlotTexture2DOutput>(vName, vBindingPoint);
    res->m_This = res;
    return res;
}

NodeSlotTexture2DOutputPtr NodeSlotTexture2DOutput::Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName) {
    auto res = std::make_shared<NodeSlotTexture2DOutput>(vName, vBindingPoint, vHideName);
    res->m_This = res;
    return res;
}

NodeSlotTexture2DOutputPtr NodeSlotTexture2DOutput::Create(
    const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget) {
    auto res = std::make_shared<NodeSlotTexture2DOutput>(vName, vBindingPoint, vHideName, vShowWidget);
    res->m_This = res;
    return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// NODESLOT CLASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotTexture2DOutput::NodeSlotTexture2DOutput() : NodeSlotOutput("", "TEXTURE_2D") {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
}

NodeSlotTexture2DOutput::NodeSlotTexture2DOutput(const std::string& vName, const uint32_t& vBindingPoint) : NodeSlotOutput(vName, "TEXTURE_2D") {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
    descriptorBinding = vBindingPoint;
}

NodeSlotTexture2DOutput::NodeSlotTexture2DOutput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName)
    : NodeSlotOutput(vName, "TEXTURE_2D", vHideName) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
    descriptorBinding = vBindingPoint;
}

NodeSlotTexture2DOutput::NodeSlotTexture2DOutput(
    const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget)
    : NodeSlotOutput(vName, "TEXTURE_2D", vHideName, vShowWidget) {
    pinID = sGetNewSlotId();
    color = sGetSlotColors()->GetSlotColor(slotType);
    colorIsSet = true;
    descriptorBinding = vBindingPoint;
}

NodeSlotTexture2DOutput::~NodeSlotTexture2DOutput() = default;

void NodeSlotTexture2DOutput::Init() {
}

void NodeSlotTexture2DOutput::Unit() {
    // ici pas besoin du assert sur le m_This
    // car NodeSlotTexture2DOutput peut etre instancié à l'ancienne en copie local donc sans shared_ptr
    // donc pour gagner du temps on va checker le this, si expiré on va pas plus loins
    if (!m_This.expired()) {
        if (!parentNode.expired()) {
            auto parentNodePtr = parentNode.lock();
            if (parentNodePtr) {
                auto graph = parentNodePtr->GetParentNode();
                if (!graph.expired()) {
                    auto graphPtr = graph.lock();
                    if (graphPtr) {
                        graphPtr->BreakAllLinksConnectedToSlot(m_This);
                    }
                }
            }
        }
    }
}

void NodeSlotTexture2DOutput::SendFrontNotification(const NotifyEvent& vEvent) {
    if (vEvent == TextureUpdateDone) {
        SendNotification("TEXTURE_2D", vEvent);
    }
}

void NodeSlotTexture2DOutput::MouseDoubleClickedOnSlot(const ImGuiMouseButton& vMouseButton) {
    auto ptr = m_GetRootNode();
    if (ptr != nullptr) {
        ptr->SelectForGraphOutput_Callback(m_This, vMouseButton);
    }
}

void NodeSlotTexture2DOutput::DrawDebugInfos() {
    ImGui::Text("--------------------");
    ImGui::Text("Slot %s", name.c_str());
    ImGui::Text(IsAnInput() ? "Input" : "Output");
    ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}