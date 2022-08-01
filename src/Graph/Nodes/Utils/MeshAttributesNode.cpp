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

#include "MeshAttributesNode.h"
#include <Modules/Utils/MeshAttributesModule.h>
#include <Interfaces/ModelOutputInterface.h>

std::shared_ptr<MeshAttributesNode> MeshAttributesNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<MeshAttributesNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

MeshAttributesNode::MeshAttributesNode() : BaseNode()
{
	m_NodeTypeString = "MESH_ATTRIBUTES";
}

MeshAttributesNode::~MeshAttributesNode()
{
	Unit();
}

bool MeshAttributesNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Mesh Attributes";

	NodeSlot slot;
	
	slot.slotType = "MESH";
	slot.name = "Mesh";
	AddInput(slot, true, false);

	slot.slotType = "TEXTURE_2D";
	slot.name = "Mask";
	slot.descriptorBinding = 0U;
	AddInput(slot, true, false);

	slot.slotType = "TEXTURE_2D";
	slot.name = "Position";
	slot.descriptorBinding = 0U;
	AddOutput(slot, true, false);

	slot.slotType = "TEXTURE_2D";
	slot.name = "Normal";
	slot.descriptorBinding = 1U;
	AddOutput(slot, true, false);

	slot.slotType = "TEXTURE_2D";
	slot.name = "Tangeant";
	slot.descriptorBinding = 2U;
	AddOutput(slot, true, false);

	slot.slotType = "TEXTURE_2D";
	slot.name = "BiTangeant";
	slot.descriptorBinding = 3U;
	AddOutput(slot, true, false);

	slot.slotType = "TEXTURE_2D";
	slot.name = "UV";
	slot.descriptorBinding = 4U;
	AddOutput(slot, true, false);

	slot.slotType = "TEXTURE_2D";
	slot.name = "Color";
	slot.descriptorBinding = 5U;
	AddOutput(slot, true, false);

	slot.slotType = "DEPTH";
	slot.name = "Depth";
	slot.descriptorBinding = 6U;
	AddOutput(slot, true, false);

	bool res = false;

	m_MeshAttributesModulePtr = MeshAttributesModule::Create(vVulkanCorePtr);
	if (m_MeshAttributesModulePtr)
	{
		res = true;
	}

	return res;
}

void MeshAttributesNode::Unit()
{
	m_MeshAttributesModulePtr.reset();
}

bool MeshAttributesNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureInputDescriptorImageInfos(m_Inputs);

	if (m_MeshAttributesModulePtr)
	{
		return m_MeshAttributesModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}
	return false;
}

bool MeshAttributesNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_MeshAttributesModulePtr)
	{
		return m_MeshAttributesModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void MeshAttributesNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_MeshAttributesModulePtr)
	{
		m_MeshAttributesModulePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void MeshAttributesNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

void MeshAttributesNode::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (m_MeshAttributesModulePtr)
	{
		m_MeshAttributesModulePtr->NeedResize(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResize(vNewSize, vCountColorBuffers);
}

void MeshAttributesNode::SetModel(SceneModelWeak vSceneModel)
{
	if (m_MeshAttributesModulePtr)
	{
		m_MeshAttributesModulePtr->SetModel(vSceneModel);
	}
}

void MeshAttributesNode::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	if (m_MeshAttributesModulePtr)
	{
		m_MeshAttributesModulePtr->SetTexture(vBinding, vImageInfo, vTextureSize);
	}
}

vk::DescriptorImageInfo* MeshAttributesNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_MeshAttributesModulePtr)
	{
		return m_MeshAttributesModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void MeshAttributesNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_MeshAttributesModulePtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == "MESH")
			{
				auto otherModelNodePtr = dynamic_pointer_cast<ModelOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherModelNodePtr)
				{
					SetModel(otherModelNodePtr->GetModel());
				}
			}
			else if (startSlotPtr->slotType == "TEXTURE_2D")
			{
				auto otherTextureNodePtr = dynamic_pointer_cast<TextureOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherTextureNodePtr)
				{
					ct::fvec2 textureSize;
					auto descPtr = otherTextureNodePtr->GetDescriptorImageInfo(endSlotPtr->descriptorBinding, &textureSize);
					SetTexture(startSlotPtr->descriptorBinding, descPtr, &textureSize);
				}
			}
		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void MeshAttributesNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_MeshAttributesModulePtr)
	{
		if (startSlotPtr->linkedSlots.empty()) // connected to nothing
		{
			if (startSlotPtr->slotType == "MESH")
			{
				SetModel();
			}
			else if (startSlotPtr->slotType == "TEXTURE_2D")
			{
				SetTexture(startSlotPtr->descriptorBinding, nullptr, nullptr);
			}
		}
	}
}

void MeshAttributesNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::ModelUpdateDone:
	{	
		auto emiterSlotPtr = vEmitterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto otherNodePtr = dynamic_pointer_cast<ModelOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					SetModel(otherNodePtr->GetModel());
				}
			}
		}
		break;
	}
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
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string MeshAttributesNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_MeshAttributesModulePtr)
		{
			res += m_MeshAttributesModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool MeshAttributesNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_MeshAttributesModulePtr)
	{
		m_MeshAttributesModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

void MeshAttributesNode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_MeshAttributesModulePtr)
	{
		m_MeshAttributesModulePtr->UpdateShaders(vFiles);
	}
}