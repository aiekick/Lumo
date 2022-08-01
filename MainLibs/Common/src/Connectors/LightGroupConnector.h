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
#include<Interfaces/LightGroupInputInterface.h>
#include<Interfaces/LightGroupOutputInterface.h>

class LightGroupConnector : public LightGroupInputInterface
{
public:
	static const std::string& GetSlotType()
	{
		static std::string m_TypeString = "LIGHT_GROUP";
		return m_TypeString;
	}

private:
	void NotificationReceived(NodeSlotWeak vEmitterSlot, NodeSlotWeak /*vReceiverSlot*/)
	{
		auto emiterSlotPtr = vEmitterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto otherLightGroupNodePtr = dynamic_pointer_cast<LightGroupOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherLightGroupNodePtr)
				{
					SetLightGroup(otherLightGroupNodePtr->GetLightGroup());
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
				startSlotPtr->slotType == GetSlotType())
			{
				auto otherLightGroupNodePtr = dynamic_pointer_cast<LightGroupOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherLightGroupNodePtr)
				{
					SetLightGroup(otherLightGroupNodePtr->GetLightGroup());
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
				startSlotPtr->slotType == GetSlotType())
			{
				SetLightGroup(SceneLightGroupWeak());
			}
		}
	}

	static void SendNotification(BaseNodeWeak vNode)
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
					slotPtr->Notify(NotifyEvent::LightGroupUpdateDone, slot);
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
		if (vEvent == NotifyEvent::LightGroupUpdateDone)
		{
			NotificationReceived(vEmitterSlot, vReceiverSlot);
			SendNotification(vBaseNode);
		}
	}
};