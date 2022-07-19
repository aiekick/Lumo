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

#include "ModelToAccelStructNode.h"
#include <Interfaces/ModelOutputInterface.h>

std::shared_ptr<ModelToAccelStructNode> ModelToAccelStructNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<ModelToAccelStructNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

ModelToAccelStructNode::ModelToAccelStructNode() : BaseNode()
{
	m_NodeTypeString = "RTX_MODEL_TO_ACCELERATION_STRUCTURE";
}

ModelToAccelStructNode::~ModelToAccelStructNode()
{
	Unit();
}

bool ModelToAccelStructNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Accel Struct Builder";

	NodeSlot slot;

	slot.slotType = "MESH";
	slot.name = "Model";
	AddInput(slot, true, false);

	slot.slotType = "RTX_ACCEL_STRUCTURE";
	slot.name = "Output";
	slot.descriptorBinding = 0U;
	AddOutput(slot, true, true);

	bool res = false;

	m_SceneAccelStructurePtr = SceneAccelStructure::Create(vVulkanCorePtr);
	if (m_SceneAccelStructurePtr)
	{
		res = true;
	}

	return res;
}

void ModelToAccelStructNode::Unit()
{
	m_SceneAccelStructurePtr.reset();
}

void ModelToAccelStructNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

void ModelToAccelStructNode::SetModel(SceneModelWeak vSceneModel)
{
	m_SceneModel = vSceneModel;

	if (m_SceneAccelStructurePtr)
	{
		m_SceneAccelStructurePtr->BuildForModel(m_SceneModel);
		if (m_SceneAccelStructurePtr->IsOk())
		{
			Notify(NotifyEvent::AccelStructureUpdateDone);
		}
	}
}

SceneAccelStructureWeak ModelToAccelStructNode::GetAccelStruct()
{
	return m_SceneAccelStructurePtr;
}

vk::WriteDescriptorSetAccelerationStructureKHR* ModelToAccelStructNode::GetTLASInfo()
{
	if (m_SceneAccelStructurePtr && 
		m_SceneAccelStructurePtr->IsOk())
	{
		return m_SceneAccelStructurePtr->GetTLASInfo();
	}

	return nullptr;
}

vk::DescriptorBufferInfo* ModelToAccelStructNode::GetBufferAddressInfo()
{
	if (m_SceneAccelStructurePtr &&
		m_SceneAccelStructurePtr->IsOk())
	{
		return m_SceneAccelStructurePtr->GetBufferAddressInfo();
	}

	return nullptr;
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void ModelToAccelStructNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr)
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
		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void ModelToAccelStructNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr)
	{
		if (startSlotPtr->linkedSlots.empty()) // connected to nothing
		{
			if (startSlotPtr->slotType == "MESH")
			{
				SetModel();
			}
		}
	}
}

void ModelToAccelStructNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::ModelUpdateDone: // input
	{
		auto emiterSlotPtr = vEmmiterSlot.getValidShared();
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
	case NotifyEvent::AccelStructureUpdateDone: // output
	{
		auto slots = GetOutputSlotsOfType("RTX_ACCEL_STRUCTURE");
		for (const auto& slot : slots)
		{
			auto slotPtr = slot.getValidShared();
			if (slotPtr)
			{
				slotPtr->Notify(NotifyEvent::AccelStructureUpdateDone, slot);
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

std::string ModelToAccelStructNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

bool ModelToAccelStructNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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