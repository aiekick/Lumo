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
#include<Interfaces/ModelInputInterface.h>
#include<Interfaces/ModelOutputInterface.h>

class ModelConnector : public ModelInputInterface
{
public:
	static const std::string& GetSlotType()
	{
		static std::string m_TypeString = "MESH";
		return m_TypeString;
	}

private:
	void NotificationReceived(
		NodeSlotWeak vEmitterSlot, 
		NodeSlotWeak vReceiverSlot)
	{
		auto emiterSlotPtr = vEmitterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto otherModelNodePtr = dynamic_pointer_cast<ModelOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherModelNodePtr)
				{
					SetModel(otherModelNodePtr->GetModel());
				}
			}
		}
	}

public:
	void Connect(
		NodeSlotWeak vStartSlot,
		NodeSlotWeak vEndSlot)
	{
		auto startSlotPtr = vStartSlot.getValidShared();
		auto endSlotPtr = vEndSlot.getValidShared();
		if (startSlotPtr && endSlotPtr)
		{
			if (startSlotPtr->IsAnInput() &&
				startSlotPtr->slotType == GetSlotType())
			{
				auto otherModelNodePtr = dynamic_pointer_cast<ModelOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherModelNodePtr)
				{
					SetModel(otherModelNodePtr->GetModel());
				}
			}
		}
	}

	void DisConnect(
		NodeSlotWeak vStartSlot, 
		NodeSlotWeak vEndSlot)
	{
		auto startSlotPtr = vStartSlot.getValidShared();
		auto endSlotPtr = vEndSlot.getValidShared();
		if (startSlotPtr && endSlotPtr)
		{
			if (startSlotPtr->IsAnInput() &&
				startSlotPtr->slotType == GetSlotType())
			{
				SetModel(SceneModelWeak());
			}
		}
	}

	static void SendNotification(
		BaseNodeWeak vNode)
	{
		auto nodePtr = vNode.getValidShared();
		if (nodePtr)
		{
			auto slots = nodePtr->GetOutputSlotsOfType(GetSlotType());
			for (const auto& slot : slots)
			{
				auto slotPtr = slot.getValidShared();
				if (slotPtr)
				{
					slotPtr->Notify(NotifyEvent::ModelUpdateDone, slot);
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
		if (vEvent == NotifyEvent::ModelUpdateDone)
		{
			NotificationReceived(vEmitterSlot, vReceiverSlot);
			SendNotification(vBaseNode);
		}
	}
};