#include "GeneratorNode.h"

#include <Graph/GeneratorCommon.h>
#include <Graph/GeneratorNodeSlotInput.h>
#include <Graph/GeneratorNodeSlotOutput.h>

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

GeneratorNodePtr GeneratorNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	GeneratorNodePtr res = std::make_shared<GeneratorNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
		res = nullptr;
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

GeneratorNode::GeneratorNode()
	: BaseNode()
{

}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string GeneratorNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string res;

	if (!m_ChildNodes.empty())
	{
		res += vOffset + "<graph>\n";

		res += vOffset + "\t<canvas>\n";
		res += vOffset + "\t\t<offset>" + ct::fvec2(GetCanvasOffset()).string() + "</offset>\n";
		res += vOffset + "\t\t<scale>" + ct::toStr(GetCanvasScale()) + "</scale>\n";
		res += vOffset + "\t</canvas>\n";

		// childs
		res += vOffset + "\t<nodes>\n";
		for (auto& node : m_ChildNodes)
		{
			auto nodePtr = node.second;
			if (nodePtr)
			{
				res += nodePtr->getXml(vOffset + "\t\t", vUserDatas);
			}
		}
		res += vOffset + "\t</nodes>\n";

		// links
		res += vOffset + "\t<links>\n";
		for (auto link : m_Links)
		{
			if (!link.second->in.expired() &&
				!link.second->out.expired())
			{
				auto inPtr = link.second->in.lock();
				auto outPtr = link.second->out.lock();
				if (inPtr && outPtr)
				{
					if (!inPtr->parentNode.expired() &&
						!outPtr->parentNode.expired())
					{
						auto inParentPtr = inPtr->parentNode.lock();
						auto outParentPtr = outPtr->parentNode.lock();

						if (inParentPtr && outParentPtr)
						{
							std::string inNodeIdSlotId = ct::toStr("%u:%u", (uint32_t)inParentPtr->nodeID.Get(), (uint32_t)inPtr->pinID.Get());
							std::string outNodeIdSlotId = ct::toStr("%u:%u", (uint32_t)outParentPtr->nodeID.Get(), (uint32_t)outPtr->pinID.Get());
							res += vOffset + "\t\t<link in=\"" + inNodeIdSlotId + "\" out=\"" + outNodeIdSlotId + "\"/>\n";
						}
					}
				}
			}
		}
		res += vOffset + "\t</links>\n";

		// outputs // left, middle, right mouse button (dont know wha is it in this common class)
		res += vOffset + "\t<outputs>\n";

		std::string outLeftSlot;
		auto slotLeftPtr = NodeSlot::sSlotGraphOutputMouseLeft.getValidShared();
		if (slotLeftPtr)
		{
			auto slotLeftParentNodePtr = slotLeftPtr->parentNode.getValidShared();
			if (slotLeftParentNodePtr)
			{
				outLeftSlot = ct::toStr("%u:%u", (uint32_t)slotLeftParentNodePtr->nodeID.Get(), (uint32_t)slotLeftPtr->pinID.Get());
				res += vOffset + "\t\t<output type=\"left\" ids=\"" + outLeftSlot + "\"/>\n";
			}
		}

		std::string outMiddleSlot;
		auto slotMiddlePtr = NodeSlot::sSlotGraphOutputMouseMiddle.getValidShared();
		if (slotMiddlePtr)
		{
			auto slotMiddleParentNodePtr = slotMiddlePtr->parentNode.getValidShared();
			if (slotMiddleParentNodePtr)
			{
				outMiddleSlot = ct::toStr("%u:%u", (uint32_t)slotMiddleParentNodePtr->nodeID.Get(), (uint32_t)slotMiddlePtr->pinID.Get());
				res += vOffset + "\t\t<output type=\"middle\" ids=\"" + outMiddleSlot + "\"/>\n";
			}
		}

		std::string outRightSlot;
		auto slotRightPtr = NodeSlot::sSlotGraphOutputMouseMiddle.getValidShared();
		if (slotRightPtr)
		{
			auto slotRightParentNodePtr = slotRightPtr->parentNode.getValidShared();
			if (slotRightParentNodePtr)
			{
				outRightSlot = ct::toStr("%u:%u", (uint32_t)slotRightParentNodePtr->nodeID.Get(), (uint32_t)slotRightPtr->pinID.Get());
				res += vOffset + "\t\t<output type=\"right\" ids=\"" + outMiddleSlot + "\"/>\n";
			}
		}

		res += vOffset + "\t</outputs>\n";

		res += vOffset + "</graph>\n";
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


		res += vOffset + "\t<generation>\n";

		res += vOffset + ct::toStr("\t\t<class_name>%s</class_name>\n", m_ClassName.c_str());
		res += vOffset + ct::toStr("\t\t<category_name>%s</category_name>\n", m_Category.c_str());
		res += vOffset + ct::toStr("\t\t<node_creation_name>%s</node_creation_name>\n", m_NodeCreationName.c_str());
		res += vOffset + ct::toStr("\t\t<node_display_name>%s</node_display_name>\n", m_NodeDisplayName.c_str());
		res += vOffset + ct::toStr("\t\t<generate_module>%s</generate_module>\n", m_GenerateAModule ? "true" : "false");
		res += vOffset + ct::toStr("\t\t<generate_pass>%s</generate_pass>\n", m_GenerateAPass ? "true" : "false");
		res += vOffset + ct::toStr("\t\t<renderer_type>%s</renderer_type>\n", m_RendererType.c_str());

		res += vOffset + "\t</generation>\n";

		res += vOffset + "</node>\n";
	}

	return res;
}

bool GeneratorNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "canvas")
	{
		if (strName == "offset")
			SetCanvasOffset(ct::toImVec2(ct::fvariant(strValue).GetV2()));
		else if (strName == "scale")
			SetCanvasScale(ct::fvariant(strValue).GetF());

		return false;
	}
	else if (strParentName == "nodes")
	{
		if (strName == "node")
		{
			std::string _name;
			std::string _type;
			ct::fvec2 _pos;
			uint32_t _nodeId = 0;

			for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
			{
				std::string attName = attr->Name();
				std::string attValue = attr->Value();

				if (attName == "name")
					_name = attValue;
				else if (attName == "type")
					_type = attValue;
				else if (attName == "pos")
					_pos = ct::fvariant(attValue).GetV2();
				else if (attName == "id")
					_nodeId = ct::ivariant(attValue).GetU();
			}

			if (LoadNodeFromXML_Callback(m_This, vElem, vParent, _name, _type, _pos, _nodeId))
			{
				RecursParsingConfigChilds(vElem, vUserDatas);
			}

			return false;
		}
	}
	else if (strParentName == "links")
	{
		if (strName == "link")
		{
			std::string inStr;
			std::string outStr;

			for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
			{
				std::string attName = attr->Name();
				std::string attValue = attr->Value();

				if (attName == "in")
					inStr = attValue;
				else if (attName == "out")
					outStr = attValue;
			}

			auto vecIn = ct::splitStringToVector(inStr, ':');
			auto vecOut = ct::splitStringToVector(outStr, ':');

			if (vecIn.size() == 2 && vecOut.size() == 2)
			{
				SlotEntry entIn;
				entIn.first = ct::ivariant(vecIn[0]).GetU();
				entIn.second = ct::ivariant(vecIn[1]).GetU();

				SlotEntry entOut;
				entOut.first = ct::ivariant(vecOut[0]).GetU();
				entOut.second = ct::ivariant(vecOut[1]).GetU();

				LinkEntry link;
				link.first = entIn;
				link.second = entOut;

				m_LinksToBuildAfterLoading.push_back(link);
			}
		}

		return false;
	}
	else if (strParentName == "node")
	{
		NodeSlot slot;
		GeneratorNodeSlotDatas slotDatas;

		if (strName == "slot")
		{
			for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
			{
				std::string attName = attr->Name();
				std::string attValue = attr->Value();

				if (attName == "index")
					slot.index = ct::ivariant(attValue).GetU();
				else if (attName == "name")
					slot.name = attValue;
				else if (attName == "type")
					slot.slotType = attValue;
				else if (attName == "place")
					slot.slotPlace = NodeSlot::sGetNodeSlotPlaceEnumFromString(attValue);
				else if (attName == "id")
					slot.pinID = ct::ivariant(attValue).GetU();
				else if (attName == "hideName")
					slot.hideName = ct::ivariant(attValue).GetB();
				else if (attName == "typeIndex")
					slotDatas.editorSlotTypeIndex = ct::ivariant(attValue).GetU();
			}

			if (slot.slotPlace == NodeSlot::PlaceEnum::INPUT)
			{
				auto slot_input_ptr = GeneratorNodeSlotInput::Create();
				slot_input_ptr->index = slot.index;
				slot_input_ptr->name = slot.name;
				slot_input_ptr->slotType = slot.slotType;
				slot_input_ptr->slotPlace = slot.slotPlace;
				slot_input_ptr->pinID = slot.pinID;
				slot_input_ptr->editorSlotTypeIndex = slotDatas.editorSlotTypeIndex;

				// selection of the first slot
				if (m_Inputs.empty())
				{
					NodeSlot::sSlotGraphOutputMouseLeft = slot_input_ptr;
				}

				bool wasSet = false;
				for (auto input : m_Inputs)
				{
					if (input.second->index == slot_input_ptr->index)
					{
						wasSet = !input.second->setFromXml(vElem, vParent);
						if (wasSet)
						{
							m_Inputs.erase(input.first);
							m_Inputs[(uint32_t)input.second->pinID.Get()] = input.second;
							break;
						}
					}
				}
				if (!wasSet)
				{
					auto slotPtr = AddInput(slot_input_ptr, false, slot.hideName).getValidShared();
					if (slotPtr)
					{
						slotPtr->idAlreadySetbyXml = true;
					}
				}
			}
			else if (slot.slotPlace == NodeSlot::PlaceEnum::OUTPUT)
			{
				auto slot_output_ptr = GeneratorNodeSlotOutput::Create();
				slot_output_ptr->index = slot.index;
				slot_output_ptr->name = slot.name;
				slot_output_ptr->slotType = slot.slotType;
				slot_output_ptr->slotPlace = slot.slotPlace;
				slot_output_ptr->pinID = slot.pinID;
				slot_output_ptr->editorSlotTypeIndex = slotDatas.editorSlotTypeIndex;

				// selection of the first slot
				if (m_Outputs.empty())
				{
					NodeSlot::sSlotGraphOutputMouseRight = slot_output_ptr;
				}

				bool wasSet = false;
				for (auto output : m_Outputs)
				{
					if (output.second->index == slot_output_ptr->index)
					{
						wasSet = !output.second->setFromXml(vElem, vParent);
						if (wasSet)
						{
							m_Outputs.erase(output.first);
							m_Outputs[(uint32_t)output.second->pinID.Get()] = output.second;
							break;
						}
					}
				}
				if (!wasSet)
				{
					auto slotPtr = AddOutput(slot_output_ptr, false, slot.hideName).getValidShared();
					if (slotPtr)
					{
						slotPtr->idAlreadySetbyXml = true;
					}
				}
			}

			return false;
		}
	}
	else if (strParentName == "outputs")
	{
		if (strName == "output")
		{
			std::string type;
			std::string ids;

			for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
			{
				std::string attName = attr->Name();
				std::string attValue = attr->Value();

				if (attName == "type")
					type = attValue;
				else if (attName == "ids")
					ids = attValue;
			}

			auto vec = ct::splitStringToVector(ids, ':');

			if (vec.size() == 2)
			{
				SlotEntry ent;
				ent.first = ct::ivariant(vec[0]).GetU();
				ent.second = ct::ivariant(vec[1]).GetU();

				if (type == "left") { m_OutputLeftSlotToSelectAfterLoading = ent; }
				else if (type == "middle") { m_OutputMiddleSlotToSelectAfterLoading = ent; }
				else if (type == "right") { m_OutputRightSlotToSelectAfterLoading = ent; }
			}
		}

		return false;
	}
	else if (strParentName == "generation")
	{
		if (strName == "class_name")
			m_ClassName = strValue;
		else if (strName == "category_name")
			m_Category = strValue;
		else if (strName == "node_creation_name")
			m_NodeCreationName = strValue;
		else if (strName == "node_display_name")
			m_NodeDisplayName = strValue;
		else if (strName == "generate_module")
			m_GenerateAModule = ct::ivariant(strValue).GetB();
		else if (strName == "generate_pass")
			m_GenerateAPass = ct::ivariant(strValue).GetB();
		else if (strName == "renderer_type")
			m_RendererType = strValue;

		return true;
	}

	return true;
}
