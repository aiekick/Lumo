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

#include "OutputNode.h"
#include <Modules/Output/OutputModule.h>
#include <Interfaces/MergedInterface.h>

std::shared_ptr<OutputNode> OutputNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<OutputNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

OutputNode::OutputNode() : BaseNode()
{
	m_NodeTypeString = "OUTPUT";
}

OutputNode::~OutputNode()
{
	Unit();
}

bool OutputNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Scene Output";

	NodeSlot slot;
	slot.slotType = NodeSlotTypeEnum::TEXTURE_2D;
	slot.name = "Output";
	m_InputSlot = AddInput(slot, true, true);

	bool res = false;

	m_OutputModulePtr = OutputModule::Create(vVulkanCorePtr, m_This);
	if (m_OutputModulePtr)
	{
		res = true;
	}

	return res;
}

void OutputNode::Unit()
{
	m_OutputModulePtr.reset();
}

bool OutputNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer *vCmd)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd);

	// for update input texture buffer infos => avoid vk crash
	UpdateInputDescriptorImageInfos(m_Inputs);

	if (m_OutputModulePtr)
	{
		return m_OutputModulePtr->Execute(vCurrentFrame, vCmd);
	}

	return false;
}

bool OutputNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_OutputModulePtr)
	{
		return m_OutputModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void OutputNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	/*if (m_OutputModulePtr)
	{
		m_OutputModulePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}*/
}

void OutputNode::DisplayInfosOnTopOfTheNode(BaseNodeStateStruct* vCanvasState)
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

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void OutputNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_OutputModulePtr)
	{
		if (startSlotPtr->IsAnInput())
		{

		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void OutputNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
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

void OutputNode::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	
}

vk::DescriptorImageInfo* OutputNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
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
					return otherNodePtr->GetDescriptorImageInfo(otherSLotPtr->descriptorBinding);
				}
			}
		}
	}

	return nullptr;
}

void OutputNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::SomeTasksWasUpdated:
	{
		if (m_CanExploreTasks && m_OutputModulePtr)
		{
			//m_OutputModulePtr->ExploreTasks();
		}
		break;
	}
	case NotifyEvent::GraphIsLoaded:
	{
		m_CanExploreTasks = true;
		if (m_OutputModulePtr)
		{
			//m_OutputModulePtr->ExploreTasks();
		}
		break;
	}
	default:
		break;
	}
}

void OutputNode::DrawOutputWidget(BaseNodeStateStruct* vCanvasState, NodeSlotWeak vSlot)
{
	// one output only
	//if (m_OutputModulePtr)
	{
		//ImGui::Text("%s", m_SmoothNormal->GetFileName().c_str());
	}
}

ct::fvec2 OutputNode::GetOutputSize()
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

std::string OutputNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_OutputModulePtr)
		{
			res += m_OutputModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool OutputNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_OutputModulePtr)
	{
		m_OutputModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}