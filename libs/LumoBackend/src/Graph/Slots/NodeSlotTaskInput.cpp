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

#include <LumoBackend/Graph/Slots/NodeSlotTaskInput.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Interfaces/TaskInterface.h>

#include <utility>
static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotTaskInputPtr NodeSlotTaskInput::Create(NodeSlotTaskInput vSlot)
{
	auto res = std::make_shared<NodeSlotTaskInput>(vSlot);
	res->m_This = res;
	return res;
}

NodeSlotTaskInputPtr NodeSlotTaskInput::Create(const std::string& vName)
{
	auto res = std::make_shared<NodeSlotTaskInput>(vName);
	res->m_This = res;
	return res;
}

NodeSlotTaskInputPtr NodeSlotTaskInput::Create(const std::string& vName, const bool& vHideName)
{
	auto res = std::make_shared<NodeSlotTaskInput>(vName, vHideName);
	res->m_This = res;
	return res;
}

NodeSlotTaskInputPtr NodeSlotTaskInput::Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget)
{
	auto res = std::make_shared<NodeSlotTaskInput>(vName, vHideName, vShowWidget);
	res->m_This = res;
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// NODESLOT CLASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotTaskInput::NodeSlotTaskInput()
	: NodeSlotInput("", "TASK")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotTaskInput::NodeSlotTaskInput(const std::string& vName)
	: NodeSlotInput(vName, "TASK")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotTaskInput::NodeSlotTaskInput(const std::string& vName, const bool& vHideName)
	: NodeSlotInput(vName, "TASK", vHideName)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotTaskInput::NodeSlotTaskInput(const std::string& vName, const bool& vHideName, const bool& vShowWidget)
	: NodeSlotInput(vName, "TASK", vHideName, vShowWidget)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotTaskInput::~NodeSlotTaskInput() = default;

void NodeSlotTaskInput::Init()
{
	
}

void NodeSlotTaskInput::Unit()
{
	// ici pas besoin du assert sur le m_This 
	// car NodeSlotTaskInput peut etre isntancié à l'ancienne en copie local donc sans shared_ptr
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

void NodeSlotTaskInput::OnConnectEvent(NodeSlotWeak vOtherSlot)
{
	if (slotType == "TASK")
	{
        /*auto endSlotPtr = vOtherSlot.lock();
		if (endSlotPtr)
		{
			auto parentNodePtr = dynamic_pointer_cast<TaskInterface>(parentNode.lock());
			if (parentNodePtr)
			{
                auto otherTaskNodePtr = dynamic_pointer_cast<TaskInterface>(endSlotPtr->parentNode.lock());
				if (otherTaskNodePtr)
				{
					parentNodePtr->SetTask(otherTaskNodePtr->GetTask());
				}
			}
		}*/
	}
}

void NodeSlotTaskInput::OnDisConnectEvent(NodeSlotWeak vOtherSlot)
{
	if (slotType == "TASK")
	{
        /*auto endSlotPtr = vOtherSlot.lock();
		if (endSlotPtr)
		{
			auto parentNodePtr = dynamic_pointer_cast<TaskInputInterface>(parentNode.lock());
			if (parentNodePtr)
			{
				parentNodePtr->SetTask(SceneTaskWeak());
			}
		}*/
	}
}

void NodeSlotTaskInput::TreatNotification(
	const NotifyEvent& vEvent,
	const NodeSlotWeak& vEmitterSlot,
	const NodeSlotWeak& /*vReceiverSlot*/)
{
    /*if (vEvent == TaskUpdateDone)
	{
		auto emiterSlotPtr = vEmitterSlot.lock();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto parentTaskInputNodePtr = dynamic_pointer_cast<TaskInputInterface>(parentNode.lock());
				if (parentTaskInputNodePtr)
				{
					auto otherNodePtr = dynamic_pointer_cast<TaskOutputInterface>(emiterSlotPtr->parentNode.lock());
					if (otherNodePtr)
					{
						parentTaskInputNodePtr->SetTask(otherNodePtr->GetTask());
					}
				}
			}
		}
	}*/
}

void NodeSlotTaskInput::DrawDebugInfos()
{
	ImGui::Text("--------------------");
	ImGui::Text("Slot %s", name.c_str());
	ImGui::Text(IsAnInput() ? "Input" : "Output");
	ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}
