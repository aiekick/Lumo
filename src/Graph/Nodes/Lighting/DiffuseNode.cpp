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

#include "DiffuseNode.h"
#include <Modules/Lighting/DiffuseModule.h>
#include <Interfaces/LightGroupOutputInterface.h>

std::shared_ptr<DiffuseNode> DiffuseNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<DiffuseNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

DiffuseNode::DiffuseNode() : BaseNode()
{
	m_NodeTypeString = "DIFFUSE";
}

DiffuseNode::~DiffuseNode()
{
	Unit();
}

bool DiffuseNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Diffuse";

	NodeSlot slot;

	slot.slotType = "LIGHT_GROUP";
	slot.name = "Lights";
	slot.descriptorBinding = 0U;
	AddInput(slot, true, false);

	slot.slotType = "TEXTURE_2D";
	slot.name = "Position";
	slot.descriptorBinding = 0U;
	AddInput(slot, true, false);

	slot.slotType = "TEXTURE_2D";
	slot.name = "Normal";
	slot.descriptorBinding = 1U;
	AddInput(slot, true, false);

	slot.slotType = "TEXTURE_2D";
	slot.name = "Output";
	slot.descriptorBinding = 0U;
	AddOutput(slot, true, true);

	bool res = false;

	m_DiffuseModulePtr = DiffuseModule::Create(vVulkanCorePtr);
	if (m_DiffuseModulePtr)
	{
		res = true;
	}

	return res;
}

bool DiffuseNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureInputDescriptorImageInfos(m_Inputs);

	if (m_DiffuseModulePtr)
	{
		return m_DiffuseModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}
	return false;
}

bool DiffuseNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_DiffuseModulePtr)
	{
		return m_DiffuseModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void DiffuseNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_DiffuseModulePtr)
	{
		m_DiffuseModulePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void DiffuseNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
{
	if (vBaseNodeState && vBaseNodeState->debug_mode)
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

void DiffuseNode::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (m_DiffuseModulePtr)
	{
		m_DiffuseModulePtr->NeedResize(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResize(vNewSize, vCountColorBuffers);
}

// le start est toujours le slot de ce node, l'autre le slot du node connect?
void DiffuseNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_DiffuseModulePtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == "TEXTURE_2D")
			{
				auto otherTextureNodePtr = dynamic_pointer_cast<TextureOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherTextureNodePtr)
				{
					ct::fvec2 textureSize;
					auto descPtr = otherTextureNodePtr->GetDescriptorImageInfo(endSlotPtr->descriptorBinding, &textureSize);
					SetTexture(startSlotPtr->descriptorBinding, descPtr, &textureSize);
				}
			}
			else if (startSlotPtr->slotType == "LIGHT_GROUP")
			{
				auto otherTextureNodePtr = dynamic_pointer_cast<LightGroupOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherTextureNodePtr)
				{
					SetLightGroup(otherTextureNodePtr->GetLightGroup());
				}
			}
		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connect?
void DiffuseNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_DiffuseModulePtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == "TEXTURE_2D")
			{
				SetTexture(startSlotPtr->descriptorBinding, nullptr, nullptr);
			}
			else if (startSlotPtr->slotType == "LIGHT_GROUP")
			{
				SetLightGroup();
			}
		}
	}
}

void DiffuseNode::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	if (m_DiffuseModulePtr)
	{
		m_DiffuseModulePtr->SetTexture(vBinding, vImageInfo, vTextureSize);
	}
}

vk::DescriptorImageInfo* DiffuseNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_DiffuseModulePtr)
	{
		return m_DiffuseModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

void DiffuseNode::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	if (m_DiffuseModulePtr)
	{
		return m_DiffuseModulePtr->SetLightGroup(vSceneLightGroup);
	}
}

void DiffuseNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::TextureUpdateDone:
	{
		auto emiterSlotPtr = vEmitterSlot.getValidShared();
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
						ct::fvec2 textureSize;
						auto descPtr = otherNodePtr->GetDescriptorImageInfo(emiterSlotPtr->descriptorBinding, &textureSize);
						SetTexture(receiverSlotPtr->descriptorBinding, descPtr, &textureSize);
					}
				}
			}
		}
		break;
	}
	case NotifyEvent::LightGroupUpdateDone:
	{
		auto emiterSlotPtr = vEmitterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto otherNodePtr = dynamic_pointer_cast<LightGroupOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					auto receiverSlotPtr = vReceiverSlot.getValidShared();
					if (receiverSlotPtr)
					{
						SetLightGroup(otherNodePtr->GetLightGroup());
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

std::string DiffuseNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_DiffuseModulePtr)
		{
			res += m_DiffuseModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool DiffuseNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_DiffuseModulePtr)
	{
		m_DiffuseModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

void DiffuseNode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_DiffuseModulePtr)
	{
		m_DiffuseModulePtr->UpdateShaders(vFiles);
	}
}