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

#include "BreakTexturesGroupNode.h"
#include <Interfaces/LightGroupOutputInterface.h>
#include <Interfaces/TextureGroupOutputInterface.h>

std::shared_ptr<BreakTexturesGroupNode> BreakTexturesGroupNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<BreakTexturesGroupNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

BreakTexturesGroupNode::BreakTexturesGroupNode() : BaseNode()
{
	m_NodeTypeString = "BREAK_TEXTURE_2D_GROUP";
}

BreakTexturesGroupNode::~BreakTexturesGroupNode()
{
	Unit();
}

bool BreakTexturesGroupNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Break Tex2D Group";

	NodeSlot slot;
	slot.slotType = NodeSlotTypeEnum::TEXTURE_2D_GROUP;
	slot.name = "Textures";
	slot.descriptorBinding = 0U; // target a texture input
	AddInput(slot, true, false);

	return true;
}

void BreakTexturesGroupNode::Unit()
{

}

bool BreakTexturesGroupNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureGroupInputDescriptorImageInfos(m_Inputs);

	return true;
}

bool BreakTexturesGroupNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	return false;
}

void BreakTexturesGroupNode::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

}

void BreakTexturesGroupNode::SetTextures(
	const uint32_t& vBinding, 
	DescriptorImageInfoVector* vImageInfos, 
	fvec2Vector* vOutSizes)
{
	m_Textures.clear();

	if (vImageInfos)
	{
		uint32_t idx = 0U;
		for (auto& info : *vImageInfos)
		{
			m_Textures.push_back(vImageInfos->at(idx));
			++idx;
		}
	}
}

vk::DescriptorImageInfo* BreakTexturesGroupNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (vBindingPoint < (uint32_t)m_Textures.size())
	{
		return &m_Textures[vBindingPoint];
	}

	return nullptr;
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void BreakTexturesGroupNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == NodeSlotTypeEnum::TEXTURE_2D_GROUP)
			{
				auto otherTextureNodePtr = dynamic_pointer_cast<TextureGroupOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherTextureNodePtr)
				{
					fvec2Vector arr; // tofix : je sens les emmerdes a ce transfert de pointeurs dans un scope court 
					auto descsPtr = otherTextureNodePtr->GetDescriptorImageInfos(endSlotPtr->descriptorBinding, &arr);
					SetTextures(startSlotPtr->descriptorBinding, descsPtr, &arr);
					ReorganizeSlots();
				}
			}
		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void BreakTexturesGroupNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == NodeSlotTypeEnum::TEXTURE_2D_GROUP)
			{
				SetTextures(startSlotPtr->descriptorBinding, nullptr, nullptr);
				ReorganizeSlots();
			}
		}
	}
}

void BreakTexturesGroupNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::TextureGroupUpdateDone:
	{
		auto emiterSlotPtr = vEmmiterSlot.getValidShared();
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

		//todo emit notification
		break;
	}
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string BreakTexturesGroupNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string res;

	if (!m_ChildNodes.empty())
	{
		res += BaseNode::getXml(vOffset, vUserDatas);
	}
	else
	{
		res += vOffset + ct::toStr("<node name=\"%s\" type=\"%s\" pos=\"%s\" id=\"%u\">\n",
			name.c_str(),
			m_NodeTypeString.c_str(),
			ct::fvec2(pos.x, pos.y).string().c_str(),
			(uint32_t)nodeID.Get());

		for (auto slot : m_Inputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		for (auto slot : m_Outputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool BreakTexturesGroupNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	BaseNode::setFromXml(vElem, vParent, vUserDatas);

	return true;
}

void BreakTexturesGroupNode::ReorganizeSlots()
{
	m_Outputs.clear();
	
	uint32_t idx = 0U;
	for (auto& info : m_Textures)
	{
		NodeSlot slot;
		slot.slotType = NodeSlotTypeEnum::TEXTURE_2D;
		slot.name = ct::toStr("Output %u", idx);
		slot.descriptorBinding = idx;
		AddOutput(slot, true, true);

		++idx;
	}
}
