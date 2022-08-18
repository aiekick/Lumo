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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "NodeSlotOutput.h"

#include <utility>
#include "BaseNode.h"
static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotOutputPtr NodeSlotOutput::Create()
{
	auto res = std::make_shared<NodeSlotOutput>();
	res->m_This = res;
	return res;
}

NodeSlotOutputPtr NodeSlotOutput::Create(const std::string& vName)
{
	auto res = std::make_shared<NodeSlotOutput>(vName);
	res->m_This = res;
	return res;
}

NodeSlotOutputPtr NodeSlotOutput::Create(const std::string& vName, const std::string& vType)
{
	auto res = std::make_shared<NodeSlotOutput>(vName, vType);
	res->m_This = res;
	return res;
}

NodeSlotOutputPtr NodeSlotOutput::Create(const std::string& vName, const std::string& vType, const bool& vHideName)
{
	auto res = std::make_shared<NodeSlotOutput>(vName, vType, vHideName);
	res->m_This = res;
	return res;
}

NodeSlotOutputPtr NodeSlotOutput::Create(const std::string& vName, const std::string& vType, const bool& vHideName, const bool& vShowWidget)
{
	auto res = std::make_shared<NodeSlotOutput>(vName, vType, vHideName, vShowWidget);
	res->m_This = res;
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// NODESLOT CLASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotOutput::NodeSlotOutput()
	: NodeSlot()
{
	pinID = sGetNewSlotId();
	slotPlace = NodeSlot::PlaceEnum::OUTPUT;
}

NodeSlotOutput::NodeSlotOutput(const std::string& vName)
	: NodeSlot(vName)
{
	pinID = sGetNewSlotId();
	slotPlace = NodeSlot::PlaceEnum::OUTPUT;
}

NodeSlotOutput::NodeSlotOutput(const std::string& vName, const std::string& vType)
	: NodeSlot(vName, vType)
{
	pinID = sGetNewSlotId();
	slotPlace = NodeSlot::PlaceEnum::OUTPUT;
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotOutput::NodeSlotOutput(const std::string& vName, const std::string& vType, const bool& vHideName)
	: NodeSlot(vName, vType, vHideName)
{
	pinID = sGetNewSlotId();
	slotPlace = NodeSlot::PlaceEnum::OUTPUT;
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotOutput::NodeSlotOutput(const std::string& vName, const std::string& vType, const bool& vHideName, const bool& vShowWidget)
	: NodeSlot(vName, vType, vHideName, vShowWidget)
{
	pinID = sGetNewSlotId();
	slotPlace = NodeSlot::PlaceEnum::OUTPUT;
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotOutput::~NodeSlotOutput() = default;

void NodeSlotOutput::Init()
{
	
}

void NodeSlotOutput::Unit()
{
	// ici pas besoin du assert sur le m_This 
	// car NodeSlotOutput peut etre isntancié à l'ancienne en copie local donc sans shared_ptr
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
						graphPtr->DisConnectSlot(m_This);
					}
				}
			}
		}
	}
}

void NodeSlotOutput::DrawDebugInfos()
{
	ImGui::Text("--------------------");
	ImGui::Text("Slot %s", name.c_str());
	ImGui::Text(IsAnInput() ? "Input" : "Output");
	ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}
