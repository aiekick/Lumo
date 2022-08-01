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

#include "WidgetColorNode.h"
#include <Modules/Widgets/WidgetColorModule.h>

std::shared_ptr<WidgetColorNode> WidgetColorNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<WidgetColorNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

WidgetColorNode::WidgetColorNode() : BaseNode()
{
	m_NodeTypeString = "WIDGET_COLOR";
}

WidgetColorNode::~WidgetColorNode()
{
	Unit();
}

bool WidgetColorNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Widget Color";

	NodeSlot slot;
	
	slot.slotType = TextureConnector<0U>::GetSlotType();
	slot.name = "Color";
	slot.showWidget = true;
	AddOutput(slot, true, true);

	bool res = false;

	m_WidgetColorModule = WidgetColorModule::Create(vVulkanCorePtr);
	if (m_WidgetColorModule)
	{
		res = true;
	}

	return res;
}

void WidgetColorNode::Unit()
{
	m_WidgetColorModule.reset();
}

bool WidgetColorNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	if (m_WidgetColorModule)
	{
		return m_WidgetColorModule->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}
	return false;
}

bool WidgetColorNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_WidgetColorModule)
	{
		return m_WidgetColorModule->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void WidgetColorNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_WidgetColorModule)
	{
		m_WidgetColorModule->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void WidgetColorNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

vk::DescriptorImageInfo* WidgetColorNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_WidgetColorModule)
	{
		return m_WidgetColorModule->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void WidgetColorNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_WidgetColorModule)
	{
		if (startSlotPtr->IsAnInput())
		{

		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void WidgetColorNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_WidgetColorModule)
	{
		if (startSlotPtr->IsAnInput())
		{

		}
	}
}

void WidgetColorNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::TextureUpdateDone:
	{
		auto slots = GetOutputSlotsOfType("TEXTURE_2D");
		for (const auto& slot : slots)
		{
			auto slotPtr = slot.getValidShared();
			if (slotPtr)
			{
				slotPtr->Notify(NotifyEvent::TextureUpdateDone, slot);
			}
		}
		break;
	}
	default:
		break;
	}
}

void WidgetColorNode::DrawOutputWidget(BaseNodeState* vBaseNodeState, NodeSlotWeak vSlot)
{
	if (vBaseNodeState)
	{
		auto slotPtr = vSlot.getValidShared();
		if (slotPtr)
		{
			if (m_WidgetColorModule)
			{
				m_WidgetColorModule->DrawNodeWidget(vBaseNodeState->m_CurrentFrame, ImGui::GetCurrentContext());
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string WidgetColorNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_WidgetColorModule)
		{
			res += m_WidgetColorModule->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool WidgetColorNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_WidgetColorModule)
	{
		m_WidgetColorModule->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}