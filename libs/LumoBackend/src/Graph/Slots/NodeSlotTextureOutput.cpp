﻿/*
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

#include <LumoBackend/Graph/Slots/NodeSlotTextureOutput.h>

#include <utility>
#include <LumoBackend/Graph/Base/BaseNode.h>

static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotTextureOutputPtr NodeSlotTextureOutput::Create(NodeSlotTextureOutput vSlot)
{
	auto res = std::make_shared<NodeSlotTextureOutput>(vSlot);
	res->m_This = res;
	return res;
}

NodeSlotTextureOutputPtr NodeSlotTextureOutput::Create(const std::string& vName, const uint32_t& vBindingPoint)
{
	auto res = std::make_shared<NodeSlotTextureOutput>(vName, vBindingPoint);
	res->m_This = res;
	return res;
}

NodeSlotTextureOutputPtr NodeSlotTextureOutput::Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName)
{
	auto res = std::make_shared<NodeSlotTextureOutput>(vName, vBindingPoint, vHideName);
	res->m_This = res;
	return res;
}

NodeSlotTextureOutputPtr NodeSlotTextureOutput::Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget)
{
	auto res = std::make_shared<NodeSlotTextureOutput>(vName, vBindingPoint, vHideName, vShowWidget);
	res->m_This = res;
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// NODESLOT CLASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotTextureOutput::NodeSlotTextureOutput()
	: NodeSlotOutput("", "TEXTURE_2D")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotTextureOutput::NodeSlotTextureOutput(const std::string& vName, const uint32_t& vBindingPoint)
	: NodeSlotOutput(vName, "TEXTURE_2D")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
	descriptorBinding = vBindingPoint;
}

NodeSlotTextureOutput::NodeSlotTextureOutput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName)
	: NodeSlotOutput(vName, "TEXTURE_2D", vHideName)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
	descriptorBinding = vBindingPoint;
}

NodeSlotTextureOutput::NodeSlotTextureOutput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget)
	: NodeSlotOutput(vName, "TEXTURE_2D", vHideName, vShowWidget)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
	descriptorBinding = vBindingPoint;
}

NodeSlotTextureOutput::~NodeSlotTextureOutput() = default;

void NodeSlotTextureOutput::Init()
{
	
}

void NodeSlotTextureOutput::Unit()
{
	// ici pas besoin du assert sur le m_This 
	// car NodeSlotTextureOutput peut etre instancié à l'ancienne en copie local donc sans shared_ptr
	// donc pour gagner du temps on va checker le this, si expiré on va pas plus loins
	if (!m_This.expired())
	{
		if (!parentNode.expired())
		{
			auto parentNodePtr = parentNode.lock();
			if (parentNodePtr)
			{
				auto graph = parentNodePtr->GetParentNode();
				if (!graph.expired())
				{
					auto graphPtr = graph.lock();
					if (graphPtr)
					{
						graphPtr->BreakAllLinksConnectedToSlot(m_This);
					}
				}
			}
		}
	}
}

void NodeSlotTextureOutput::SendFrontNotification(const NotifyEvent& vEvent)
{
	if (vEvent == TextureUpdateDone)
	{
		SendNotification("TEXTURE_2D", vEvent);
	}
}

void NodeSlotTextureOutput::MouseDoubleClickedOnSlot(const ImGuiMouseButton& vMouseButton) {
    auto ptr = m_GetRootNode();
    if (ptr != nullptr) {
        ptr->SelectForGraphOutput_Callback(m_This, vMouseButton);
    }
}

void NodeSlotTextureOutput::DrawDebugInfos()
{
	ImGui::Text("--------------------");
	ImGui::Text("Slot %s", name.c_str());
	ImGui::Text(IsAnInput() ? "Input" : "Output");
	ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}