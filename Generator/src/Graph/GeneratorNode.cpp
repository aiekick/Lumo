#include "GeneratorNode.h"

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Graph/Base/NodeSlot.h>
#include <Graph/Base/BaseNode.h>
#include <Graph/GeneratorCommon.h>
#include <Graph/GeneratorNodeSlotInput.h>
#include <Graph/GeneratorNodeSlotOutput.h>
#include <Graph/Slots/NodeSlotModelInput.h>
#include <Graph/Slots/NodeSlotModelOutput.h>
#include <Graph/Slots/NodeSlotTextureInput.h>
#include <Graph/Slots/NodeSlotTextureOutput.h>
#include <Graph/Slots/NodeSlotVariableInput.h>
#include <Graph/Slots/NodeSlotVariableOutput.h>
#include <Graph/Slots/NodeSlotLightGroupInput.h>
#include <Graph/Slots/NodeSlotLightGroupOutput.h>
#include <Graph/Slots/NodeSlotTexelBufferInput.h>
#include <Graph/Slots/NodeSlotTexelBufferOutput.h>
#include <Graph/Slots/NodeSlotTextureGroupInput.h>
#include <Graph/Slots/NodeSlotTextureGroupOutput.h>
#include <Graph/Slots/NodeSlotStorageBufferInput.h>
#include <Graph/Slots/NodeSlotStorageBufferOutput.h>

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
		res += vOffset + ct::toStr("\t\t<category_name>%s</category_name>\n", m_CategoryName.c_str());
		res += vOffset + ct::toStr("\t\t<node_creation_name>%s</node_creation_name>\n", m_NodeCreationName.c_str());
		res += vOffset + ct::toStr("\t\t<node_display_name>%s</node_display_name>\n", m_NodeDisplayName.c_str());
		res += vOffset + ct::toStr("\t\t<generate_module>%s</generate_module>\n", m_GenerateAModule ? "true" : "false");
		res += vOffset + ct::toStr("\t\t<generate_pass>%s</generate_pass>\n", m_GenerateAPass ? "true" : "false");
		res += vOffset + ct::toStr("\t\t<renderer_type>%s</renderer_type>\n", m_RendererType.c_str());
		res += vOffset + ct::toStr("\t\t<module_display_name>%s</module_display_name>\n", m_ModuleDisplayName.c_str());
		res += vOffset + ct::toStr("\t\t<module_xml_name>%s</module_xml_name>\n", m_ModuleXmlName.c_str());
		
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
			m_CategoryName = strValue;
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
		else if (strName == "module_display_name")
			m_ModuleDisplayName = strValue;
		else if (strName == "module_xml_name")
			m_ModuleXmlName = strValue;

		return true;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// GENERATOR NODE //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorNode::GenerateNodeClasses(const std::string& vPath, const ProjectFile* vDatas)
{
	std::string node_class_name = m_ClassName + "Node";
	std::string cpp_node_file_name = node_class_name + ".cpp";
	std::string h_node_file_name = node_class_name + ".h";

	std::string module_class_name = m_ClassName + "Module";
	std::string cpp_module_file_name = module_class_name + ".cpp";
	std::string h_module_file_name = module_class_name + ".h";

	std::string cpp_node_file_code;
	std::string h_node_file_code;
	std::string h_node_interfaces_code;

	auto slotDico = GetSlotDico();

	cpp_node_file_code += GetLicenceHeader();
	h_node_file_code += GetLicenceHeader();
	cpp_node_file_code += GetPVSStudioHeader();

	h_node_file_code += u8R"(
#include <Graph/Graph.h>
#include <Graph/Base/BaseNode.h>
)";

	cpp_node_file_code += ct::toStr(u8R"(
#include "%s.h"
)", node_class_name.c_str());

	if (m_GenerateAModule)
	{
		cpp_node_file_code += ct::toStr(u8R"(#include <Modules/%s/%s.h>
)", m_CategoryName.c_str(), module_class_name.c_str());
		GenerateModules(vPath, vDatas);
	}

	cpp_node_file_code += GetNodeSlotsInputIncludesSlots(slotDico);
	cpp_node_file_code += u8R"(
)";
	cpp_node_file_code += GetNodeSlotsOutputIncludesSlots(slotDico);
	h_node_file_code += GetNodeSlotsInputIncludesInterfaces(slotDico);
	h_node_file_code += u8R"(
)";
	h_node_file_code += GetNodeSlotsOutputIncludesInterfaces(slotDico);
	h_node_file_code += u8R"(
)";
	h_node_file_code += u8R"(
class MODULE_CLASS_NAME;
class NODE_CLASS_NAME :
	public BaseNode,)";
	cpp_node_file_code += u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<NODE_CLASS_NAME> NODE_CLASS_NAME::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	auto res = std::make_shared<NODE_CLASS_NAME>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

NODE_CLASS_NAME::NODE_CLASS_NAME() : BaseNode()
{
	ZoneScoped;

	m_NodeTypeString = "NODE_CREATION_NAME";
}

NODE_CLASS_NAME::~NODE_CLASS_NAME()
{
	ZoneScoped;

	Unit();
}		

//////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool NODE_CLASS_NAME::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	bool res = false;

	name = "NODE_DISPLAY_NAME";
)";

	cpp_node_file_code += GetNodeSlotsInputFuncs(slotDico);
	cpp_node_file_code += u8R"(
)";
	cpp_node_file_code += GetNodeSlotsOutputFuncs(slotDico);
	cpp_node_file_code += u8R"(
)";

	if (m_GenerateAModule)
	{
		cpp_node_file_code += u8R"(

	m_MODULE_CLASS_NAMEPtr = MODULE_CLASS_NAME::Create(vVulkanCorePtr);
	if (m_MODULE_CLASS_NAMEPtr)
	{
		res = true;
	})";
	}

	cpp_node_file_code += u8R"(

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TASK EXECUTE ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool NODE_CLASS_NAME::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	bool res = false;

	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureInputDescriptorImageInfos(m_Inputs);
)";

	if (m_GenerateAModule)
	{
		cpp_node_file_code += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		res = m_MODULE_CLASS_NAMEPtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}
)";
	}
		
	cpp_node_file_code += u8R"(
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool NODE_CLASS_NAME::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	bool res = false;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
)";

	if (m_GenerateAModule)
	{
		cpp_node_file_code += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		res = m_MODULE_CLASS_NAMEPtr->DrawWidgets(vCurrentFrame, vContext);
	}
)";
	}

	cpp_node_file_code += u8R"(
	return res;
}

void NODE_CLASS_NAME::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);)";

	if (m_GenerateAModule)
	{
		cpp_node_file_code += u8R"(

	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	})";
	}

	cpp_node_file_code += u8R"(
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW NODE ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	if (vBaseNodeState && vBaseNodeState->debug_mode)
	{
		auto drawList = nd::GetNodeBackgroundDrawList(nodeID);
		if (drawList)
		{
			char debugBuffer[255] = "\0";
			snprintf(debugBuffer, 254,
				"Used[%s]\nCell[%i, %i]",
				(used ? "true" : "false"), cell.x, cell.y);
			ImVec2 txtSize = ImGui::CalcTextSize(debugBuffer);
			drawList->AddText(pos - ImVec2(0, txtSize.y), ImGui::GetColorU32(ImGuiCol_Text), debugBuffer);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// RESIZE //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		cpp_node_file_code += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}
)";
	}

	cpp_node_file_code += u8R"(
	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}
)";

	cpp_node_file_code += GetNodeSlotsInputCppFuncs(slotDico);
	cpp_node_file_code += GetNodeSlotsOutputCppFuncs(slotDico);

	cpp_node_file_code += u8R"(

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string NODE_CLASS_NAME::getXml(const std::string& vOffset, const std::string& vUserDatas)
{	
	ZoneScoped;

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
)";

	if (m_GenerateAModule)
	{
		cpp_node_file_code += u8R"(
		if (m_MODULE_CLASS_NAMEPtr)
		{
			res += m_MODULE_CLASS_NAMEPtr->getXml(vOffset + "\t", vUserDatas);
		}
)";
	}

	cpp_node_file_code += u8R"(
		res += vOffset + "</node>\n";
	}

	return res;
}

bool NODE_CLASS_NAME::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{	
	ZoneScoped;

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
)";

	if (m_GenerateAModule)
	{
		cpp_node_file_code += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->setFromXml(vElem, vParent, vUserDatas);
	}
)";
	}

	cpp_node_file_code += u8R"(
	// continue recurse child exploring
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SHADER UPDATE ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::UpdateShaders(const std::set<std::string>& vFiles)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		cpp_node_file_code += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->UpdateShaders(vFiles);
	})";
	}
	else
	{
		cpp_node_file_code += u8R"(
)";
	}

	cpp_node_file_code += u8R"(
}
)";

	h_node_file_code += u8R"(
{
public:
	static std::shared_ptr<NODE_CLASS_NAME> Create(vkApi::VulkanCorePtr vVulkanCorePtr);
)";

	if (m_GenerateAModule)
	{
		h_node_file_code += u8R"(
private:
	std::shared_ptr<MODULE_CLASS_NAME> m_MODULE_CLASS_NAMEPtr = nullptr;
)";
	}

	h_node_file_code += u8R"(
public:
	NODE_CLASS_NAME();
	~NODE_CLASS_NAME() override;

	// Init / Unit
	bool Init(vkApi::VulkanCorePtr vVulkanCorePtr) override;
	
	// Execute Task
	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
	
	// Draw Widgets
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) override;
	
	// Resize
	void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
)";

	h_node_file_code += GetNodeSlotsInputHFuncs(slotDico);
	cpp_node_file_code += u8R"(
)";
	h_node_file_code += GetNodeSlotsOutputHFuncs(slotDico);
	cpp_node_file_code += u8R"(
)";

	h_node_file_code += u8R"(

	// Configuration
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
	
	// Shader Update
	void UpdateShaders(const std::set<std::string>& vFiles) override;
};
)";

	ct::replaceString(cpp_node_file_code, "NODE_CLASS_NAME", node_class_name);
	ct::replaceString(h_node_file_code, "NODE_CLASS_NAME", node_class_name);

	ct::replaceString(cpp_node_file_code, "NODE_CREATION_NAME", m_NodeCreationName);
	ct::replaceString(h_node_file_code, "NODE_CREATION_NAME", m_NodeCreationName);

	ct::replaceString(cpp_node_file_code, "NODE_DISPLAY_NAME", m_NodeDisplayName);
	ct::replaceString(h_node_file_code, "NODE_DISPLAY_NAME", m_NodeDisplayName);

	ct::replaceString(cpp_node_file_code, "MODULE_CLASS_NAME", module_class_name);
	ct::replaceString(h_node_file_code, "MODULE_CLASS_NAME", module_class_name);

	FileHelper::Instance()->SaveStringToFile(cpp_node_file_code, cpp_node_file_name);
	FileHelper::Instance()->SaveStringToFile(h_node_file_code, h_node_file_name);

	if (m_GenerateAModule)
	{
		GenerateModules(vPath, vDatas);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// GENERATOR MODULLE ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorNode::GenerateModules(const std::string& vPath, const ProjectFile* vDatas)
{
	std::string module_class_name = m_ClassName + "Module";
	std::string cpp_module_file_name = module_class_name + ".cpp";
	std::string h_module_file_name = module_class_name + ".h";

	std::string pass_renderer_display_type = GetRendererDisplayName();
	std::string pass_class_name = m_ClassName + "_" + pass_renderer_display_type + "_Pass";
	std::string cpp_pass_file_name = pass_class_name + ".cpp";
	std::string h_pass_file_name = pass_class_name + ".h";

	std::string cpp_module_file_code;
	std::string h_module_file_code;

	cpp_module_file_code += GetLicenceHeader();
	h_module_file_code += GetLicenceHeader();
	cpp_module_file_code += GetPVSStudioHeader();

	// MODULE_XML_NAME							grayscott_module_sim
	// MODULE_DISPLAY_NAME						Gray Scott
	// MODULE_CATEGORY_NAME						Simulation
	// MODULE_CLASS_NAME		
	// PASS_CLASS_NAME
	// MODULE_RENDERER_INIT_FUNC				InitCompute2D
	// RENDERER_DISPLAY_TYPE (ex (Comp)			Comp

	cpp_module_file_code += u8R"(
#include "MODULE_CLASS_NAME.h"

#include <cinttypes>
#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanSubmitter.h>
#include <utils/Mesh/VertexStruct.h>
#include <Base/FrameBuffer.h>
)";
	if (m_GenerateAPass)
	{
		cpp_module_file_code += u8R"(
#include <Modules/MODULE_CATEGORY_NAME/Pass/PASS_CLASS_NAME.h>
)";
	}

	cpp_module_file_code += u8R"(
using namespace vkApi;

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<MODULE_CLASS_NAME> MODULE_CLASS_NAME::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<MODULE_CLASS_NAME>(vVulkanCorePtr);
	res->m_This = res;
	if (!res->Init())
	{
		res.reset();
	}
	return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

MODULE_CLASS_NAME::MODULE_CLASS_NAME(vkApi::VulkanCorePtr vVulkanCorePtr)
	: BaseRenderer(vVulkanCorePtr)
{
	ZoneScoped;
}

MODULE_CLASS_NAME::~MODULE_CLASS_NAME()
{
	ZoneScoped;

	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool MODULE_CLASS_NAME::Init()
{
	ZoneScoped;

	m_Loaded = false;

)";

	if (m_GenerateAPass)
	{
		if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
		{
			cpp_module_file_code += u8R"(

	ct::uvec2 map_size = 512;

	if (BaseRenderer::InitPixel(map_size))
	{
		//SetExecutionWhenNeededOnly(true);

		m_PASS_CLASS_NAME_Ptr = std::make_shared<PASS_CLASS_NAME>(m_VulkanCorePtr);
		if (m_PASS_CLASS_NAME_Ptr)
		{
			// by default but can be changed via widget
			//m_PASS_CLASS_NAME_Ptr->AllowResizeOnResizeEvents(false);
			//m_PASS_CLASS_NAME_Ptr->AllowResizeByHand(true);

			if (m_PASS_CLASS_NAME_Ptr->InitPixel(map_size, 1U, true, true, 0.0f,
				false, vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1))
			{
				AddGenericPass(m_PASS_CLASS_NAME_Ptr);
				m_Loaded = true;
			}
		}
	}
)";
		}
		else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D)
		{
			cpp_module_file_code += u8R"(

	uint32_t map_size = 512;

	if (BaseRenderer::InitCompute1D(map_size))
	{
		//SetExecutionWhenNeededOnly(true);

		m_PASS_CLASS_NAME_Ptr = std::make_shared<PASS_CLASS_NAME>(m_VulkanCorePtr);
		if (m_PASS_CLASS_NAME_Ptr)
		{
			// by default but can be changed via widget
			//m_PASS_CLASS_NAME_Ptr->AllowResizeOnResizeEvents(false);
			//m_PASS_CLASS_NAME_Ptr->AllowResizeByHand(true);

			if (m_PASS_CLASS_NAME_Ptr->InitCompute1D(map_size))
			{
				AddGenericPass(m_PASS_CLASS_NAME_Ptr);
				m_Loaded = true;
			}
		}
	}
)";
		}
		else if (m_RendererType == RENDERER_TYPE_COMPUTE_2D)
		{
			cpp_module_file_code += u8R"(

	ct::uvec2 map_size = 512;

	if (BaseRenderer::InitCompute2D(map_size))
	{
		//SetExecutionWhenNeededOnly(true);

		m_PASS_CLASS_NAME_Ptr = std::make_shared<PASS_CLASS_NAME>(m_VulkanCorePtr);
		if (m_PASS_CLASS_NAME_Ptr)
		{
			// by default but can be changed via widget
			//m_PASS_CLASS_NAME_Ptr->AllowResizeOnResizeEvents(false);
			//m_PASS_CLASS_NAME_Ptr->AllowResizeByHand(true);

			if (m_PASS_CLASS_NAME_Ptr->InitCompute2D(map_size, 1U, false, vk::Format::eR32G32B32A32Sfloat))
			{
				AddGenericPass(m_PASS_CLASS_NAME_Ptr);
				m_Loaded = true;
			}
		}
	}
)";
		}
		else if (m_RendererType == RENDERER_TYPE_COMPUTE_3D)
		{
			cpp_module_file_code += u8R"(

	ct::uvec3 map_size = 512;

	if (BaseRenderer::InitCompute3D(map_size))
	{
		//SetExecutionWhenNeededOnly(true);

		m_PASS_CLASS_NAME_Ptr = std::make_shared<PASS_CLASS_NAME>(m_VulkanCorePtr);
		if (m_PASS_CLASS_NAME_Ptr)
		{
			// by default but can be changed via widget
			//m_PASS_CLASS_NAME_Ptr->AllowResizeOnResizeEvents(false);
			//m_PASS_CLASS_NAME_Ptr->AllowResizeByHand(true);

			if (m_PASS_CLASS_NAME_Ptr->InitCompute3D(map_size))
			{
				AddGenericPass(m_PASS_CLASS_NAME_Ptr);
				m_Loaded = true;
			}
		}
	}
)";
		}
		else if (m_RendererType == RENDERER_TYPE_RTX)
		{
			cpp_module_file_code += u8R"(

	ct::uvec2 map_size = 512;

	if (BaseRenderer::InitRtx(map_size))
	{
		//SetExecutionWhenNeededOnly(true);

		m_PASS_CLASS_NAME_Ptr = std::make_shared<PASS_CLASS_NAME>(m_VulkanCorePtr);
		if (m_PASS_CLASS_NAME_Ptr)
		{
			// by default but can be changed via widget
			//m_PASS_CLASS_NAME_Ptr->AllowResizeOnResizeEvents(false);
			//m_PASS_CLASS_NAME_Ptr->AllowResizeByHand(true);

			if (m_PASS_CLASS_NAME_Ptr->InitRtx(map_size, 1U, false, vk::Format::eR32G32B32A32Sfloat))
			{
				AddGenericPass(m_PASS_CLASS_NAME_Ptr);
				m_Loaded = true;
			}
		}
	}
)";
		}
	}

	cpp_module_file_code += u8R"(
	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool MODULE_CLASS_NAME::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("MODULE_DISPLAY_NAME", vCmd);

	return true;
}

bool MODULE_CLASS_NAME::ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("MODULE_DISPLAY_NAME", vCmd);

	return true;
}

bool MODULE_CLASS_NAME::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("MODULE_DISPLAY_NAME", -1.0f, true, true, &m_CanWeRender))
		{
			bool change = false;

			change |= DrawResizeWidget();
)";
	if (m_GenerateAPass)
	{
		cpp_module_file_code += u8R"(
			if (m_PASS_CLASS_NAME_Ptr)
			{
				change |= m_PASS_CLASS_NAME_Ptr->DrawWidgets(vCurrentFrame, vContext);
			}
)";
	}

	cpp_module_file_code += u8R"(
			return change;
		}
	}

	return false;
}

void MODULE_CLASS_NAME::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void MODULE_CLASS_NAME::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void MODULE_CLASS_NAME::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	// do some code
	
	BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void MODULE_CLASS_NAME::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	ZoneScoped;
)";

	if (m_GenerateAPass)
	{
		cpp_module_file_code += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		m_PASS_CLASS_NAME_Ptr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	}
)";
	}

	cpp_module_file_code += u8R"(
}

vk::DescriptorImageInfo* MODULE_CLASS_NAME::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	ZoneScoped;
)";
	if (m_GenerateAPass)
	{
		cpp_module_file_code += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		return m_PASS_CLASS_NAME_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}
)";
	}

	cpp_module_file_code += u8R"(
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MODULE_CLASS_NAME::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += vOffset + "<MODULE_XML_NAME>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";
)";
	if (m_GenerateAPass)
	{
		cpp_module_file_code += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		str += m_PASS_CLASS_NAME_Ptr->getXml(vOffset + "\t", vUserDatas);
	}
)";
	}

	cpp_module_file_code += u8R"(
	str += vOffset + "</MODULE_XML_NAME>\n";

	return str;
}

bool MODULE_CLASS_NAME::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	ZoneScoped;

	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strParentName == "MODULE_XML_NAME")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();
	}
)";
	if (m_GenerateAPass)
	{
		cpp_module_file_code += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		m_PASS_CLASS_NAME_Ptr->setFromXml(vElem, vParent, vUserDatas);
	}
)";
	}

	cpp_module_file_code += u8R"(
	return true;
}
)";

	ct::replaceString(cpp_module_file_code, "MODULE_XML_NAME", m_ModuleXmlName);
	ct::replaceString(h_module_file_code, "MODULE_XML_NAME", m_ModuleXmlName);

	ct::replaceString(cpp_module_file_code, "MODULE_DISPLAY_NAME", m_ModuleDisplayName);
	ct::replaceString(h_module_file_code, "MODULE_DISPLAY_NAME", m_ModuleDisplayName);

	ct::replaceString(cpp_module_file_code, "MODULE_CATEGORY_NAME", m_CategoryName);
	ct::replaceString(h_module_file_code, "MODULE_CATEGORY_NAME", m_CategoryName);

	ct::replaceString(cpp_module_file_code, "MODULE_CLASS_NAME", module_class_name);
	ct::replaceString(h_module_file_code, "MODULE_CLASS_NAME", module_class_name);

	ct::replaceString(cpp_module_file_code, "PASS_CLASS_NAME", pass_class_name);
	ct::replaceString(h_module_file_code, "PASS_CLASS_NAME", pass_class_name);

	ct::replaceString(cpp_module_file_code, "MODULE_RENDERER_INIT_FUNC", m_ModuleRendererInitFunc);
	ct::replaceString(h_module_file_code, "MODULE_RENDERER_INIT_FUNC", m_ModuleRendererInitFunc);

	ct::replaceString(cpp_module_file_code, "RENDERER_DISPLAY_TYPE", m_ModuleRendererDisplayType);
	ct::replaceString(h_module_file_code, "RENDERER_DISPLAY_TYPE", m_ModuleRendererDisplayType);

	FileHelper::Instance()->SaveStringToFile(cpp_module_file_code, cpp_module_file_name);
	FileHelper::Instance()->SaveStringToFile(h_module_file_code, h_module_file_name);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// GENERATOR PASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorNode::GeneratePasses(const std::string& vPath, const ProjectFile* vDatas)
{

}

//////////////////////////////////////////////////////////////////////////////////////////////
//// GENERATOR LICENSE HEADER ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string GeneratorNode::GetLicenceHeader()
{
	return
		u8R"(/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/
)";
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// GENERATOR PVS STUDIO HEADER /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string GeneratorNode::GetPVSStudioHeader()
{
	return
		u8R"(
// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
)";
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// GENERATOR RENDERER DISPLAY NAME /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string GeneratorNode::GetRendererDisplayName()
{
	std::string res;

	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		res = "Pixel";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D)
	{
		res = "Comp_1D";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_2D)
	{
		res = "Comp_2D";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_3D)
	{
		res = "Comp_3D";
	}
	else if (m_RendererType == RENDERER_TYPE_RTX)
	{
		res = "Rtx";
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// GENERATOR SLOTS DICO ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SlotDico GeneratorNode::GetSlotDico()
{
	SlotDico res;

	for (const auto& inputSlot : m_Inputs)
	{
		if (inputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(inputSlot.second);
			if (slotDatasPtr)
			{
				m_InputSlotCounter[slotDatasPtr->editorSlotTypeIndex]++;
				switch(slotDatasPtr->editorSlotTypeIndex)
				{
				case BaseTypeEnum::BASE_TYPE_None: // None
					res[BASE_TYPE_None][NodeSlot::PlaceEnum::INPUT] = GetSlotNoneInput(inputSlot.second);
					break;
				case BaseTypeEnum::BASE_TYPE_LightGroup: // LightGroup
					res[BASE_TYPE_LightGroup][NodeSlot::PlaceEnum::INPUT] = GetSlotLightGroupInput(inputSlot.second);
					break;
				case BaseTypeEnum::BASE_TYPE_Model: // Model
					res[BASE_TYPE_Model][NodeSlot::PlaceEnum::INPUT] = GetSlotModelInput(inputSlot.second);
					break;
				case BaseTypeEnum::BASE_TYPE_StorageBuffer: // StorageBuffer
					res[BASE_TYPE_StorageBuffer][NodeSlot::PlaceEnum::INPUT] = GetSlotStorageBufferInput(inputSlot.second);
					break;
				case BaseTypeEnum::BASE_TYPE_TexelBuffer: // TexelBuffer
					res[BASE_TYPE_TexelBuffer][NodeSlot::PlaceEnum::INPUT] = GetSlotTexelBufferInput(inputSlot.second);
					break;
				case BaseTypeEnum::BASE_TYPE_Texture: // Texture
					res[BASE_TYPE_Texture][NodeSlot::PlaceEnum::INPUT] = GetSlotTextureInput(inputSlot.second);
					break;
				case BaseTypeEnum::BASE_TYPE_TextureGroup: // TextureGroup
					res[BASE_TYPE_TextureGroup][NodeSlot::PlaceEnum::INPUT] = GetSlotTextureGroupInput(inputSlot.second);
					break;
				case BaseTypeEnum::BASE_TYPE_Variable: // Variable
					res[BASE_TYPE_Variable][NodeSlot::PlaceEnum::INPUT] = GetSlotVariableInput(inputSlot.second);
					break;
				case BaseTypeEnum::BASE_TYPE_Custom: // Custom
					res[BASE_TYPE_Custom][NodeSlot::PlaceEnum::INPUT] = GetSlotCustomInput(inputSlot.second);
					break;
				}
				
			}
		}
	}

	for (const auto& outputSlot : m_Outputs)
	{
		if (outputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(outputSlot.second);
			if (slotDatasPtr)
			{
				m_OutputSlotCounter[slotDatasPtr->editorSlotTypeIndex]++;
				switch (slotDatasPtr->editorSlotTypeIndex)
				{
				case BaseTypeEnum::BASE_TYPE_None: // None
					res[BASE_TYPE_None][NodeSlot::PlaceEnum::OUTPUT] = GetSlotNoneOutput(outputSlot.second);
					break;
				case BaseTypeEnum::BASE_TYPE_LightGroup: // LightGroup
					res[BASE_TYPE_LightGroup][NodeSlot::PlaceEnum::OUTPUT] = GetSlotLightGroupOutput(outputSlot.second);
					break;
				case BaseTypeEnum::BASE_TYPE_Model: // Model
					res[BASE_TYPE_Model][NodeSlot::PlaceEnum::OUTPUT] = GetSlotModelOutput(outputSlot.second);
					break;
				case BaseTypeEnum::BASE_TYPE_StorageBuffer: // StorageBuffer
					res[BASE_TYPE_StorageBuffer][NodeSlot::PlaceEnum::OUTPUT] = GetSlotStorageBufferOutput(outputSlot.second);
					break;
				case BaseTypeEnum::BASE_TYPE_TexelBuffer: // TexelBuffer
					res[BASE_TYPE_TexelBuffer][NodeSlot::PlaceEnum::OUTPUT] = GetSlotTexelBufferOutput(outputSlot.second);
					break;
				case BaseTypeEnum::BASE_TYPE_Texture: // Texture
					res[BASE_TYPE_Texture][NodeSlot::PlaceEnum::OUTPUT] = GetSlotTextureOutput(outputSlot.second);
					break;
				case BaseTypeEnum::BASE_TYPE_TextureGroup: // TextureGroup
					res[BASE_TYPE_TextureGroup][NodeSlot::PlaceEnum::OUTPUT] = GetSlotTextureGroupOutput(outputSlot.second);
					break;
				case BaseTypeEnum::BASE_TYPE_Variable: // Variable
					res[BASE_TYPE_Variable][NodeSlot::PlaceEnum::OUTPUT] = GetSlotVariableOutput(outputSlot.second);
					break;
				case BaseTypeEnum::BASE_TYPE_Custom: // Custom
					res[BASE_TYPE_Custom][NodeSlot::PlaceEnum::OUTPUT] = GetSlotCustomOutput(outputSlot.second);
					break;
				}
			}
		}
	}

	
	return res;
}

SlotStringStruct GeneratorNode::GetSlotNoneInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"()";

	res.h_func = u8R"()";

	res.include_interface = u8R"(
#include <Interfaces/NoneInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotNoneInput.h>)";

	res.node_public_interface = u8R"()";

	res.node_slot_func = u8R"()";

	return res;
}

SlotStringStruct GeneratorNode::GetSlotNoneOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"()";

	res.h_func = u8R"()";

	res.include_interface = u8R"(
#include <Interfaces/NoneOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotNoneOutput.h>)";

	res.node_public_interface = u8R"()";

	res.node_slot_func = u8R"()";

	return res;
}

SlotStringStruct GeneratorNode::GetSlotLightGroupInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"()";

	res.h_func = u8R"()";

	res.include_interface = u8R"(
#include <Interfaces/LightGroupInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotLightGroupInput.h>)";

	res.node_public_interface = u8R"(
	public LightGroupInputInterface,)";

	res.node_slot_func = ct::toStr(u8R"(
	AddInput(NodeSlotLightGroupInput::Create("%s"), false, %s);)",
		vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotLightGroupOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"()";

	res.h_func = u8R"()";

	res.include_interface = u8R"(
#include <Interfaces/LightGroupOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotLightGroupOutput.h>)";

	res.node_public_interface = u8R"(
	public LightGroupOutputInterface,)";

	res.node_slot_func = ct::toStr(u8R"(
	AddOutput(NodeSlotLightGroupOutput::Create("%s"), false, %s);)",
		vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotModelInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"()";

	res.h_func = u8R"()";

	res.include_interface = u8R"(
#include <Interfaces/ModelInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotModelInput.h>)";

	res.node_public_interface = u8R"(
	public ModelInputInterface,)";

	res.node_slot_func = ct::toStr(u8R"(
	AddInput(NodeSlotModelInput::Create("%s"), false, %s);)",
		vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotModelOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"()";

	res.h_func = u8R"()";

	res.include_interface = u8R"(
#include <Interfaces/ModelOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotModelOutput.h>)";

	res.node_public_interface = u8R"(
	public ModelOutputInterface,)";

	res.node_slot_func = ct::toStr(u8R"(
	AddOutput(NodeSlotModelOutput::Create("%s"), false, %s);)",
		vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotStorageBufferInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"()";

	res.h_func = u8R"()";

	res.include_interface = u8R"(
#include <Interfaces/StorageBufferInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotStorageBufferInput.h>)";

	res.node_public_interface = u8R"(
	public StorageBufferInputInterface,)";

	res.node_slot_func = ct::toStr(u8R"(
	AddInput(NodeSlotStorageBufferInput::Create("%s"), false, %s);)",
		vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotStorageBufferOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"()";

	res.h_func = u8R"()";

	res.include_interface = u8R"(
#include <Interfaces/StorageBufferOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotStorageBufferOutput.h>)";

	res.node_public_interface = u8R"(
	public StorageBufferOutputInterface,)";

	res.node_slot_func = ct::toStr(u8R"(
	AddOutput(NodeSlotStorageBufferOutput::Create("%s"), false, %s);)",
		vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotTexelBufferInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"()";

	res.h_func = u8R"()";

	res.include_interface = u8R"(
#include <Interfaces/TexelBufferInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotTexelBufferInput.h>)";

	res.node_public_interface = u8R"(
	public TexelBufferInputInterface,)";

	res.node_slot_func = ct::toStr(u8R"(
	AddInput(NodeSlotTexelBufferInput::Create("%s"), false, %s);)",
		vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotTexelBufferOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"()";

	res.h_func = u8R"()";

	res.include_interface = u8R"(
#include <Interfaces/TexelBufferOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotTexelBufferOutput.h>)";

	res.node_public_interface = u8R"(
	public TexelBufferOutputInterface,)";

	res.node_slot_func = ct::toStr(u8R"(
	AddOutput(NodeSlotTexelBufferOutput::Create("%s"), false, %s);)",
		vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotTextureInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	})";
	}
	else
	{
		res.cpp_func += u8R"(
)";
	}

	res.cpp_func += u8R"(
}
)";

	res.h_func = u8R"(
	void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;)";

	res.include_interface = u8R"(
#include <Interfaces/TextureInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotTextureInput.h>)";

	res.node_public_interface = ct::toStr(u8R"(
	public TextureInputInterface<%u>,)", m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_Texture]);

	res.node_slot_func = ct::toStr(u8R"(
	AddInput(NodeSlotTextureInput::Create("%s", %u), false, %s);)",
		vSlot->name.c_str(),
		vSlot->descriptorBinding,
		vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotTextureOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* NODE_CLASS_NAME::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		return m_MODULE_CLASS_NAMEPtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}
)";
	}

	res.cpp_func += u8R"(
	return nullptr;
}
)";

	res.h_func = u8R"(
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;)";

	res.include_interface = u8R"(
#include <Interfaces/TextureOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotTextureOutput.h>)";

	res.node_public_interface = u8R"(
	public TextureOutputInterface,)";

	res.node_slot_func = ct::toStr(u8R"(
	AddOutput(NodeSlotTextureGroupOutput::Create("%s"), false, %s);)",
		vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotTextureGroupInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"()";

	res.h_func = u8R"()";

	res.include_interface = u8R"(
#include <Interfaces/TextureGroupInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotTextureGroupInput.h>)";

	res.node_public_interface = u8R"(
	public TextureGroupInputInterface,)";

	res.node_slot_func = ct::toStr(u8R"(
	AddInput(NodeSlotTextureGroupInput::Create("%s"), false, %s);)",
		vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotTextureGroupOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"()";

	res.h_func = u8R"()";

	res.include_interface = u8R"(
#include <Interfaces/TextureGroupOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotTextureGroupOutput.h>)";

	res.node_public_interface = u8R"(
	public TextureGroupOutputInterface,)";

	res.node_slot_func = ct::toStr(u8R"(
	AddOutput(NodeSlotTextureGroupOutput::Create("%s"), false, %s);)",
		vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotVariableInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"()";

	res.h_func = u8R"()";

	res.include_interface = u8R"(
#include <Interfaces/VariableInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotVariableInput.h>)";

	res.node_public_interface = u8R"(
	public VariableInputInterface,)";

	res.node_slot_func = ct::toStr(u8R"(
	AddInput(NodeSlotVariableInput::Create("%s", %s, %u), false, %s);)",
		vSlot->name.c_str(),
		vSlot->slotType.c_str(),
		vSlot->variableIndex,
		vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotVariableOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"()";

	res.h_func = u8R"()";

	res.include_interface = u8R"(
#include <Interfaces/VariableOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotVariableOutput.h>)";

	res.node_public_interface = u8R"(
	public VariableOutputInterface,)";

	res.node_slot_func = ct::toStr(u8R"(
	AddOutput(NodeSlotVariableOutput::Create("%s", %s, %u), false, %s);)",
		vSlot->name.c_str(),
		vSlot->slotType.c_str(),
		vSlot->variableIndex,
		vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotCustomInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"()";

	res.h_func = u8R"()";

	res.include_interface = u8R"(
#include <Interfaces/CustomInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotCustomInput.h>)";

	res.node_public_interface = u8R"()";

	res.node_slot_func = u8R"()";

	return res;
}

SlotStringStruct GeneratorNode::GetSlotCustomOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	res.cpp_func = u8R"()";

	res.h_func = u8R"()";

	res.include_interface = u8R"(
#include <Interfaces/CustomOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotCustomOutput.h>)";

	res.node_public_interface = u8R"()";

	res.node_slot_func = u8R"()";

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// GENERATOR NODE SLOTS SUMMARY CODE ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string GeneratorNode::GetNodeSlotsInputFuncs(const SlotDico& vDico)
{
	std::string res;

	for (const auto& inputSlot : m_Inputs)
	{
		if (inputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(inputSlot.second);
			if (slotDatasPtr)
			{
				BaseTypeEnum type = (BaseTypeEnum)slotDatasPtr->editorSlotTypeIndex;
				auto typeString = m_BaseTypes.m_TypeArray[slotDatasPtr->editorSlotTypeIndex];
				if (typeString == "None" ||
					typeString == "Custom")
				{
					continue;
				}

				if (vDico.find(type) != vDico.end())
				{
					if (vDico.at(type).find(NodeSlot::PlaceEnum::INPUT) != vDico.at(type).end())
					{
						res += vDico.at(type).at(NodeSlot::PlaceEnum::INPUT).node_slot_func;
					}
					else
					{
						CTOOL_DEBUG_BREAK;
					}
				}
				else
				{
					CTOOL_DEBUG_BREAK;
				}
			}
		}
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsOutputFuncs(const SlotDico& vDico)
{
	std::string res;

	for (const auto& outputSlot : m_Outputs)
	{
		if (outputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(outputSlot.second);
			if (slotDatasPtr)
			{
				BaseTypeEnum type = (BaseTypeEnum)slotDatasPtr->editorSlotTypeIndex;
				auto typeString = m_BaseTypes.m_TypeArray[slotDatasPtr->editorSlotTypeIndex];
				if (typeString == "None" ||
					typeString == "Custom")
				{
					continue;
				}

				if (vDico.find(type) != vDico.end())
				{
					if (vDico.at(type).find(NodeSlot::PlaceEnum::OUTPUT) != vDico.at(type).end())
					{
						res += vDico.at(type).at(NodeSlot::PlaceEnum::OUTPUT).node_slot_func;
					}
					else
					{
						CTOOL_DEBUG_BREAK;
					}
				}
				else
				{
					CTOOL_DEBUG_BREAK;
				}
			}
		}
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsInputPublicInterfaces(const SlotDico& vDico)
{
	std::string res;

	for (const auto& inputSlot : m_Inputs)
	{
		if (inputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(inputSlot.second);
			if (slotDatasPtr)
			{
				BaseTypeEnum type = (BaseTypeEnum)slotDatasPtr->editorSlotTypeIndex;
				auto typeString = m_BaseTypes.m_TypeArray[slotDatasPtr->editorSlotTypeIndex];
				if (typeString == "None" ||
					typeString == "Custom")
				{
					continue;
				}

				if (vDico.find(type) != vDico.end())
				{
					if (vDico.at(type).find(NodeSlot::PlaceEnum::INPUT) != vDico.at(type).end())
					{
						res += vDico.at(type).at(NodeSlot::PlaceEnum::INPUT).node_public_interface;
					}
					else
					{
						CTOOL_DEBUG_BREAK;
					}
				}
				else
				{
					CTOOL_DEBUG_BREAK;
				}
			}
		}
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsOutputPublicInterfaces(const SlotDico& vDico)
{
	std::string res;

	for (const auto& outputSlot : m_Outputs)
	{
		if (outputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(outputSlot.second);
			if (slotDatasPtr)
			{
				BaseTypeEnum type = (BaseTypeEnum)slotDatasPtr->editorSlotTypeIndex;
				auto typeString = m_BaseTypes.m_TypeArray[slotDatasPtr->editorSlotTypeIndex];
				if (typeString == "None" ||
					typeString == "Custom")
				{
					continue;
				}

				if (vDico.find(type) != vDico.end())
				{
					if (vDico.at(type).find(NodeSlot::PlaceEnum::OUTPUT) != vDico.at(type).end())
					{
						res += vDico.at(type).at(NodeSlot::PlaceEnum::OUTPUT).node_public_interface;
					}
					else
					{
						CTOOL_DEBUG_BREAK;
					}
				}
				else
				{
					CTOOL_DEBUG_BREAK;
				}
			}
		}
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsInputIncludesSlots(const SlotDico& vDico)
{
	std::string res;

	for (const auto& inputSlot : m_Inputs)
	{
		if (inputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(inputSlot.second);
			if (slotDatasPtr)
			{
				BaseTypeEnum type = (BaseTypeEnum)slotDatasPtr->editorSlotTypeIndex;
				auto typeString = m_BaseTypes.m_TypeArray[slotDatasPtr->editorSlotTypeIndex];
				if (typeString == "None" ||
					typeString == "Custom")
				{
					continue;
				}

				if (vDico.find(type) != vDico.end())
				{
					if (vDico.at(type).find(NodeSlot::PlaceEnum::INPUT) != vDico.at(type).end())
					{
						res += vDico.at(type).at(NodeSlot::PlaceEnum::INPUT).include_slot;
					}
					else
					{
						CTOOL_DEBUG_BREAK;
					}
				}
				else
				{
					CTOOL_DEBUG_BREAK;
				}
			}
		}
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsOutputIncludesSlots(const SlotDico& vDico)
{
	std::string res;

	for (const auto& outputSlot : m_Outputs)
	{
		if (outputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(outputSlot.second);
			if (slotDatasPtr)
			{
				BaseTypeEnum type = (BaseTypeEnum)slotDatasPtr->editorSlotTypeIndex;
				auto typeString = m_BaseTypes.m_TypeArray[slotDatasPtr->editorSlotTypeIndex];
				if (typeString == "None" ||
					typeString == "Custom")
				{
					continue;
				}

				if (vDico.find(type) != vDico.end())
				{
					if (vDico.at(type).find(NodeSlot::PlaceEnum::OUTPUT) != vDico.at(type).end())
					{
						res += vDico.at(type).at(NodeSlot::PlaceEnum::OUTPUT).include_slot;
					}
					else
					{
						CTOOL_DEBUG_BREAK;
					}
				}
				else
				{
					CTOOL_DEBUG_BREAK;
				}
			}
		}
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsInputIncludesInterfaces(const SlotDico& vDico)
{
	std::string res;

	for (const auto& inputSlot : m_Inputs)
	{
		if (inputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(inputSlot.second);
			if (slotDatasPtr)
			{
				BaseTypeEnum type = (BaseTypeEnum)slotDatasPtr->editorSlotTypeIndex;
				auto typeString = m_BaseTypes.m_TypeArray[slotDatasPtr->editorSlotTypeIndex];
				if (typeString == "None" ||
					typeString == "Custom")
				{
					continue;
				}

				if (vDico.find(type) != vDico.end())
				{
					if (vDico.at(type).find(NodeSlot::PlaceEnum::INPUT) != vDico.at(type).end())
					{
						res += vDico.at(type).at(NodeSlot::PlaceEnum::INPUT).include_interface;
					}
					else
					{
						CTOOL_DEBUG_BREAK;
					}
				}
				else
				{
					CTOOL_DEBUG_BREAK;
				}
			}
		}
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsOutputIncludesInterfaces(const SlotDico& vDico)
{
	std::string res;

	for (const auto& outputSlot : m_Outputs)
	{
		if (outputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(outputSlot.second);
			if (slotDatasPtr)
			{
				BaseTypeEnum type = (BaseTypeEnum)slotDatasPtr->editorSlotTypeIndex;
				auto typeString = m_BaseTypes.m_TypeArray[slotDatasPtr->editorSlotTypeIndex];
				if (typeString == "None" ||
					typeString == "Custom")
				{
					continue;
				}

				if (vDico.find(type) != vDico.end())
				{
					if (vDico.at(type).find(NodeSlot::PlaceEnum::OUTPUT) != vDico.at(type).end())
					{
						res += vDico.at(type).at(NodeSlot::PlaceEnum::OUTPUT).include_interface;
					}
					else
					{
						CTOOL_DEBUG_BREAK;
					}
				}
				else
				{
					CTOOL_DEBUG_BREAK;
				}
			}
		}
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsInputCppFuncs(const SlotDico& vDico)
{
	std::string res;

	for (const auto& inputSlot : m_Inputs)
	{
		if (inputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(inputSlot.second);
			if (slotDatasPtr)
			{
				BaseTypeEnum type = (BaseTypeEnum)slotDatasPtr->editorSlotTypeIndex;
				auto typeString = m_BaseTypes.m_TypeArray[slotDatasPtr->editorSlotTypeIndex];
				if (typeString == "None" ||
					typeString == "Custom")
				{
					continue;
				}

				if (vDico.find(type) != vDico.end())
				{
					if (vDico.at(type).find(NodeSlot::PlaceEnum::INPUT) != vDico.at(type).end())
					{
						res += vDico.at(type).at(NodeSlot::PlaceEnum::INPUT).cpp_func;
					}
					else
					{
						CTOOL_DEBUG_BREAK;
					}
				}
				else
				{
					CTOOL_DEBUG_BREAK;
				}
			}
		}
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsOutputCppFuncs(const SlotDico& vDico)
{
	std::string res;

	for (const auto& outputSlot : m_Outputs)
	{
		if (outputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(outputSlot.second);
			if (slotDatasPtr)
			{
				BaseTypeEnum type = (BaseTypeEnum)slotDatasPtr->editorSlotTypeIndex;
				auto typeString = m_BaseTypes.m_TypeArray[slotDatasPtr->editorSlotTypeIndex];
				if (typeString == "None" ||
					typeString == "Custom")
				{
					continue;
				}

				if (vDico.find(type) != vDico.end())
				{
					if (vDico.at(type).find(NodeSlot::PlaceEnum::OUTPUT) != vDico.at(type).end())
					{
						res += vDico.at(type).at(NodeSlot::PlaceEnum::OUTPUT).cpp_func;
					}
					else
					{
						CTOOL_DEBUG_BREAK;
					}
				}
				else
				{
					CTOOL_DEBUG_BREAK;
				}
			}
		}
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsInputHFuncs(const SlotDico& vDico)
{
	std::string res;

	res += u8R"(
	// Interfaces Setters
)";

	for (const auto& inputSlot : m_Inputs)
	{
		if (inputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(inputSlot.second);
			if (slotDatasPtr)
			{
				BaseTypeEnum type = (BaseTypeEnum)slotDatasPtr->editorSlotTypeIndex;
				auto typeString = m_BaseTypes.m_TypeArray[slotDatasPtr->editorSlotTypeIndex];
				if (typeString == "None" ||
					typeString == "Custom")
				{
					continue;
				}

				if (vDico.find(type) != vDico.end())
				{
					if (vDico.at(type).find(NodeSlot::PlaceEnum::INPUT) != vDico.at(type).end())
					{
						res += vDico.at(type).at(NodeSlot::PlaceEnum::INPUT).h_func;
					}
					else
					{
						CTOOL_DEBUG_BREAK;
					}
				}
				else
				{
					CTOOL_DEBUG_BREAK;
				}
			}
		}
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsOutputHFuncs(const SlotDico& vDico)
{
	std::string res;

	res += u8R"(
	// Interfaces Getters
)";

	for (const auto& outputSlot : m_Outputs)
	{
		if (outputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(outputSlot.second);
			if (slotDatasPtr)
			{
				BaseTypeEnum type = (BaseTypeEnum)slotDatasPtr->editorSlotTypeIndex;
				auto typeString = m_BaseTypes.m_TypeArray[slotDatasPtr->editorSlotTypeIndex];
				if (typeString == "None" ||
					typeString == "Custom")
				{
					continue;
				}

				if (vDico.find(type) != vDico.end())
				{
					if (vDico.at(type).find(NodeSlot::PlaceEnum::OUTPUT) != vDico.at(type).end())
					{
						res += vDico.at(type).at(NodeSlot::PlaceEnum::OUTPUT).h_func;
					}
					else
					{
						CTOOL_DEBUG_BREAK;
					}
				}
				else
				{
					CTOOL_DEBUG_BREAK;
				}
			}
		}
	}

	return res;
}
