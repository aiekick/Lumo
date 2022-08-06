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

#include "NodeSlotVariableOutput.h"

#include <utility>
#include <Graph/Base/BaseNode.h>
#include <SceneGraph/SceneVariable.h>

static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotVariableOutputPtr NodeSlotVariableOutput::Create(NodeSlotVariableOutput vSlot)
{
	auto res = std::make_shared<NodeSlotVariableOutput>(vSlot);
	res->m_This = res;
	return res;
}

NodeSlotVariableOutputPtr NodeSlotVariableOutput::Create(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex)
{
	auto res = std::make_shared<NodeSlotVariableOutput>(vName, vType, vVariableIndex);
	res->m_This = res;
	if (!SceneVariable::IsAllowedType(res->slotType))
	{
		LogVarError("Variable Type %s is not supported", res->slotType.c_str());
		res.reset();
	}
	return res;
}

NodeSlotVariableOutputPtr NodeSlotVariableOutput::Create(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName)
{
	auto res = std::make_shared<NodeSlotVariableOutput>(vName, vType, vVariableIndex, vHideName);
	res->m_This = res;
	if (!SceneVariable::IsAllowedType(res->slotType))
	{
		LogVarError("Variable Type %s is not supported", res->slotType.c_str());
		res.reset();
	}
	return res;
}

NodeSlotVariableOutputPtr NodeSlotVariableOutput::Create(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName, const bool& vShowWidget)
{
	auto res = std::make_shared<NodeSlotVariableOutput>(vName, vType, vVariableIndex, vHideName, vShowWidget);
	res->m_This = res;
	if (!SceneVariable::IsAllowedType(res->slotType))
	{
		LogVarError("Variable Type %s is not supported", res->slotType.c_str());
		res.reset();
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// NODESLOT CLASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlotVariableOutput::NodeSlotVariableOutput()
	: NodeSlotOutput("", "")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlotVariableOutput::NodeSlotVariableOutput(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex)
	: NodeSlotOutput(vName, vType)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
	variableIndex = vVariableIndex;
}

NodeSlotVariableOutput::NodeSlotVariableOutput(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName)
	: NodeSlotOutput(vName, vType, vHideName)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
	variableIndex = vVariableIndex;
}

NodeSlotVariableOutput::NodeSlotVariableOutput(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName, const bool& vShowWidget)
	: NodeSlotOutput(vName, vType, vHideName, vShowWidget)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
	variableIndex = vVariableIndex;
}

NodeSlotVariableOutput::~NodeSlotVariableOutput() = default;

void NodeSlotVariableOutput::Init()
{
	
}

void NodeSlotVariableOutput::Unit()
{
	// ici pas besoin du assert sur le m_This 
	// car NodeSlotVariableOutput peut etre instancié à l'ancienne en copie local donc sans shared_ptr
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

void NodeSlotVariableOutput::SendFrontNotification(const NotifyEvent& vEvent)
{
	if (vEvent == VariableUpdateDone)
	{
		if (SceneVariable::IsAllowedType(slotType))
		{
			SendNotification(slotType, vEvent);
		}
	}
}

void NodeSlotVariableOutput::DrawDebugInfos()
{
	ImGui::Text("--------------------");
	ImGui::Text("Slot %s", name.c_str());
	ImGui::Text(IsAnInput() ? "Input" : "Output");
	ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string NodeSlotVariableOutput::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
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

bool NodeSlotVariableOutput::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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