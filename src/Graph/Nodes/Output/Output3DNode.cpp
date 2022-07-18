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

#include "Output3DNode.h"
#include <Modules/Output/Output3DModule.h>
#include <Interfaces/MergedInterface.h>

std::shared_ptr<Output3DNode> Output3DNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<Output3DNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

Output3DNode::Output3DNode() : BaseNode()
{
	m_NodeTypeString = "OUTPUT_3D";
}

Output3DNode::~Output3DNode()
{
	Unit();
}

bool Output3DNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Scene Output 3D";

	NodeSlot slot;
	slot.slotType = "TEXTURE_2D";
	slot.name = "Output";
	m_InputSlot = AddInput(slot, true, true);

	bool res = false;

	m_Output3DModulePtr = Output3DModule::Create(vVulkanCorePtr, m_This);
	if (m_Output3DModulePtr)
	{
		res = true;
	}

	return res;
}

void Output3DNode::Unit()
{
	m_Output3DModulePtr.reset();
}

bool Output3DNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureInputDescriptorImageInfos(m_Inputs);

	if (m_Output3DModulePtr)
	{
		return m_Output3DModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}

	return false;
}

bool Output3DNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_Output3DModulePtr)
	{
		return m_Output3DModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void Output3DNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	/*if (m_Output3DModulePtr)
	{
		m_Output3DModulePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}*/
}

void Output3DNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void Output3DNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_Output3DModulePtr)
	{
		if (startSlotPtr->IsAnInput())
		{

		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void Output3DNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			
		}
	}
}

void Output3DNode::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	
}

vk::DescriptorImageInfo* Output3DNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	auto slotPtr = m_InputSlot.getValidShared();
	if (slotPtr)
	{
		if (!slotPtr->linkedSlots.empty())
		{
			auto otherSLotPtr = slotPtr->linkedSlots[0].getValidShared();
			if (otherSLotPtr)
			{
				auto otherNodePtr = dynamic_pointer_cast<TextureOutputInterface>(otherSLotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					return otherNodePtr->GetDescriptorImageInfo(otherSLotPtr->descriptorBinding, vOutSize);
				}
			}
		}
	}

	return nullptr;
}

void Output3DNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::SomeTasksWasUpdated:
	{
		if (m_CanExploreTasks && m_Output3DModulePtr)
		{
			//m_Output3DModulePtr->ExploreTasks();
		}
		break;
	}
	case NotifyEvent::GraphIsLoaded:
	{
		m_CanExploreTasks = true;
		if (m_Output3DModulePtr)
		{
			//m_Output3DModulePtr->ExploreTasks();
		}
		break;
	}
	default:
		break;
	}
}

void Output3DNode::DrawOutputWidget(BaseNodeState* vBaseNodeState, NodeSlotWeak vSlot)
{
	// one output only
	//if (m_Output3DModulePtr)
	{
		//ImGui::Text("%s", m_SmoothNormal->GetFileName().c_str());
	}
}

ct::fvec2 Output3DNode::GetOutputSize()
{
	auto slotPtr = m_InputSlot.getValidShared();
	if (slotPtr)
	{
		if (!slotPtr->linkedSlots.empty())
		{
			auto otherSLotPtr = slotPtr->linkedSlots[0].getValidShared();
			if (otherSLotPtr)
			{
				auto otherNodePtr = dynamic_pointer_cast<ResizerInterface>(otherSLotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					return otherNodePtr->GetOutputSize();
				}
			}
		}
	}

	return 0.0f;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string Output3DNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_Output3DModulePtr)
		{
			res += m_Output3DModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool Output3DNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_Output3DModulePtr)
	{
		m_Output3DModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}