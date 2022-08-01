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
public:
	static const std::string& GetSlotType()
	{
		static std::string m_TypeString = "TEXTURE_2D_GROUP";
		return m_TypeString;
	}

private:
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

public:
	void Connect(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
	{
		auto startSlotPtr = vStartSlot.getValidShared();
		auto endSlotPtr = vEndSlot.getValidShared();
		if (startSlotPtr && endSlotPtr)
		{
			if (startSlotPtr->slotType == GetSlotType())
			{
				auto otherTextureGroupNodePtr = dynamic_pointer_cast<TextureGroupOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherTextureGroupNodePtr)
				{
					ct::fvec2 textureSize;
					auto descPtr = otherTextureGroupNodePtr->GetDescriptorImageInfo(endSlotPtr->descriptorBinding, &textureSize);
					SetTextureGroup(startSlotPtr->descriptorBinding, descPtr, &textureSize);
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
			if (startSlotPtr->slotType == GetSlotType())
			{
				SetTextureGroup(SceneTextureGroupWeak());
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
					slotPtr->Notify(NotifyEvent::TextureGroupUpdateDone, slot);
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
		if (vEvent == NotifyEvent::TextureGroupUpdateDone)
		{
			NotificationReceived(vEmitterSlot, vReceiverSlot);
			SendNotification(vBaseNode);
		}
	}
};