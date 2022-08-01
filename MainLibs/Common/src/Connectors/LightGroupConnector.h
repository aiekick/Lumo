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
private:
	const std::string m_TypeString = "LIGHT_GROUP";

public:
	bool Connect(NodeSlotPtr vStartSlotPtr, NodeSlotPtr vEndSlotPtr)
	{
		if (vStartSlotPtr->IsAnInput() &&
			vStartSlotPtr->slotType == m_TypeString)
		{
			auto otherLightGroupNodePtr = dynamic_pointer_cast<LightGroupOutputInterface>(vEndSlotPtr->parentNode.getValidShared());
			if (otherLightGroupNodePtr)
			{
				SetLightGroup(otherLightGroupNodePtr->GetLightGroup());
			}

			return true;
		}

		return false;
	}

	bool DisConnect(NodeSlotPtr vStartSlotPtr, NodeSlotPtr vEndSlotPtr)
	{
		if (vStartSlotPtr->IsAnInput() &&
			vStartSlotPtr->slotType == m_TypeString)
		{
			SetLightGroup(SceneLightGroupWeak());

			return true;
		}

		return false;
	}

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

	void SendNotification(BaseNodeWeak vNode)
	{
		auto nodePtr = vNode.getValidShared();
		if (nodePtr)
		{
			auto slots = nodePtr->GetOutputSlotsOfType(m_TypeString);
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
};