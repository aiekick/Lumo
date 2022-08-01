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

#pragma once

#include<Graph/Base/BaseNode.h>
#include<Interfaces/TexelBufferInputInterface.h>
#include<Interfaces/TexelBufferOutputInterface.h>

template<size_t size_of_array>
class TexelBufferConnector : public TexelBufferInputInterface<size_of_array>
{
public:
	bool Connect(NodeSlotPtr vStartSlotPtr, NodeSlotPtr vEndSlotPtr, const std::string& vType)
	{
		if (vStartSlotPtr->IsAnInput() &&
			vStartSlotPtr->slotType == vType)
		{
			auto otherTextureNodePtr = dynamic_pointer_cast<TexelBufferOutputInterface>(endSlotPtr->parentNode.getValidShared());
			if (otherTextureNodePtr)
			{
				ct::uvec2 texelBufferSize;
				auto bufferPtr = otherTextureNodePtr->GetTexelBuffer(endSlotPtr->descriptorBinding, &texelBufferSize);
				SetTexelBuffer(startSlotPtr->descriptorBinding, bufferPtr, &texelBufferSize);
				auto bufferViewPtr = otherTextureNodePtr->GetTexelBufferView(endSlotPtr->descriptorBinding, &texelBufferSize);
				SetTexelBufferView(startSlotPtr->descriptorBinding, bufferViewPtr, &texelBufferSize);
			}

			return true;
		}

		return false;
	}

	bool DisConnect(NodeSlotPtr vStartSlotPtr, NodeSlotPtr vEndSlotPtr, const std::string& vType)
	{
		if (vStartSlotPtr->IsAnInput() &&
			vStartSlotPtr->slotType == vType)
		{
			SetTexelBuffer(startSlotPtr->descriptorBinding, nullptr, nullptr);

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
				auto otherNodePtr = dynamic_pointer_cast<TexelBufferOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					auto receiverSlotPtr = vReceiverSlot.getValidShared();
					if (receiverSlotPtr)
					{
						ct::uvec2 texelBufferSize;
						auto bufferPtr = otherNodePtr->GetTexelBuffer(emiterSlotPtr->descriptorBinding, &texelBufferSize);
						SetTexelBuffer(receiverSlotPtr->descriptorBinding, bufferPtr, &texelBufferSize);
						auto bufferViewPtr = otherNodePtr->GetTexelBufferView(emiterSlotPtr->descriptorBinding, &texelBufferSize);
						SetTexelBufferView(receiverSlotPtr->descriptorBinding, bufferViewPtr, &texelBufferSize);
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
					slotPtr->Notify(NotifyEvent::TexelBufferUpdateDone, slot);
				}
			}
		}
	}
};