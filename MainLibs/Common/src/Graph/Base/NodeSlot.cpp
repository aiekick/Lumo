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

#include "NodeSlot.h"

#include <utility>
#include "BaseNode.h"
static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

size_t NodeSlot::GetNewSlotId()
{
	#define SLOT_ID_OFFSET 100000
	return SLOT_ID_OFFSET + (++BaseNode::freeNodeId);
}

ImVec4 NodeSlot::GetSlotColorAccordingToType(const NodeSlotTypeEnum& vNodeSlotType)
{
	ImVec4 res = ImVec4(0.8f, 0.8f, 0.0f, 1.0f);

	switch (vNodeSlotType)
	{
	case NodeSlotTypeEnum::NONE:
		res = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
		break;
	case NodeSlotTypeEnum::MESH:
		res = ImVec4(0.5f, 0.5f, 0.9f, 1.0f);
		break;
	case NodeSlotTypeEnum::MESH_GROUP:
		res = ImVec4(0.1f, 0.1f, 0.8f, 1.0f);
		break;
	case NodeSlotTypeEnum::LIGHT:
		res = ImVec4(0.9f, 0.9f, 0.1f, 1.0f);
		break;
	case NodeSlotTypeEnum::ENVIRONMENT:
		res = ImVec4(0.1f, 0.9f, 0.1f, 1.0f);
		break;
	case NodeSlotTypeEnum::MERGED:
		res = ImVec4(0.1f, 0.5f, 0.9f, 1.0f);
		break;
	case NodeSlotTypeEnum::TEXTURE_2D:
		res = ImVec4(0.9f, 0.5f, 0.1f, 1.0f);
		break;
	case NodeSlotTypeEnum::MIXED:
		res = ImVec4(0.3f, 0.5f, 0.1f, 1.0f);
		break;
	case NodeSlotTypeEnum::TYPE_BOOLEAN:
	case NodeSlotTypeEnum::TYPE_UINT:
	case NodeSlotTypeEnum::TYPE_INT:
	case NodeSlotTypeEnum::TYPE_FLOAT:
		res = ImVec4(0.8f, 0.7f, 0.6f, 1.0f);
		break;
	case NodeSlotTypeEnum::DEPTH:
		res = ImVec4(0.2f, 0.7f, 0.6f, 1.0f);
		break;
	default:
		break;
	}

	return res;
}

NodeSlotPtr NodeSlot::Create(NodeSlot vSlot)
{
	auto res = std::make_shared<NodeSlot>(vSlot);
	res->m_This = res;
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// NODESLOT CLASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlot::NodeSlot()
{
	pinID = GetNewSlotId();
}

NodeSlot::NodeSlot(std::string vName)
{
	pinID = GetNewSlotId();
	name = vName;
}

NodeSlot::NodeSlot(std::string vName, NodeSlotTypeEnum vType)
{
	pinID = GetNewSlotId();
	name = vName;
	slotType = vType;
	color = GetSlotColorAccordingToType(slotType);
	colorIsSet = true;
	//stamp.typeStamp = ConvertUniformsTypeEnumToString(type);
}

NodeSlot::NodeSlot(std::string vName, NodeSlotTypeEnum vType, bool vHideName)
{
	pinID = GetNewSlotId();
	name = vName;
	slotType = vType;
	color = GetSlotColorAccordingToType(slotType);
	colorIsSet = true;
	//stamp.typeStamp = ConvertUniformsTypeEnumToString(type);
	hideName = vHideName;
}

NodeSlot::NodeSlot(std::string vName, NodeSlotTypeEnum vType, bool vHideName, bool vShowWidget)
{
	pinID = GetNewSlotId();
	name = vName;
	slotType = vType;
	color = GetSlotColorAccordingToType(slotType);
	colorIsSet = true;
	//stamp.typeStamp = ConvertUniformsTypeEnumToString(type);
	hideName = vHideName;
	showWidget = vShowWidget;
}

NodeSlot::~NodeSlot()
{
	//Unit();
}

void NodeSlot::Init()
{
	
}

void NodeSlot::Unit()
{
	// ici pas besoin du assert sur le m_This 
	// car NodeSlot peut etre isntancié à l'ancienne en copie local donc sans shared_ptr
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

// name : toto, stamp : vec3(vec4) => result : vec3 toto(vec4)
std::string NodeSlot::GetFullStamp()
{
	std::string res;

	if (!name.empty() && !stamp.typeStamp.empty())
	{
		size_t par = stamp.typeStamp.find('(');
		if (par != std::string::npos)
		{
			res = stamp.typeStamp;
			res.insert(par, " " + name);
		}
	}

	return res;
}

void NodeSlot::NotifyConnectionChangeToParent(bool vConnected) // va contacter le parent pour lui dire que ce slot est connecté a un autre
{
	assert(!m_This.expired());
	if (!parentNode.expired())
	{
		auto parentNodePtr = parentNode.lock();
		if (parentNodePtr)
		{
			parentNodePtr->NotifyConnectionChangeOfThisSlot(m_This, vConnected);
		}
	}
}

bool NodeSlot::CanWeConnectToSlot(NodeSlotWeak vSlot)
{
	if (!parentNode.expired())
	{
		auto parentNodePtr = parentNode.lock();
		if (parentNodePtr)
		{
			assert(!m_This.expired());
			return parentNodePtr->CanWeConnectSlots(m_This, vSlot);
		}
	}

	return false;
}

std::vector<NodeSlotWeak> NodeSlot::InjectTypeInSlot(uType::uTypeEnum vType)
{
	std::vector<NodeSlotWeak> res;

	if (!parentNode.expired())
	{
		auto parentNodePtr = parentNode.lock();
		if (parentNodePtr)
		{
			assert(!m_This.expired());
			res = parentNodePtr->InjectTypeInSlot(m_This, vType);
		}
	}

	return res;
}

void NodeSlot::DrawContent(BaseNodeStateStruct *vCanvasState)
{
	if (vCanvasState && !hidden)
	{
		if (slotPlace == NodeSlotPlaceEnum::INPUT)
		{
			nd::BeginPin(pinID, nd::PinKind::Input);
			{
				ImGui::BeginHorizontal(pinID.AsPointer());
				
				nd::PinPivotAlignment(ImVec2(0.0f, 0.5f));
				nd::PinPivotSize(ImVec2(0, 0));
				
				DrawSlot(vCanvasState, ImVec2(slotIconSize, slotIconSize));

				if (showWidget)
				{
					DrawInputWidget(vCanvasState);
				}
				if (!hideName)
				{
					ImGui::TextUnformatted(name.c_str());
				}

				ImGui::Spring(1);

				ImGui::EndHorizontal();
			}
			nd::EndPin();
		}
		else if (slotPlace == NodeSlotPlaceEnum::OUTPUT)
		{
			nd::BeginPin(pinID, nd::PinKind::Output);
			{
				ImGui::BeginHorizontal(pinID.AsPointer());

				ImGui::Spring(1);

				if (!hideName)
				{
					ImGui::TextUnformatted(name.c_str());
				}
				if (showWidget)
				{
					DrawOutputWidget(vCanvasState);
				}

				nd::PinPivotAlignment(ImVec2(1.0f, 0.5f));
				nd::PinPivotSize(ImVec2(0, 0));

				DrawSlot(vCanvasState, ImVec2(slotIconSize, slotIconSize));
				
				ImGui::EndHorizontal();
			}
			nd::EndPin();
		}
	}
}

void NodeSlot::DrawSlot(BaseNodeStateStruct *vCanvasState, ImVec2 vSlotSize, ImVec2 vSlotOffset)
{
	if (vCanvasState)
	{
		ImGui::Dummy(vSlotSize);

		ImRect slotRect = GImGui->LastItemData.Rect;
		slotRect.Min += vSlotOffset;
		slotRect.Max += vSlotOffset;

		ImVec2 slotCenter = slotRect.GetCenter();
		pos = slotCenter;

		nd::PinPivotRect(slotCenter, slotCenter);
		nd::PinRect(slotRect.Min, slotRect.Max);

		highLighted = false;

		if (ImGui::IsRectVisible(vSlotSize))
		{
			auto draw_list = ImGui::GetWindowDrawList();
			if (draw_list)
			{
				if (!colorIsSet)
				{
					color = GetSlotColorAccordingToType(slotType);
					colorIsSet = true;
				}

				DrawNodeSlot(draw_list, slotCenter, vCanvasState->graphStyle.SLOT_RADIUS,
					connected, ImGui::GetColorU32(color), ImGui::GetColorU32(color));
			}

			if (ImGui::IsItemHovered())
			{
				highLighted = true;

				DrawSlotText(vCanvasState);
			}
		}
	}
}

bool NodeSlot::IsAnInput()
{
	return slotPlace == NodeSlotPlaceEnum::INPUT;
}

bool NodeSlot::IsAnOutput()
{
	return slotPlace == NodeSlotPlaceEnum::OUTPUT;
}

void NodeSlot::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& /*vReceiverSlot*/)
{
	// une notifcation doit toujours aller 
	// d'un output a un input

	if (vEmmiterSlot.expired() || vEmmiterSlot.getValidShared() == m_This.getValidShared())
	{
		for (const auto& otherSlot : linkedSlots)
		{
			auto otherSlotPtr = otherSlot.getValidShared();
			if (otherSlotPtr)
			{
				otherSlotPtr->Notify(vEvent, m_This, otherSlot);
			}
		}
	}
	else if (IsAnInput())
	{
		// on notify au parent
		auto parentPtr = parentNode.getValidShared();
		if (parentPtr)
		{
			parentPtr->Notify(vEvent, vEmmiterSlot, m_This);
		}
	}
}

//////////////////////////////////////////////////////////////
//// PRIVATE /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void NodeSlot::DrawInputWidget(BaseNodeStateStruct *vCanvasState)
{
	if (vCanvasState && !parentNode.expired())
	{
		assert(!m_This.expired());
		auto ptr = parentNode.lock();
		if (ptr)
		{
			ptr->DrawInputWidget(vCanvasState, m_This);
		}
	}
}

void NodeSlot::DrawOutputWidget(BaseNodeStateStruct *vCanvasState)
{
	if (vCanvasState && !parentNode.expired())
	{
		assert(!m_This.expired());
		auto ptr = parentNode.lock();
		if (ptr)
		{
			ptr->DrawOutputWidget(vCanvasState, m_This);
		}
	}
}

void NodeSlot::DrawSlotText(BaseNodeStateStruct *vCanvasState)
{
	if (vCanvasState)
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		if (draw_list)
		{
			std::string slotUType;
			std::string slotName;

			if (slotPlace == NodeSlotPlaceEnum::INPUT)
			{
				//slotName = stamp.typeStamp; // valable seulement en mode blueprint

				slotName = GetStringFromNodeSlotTypeEnum(slotType);

				if (vCanvasState->debug_mode)
				{
					slotName = "in " + slotUType + " links(";

					int idx = 0;
					for (auto link : linkedSlots)
					{
						auto linkPtr = link.getValidShared();
						if (linkPtr)
						{
							if (idx > 0)
								slotName += ", ";

							if (!linkPtr->parentNode.expired())
							{
								auto parentNodePtr = linkPtr->parentNode.lock();
								if (parentNodePtr)
								{
									slotName += parentNodePtr->name;
								}
							}
						}

						idx++;
					}

					slotName += ")";
				}
				size_t len = slotName.length();
				if (len > 0)
				{
					const char *beg = slotName.c_str();
					ImVec2 txtSize = ImGui::CalcTextSize(beg);
					ImVec2 min = ImVec2(pos.x - slotIconSize * 0.5f - txtSize.x, pos.y - slotIconSize * 0.5f);
					ImVec2 max = min + ImVec2(txtSize.x, slotIconSize);
					ImGui::RenderFrame(min, max, ImGui::GetColorU32(ImVec4(0.1f, 0.1f, 0.1f, 1.0f)));
					draw_list->AddText(ImVec2(min.x, pos.y - txtSize.y * 0.55f), ImColor(200, 200, 200, 255), beg);
				}
			}
			else if (slotPlace == NodeSlotPlaceEnum::OUTPUT)
			{
				//slotName = stamp.typeStamp; // valable seulement en mode blueprint

				slotName = GetStringFromNodeSlotTypeEnum(slotType);

				if (vCanvasState->debug_mode)
				{
					slotName = "links(" + ct::toStr(linkedSlots.size()) + ")" + slotUType + " out";
				}
				size_t len = slotName.length();
				if (len > 0)
				{
					const char *beg = slotName.c_str();
					ImVec2 txtSize = ImGui::CalcTextSize(beg);
					ImVec2 min = ImVec2(pos.x + slotIconSize * 0.5f, pos.y - slotIconSize * 0.5f);
					ImVec2 max = min + ImVec2(txtSize.x, slotIconSize);
					ImGui::RenderFrame(min, max, ImGui::GetColorU32(ImVec4(0.1f, 0.1f, 0.1f, 1.0f)));
					draw_list->AddText(ImVec2(min.x, pos.y - txtSize.y * 0.55f), ImColor(200, 200, 200, 255), beg);
				}
			}
		}
	}
}
void NodeSlot::DrawNodeSlot(ImDrawList *vDrawList, ImVec2 vCenter, float vSlotRadius, bool vConnected, ImU32 vColor, ImU32 vInnerColor)
{
	UNUSED(vInnerColor);
	UNUSED(vConnected);

	if (vDrawList)
	{
		vDrawList->AddRectFilled(
			ImVec2(vCenter.x - vSlotRadius, vCenter.y - vSlotRadius),
			ImVec2(vCenter.x + vSlotRadius, vCenter.y + vSlotRadius),
			vColor, 0.5f);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string NodeSlot::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string res;

	res += vOffset + ct::toStr("<slot name=\"%s\" type=\"%s\" place=\"%s\" id=\"%u\"/>\n",
		name.c_str(),
		GetStringFromNodeSlotTypeEnum(slotType).c_str(),
		GetStringFromNodeSlotPlaceEnum(slotPlace).c_str(),
		(uint32_t)pinID.Get());

	return res;
}

bool NodeSlot::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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
		std::string _name;
		NodeSlotTypeEnum _type = NodeSlotTypeEnum::NONE;
		NodeSlotPlaceEnum _place = NodeSlotPlaceEnum::NONE;
		uint32_t _pinId = 0U;

		for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
		{
			std::string attName = attr->Name();
			std::string attValue = attr->Value();

			if (attName == "name")
				_name = attValue;
			if (attName == "type")
				_type = GetNodeSlotTypeEnumFromString(attValue);
			if (attName == "place")
				_place = GetNodeSlotPlaceEnumFromString(attValue);
			if (attName == "id")
				_pinId = ct::ivariant(attValue).GetU();
		}

		if (name == _name && 
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