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

#include "NodeSlotVariableInput.h"
#include <Graph/Base/BaseNode.h>
#include <Interfaces/VariableInputInterface.h>
#include <Interfaces/VariableOutputInterface.h>

#include <utility>
static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotVariableInputPtr NodeSlotVariableInput::Create(NodeSlotVariableInput vSlot)
{
	auto res = std::make_shared<NodeSlotVariableInput>(vSlot);
	res->m_This = res;
	if (res->m_AvailableTypes.find(res->slotType) == res->m_AvailableTypes.end())
	{
		LogVarError("Variable Type %s is not supported", res->slotType.c_str());
		res.reset();
	}
	return res;
}

NodeSlotVariableInputPtr NodeSlotVariableInput::Create(const std::string& vName, const std::string& vType)
{
	auto res = std::make_shared<NodeSlotVariableInput>(vName, vType);
	res->m_This = res;
	return res;
}

NodeSlotVariableInputPtr NodeSlotVariableInput::Create(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex)
{
	auto res = std::make_shared<NodeSlotVariableInput>(vName, vType, vVariableIndex);
	res->m_This = res;
	if (res->m_AvailableTypes.find(res->slotType) == res->m_AvailableTypes.end())
	{
		LogVarError("Variable Type %s is not supported", res->slotType.c_str());
		res.reset();
	}
	return res;
}

NodeSlotVariableInputPtr NodeSlotVariableInput::Create(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName)
{
	auto res = std::make_shared<NodeSlotVariableInput>(vName, vType, vVariableIndex, vHideName);
	res->m_This = res;
	if (res->m_AvailableTypes.find(res->slotType) == res->m_AvailableTypes.end())
	{
		LogVarError("Variable Type %s is not supported", res->slotType.c_str());
		res.reset();
	}
	return res;
}

NodeSlotVariableInputPtr NodeSlotVariableInput::Create(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName, const bool& vShowWidget)
{
	auto res = std::make_shared<NodeSlotVariableInput>(vName, vType, vVariableIndex, vHideName, vShowWidget);
	res->m_This = res;
	if (res->m_AvailableTypes.find(res->slotType) == res->m_AvailableTypes.end())
	{
		LogVarError("Variable Type %s is not supported", res->slotType.c_str());
		res.reset();
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// NODESLOT CLASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotVariableInput::NodeSlotVariableInput()
	: NodeSlotInput("", "")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotVariableInput::NodeSlotVariableInput(const std::string& vName, const std::string& vType)
	: NodeSlotInput(vName, vType)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotVariableInput::NodeSlotVariableInput(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex)
	: NodeSlotInput(vName, vType)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true; 
	variableIndex = vVariableIndex;
}

NodeSlotVariableInput::NodeSlotVariableInput(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName)
	: NodeSlotInput(vName, vType, vHideName)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
	variableIndex = vVariableIndex;
}

NodeSlotVariableInput::NodeSlotVariableInput(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName, const bool& vShowWidget)
	: NodeSlotInput(vName, vType, vHideName, vShowWidget)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
	variableIndex = vVariableIndex;
}

NodeSlotVariableInput::~NodeSlotVariableInput() = default;

void NodeSlotVariableInput::Init()
{
	
}

void NodeSlotVariableInput::Unit()
{
	// ici pas besoin du assert sur le m_This 
	// car NodeSlotVariableInput peut etre isntancié à l'ancienne en copie local donc sans shared_ptr
	// donc pour gagner du temps on va checker le this, si expiré on va pas plus loins
	if (!m_This.expired())
	{
		if (!parentNode.expired())
		{
			auto parentNodePtr = parentNode.lock();
			if (parentNodePtr)
			{
				auto graph = parentNodePtr->m_ParentNode;
				if (!graph.expired())
				{
					auto graphPtr = graph.lock();
					if (graphPtr)
					{
						graphPtr->DisConnectSlot(m_This);
					}
				}
			}
		}
	}
}

void NodeSlotVariableInput::Connect(NodeSlotWeak vOtherSlot)
{
	if (m_AvailableTypes.find(slotType) != m_AvailableTypes.end())
	{
		auto endSlotPtr = vOtherSlot.getValidShared();
		if (endSlotPtr)
		{
			auto parentNodePtr = dynamic_pointer_cast<VariableInputInterface<0u>>(parentNode.getValidShared());
			if (parentNodePtr)
			{
				auto otherVariableNodePtr = dynamic_pointer_cast<VariableOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherVariableNodePtr)
				{
					parentNodePtr->SetVariable(variableIndex, 
						otherVariableNodePtr->GetVariable(endSlotPtr->variableIndex));
				}
			}
		}
	}
}

void NodeSlotVariableInput::DisConnect(NodeSlotWeak vOtherSlot)
{
	if (m_AvailableTypes.find(slotType) != m_AvailableTypes.end())
	{
		auto endSlotPtr = vOtherSlot.getValidShared();
		if (endSlotPtr)
		{
			auto parentNodePtr = dynamic_pointer_cast<VariableInputInterface<0u>>(parentNode.getValidShared());
			if (parentNodePtr)
			{
				parentNodePtr->SetVariable(variableIndex, SceneVariableWeak());
			}
		}
	}
}

void NodeSlotVariableInput::TreatNotification(
	const NotifyEvent& vEvent,
	const NodeSlotWeak& vEmitterSlot,
	const NodeSlotWeak& vReceiverSlot)
{
	if (vEvent == NotifyEvent::VariableUpdateDone)
	{
		auto emiterSlotPtr = vEmitterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto parentNodePtr = parentNode.getValidShared();
				if (parentNodePtr)
				{
					auto parentVariableInputNodePtr = dynamic_pointer_cast<VariableInputInterface<0u>>(parentNodePtr);
					if (parentVariableInputNodePtr)
					{
						auto otherVariableNodePtr = dynamic_pointer_cast<VariableOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
						if (otherVariableNodePtr)
						{
							auto receiverSlotPtr = vReceiverSlot.getValidShared();
							if (receiverSlotPtr)
							{
								parentVariableInputNodePtr->SetVariable(receiverSlotPtr->variableIndex, 
									otherVariableNodePtr->GetVariable(emiterSlotPtr->variableIndex));
							}
						}
					}
				}
			}
		}
	}
}

void NodeSlotVariableInput::DrawDebugInfos()
{
	ImGui::Text("--------------------");
	ImGui::Text("Slot %s", name.c_str());
	ImGui::Text(IsAnInput() ? "Input" : "Output");
	ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string NodeSlotVariableInput::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
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

bool NodeSlotVariableInput::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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