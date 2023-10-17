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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "NodeSlotAccelStructureInput.h"
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <Interfaces/AccelStructureInputInterface.h>
#include <Interfaces/AccelStructureOutputInterface.h>
#include <Headers/RTXCommon.h>

#include <utility>
static const float slotIconSize = 15.0f;

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotAccelStructureInputPtr NodeSlotAccelStructureInput::Create(NodeSlotAccelStructureInput vSlot)
{
	auto res = std::make_shared<NodeSlotAccelStructureInput>(vSlot);
	res->m_This = res;
	return res;
}

NodeSlotAccelStructureInputPtr NodeSlotAccelStructureInput::Create(const std::string& vName)
{
	auto res = std::make_shared<NodeSlotAccelStructureInput>(vName);
	res->m_This = res;
	return res;
}

NodeSlotAccelStructureInputPtr NodeSlotAccelStructureInput::Create(const std::string& vName, const bool& vHideName)
{
	auto res = std::make_shared<NodeSlotAccelStructureInput>(vName, vHideName);
	res->m_This = res;
	return res;
}

NodeSlotAccelStructureInputPtr NodeSlotAccelStructureInput::Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget)
{
	auto res = std::make_shared<NodeSlotAccelStructureInput>(vName, vHideName, vShowWidget);
	res->m_This = res;
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// NODESLOT CLASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotAccelStructureInput::NodeSlotAccelStructureInput()
	: NodeSlotInput("", "RTX_ACCEL_STRUCTURE")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotAccelStructureInput::NodeSlotAccelStructureInput(const std::string& vName)
	: NodeSlotInput(vName, "RTX_ACCEL_STRUCTURE")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotAccelStructureInput::NodeSlotAccelStructureInput(const std::string& vName, const bool& vHideName)
	: NodeSlotInput(vName, "RTX_ACCEL_STRUCTURE", vHideName)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotAccelStructureInput::NodeSlotAccelStructureInput(const std::string& vName, const bool& vHideName, const bool& vShowWidget)
	: NodeSlotInput(vName, "RTX_ACCEL_STRUCTURE", vHideName, vShowWidget)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotAccelStructureInput::~NodeSlotAccelStructureInput() = default;

void NodeSlotAccelStructureInput::Init()
{
	
}

void NodeSlotAccelStructureInput::Unit()
{
	// ici pas besoin du assert sur le m_This 
	// car NodeSlotAccelStructureInput peut etre isntancié à l'ancienne en copie local donc sans shared_ptr
	// donc pour gagner du temps on va checker le this, si expiré on va pas plus loins
	if (!m_This.expired())
	{
		if (!parentNode.expired())
		{
			auto parentNodePtr = parentNode.lock();
			if (parentNodePtr)
			{
				auto graph = parentNodePtr->GetParentNode();
				if (!graph.expired())
				{
					auto graphPtr = graph.lock();
					if (graphPtr)
					{
						graphPtr->BreakAllLinksConnectedToSlot(m_This);
					}
				}
			}
		}
	}
}

void NodeSlotAccelStructureInput::OnConnectEvent(NodeSlotWeak vOtherSlot)
{
	if (slotType == "RTX_ACCEL_STRUCTURE")
	{
		auto endSlotPtr = vOtherSlot.lock();
		if (endSlotPtr)
		{
			auto parentNodePtr = dynamic_pointer_cast<AccelStructureInputInterface>(parentNode.lock());
			if (parentNodePtr)
			{
				auto otherAccelStructureNodePtr = dynamic_pointer_cast<AccelStructureOutputInterface>(endSlotPtr->parentNode.lock());
				if (otherAccelStructureNodePtr)
				{
					parentNodePtr->SetAccelStructure(otherAccelStructureNodePtr->GetAccelStruct());
				}
			}
		}
	}
}

void NodeSlotAccelStructureInput::OnDisConnectEvent(NodeSlotWeak vOtherSlot)
{
	if (slotType == "RTX_ACCEL_STRUCTURE")
	{
		auto endSlotPtr = vOtherSlot.lock();
		if (endSlotPtr)
		{
			auto parentNodePtr = dynamic_pointer_cast<AccelStructureInputInterface>(parentNode.lock());
			if (parentNodePtr)
			{
				parentNodePtr->SetAccelStructure(SceneAccelStructureWeak());
			}
		}
	}
}

void NodeSlotAccelStructureInput::TreatNotification(
	NotifyEvent vEvent,
	const NodeSlotWeak& vEmitterSlot,
	const NodeSlotWeak& /*vReceiverSlot*/)
{
	if (vEvent == AccelStructureUpdateDone)
	{
		auto emiterSlotPtr = vEmitterSlot.lock();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto parentAccelStructureInputNodePtr = dynamic_pointer_cast<AccelStructureInputInterface>(parentNode.lock());
				if (parentAccelStructureInputNodePtr)
				{
					auto otherNodePtr = dynamic_pointer_cast<AccelStructureOutputInterface>(emiterSlotPtr->parentNode.lock());
					if (otherNodePtr)
					{
						parentAccelStructureInputNodePtr->SetAccelStructure(otherNodePtr->GetAccelStruct());
					}
				}
			}
		}
	}
}

void NodeSlotAccelStructureInput::DrawDebugInfos()
{
	ImGui::Text("--------------------");
	ImGui::Text("Slot %s", name.c_str());
	ImGui::Text(IsAnInput() ? "Input" : "Output");
	ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string NodeSlotAccelStructureInput::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string res;

	res += vOffset + ct::toStr("<slot index=\"%u\" name=\"%s\" type=\"%s\" place=\"%s\" id=\"%u\"/>\n",
		index,
		name.c_str(),
		slotType.c_str(),
		NodeSlot::sGetStringFromNodeSlotPlaceEnum(slotPlace).c_str(),
		(uint32_t)pinID.Get());

	return res;
}

bool NodeSlotAccelStructureInput::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strName == "slot" && strParentName == "node")
	{
		uint32_t _index = 0U;
		std::string _name;
		std::string _type = "NONE";
		auto _place = NodeSlot::PlaceEnum::NONE;
		uint32_t _pinId = 0U;

		for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
		{
			std::string attName = attr->Name();
			std::string attValue = attr->Value();

			if (attName == "index")
				_index = ct::ivariant(attValue).GetU();
			else if (attName == "name")
				_name = attValue;
			else if (attName == "type")
				_type = attValue;
			else if (attName == "place")
				_place = NodeSlot::sGetNodeSlotPlaceEnumFromString(attValue);
			else if (attName == "id")
				_pinId = ct::ivariant(attValue).GetU();
		}

		if (index == _index &&
			slotType == _type && 
			slotPlace == _place && 
			!idAlreadySetbyXml)
		{
			pinID = _pinId;
			idAlreadySetbyXml = true;

			// pour eviter que des slots aient le meme id qu'un nodePtr
			BaseNode::freeNodeId = ct::maxi<uint32_t>(BaseNode::freeNodeId, (uint32_t)pinID.Get());

			return false;
		}
	}	

	return true;
}