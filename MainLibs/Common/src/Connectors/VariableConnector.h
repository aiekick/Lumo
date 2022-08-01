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

#include<Graph/Base/BaseNode.h>
#include<Interfaces/VariableInputInterface.h>
#include<Interfaces/VariableOutputInterface.h>

enum class VariableType
{
	BOOLEAN = 0,
	FLAOT,
	INT,
	UINT
};

template<size_t size_of_array>
class VariableConnector : public VariableInputInterface<size_of_array>
{
private:
	const std::set<std::string> m_TypeStrings = { "TYPE_BOOLEAN", "TYPE_FLOAT", "TYPE_INT", "TYPE_UINT" };

private:
	void NotificationReceived(NodeSlotWeak vEmitterSlot, NodeSlotWeak vReceiverSlot)
	{
		auto emiterSlotPtr = vEmitterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto otherNodePtr = dynamic_pointer_cast<VariableOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					auto receiverSlotPtr = vReceiverSlot.getValidShared();
					if (receiverSlotPtr)
					{
						SetVariable(receiverSlotPtr->variableIndex, otherNodePtr->GetVariable(emiterSlotPtr->variableIndex));
					}
				}
			}
		}
	}

public:
	void Connect(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
	{
		auto startSlotPtr = vStartSlot.getValidShared();
		auto endSlotPtr = vEndSlot.getValidShared();
		if (startSlotPtr && endSlotPtr)
		{
			if (startSlotPtr->IsAnInput() &&
				m_TypeStrings.find(startSlotPtr->slotType) != m_TypeStrings.end())
			{
				auto otherNodePtr = dynamic_pointer_cast<VariableOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					SetVariable(startSlotPtr->variableIndex, otherNodePtr->GetVariable(endSlotPtr->variableIndex));
				}
			}
		}
	}

	void DisConnect(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
	{
		auto startSlotPtr = vStartSlot.getValidShared();
		auto endSlotPtr = vEndSlot.getValidShared();
		if (startSlotPtr && endSlotPtr)
		{
			if (startSlotPtr->IsAnInput() &&
				m_TypeStrings.find(startSlotPtr->slotType) != m_TypeStrings.end())
			{
				SetVariable(startSlotPtr->variableIndex);
			}
		}
	}

	static void SendNotification(BaseNodeWeak vNode)
	{
		auto nodePtr = vNode.getValidShared();
		if (nodePtr)
		{
			for (auto varType : m_TypeStrings)
			{
				auto slots = nodePtr->GetOutputSlotsOfType(varType);
				for (const auto& slot : slots)
				{
					auto slotPtr = slot.getValidShared();
					if (slotPtr)
					{
						slotPtr->Notify(NotifyEvent::VariableUpdateDone, slot);
					}
				}
			}
		}
	}

	void TreatNotification(
		const NotifyEvent& vEvent,
		const BaseNodeWeak& vBaseNode,
		const NodeSlotWeak& vEmitterSlot,
		const NodeSlotWeak& vReceiverSlot = NodeSlotWeak())
	{
		if (vEvent == NotifyEvent::VariableUpdateDone)
		{
			NotificationReceived(vEmitterSlot, vReceiverSlot);
			SendNotification(vBaseNode);
		}
	}
};