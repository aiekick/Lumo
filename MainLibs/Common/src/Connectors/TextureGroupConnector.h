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
#include<Interfaces/TextureGroupInputInterface.h>
#include<Interfaces/TextureGroupOutputInterface.h>

template<size_t size_of_array>
class TextureGroupConnector : public TextureGroupInputInterface<size_of_array>
{
private:
	const std::string m_TypeString = "TEXTURE_2D_GROUP";

public:
	bool Connect(NodeSlotPtr vStartSlotPtr, NodeSlotPtr vEndSlotPtr)
	{
		if (vStartSlotPtr->slotType == m_TypeString)
		{
			auto otherTextureGroupNodePtr = dynamic_pointer_cast<TextureGroupOutputInterface>(vEndSlotPtr->parentNode.getValidShared());
			if (otherTextureGroupNodePtr)
			{
				ct::fvec2 textureSize;
				auto descPtr = otherTextureGroupNodePtr->GetDescriptorImageInfo(vEndSlotPtr->descriptorBinding, &textureSize);
				SetTextureGroup(vStartSlotPtr->descriptorBinding, descPtr, &textureSize);
			}

			return true;
		}

		return false;
	}

	bool DisConnect(NodeSlotPtr vStartSlotPtr, NodeSlotPtr vEndSlotPtr)
	{
		if (vStartSlotPtr->slotType == m_TypeString)
		{
			SetTextureGroup(SceneTextureGroupWeak());

			return true;
		}

		return false;
	}

	void NotificationReceived(NodeSlotWeak vEmitterSlot, NodeSlotWeak vReceiverSlot)
	{
		auto emiterSlotPtr = vEmitterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto otherNodePtr = dynamic_pointer_cast<TextureGroupOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					auto receiverSlotPtr = vReceiverSlot.getValidShared();
					if (receiverSlotPtr)
					{
						fvec2Vector arr; // tofix : je sens les emmerdes a ce transfert de pointeurs dans un scope court 
						auto descsPtr = otherNodePtr->GetDescriptorImageInfos(emiterSlotPtr->descriptorBinding, &arr);
						SetTextures(receiverSlotPtr->descriptorBinding, descsPtr, &arr);
					}
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
					slotPtr->Notify(NotifyEvent::TextureGroupUpdateDone, slot);
				}
			}
		}
	}
};