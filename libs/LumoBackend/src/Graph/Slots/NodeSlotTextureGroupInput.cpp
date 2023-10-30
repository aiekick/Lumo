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

#include <LumoBackend/Graph/Slots/NodeSlotTextureGroupInput.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Interfaces/TextureGroupInputInterface.h>
#include <LumoBackend/Interfaces/TextureGroupOutputInterface.h>

#include <utility>
static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotTextureGroupInputPtr NodeSlotTextureGroupInput::Create(NodeSlotTextureGroupInput vSlot)
{
	auto res = std::make_shared<NodeSlotTextureGroupInput>(vSlot);
	res->m_This = res;
	return res;
}

NodeSlotTextureGroupInputPtr NodeSlotTextureGroupInput::Create(const std::string& vName)
{
	auto res = std::make_shared<NodeSlotTextureGroupInput>(vName);
	res->m_This = res;
	return res;
}

NodeSlotTextureGroupInputPtr NodeSlotTextureGroupInput::Create(const std::string& vName, const bool& vHideName)
{
	auto res = std::make_shared<NodeSlotTextureGroupInput>(vName, vHideName);
	res->m_This = res;
	return res;
}

NodeSlotTextureGroupInputPtr NodeSlotTextureGroupInput::Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget)
{
	auto res = std::make_shared<NodeSlotTextureGroupInput>(vName, vHideName, vShowWidget);
	res->m_This = res;
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// NODESLOT CLASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotTextureGroupInput::NodeSlotTextureGroupInput()
	: NodeSlotInput("", "TEXTURE_2D_GROUP")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotTextureGroupInput::NodeSlotTextureGroupInput(const std::string& vName)
	: NodeSlotInput(vName, "TEXTURE_2D_GROUP")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotTextureGroupInput::NodeSlotTextureGroupInput(const std::string& vName, const bool& vHideName)
	: NodeSlotInput(vName, "TEXTURE_2D_GROUP", vHideName)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotTextureGroupInput::NodeSlotTextureGroupInput(const std::string& vName, const bool& vHideName, const bool& vShowWidget)
	: NodeSlotInput(vName, "TEXTURE_2D_GROUP", vHideName, vShowWidget)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotTextureGroupInput::~NodeSlotTextureGroupInput() = default;

void NodeSlotTextureGroupInput::Init()
{
	
}

void NodeSlotTextureGroupInput::Unit()
{
	// ici pas besoin du assert sur le m_This 
	// car NodeSlotTextureGroupInput peut etre isntancié à l'ancienne en copie local donc sans shared_ptr
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

void NodeSlotTextureGroupInput::OnConnectEvent(NodeSlotWeak vOtherSlot)
{
	if (slotType == "TEXTURE_2D_GROUP")
	{
		auto endSlotPtr = vOtherSlot.lock();
		if (endSlotPtr)
		{
			auto parentNodePtr = dynamic_pointer_cast<TextureGroupInputInterface<0u>>(parentNode.lock());
			if (parentNodePtr)
			{
				auto otherTextureGroupNodePtr = dynamic_pointer_cast<TextureGroupOutputInterface>(endSlotPtr->parentNode.lock());
				if (otherTextureGroupNodePtr)
				{
					fvec2Vector textureSizes;
					auto descPtr = otherTextureGroupNodePtr->GetDescriptorImageInfos(endSlotPtr->descriptorBinding, &textureSizes);
					parentNodePtr->SetTextures(descriptorBinding, descPtr, &textureSizes);
				}
			}
		}
	}
}

void NodeSlotTextureGroupInput::OnDisConnectEvent(NodeSlotWeak vOtherSlot)
{
	if (slotType == "TEXTURE_2D_GROUP")
	{
		auto endSlotPtr = vOtherSlot.lock();
		if (endSlotPtr)
		{
			auto parentNodePtr = dynamic_pointer_cast<TextureGroupInputInterface<0u>>(parentNode.lock());
			if (parentNodePtr)
			{
				parentNodePtr->SetTextures(descriptorBinding, nullptr, nullptr);
			}
		}
	}
}

void NodeSlotTextureGroupInput::TreatNotification(
	const NotifyEvent& vEvent,
	const NodeSlotWeak& vEmitterSlot,
	const NodeSlotWeak& vReceiverSlot)
{
	if (vEvent == TextureGroupUpdateDone)
	{
		auto emiterSlotPtr = vEmitterSlot.lock();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto parentNodePtr = parentNode.lock();
				if (parentNodePtr)
				{
					auto parentTextureGroupInputNodePtr = dynamic_pointer_cast<TextureGroupInputInterface<0u>>(parentNodePtr);
					if (parentTextureGroupInputNodePtr)
					{
						auto otherTextureGroupNodePtr = dynamic_pointer_cast<TextureGroupOutputInterface>(emiterSlotPtr->parentNode.lock());
						if (otherTextureGroupNodePtr)
						{
							auto receiverSlotPtr = vReceiverSlot.lock();
							if (receiverSlotPtr)
							{
								fvec2Vector textureSizes;
								auto descPtr = otherTextureGroupNodePtr->GetDescriptorImageInfos(emiterSlotPtr->descriptorBinding, &textureSizes);
								parentTextureGroupInputNodePtr->SetTextures(receiverSlotPtr->descriptorBinding, descPtr, &textureSizes);
							}
						}
					}
				}
			}
		}
	}
}

void NodeSlotTextureGroupInput::DrawDebugInfos()
{
	ImGui::Text("--------------------");
	ImGui::Text("Slot %s", name.c_str());
	ImGui::Text(IsAnInput() ? "Input" : "Output");
	ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}
