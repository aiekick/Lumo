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

#include <LumoBackend/Graph/Slots/NodeSlotTextureCubeInput.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Interfaces/TextureCubeInputInterface.h>
#include <LumoBackend/Interfaces/TextureCubeOutputInterface.h>

#include <utility>
static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotTextureCubeInputPtr NodeSlotTextureCubeInput::Create(NodeSlotTextureCubeInput vSlot)
{
	auto res = std::make_shared<NodeSlotTextureCubeInput>(vSlot);
	res->m_This = res;
	return res;
}

NodeSlotTextureCubeInputPtr NodeSlotTextureCubeInput::Create(const std::string& vName, const uint32_t& vBindingPoint)
{
	auto res = std::make_shared<NodeSlotTextureCubeInput>(vName, vBindingPoint);
	res->m_This = res;
	return res;
}

NodeSlotTextureCubeInputPtr NodeSlotTextureCubeInput::Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName)
{
	auto res = std::make_shared<NodeSlotTextureCubeInput>(vName, vBindingPoint, vHideName);
	res->m_This = res;
	return res;
}

NodeSlotTextureCubeInputPtr NodeSlotTextureCubeInput::Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget)
{
	auto res = std::make_shared<NodeSlotTextureCubeInput>(vName, vBindingPoint, vHideName, vShowWidget);
	res->m_This = res;
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// NODESLOT CLASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotTextureCubeInput::NodeSlotTextureCubeInput()
	: NodeSlotInput("", "TEXTURE_CUBE")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotTextureCubeInput::NodeSlotTextureCubeInput(const std::string& vName, const uint32_t& vBindingPoint)
	: NodeSlotInput(vName, "TEXTURE_CUBE")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true; 
	descriptorBinding = vBindingPoint;
}

NodeSlotTextureCubeInput::NodeSlotTextureCubeInput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName)
	: NodeSlotInput(vName, "TEXTURE_CUBE", vHideName)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
	descriptorBinding = vBindingPoint;
}

NodeSlotTextureCubeInput::NodeSlotTextureCubeInput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget)
	: NodeSlotInput(vName, "TEXTURE_CUBE", vHideName, vShowWidget)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
	descriptorBinding = vBindingPoint;
}

NodeSlotTextureCubeInput::~NodeSlotTextureCubeInput() = default;

void NodeSlotTextureCubeInput::Init()
{
	
}

void NodeSlotTextureCubeInput::Unit()
{
	// ici pas besoin du assert sur le m_This 
	// car NodeSlotTextureCubeInput peut etre isntancié à l'ancienne en copie local donc sans shared_ptr
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

void NodeSlotTextureCubeInput::OnConnectEvent(NodeSlotWeak vOtherSlot)
{
	if (slotType == "TEXTURE_CUBE")
	{
		auto endSlotPtr = vOtherSlot.lock();
		if (endSlotPtr)
		{
			auto parentNodePtr = dynamic_pointer_cast<TextureCubeInputInterface<0u>>(parentNode.lock());
			if (parentNodePtr)
			{
				auto otherTextureNodePtr = dynamic_pointer_cast<TextureCubeOutputInterface>(endSlotPtr->parentNode.lock());
				if (otherTextureNodePtr)
				{
					ct::fvec2 textureSize;
					auto descPtr = otherTextureNodePtr->GetTextureCube(endSlotPtr->descriptorBinding, &textureSize);
					parentNodePtr->SetTextureCube(descriptorBinding, descPtr, &textureSize);
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

void NodeSlotTextureCubeInput::OnDisConnectEvent(NodeSlotWeak vOtherSlot)
{
	if (slotType == "TEXTURE_CUBE")
	{
		auto endSlotPtr = vOtherSlot.lock();
		if (endSlotPtr)
		{
			auto parentNodePtr = dynamic_pointer_cast<TextureCubeInputInterface<0u>>(parentNode.lock());
			if (parentNodePtr)
			{
				parentNodePtr->SetTextureCube(descriptorBinding, nullptr, nullptr);
			}
			else
			{
				CTOOL_DEBUG_BREAK;
			}
		}
	}
}

void NodeSlotTextureCubeInput::TreatNotification(
	const NotifyEvent& vEvent,
	const NodeSlotWeak& vEmitterSlot,
	const NodeSlotWeak& vReceiverSlot)
{
	if (vEvent == TextureUpdateDone)
	{
		auto emiterSlotPtr = vEmitterSlot.lock();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto parentNodePtr = parentNode.lock();
				if (parentNodePtr)
				{
					auto parentTextureInputNodePtr = dynamic_pointer_cast<TextureCubeInputInterface<0u>>(parentNodePtr);
					if (parentTextureInputNodePtr)
					{
						auto otherNodePtr = dynamic_pointer_cast<TextureCubeOutputInterface>(emiterSlotPtr->parentNode.lock());
						if (otherNodePtr)
						{
							auto receiverSlotPtr = vReceiverSlot.lock();
							if (receiverSlotPtr)
							{
								ct::fvec2 textureSize;
								auto descPtr = otherNodePtr->GetTextureCube(emiterSlotPtr->descriptorBinding, &textureSize);
								parentTextureInputNodePtr->SetTextureCube(receiverSlotPtr->descriptorBinding, descPtr, &textureSize);
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

void NodeSlotTextureCubeInput::DrawDebugInfos()
{
	ImGui::Text("--------------------");
	ImGui::Text("Slot %s", name.c_str());
	ImGui::Text(IsAnInput() ? "Input" : "Output");
	ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}
