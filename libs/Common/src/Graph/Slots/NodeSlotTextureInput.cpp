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

#include "NodeSlotTextureInput.h"
#include <Graph/Base/BaseNode.h>
#include <Interfaces/TextureInputInterface.h>
#include <Interfaces/TextureOutputInterface.h>

#include <utility>
static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotTextureInputPtr NodeSlotTextureInput::Create(NodeSlotTextureInput vSlot)
{
	auto res = std::make_shared<NodeSlotTextureInput>(vSlot);
	res->m_This = res;
	return res;
}

NodeSlotTextureInputPtr NodeSlotTextureInput::Create(const std::string& vName, const uint32_t& vBindingPoint)
{
	auto res = std::make_shared<NodeSlotTextureInput>(vName, vBindingPoint);
	res->m_This = res;
	return res;
}

NodeSlotTextureInputPtr NodeSlotTextureInput::Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName)
{
	auto res = std::make_shared<NodeSlotTextureInput>(vName, vBindingPoint, vHideName);
	res->m_This = res;
	return res;
}

NodeSlotTextureInputPtr NodeSlotTextureInput::Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget)
{
	auto res = std::make_shared<NodeSlotTextureInput>(vName, vBindingPoint, vHideName, vShowWidget);
	res->m_This = res;
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// NODESLOT CLASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotTextureInput::NodeSlotTextureInput()
	: NodeSlotInput("", "TEXTURE_2D")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotTextureInput::NodeSlotTextureInput(const std::string& vName, const uint32_t& vBindingPoint)
	: NodeSlotInput(vName, "TEXTURE_2D")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true; 
	descriptorBinding = vBindingPoint;
}

NodeSlotTextureInput::NodeSlotTextureInput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName)
	: NodeSlotInput(vName, "TEXTURE_2D", vHideName)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
	descriptorBinding = vBindingPoint;
}

NodeSlotTextureInput::NodeSlotTextureInput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget)
	: NodeSlotInput(vName, "TEXTURE_2D", vHideName, vShowWidget)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
	descriptorBinding = vBindingPoint;
}

NodeSlotTextureInput::~NodeSlotTextureInput() = default;

void NodeSlotTextureInput::Init()
{
	
}

void NodeSlotTextureInput::Unit()
{
	// ici pas besoin du assert sur le m_This 
	// car NodeSlotTextureInput peut etre isntancié à l'ancienne en copie local donc sans shared_ptr
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

void NodeSlotTextureInput::OnConnectEvent(NodeSlotWeak vOtherSlot)
{
	if (slotType == "TEXTURE_2D")
	{
		auto endSlotPtr = vOtherSlot.getValidShared();
		if (endSlotPtr)
		{
			auto parentNodePtr = dynamic_pointer_cast<TextureInputInterface<0u>>(parentNode.getValidShared());
			if (parentNodePtr)
			{
				auto otherTextureNodePtr = dynamic_pointer_cast<TextureOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherTextureNodePtr)
				{
					ct::fvec2 textureSize;
					auto descPtr = otherTextureNodePtr->GetDescriptorImageInfo(endSlotPtr->descriptorBinding, &textureSize);
					parentNodePtr->SetTexture(descriptorBinding, descPtr, &textureSize);
				}
				else
				{
					CTOOL_DEBUG_BREAK;
				}
			}
			else
			{
				CTOOL_DEBUG_BREAK;
			}
		}
	}
}

void NodeSlotTextureInput::OnDisConnectEvent(NodeSlotWeak vOtherSlot)
{
	if (slotType == "TEXTURE_2D")
	{
		auto endSlotPtr = vOtherSlot.getValidShared();
		if (endSlotPtr)
		{
			auto parentNodePtr = dynamic_pointer_cast<TextureInputInterface<0u>>(parentNode.getValidShared());
			if (parentNodePtr)
			{
				parentNodePtr->SetTexture(descriptorBinding, nullptr, nullptr);
			}
			else
			{
				CTOOL_DEBUG_BREAK;
			}
		}
	}
}

void NodeSlotTextureInput::TreatNotification(
	const NotifyEvent& vEvent,
	const NodeSlotWeak& vEmitterSlot,
	const NodeSlotWeak& vReceiverSlot)
{
	if (vEvent == TextureUpdateDone)
	{
		auto emiterSlotPtr = vEmitterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto parentNodePtr = parentNode.getValidShared();
				if (parentNodePtr)
				{
					auto parentTextureInputNodePtr = dynamic_pointer_cast<TextureInputInterface<0u>>(parentNodePtr);
					if (parentTextureInputNodePtr)
					{
						auto otherNodePtr = dynamic_pointer_cast<TextureOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
						if (otherNodePtr)
						{
							auto receiverSlotPtr = vReceiverSlot.getValidShared();
							if (receiverSlotPtr)
							{
								ct::fvec2 textureSize;
								auto descPtr = otherNodePtr->GetDescriptorImageInfo(emiterSlotPtr->descriptorBinding, &textureSize);
								parentTextureInputNodePtr->SetTexture(receiverSlotPtr->descriptorBinding, descPtr, &textureSize);
							}
						}
						else
						{
							CTOOL_DEBUG_BREAK;
						}
					}
					else
					{
						CTOOL_DEBUG_BREAK;
					}
				}
			}
		}
	}
}

void NodeSlotTextureInput::MouseDoubleClickedOnSlot(const ImGuiMouseButton& vMouseButton)
{
	if (!linkedSlots.empty())
	{
		BaseNode::SelectForGraphOutput_Callback(linkedSlots[0], vMouseButton);
	}
}

void NodeSlotTextureInput::DrawDebugInfos()
{
	ImGui::Text("--------------------");
	ImGui::Text("Slot %s", name.c_str());
	ImGui::Text(IsAnInput() ? "Input" : "Output");
	ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}
