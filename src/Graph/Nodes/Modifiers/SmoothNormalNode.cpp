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

#include "SmoothNormalNode.h"
#include <Modules/Modifiers/SmoothNormalModule.h>

std::shared_ptr<SmoothNormalNode> SmoothNormalNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<SmoothNormalNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

SmoothNormalNode::SmoothNormalNode() : BaseNode()
{
	m_NodeTypeString = "SMOOTH_NORMAL";
}

SmoothNormalNode::~SmoothNormalNode()
{

}

bool SmoothNormalNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Smooth Normal";

	NodeSlot slot;
	slot.slotType = NodeSlotTypeEnum::MESH;
	slot.showWidget = true;
	AddInput(slot, true, true);

	slot.slotType = NodeSlotTypeEnum::MESH;
	slot.name = "Output";
	slot.showWidget = true;
	AddOutput(slot, true, true);

	// we keep this node in ExecuteAllTime, because we need to propagate to inputs for each frames
	
	m_SmoothNormalModulePtr = SmoothNormalModule::Create(vVulkanCorePtr, m_This);
	if (m_SmoothNormalModulePtr)
	{
		return true;
	}

	return false;
}

bool SmoothNormalNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer *vCmd)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd);

	if (m_SmoothNormalModulePtr)
	{
		return m_SmoothNormalModulePtr->Execute(vCurrentFrame, vCmd);
	}
	return false;
}

bool SmoothNormalNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_SmoothNormalModulePtr)
	{
		return m_SmoothNormalModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void SmoothNormalNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_SmoothNormalModulePtr)
	{
		m_SmoothNormalModulePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void SmoothNormalNode::DisplayInfosOnTopOfTheNode(BaseNodeStateStruct* vCanvasState)
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

void SmoothNormalNode::SetModel(SceneModelWeak vSceneModel)
{
	if (m_SmoothNormalModulePtr)
	{
		m_SmoothNormalModulePtr->SetModel(vSceneModel);
	}
}

SceneModelWeak SmoothNormalNode::GetModel()
{
	if (m_SmoothNormalModulePtr)
	{
		return m_SmoothNormalModulePtr->GetModel();
	}

	return SceneModelWeak();
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void SmoothNormalNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_SmoothNormalModulePtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			auto otherNodePtr = dynamic_pointer_cast<ModelOutputInterface>(endSlotPtr->parentNode.getValidShared());
			if (otherNodePtr)
			{
				SetModel(otherNodePtr->GetModel());
			}
			else
			{
#ifdef _DEBUG
				LogVarInfo("This node not inherit of ModelInterface");
#endif
			}
		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void SmoothNormalNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			SetModel();
		}
	}
}

void SmoothNormalNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::ModelUpdateDone:
	{
		// traitment on inputs
		auto emiterSlotPtr = vEmmiterSlot.getValidShared();
		if (emiterSlotPtr && m_SmoothNormalModulePtr)
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

		// notify on outputs than the mesh have changed
		auto slots = GetOutputSlotsOfType(NodeSlotTypeEnum::MESH);
		for (const auto& slot : slots)
		{
			auto slotPtr = slot.getValidShared();
			if (slotPtr)
			{
				slotPtr->Notify(NotifyEvent::ModelUpdateDone, slot);
			}
		}
		break;
	}
	default:
		break;
	}
}

void SmoothNormalNode::DrawOutputWidget(BaseNodeStateStruct* vCanvasState, NodeSlotWeak vSlot)
{
	// one output only
	//if (m_SmoothNormalModulePtr)
	{
		//ImGui::Text("%s", m_SmoothNormal->GetFileName().c_str());
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string SmoothNormalNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_SmoothNormalModulePtr)
		{
			res += m_SmoothNormalModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool SmoothNormalNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_SmoothNormalModulePtr)
	{
		m_SmoothNormalModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}