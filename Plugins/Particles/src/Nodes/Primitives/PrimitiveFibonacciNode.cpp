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

#include "PrimitiveFibonacciNode.h"
#include <Modules/Primitives/PrimitiveFibonacciModule.h>
#include <Interfaces/LightGroupOutputInterface.h>

std::shared_ptr<PrimitiveFibonacciNode> PrimitiveFibonacciNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<PrimitiveFibonacciNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

PrimitiveFibonacciNode::PrimitiveFibonacciNode() : BaseNode()
{
	ZoneScoped;

	m_NodeTypeString = "PRIMITIVE_FIBONACCI";
}

PrimitiveFibonacciNode::~PrimitiveFibonacciNode()
{
	ZoneScoped;

	Unit();
}

bool PrimitiveFibonacciNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	name = "Fibonacci";

	NodeSlot slot;

	slot.slotType = "PARTICLES";
	slot.name = "particles";
	slot.descriptorBinding = 0U;
	AddOutput(slot, true, false);

	m_PrimitiveFibonacciModulePtr = PrimitiveFibonacciModule::Create(vVulkanCorePtr, m_This);
	if (m_PrimitiveFibonacciModulePtr)
	{
		return true;
	}

	return false;
}

bool PrimitiveFibonacciNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	if (m_PrimitiveFibonacciModulePtr)
	{
		return m_PrimitiveFibonacciModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}
	return false;
}

bool PrimitiveFibonacciNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext);

	if (m_PrimitiveFibonacciModulePtr)
	{
		return m_PrimitiveFibonacciModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void PrimitiveFibonacciNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext);

	if (m_PrimitiveFibonacciModulePtr)
	{
		m_PrimitiveFibonacciModulePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void PrimitiveFibonacciNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

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
void PrimitiveFibonacciNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	ZoneScoped;

	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_PrimitiveFibonacciModulePtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			
		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void PrimitiveFibonacciNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	ZoneScoped;

	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_PrimitiveFibonacciModulePtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			
		}
	}
}

vk::Buffer* PrimitiveFibonacciNode::GetTexelBuffer(const uint32_t& vBindingPoint, ct::uvec2* vOutSize)
{
	ZoneScoped;

	if (m_PrimitiveFibonacciModulePtr)
	{
		return m_PrimitiveFibonacciModulePtr->GetTexelBuffer(vBindingPoint, vOutSize);
	}

	return nullptr;
}

vk::BufferView* PrimitiveFibonacciNode::GetTexelBufferView(const uint32_t& vBindingPoint, ct::uvec2* vOutSize)
{
	ZoneScoped;

	if (m_PrimitiveFibonacciModulePtr)
	{
		return m_PrimitiveFibonacciModulePtr->GetTexelBufferView(vBindingPoint, vOutSize);
	}

	return nullptr;
}

void PrimitiveFibonacciNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& vReceiverSlot)
{
	ZoneScoped;

	switch (vEvent)
	{
	case NotifyEvent::TexelBufferUpdateDone:
	{
		auto slots = GetOutputSlotsOfType("PARTICLES");
		for (const auto& slot : slots)
		{
			auto slotPtr = slot.getValidShared();
			if (slotPtr)
			{
				slotPtr->Notify(NotifyEvent::TexelBufferUpdateDone, slot);
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

std::string PrimitiveFibonacciNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_PrimitiveFibonacciModulePtr)
		{
			res += m_PrimitiveFibonacciModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool PrimitiveFibonacciNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_PrimitiveFibonacciModulePtr)
	{
		m_PrimitiveFibonacciModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

void PrimitiveFibonacciNode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_PrimitiveFibonacciModulePtr)
	{
		m_PrimitiveFibonacciModulePtr->UpdateShaders(vFiles);
	}
}