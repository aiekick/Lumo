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

#include "PosToDepthNode.h"
#include <Modules/Utils/PosToDepthModule.h>

std::shared_ptr<PosToDepthNode> PosToDepthNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<PosToDepthNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

PosToDepthNode::PosToDepthNode() : BaseNode()
{
	m_NodeTypeString = "POS_TO_DEPTH";
}

PosToDepthNode::~PosToDepthNode()
{
	Unit();
}

bool PosToDepthNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Pos To Depth";

	NodeSlot slot;

	slot.slotType = NodeSlotTypeEnum::TEXTURE_2D;
	slot.name = "Pos";
	slot.descriptorBinding = 0U;
	AddInput(slot, true, false);

	slot.slotType = NodeSlotTypeEnum::DEPTH;
	slot.name = "Depth";
	slot.descriptorBinding = 0U;
	AddOutput(slot, true, false);

	bool res = false;

	m_PosToDepthModulePtr = PosToDepthModule::Create(vVulkanCorePtr);
	if (m_PosToDepthModulePtr)
	{
		res = true;
	}

	return res;
}

bool PosToDepthNode::Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd);

	// for update input texture buffer infos => avoid vk crash
	UpdateInputDescriptorImageInfos(m_Inputs);

	if (m_PosToDepthModulePtr)
	{
		return m_PosToDepthModulePtr->Execute(vCurrentFrame, vCmd);
	}
	return false;
}

bool PosToDepthNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_PosToDepthModulePtr)
	{
		return m_PosToDepthModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void PosToDepthNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	if (m_PosToDepthModulePtr)
	{
		m_PosToDepthModulePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void PosToDepthNode::DisplayInfosOnTopOfTheNode(BaseNodeStateStruct* vCanvasState)
{
	if (vCanvasState && vCanvasState->debug_mode)
	{
		auto drawList = nd::GetNodeBackgroundDrawList(nodeID);
		if (drawList)
		{
			char debugBuffer[255] = "\0";
			snprintf(debugBuffer, 254,
				"Used(%s)\nCell(%i, %i)"/*\nPos(%.1f, %.1f)\nSize(%.1f, %.1f)*/,
				(used ? "true" : "false"), cell.x, cell.y/*, pos.x, pos.y, size.x, size.y*/);
			ImVec2 txtSize = ImGui::CalcTextSize(debugBuffer);
			drawList->AddText(pos - ImVec2(0, txtSize.y), ImGui::GetColorU32(ImGuiCol_Text), debugBuffer);
		}
	}
}

void PosToDepthNode::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	if (m_PosToDepthModulePtr)
	{
		m_PosToDepthModulePtr->NeedResize(vNewSize, vCountColorBuffer);
	}

	// on fait ca apres
	BaseNode::NeedResize(vNewSize, vCountColorBuffer);
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void PosToDepthNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_PosToDepthModulePtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == NodeSlotTypeEnum::TEXTURE_2D)
			{
				auto otherTextureNodePtr = dynamic_pointer_cast<TextureOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherTextureNodePtr)
				{
					SetTexture(startSlotPtr->descriptorBinding, otherTextureNodePtr->GetDescriptorImageInfo(0U)); // output
				}
			}
		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void PosToDepthNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_PosToDepthModulePtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == NodeSlotTypeEnum::TEXTURE_2D)
			{
				SetTexture(startSlotPtr->descriptorBinding, nullptr);
			}
		}
	}
}

void PosToDepthNode::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	if (m_PosToDepthModulePtr)
	{
		m_PosToDepthModulePtr->SetTexture(vBinding, vImageInfo);
	}
}

vk::DescriptorImageInfo* PosToDepthNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	if (m_PosToDepthModulePtr)
	{
		return m_PosToDepthModulePtr->GetDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

void PosToDepthNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::TextureUpdateDone:
	{
		auto emiterSlotPtr = vEmmiterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto otherNodePtr = dynamic_pointer_cast<TextureOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					auto receiverSlotPtr = vReceiverSlot.getValidShared();
					if (receiverSlotPtr)
					{
						SetTexture(receiverSlotPtr->descriptorBinding, otherNodePtr->GetDescriptorImageInfo(emiterSlotPtr->descriptorBinding)); // output
					}
				}
			}
		}
		break;
	}
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string PosToDepthNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_PosToDepthModulePtr)
		{
			res += m_PosToDepthModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool PosToDepthNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_PosToDepthModulePtr)
	{
		m_PosToDepthModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

void PosToDepthNode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_PosToDepthModulePtr)
	{
		m_PosToDepthModulePtr->UpdateShaders(vFiles);
	}
}