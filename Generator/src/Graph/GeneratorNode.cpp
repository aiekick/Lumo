#include "GeneratorNode.h"

#include <filesystem>
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

namespace fs = std::filesystem;

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
		res += vOffset + ct::toStr("\t\t<pass_specialization_type>%s</pass_specialization_type>\n", m_RendererTypePixel2DSpecializationType.c_str());
		res += vOffset + ct::toStr("\t\t<pass_use_ubo>%s</pass_use_ubo>\n", m_UseAUbo ? "true" : "false");
		res += vOffset + ct::toStr("\t\t<pass_use_sbo>%s</pass_use_sbo>\n", m_UseASbo ? "true" : "false");
		res += vOffset + ct::toStr("\t\t<node_is_a_task>%s</node_is_a_task>\n", m_IsATask ? "true" : "false");
		
		res += vOffset + "\t</generation>\n";

		for (auto& editor : m_UBOEditors)
		{
			res += editor.getXml(vOffset + "\t", vUserDatas);
		}

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
				else if (attName == "showWidget")
					slot.showWidget = ct::ivariant(attValue).GetB();
				else if (attName == "typeIndex")
					slotDatas.editorSlotTypeIndex = ct::ivariant(attValue).GetU();
				else if (attName == "bindingIndex")
					slot.descriptorBinding = ct::ivariant(attValue).GetU();
			}

			if (slot.slotPlace == NodeSlot::PlaceEnum::INPUT)
			{
				auto slot_input_ptr = GeneratorNodeSlotInput::Create();
				slot_input_ptr->index = slot.index;
				slot_input_ptr->name = slot.name;
				slot_input_ptr->slotType = slot.slotType;
				slot_input_ptr->slotPlace = slot.slotPlace;
				slot_input_ptr->pinID = slot.pinID;
				slot_input_ptr->showWidget = slot.showWidget;
				slot_input_ptr->editorSlotTypeIndex = slotDatas.editorSlotTypeIndex;
				slot_input_ptr->descriptorBinding = slot.descriptorBinding;

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
				slot_output_ptr->showWidget = slot.showWidget;
				slot_output_ptr->editorSlotTypeIndex = slotDatas.editorSlotTypeIndex;
				slot_output_ptr->descriptorBinding = slot.descriptorBinding;

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

		if (strName == "UBO")
		{
			m_CurrentXMLEditor = UBOEditor();
			m_CurrentXMLEditor.setFromXml(vElem, vParent, vUserDatas);
			m_UBOEditors.push_back(m_CurrentXMLEditor);

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
		else if (strName == "pass_specialization_type")
			m_RendererTypePixel2DSpecializationType = strValue;
		else if (strName == "pass_use_ubo")
			m_UseAUbo = ct::ivariant(strValue).GetB();
		else if (strName == "pass_use_sbo")
			m_UseASbo = ct::ivariant(strValue).GetB();
		else if (strName == "node_is_a_task")
			m_IsATask = ct::ivariant(strValue).GetB();

		return true;
	}
	else if (strParentName == "UBO")
	{
		m_CurrentXMLEditor.setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// GENERATOR NODE //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorNode::GenerateNodeClasses(const std::string& vPath, const ProjectFile* vDatas)
{
	fs::path root_path = vPath;
	if (!std::filesystem::exists(vPath))
		return;

	fs::path nodes_path = vPath + "/Nodes/";
	if (!std::filesystem::exists(nodes_path))
		fs::create_directory(nodes_path);

	nodes_path = vPath + "/Nodes/" + m_CategoryName;
	if (!std::filesystem::exists(nodes_path))
		fs::create_directory(nodes_path);

	fs::path module_path = vPath + "/Modules/";
	if (!std::filesystem::exists(module_path))
		fs::create_directory(module_path);
	
	module_path = vPath + "/Modules/" + m_CategoryName;
	if (!std::filesystem::exists(module_path))
		fs::create_directory(module_path);

	std::string node_class_name = m_ClassName + "Node";
	std::string cpp_node_file_name = nodes_path.string() + "/" + node_class_name + ".cpp";
	std::string h_node_file_name = nodes_path.string() + "/" + node_class_name + ".h";

	std::string module_class_name = m_ClassName + "Module";
	std::string cpp_module_file_name = module_path.string() + "/" + module_class_name + ".cpp";
	std::string h_module_file_name = module_path.string() + "/" + module_class_name + ".h";

	std::string cpp_node_file_code;
	std::string h_node_file_code;
	std::string h_node_interfaces_code;

	auto slotDico = GetSlotDico();

	cpp_node_file_code += GetLicenceHeader();
	h_node_file_code += GetLicenceHeader();
	cpp_node_file_code += GetPVSStudioHeader();

	h_node_file_code += u8R"(
#include <Graph/Graph.h>
#include <Graph/Base/BaseNode.h>)";

	cpp_node_file_code += ct::toStr(u8R"(
#include "%s.h")", node_class_name.c_str());

	if (m_GenerateAModule)
	{
		cpp_node_file_code += ct::toStr(u8R"(
#include <Modules/%s/%s.h>)", m_CategoryName.c_str(), module_class_name.c_str());
	}

	cpp_node_file_code += GetNodeSlotsInputIncludesSlots(slotDico);
	cpp_node_file_code += GetNodeSlotsOutputIncludesSlots(slotDico);
	h_node_file_code += GetNodeSlotsInputIncludesInterfaces(slotDico);
	h_node_file_code += GetNodeSlotsOutputIncludesInterfaces(slotDico);
	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		h_node_file_code += u8R"(
#include <Interfaces/ShaderUpdateInterface.h>
)";
	}
	h_node_file_code += u8R"(
class MODULE_CLASS_NAME;
class NODE_CLASS_NAME :)";
	h_node_file_code += GetNodeSlotsInputPublicInterfaces(slotDico);
	h_node_file_code += GetNodeSlotsOutputPublicInterfaces(slotDico);
	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		h_node_file_code += u8R"(
	public ShaderUpdateInterface,)";
	}
	h_node_file_code += u8R"(
	public BaseNode)";

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

	name = "NODE_DISPLAY_NAME";)";

	cpp_node_file_code += GetNodeSlotsInputFuncs(slotDico);
	cpp_node_file_code += u8R"(
)";
	cpp_node_file_code += GetNodeSlotsOutputFuncs(slotDico);
	cpp_node_file_code += u8R"(
)";

	if (m_GenerateAModule)
	{
		cpp_node_file_code += u8R"(
	m_MODULE_CLASS_NAMEPtr = MODULE_CLASS_NAME::Create(vVulkanCorePtr, m_This);
	if (m_MODULE_CLASS_NAMEPtr)
	{
		res = true;
	})";
	}

	cpp_node_file_code += u8R"(

	return res;
}
)";

	if (m_IsATask)
	{
		cpp_node_file_code += u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TASK EXECUTE ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool NODE_CLASS_NAME::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	bool res = false;

	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);
)";

		if (m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_Texture])
		{
			cpp_node_file_code += u8R"(
	// for update input texture buffer infos => avoid vk crash
	UpdateTextureInputDescriptorImageInfos(m_Inputs);)";
		}
		/*else if (m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_TextureCube])
		{
			cpp_node_file_code += u8R"(
		// for update input texture buffer infos => avoid vk crash
		UpdateTextureCubeInputDescriptorImageInfos(m_Inputs);)";
		}*/

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
})";
	}

	cpp_node_file_code += u8R"(
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
)";
	if (m_ShowInputWidgets || m_ShowOutputWidgets)
	{
		cpp_node_file_code += u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW SLOTS WIDGET ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
)";
	}

	if (m_ShowInputWidgets)
	{
		cpp_node_file_code += u8R"(
void NODE_CLASS_NAME::DrawInputWidget(BaseNodeState* vBaseNodeState, NodeSlotWeak vSlot)
{
	auto slotPtr = vSlot.getValidShared();
	if (slotPtr && slotPtr->showWidget)
	{
		if (m_MODULE_CLASS_NAMEPtr)
		{
			//m_MODULE_CLASS_NAMEPtr->DrawTexture(50);
		}
	}
}
)";
	}

	if (m_ShowOutputWidgets)
	{
		cpp_node_file_code += u8R"(
void NODE_CLASS_NAME::DrawOutputWidget(BaseNodeState* vBaseNodeState, NodeSlotWeak vSlot)
{
	auto slotPtr = vSlot.getValidShared();
	if (slotPtr && slotPtr->showWidget)
	{
		if (m_MODULE_CLASS_NAMEPtr)
		{
			//m_MODULE_CLASS_NAMEPtr->DrawTexture(50);
		}
	}
}
)";
	}

	cpp_node_file_code += u8R"(
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
})";

	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		cpp_node_file_code += u8R"(

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
	}

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

void NODE_CLASS_NAME::AfterNodeXmlLoading()
{)";
	if (m_GenerateAModule)
	{
		cpp_node_file_code += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->AfterNodeXmlLoading();
	})";
	}

	cpp_node_file_code += u8R"(
}
)";

	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		cpp_node_file_code += u8R"(
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
	}

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
)";
	if (m_IsATask)
	{
		h_node_file_code += u8R"(
	// Execute Task
	bool ExecuteAllTime(const uint32_t & vCurrentFrame, vk::CommandBuffer * vCmd = nullptr, BaseNodeState * vBaseNodeState = nullptr) override;
)";
	}
	
	h_node_file_code += u8R"(
	// Draw Widgets
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) override;)";
	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		h_node_file_code += u8R"(

	// Resize
	void NeedResizeByResizeEvent(ct::ivec2 * vNewSize, const uint32_t * vCountColorBuffers) override;
)";
	}

	h_node_file_code += GetNodeSlotsInputHFuncs(slotDico);
	h_node_file_code += u8R"(
)";
	h_node_file_code += GetNodeSlotsOutputHFuncs(slotDico);
	h_node_file_code += u8R"(
)";
	if (m_ShowInputWidgets || m_ShowOutputWidgets)
	{
		h_node_file_code += u8R"(
	// Input / Ouput slot widgets)";
	}

	if (m_ShowInputWidgets)
	{
		h_node_file_code += u8R"(
	void DrawInputWidget(BaseNodeState* vBaseNodeState, NodeSlotWeak vSlot) override;)";
	}

	if (m_ShowOutputWidgets)
	{
		h_node_file_code += u8R"(
	void DrawOutputWidget(BaseNodeState* vBaseNodeState, NodeSlotWeak vSlot) override;)";
	}

	if (m_ShowInputWidgets || m_ShowOutputWidgets)
	{
		h_node_file_code += u8R"(
)";
	}

	h_node_file_code += u8R"(
	// Configuration
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
	void AfterNodeXmlLoading() override;
)";
	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		h_node_file_code += u8R"(
	// Shader Update
	void UpdateShaders(const std::set<std::string>& vFiles) override;
)";
	}
	h_node_file_code += u8R"(
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
		GenerateModules(vPath, vDatas, slotDico);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// GENERATOR MODULLE ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorNode::GenerateModules(const std::string& vPath, const ProjectFile* vDatas, const SlotDico& vDico)
{
	fs::path module_path = vPath + "/Modules/" + m_CategoryName;
	if (!std::filesystem::exists(module_path))
		fs::create_directory(module_path);

	std::string module_class_name = m_ClassName + "Module";
	std::string cpp_module_file_name = module_path.string() + "/" + module_class_name + ".cpp";
	std::string h_module_file_name = module_path.string() + "/" + module_class_name + ".h";

	std::string pass_renderer_display_type = GetRendererDisplayName();
	std::string pass_class_name = m_ClassName + "Module_" + pass_renderer_display_type + "Pass";
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
#include <Graph/Base/BaseNode.h>
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

std::shared_ptr<MODULE_CLASS_NAME> MODULE_CLASS_NAME::Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<MODULE_CLASS_NAME>(vVulkanCorePtr);
	res->SetParentNode(vParentNode);
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

MODULE_CLASS_NAME::MODULE_CLASS_NAME(vkApi::VulkanCorePtr vVulkanCorePtr))";
	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		cpp_module_file_code += u8R"(
	: BaseRenderer(vVulkanCorePtr))";
	}
	if (!m_GenerateAPass)
	{
		if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
		{
			cpp_module_file_code += u8R"(,)";
		}
		cpp_module_file_code += u8R"(
	m_VulkanCorePtr(vVulkanCorePtr))";
	}
		cpp_module_file_code += u8R"(
{
	ZoneScoped;)";
	cpp_module_file_code += u8R"(
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
)";
	if (m_GenerateAPass)
	{
		cpp_module_file_code += u8R"(
	m_Loaded = false;
)";
	}

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

	if (m_GenerateAPass)
	{
		cpp_module_file_code += u8R"(
	return m_Loaded;
}
)";
	}
	else
	{
		cpp_module_file_code += u8R"(
	return true;
}
)";
	}

	if (!m_GenerateAPass && m_RendererType == RENDERER_TYPE_NONE)
	{
		cpp_module_file_code += u8R"(
void MODULE_CLASS_NAME::Unit()
{
	ZoneScoped;
}
)";
	}

	if (m_IsATask)
	{
		cpp_module_file_code += u8R"(
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
})";
	}

	cpp_module_file_code += u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool MODULE_CLASS_NAME::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
)";
	if (m_IsATask)
	{
		cpp_module_file_code += u8R"(
	if (m_LastExecutedFrame == vCurrentFrame)
	{)";
	}
	
	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		cpp_module_file_code += u8R"(
		if (ImGui::CollapsingHeader_CheckBox("MODULE_DISPLAY_NAME##MODULE_CLASS_NAME", -1.0f, true, true, &m_CanWeRender))
		{
			bool change = false;
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
)";
	}

	if (m_IsATask)
	{
		cpp_module_file_code += u8R"(
	}
)";
	}
	cpp_module_file_code += u8R"(
	return false;
}

void MODULE_CLASS_NAME::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
)";
	if (m_IsATask)
	{
		cpp_module_file_code += u8R"(
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
)";
	}
	cpp_module_file_code += u8R"(
}

void MODULE_CLASS_NAME::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
)";
	if (m_IsATask)
	{
		cpp_module_file_code += u8R"(
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
)";
	}
	cpp_module_file_code += u8R"(
}
)";

	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{

		cpp_module_file_code += u8R"(
void MODULE_CLASS_NAME::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	// do some code
	
	BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}
)";
	}

	cpp_module_file_code += GetModuleInputCppFuncs(vDico);
	cpp_module_file_code += GetModuleOutputCppFuncs(vDico);
	cpp_module_file_code += u8R"(
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MODULE_CLASS_NAME::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += vOffset + "<MODULE_XML_NAME>\n";
)";
	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		cpp_module_file_code += u8R"(
	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";
)";
	}

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
	{)";
	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		cpp_module_file_code += u8R"(
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();)";
	}
	cpp_module_file_code += u8R"(
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
	}

	return true;
}

void MODULE_CLASS_NAME::AfterNodeXmlLoading()
{)";
	if (m_GenerateAModule)
	{
		cpp_module_file_code += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		m_PASS_CLASS_NAME_Ptr->AfterNodeXmlLoading();
	})";
	}

	cpp_module_file_code += u8R"(
}
)";

	/////////////////////////////////////////////////////////////////
	////// HEADER ///////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
	h_module_file_code += u8R"(

#pragma once

#include <set>
#include <array>
#include <string>
#include <memory>

#include <Headers/Globals.h>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <Base/BaseRenderer.h>
#include <Base/QuadShaderPass.h>

#include <vulkan/vulkan.hpp>
#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanDevice.h>
#include <vkFramework/vk_mem_alloc.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/ImGuiTexture.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/VulkanFrameBuffer.h>
)";

	h_module_file_code += u8R"(
#include <Interfaces/GuiInterface.h>
#include <Interfaces/NodeInterface.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/NodeInterface.h>
#include <Interfaces/ResizerInterface.h>
)";
	h_module_file_code += GetNodeSlotsInputIncludesInterfaces(vDico);
	h_module_file_code += GetNodeSlotsOutputIncludesInterfaces(vDico);
	h_module_file_code += u8R"(
)";
	if (m_GenerateAPass)
	{
		h_module_file_code += u8R"(
class PASS_CLASS_NAME;)";
	}

	h_module_file_code += u8R"(
class MODULE_CLASS_NAME :
	public NodeInterface,)";
	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		h_module_file_code += u8R"(
	public BaseRenderer,
	public ResizerInterface,)";
	}
	else
	{
		h_module_file_code += u8R"(
	public conf::ConfigAbstract,)";
	}

	if (m_IsATask)
	{
		h_module_file_code += u8R"(
	public TaskInterface,)";
	}

	h_module_file_code += GetNodeSlotsInputPublicInterfaces(vDico);
	h_module_file_code += GetNodeSlotsOutputPublicInterfaces(vDico);

	h_module_file_code += u8R"(
	public GuiInterface
{
public:
	static std::shared_ptr<MODULE_CLASS_NAME> Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode);

private:
	ct::cWeak<MODULE_CLASS_NAME> m_This;
)";
	if (!m_GenerateAPass)
	{
		h_module_file_code += u8R"(\tvkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;
)";
	}

	if (m_GenerateAPass)
	{
		h_module_file_code += u8R"(
	std::shared_ptr<PASS_CLASS_NAME> m_PASS_CLASS_NAME_Ptr = nullptr;
)";
	}

	h_module_file_code += u8R"(
public:
	MODULE_CLASS_NAME(vkApi::VulkanCorePtr vVulkanCorePtr);
	~MODULE_CLASS_NAME())";
	if (m_GenerateAPass)
	{
		h_module_file_code += u8R"( override;
)";
	}
	else
	{
		h_module_file_code += u8R"(;
)";
	}
	h_module_file_code += u8R"(
	bool Init();)";

	if (!m_GenerateAPass)
	{
		h_module_file_code += u8R"(
	void Unit();)";
	}
	h_module_file_code += u8R"(
)";

	if (m_IsATask)
	{
		h_module_file_code += u8R"(
	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
	bool ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
)";
	}
		h_module_file_code += u8R"(
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
)";
	if (m_GenerateAPass && m_RendererType != RENDERER_TYPE_NONE)
	{
		h_module_file_code += u8R"(
	void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
)";
	}

	h_module_file_code += GetNodeSlotsInputHFuncs(vDico);
	h_module_file_code += GetNodeSlotsOutputHFuncs(vDico);
	h_module_file_code += u8R"(
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
	void AfterNodeXmlLoading() override;
};
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

	if (m_GenerateAPass)
	{
		GeneratePasses(vPath, vDatas, vDico);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// GENERATOR PASS //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void GeneratorNode::GeneratePasses(const std::string& vPath, const ProjectFile* vDatas, const SlotDico& vDico)
{
	fs::path path_path = vPath + "/Modules/" + m_CategoryName + "/Pass/";
	if (!std::filesystem::exists(path_path))
		fs::create_directory(path_path);

	std::string pass_renderer_display_type = GetRendererDisplayName();
	std::string pass_class_name = m_ClassName + "Module_" + pass_renderer_display_type + "Pass";
	std::string cpp_pass_file_name = path_path.string() + "/" + pass_class_name + ".cpp";
	std::string h_pass_file_name = path_path.string() + "/" + pass_class_name + ".h";

	std::string cpp_pass_file_code;
	std::string h_pass_file_code;

	cpp_pass_file_code += GetLicenceHeader();
	h_pass_file_code += GetLicenceHeader();
	cpp_pass_file_code += GetPVSStudioHeader();

	// MODULE_XML_NAME							grayscott_module_sim
	// MODULE_DISPLAY_NAME						Gray Scott
	// MODULE_CATEGORY_NAME						Simulation
	// MODULE_CLASS_NAME		
	// PASS_CLASS_NAME
	// MODULE_RENDERER_INIT_FUNC				InitCompute2D
	// RENDERER_DISPLAY_TYPE (ex (Comp)			Comp

	cpp_pass_file_code += u8R"(
#include "PASS_CLASS_NAME.h"

#include <cinttypes>
#include <functional>
#include <Gui/MainFrame.h>
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

using namespace vkApi;

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

PASS_CLASS_NAME::PASS_CLASS_NAME(vkApi::VulkanCorePtr vVulkanCorePtr))";

	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD)
		{
			cpp_pass_file_code += u8R"(
	: QuadShaderPass(vVulkanCorePtr, MeshShaderPassType::PIXEL))";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH)
		{
			cpp_pass_file_code += u8R"(
	: MeshShaderPass(vVulkanCorePtr, MeshShaderPassType::PIXEL))";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX)
		{
			cpp_pass_file_code += u8R"(
	: VertexShaderPass(vVulkanCorePtr))";
		}
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D)
	{
		cpp_pass_file_code += u8R"(
	: ShaderPass(vVulkanCorePtr))";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_2D)
	{
		cpp_pass_file_code += u8R"(
	: ShaderPass(vVulkanCorePtr))";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_3D)
	{
		cpp_pass_file_code += u8R"(
	: ShaderPass(vVulkanCorePtr))";
	}
	else if (m_RendererType == RENDERER_TYPE_RTX)
	{
		cpp_pass_file_code += u8R"(
	: RtxShaderPass(vVulkanCorePtr))";
	}

	cpp_pass_file_code += u8R"(
{)";

	cpp_pass_file_code += u8R"(
	SetRenderDocDebugName("RENDERER_DISPLAY_TYPE Pass : MODULE_DISPLAY_NAME", COMPUTE_SHADER_PASS_DEBUG_COLOR);
)";

	cpp_pass_file_code += u8R"(
	//m_DontUseShaderFilesOnDisk = true;
}

PASS_CLASS_NAME::~PASS_CLASS_NAME()
{
	Unit();
}

void PASS_CLASS_NAME::ActionBeforeInit()
{
	//m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);
)";

	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX)
		{
			cpp_pass_file_code += u8R"(
	m_PrimitiveTopology = vk::PrimitiveTopology::eLineList; // display lines
	m_LineWidth.x = 0.5f;	// min value
	m_LineWidth.y = 10.0f;	// max value
	m_LineWidth.z = 2.0f;	// default value)";
		}
	}

	if (m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_StorageBuffer])
	{
		cpp_pass_file_code += u8R"(
	for (auto& info : m_StorageBuffers)
	{
		info = m_VulkanCorePtr->getEmptyDescriptorBufferInfo();
	})";
	}
	else if (m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_TexelBuffer])
	{
		cpp_pass_file_code += u8R"(
	for (auto& info : m_TexelBuffers)
	{
		info = nullptr;
	}
	for (auto& info : m_TexelBufferViews)
	{
		info = m_VulkanCorePtr->getEmptyBufferView();
	})";
	}
	else if (m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_Texture])
	{
		cpp_pass_file_code += u8R"(
	for (auto& info : m_ImageInfos)
	{
		info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
	})";
	}
	else if (m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_TextureCube])
	{
		cpp_pass_file_code += u8R"(
	for (auto& info : m_ImageCubeInfos)
	{
		info = m_VulkanCorePtr->getEmptyTextureCubeDescriptorImageInfo();
	})";
	}
	else if (m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_TextureGroup])
	{
		cpp_pass_file_code += u8R"(
	for (auto& infos : m_ImageGroups)
	{
		for(auto& info : infos)
		{
			info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
		}
	})";
	}

	cpp_pass_file_code += u8R"(
}

bool PASS_CLASS_NAME::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	ZoneScoped;

	bool change = false;

	//change |= DrawResizeWidget();
)";
	if (m_UseAUbo)
	{
		for (auto& editor : m_UBOEditors)
		{
			cpp_pass_file_code += editor.GetUBOCode_Widgets();
		}
	}

	cpp_pass_file_code += u8R"(
	if (change)
	{
		//NeedNewUBOUpload();
		//NeedNewSBOUpload();
	}

	return change;
}

void PASS_CLASS_NAME::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	ZoneScoped;
}

void PASS_CLASS_NAME::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	ZoneScoped;
})";

	cpp_pass_file_code += GetPassInputCppFuncs(vDico);
	cpp_pass_file_code += GetPassOutputCppFuncs(vDico);

	cpp_pass_file_code += u8R"(

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PASS_CLASS_NAME::WasJustResized()
{
	ZoneScoped;
}
)";
	cpp_pass_file_code += GetPassRendererFunctionHeader();

	if (m_UseAUbo)
	{
		cpp_pass_file_code += u8R"(

bool PASS_CLASS_NAME::CreateUBO()
{
	ZoneScoped;
)";

		for (auto& editor : m_UBOEditors)
		{
			cpp_pass_file_code += editor.GetFunction_CreateUBO();
		}

		cpp_pass_file_code += u8R"(

	NeedNewUBOUpload();

	return true;
})";

		cpp_pass_file_code += u8R"(

bool PASS_CLASS_NAME::UploadUBO()
{
	ZoneScoped;
)";

		for (auto& editor : m_UBOEditors)
		{
			cpp_pass_file_code += editor.GetFunction_UploadUBO();
		}

		cpp_pass_file_code += u8R"(
})";

		cpp_pass_file_code += u8R"(

bool PASS_CLASS_NAME::DestroyUBO()
{
	ZoneScoped;)";

		for (auto& editor : m_UBOEditors)
		{
			cpp_pass_file_code += editor.GetFunction_DestroyUBO();
		}
		
		cpp_pass_file_code += u8R"(
})";
	}

	if (m_UseASbo)
	{
		cpp_pass_file_code += u8R"(
bool PASS_CLASS_NAME::CreateSBO()
{
	ZoneScoped;

	NeedNewSBOUpload();

	return true;
}

void PASS_CLASS_NAME::UploadSBO()
{
	ZoneScoped;

	//VulkanRessource::upload(m_VulkanCorePtr, m_UBOCompPtr, &m_UBOComp, sizeof(UBOComp));
}

void PASS_CLASS_NAME::DestroySBO()
{
	ZoneScoped;

	//m_SBOCompPtr.reset();
	//m_SBO_Comp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
}
)";
	}

	cpp_pass_file_code += u8R"(
bool PASS_CLASS_NAME::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_DescriptorSets[0].m_LayoutBindings.clear();)";

	cpp_pass_file_code += GetPassUpdateLayoutBindingInRessourceDescriptorHeader();
	cpp_pass_file_code += u8R"(

	return true;
}

bool PASS_CLASS_NAME::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	m_DescriptorSets[0].m_WriteDescriptorSets.clear();

	assert(m_ComputeBufferPtr);)";

	cpp_pass_file_code += GetPassUpdateBufferInfoInRessourceDescriptorHeader();
	cpp_pass_file_code += u8R"(
	
	return true;
})";

	cpp_pass_file_code += GetPassShaderCode();

	cpp_pass_file_code += u8R"(

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string PASS_CLASS_NAME::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += ShaderPass::getXml(vOffset, vUserDatas);
	//str += vOffset + "<mouse_radius>" + ct::toStr(m_UBOComp.mouse_radius) + "</mouse_radius>\n";
	
	return str;
}

bool PASS_CLASS_NAME::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	ShaderPass::setFromXml(vElem, vParent, vUserDatas);

	if (strParentName == "MODULE_XML_NAME")
	{
		//if (strName == "mouse_radius")
		//	m_UBOComp.mouse_radius = ct::fvariant(strValue).GetF();
	}

	return true;
}

void PASS_CLASS_NAME::AfterNodeXmlLoading()
{
	// code to do after end of the wml loading of this node
}
)";

	h_pass_file_code += u8R"(
#pragma once

#include <set>
#include <array>
#include <string>
#include <memory>

#include <Headers/Globals.h>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <Base/BaseRenderer.h>
)";

	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD)
		{
			h_pass_file_code += u8R"(#include <Base/QuadShaderPass.h>)";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH)
		{
			h_pass_file_code += u8R"(#include <Base/MeshShaderPass.h>)";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX)
		{
			h_pass_file_code += u8R"(#include <Base/VertexShaderPass.h>)";
		}
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D)
	{
		h_pass_file_code += u8R"(#include <Base/ShaderPass.h>)";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_2D)
	{
		h_pass_file_code += u8R"(#include <Base/ShaderPass.h>)";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_3D)
	{
		h_pass_file_code += u8R"(#include <Base/ShaderPass.h>)";
	}
	else if (m_RendererType == RENDERER_TYPE_RTX)
	{
		h_pass_file_code += u8R"(#include <Base/RtxShaderPass.h>)";
	}

	h_pass_file_code += u8R"(
#include <vulkan/vulkan.hpp>
#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanDevice.h>
#include <vkFramework/vk_mem_alloc.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/ImGuiTexture.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/VulkanFrameBuffer.h>

#include <Interfaces/GuiInterface.h>
#include <Interfaces/NodeInterface.h>)";
	h_pass_file_code += GetNodeSlotsInputIncludesInterfaces(vDico);
	h_pass_file_code += GetNodeSlotsOutputIncludesInterfaces(vDico);
	h_pass_file_code += u8R"(

class PASS_CLASS_NAME :)";

	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD)
		{
			h_pass_file_code += u8R"(
	public QuadShaderPass,)";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH)
		{
			h_pass_file_code += u8R"(
	public MeshShaderPass,)";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX)
		{
			h_pass_file_code += u8R"(
	public VertexShaderPass,)";
		}
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D)
	{
		h_pass_file_code += u8R"(
	public ShaderPass,)";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_2D)
	{
		h_pass_file_code += u8R"(
	public ShaderPass,)";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_3D)
	{
		h_pass_file_code += u8R"(
	public ShaderPass,)";
	}
	else if (m_RendererType == RENDERER_TYPE_RTX)
	{
		h_pass_file_code += u8R"(
	public RtxShaderPass,)";
	}

	h_pass_file_code += GetPassInputPublicInterfaces(vDico);
	h_pass_file_code += GetNodeSlotsOutputPublicInterfaces(vDico);

	h_pass_file_code += u8R"(
	public GuiInterface,
	public NodeInterface
{
private:)";

	if (m_UseAUbo)
	{
		for (auto& editor : m_UBOEditors)
		{
			h_pass_file_code += editor.GetUBOHeader();
		}
	}

	h_pass_file_code += u8R"(

public:
	PASS_CLASS_NAME(vkApi::VulkanCorePtr vVulkanCorePtr);
	~PASS_CLASS_NAME() override;

	void ActionBeforeInit() override;
	void WasJustResized() override;
)";
	
	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD)
		{
			// nothing because derived
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH)
		{
			h_pass_file_code += u8R"(
		void DrawModel(vk::CommandBuffer * vCmdBuffer, const int& vIterationNumber) override;)";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX)
		{
			// nothing because derived
		}
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D ||
		m_RendererType == RENDERER_TYPE_COMPUTE_2D ||
		m_RendererType == RENDERER_TYPE_COMPUTE_3D)
	{
		h_pass_file_code += u8R"(
		void Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber) override;)";
	}
	else if (m_RendererType == RENDERER_TYPE_RTX)
	{
		// nothing because derived
	}
	
	h_pass_file_code += u8R"(
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
)";

	h_pass_file_code += GetNodeSlotsInputHFuncs(vDico);
	h_pass_file_code += GetNodeSlotsOutputHFuncs(vDico);

	h_pass_file_code += u8R"(

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
	void AfterNodeXmlLoading() override;

protected:)";

	if (m_UseAUbo)
	{
		h_pass_file_code += u8R"(
	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;
)";
	}

	if (m_UseASbo)
	{
		h_pass_file_code += u8R"(
	bool CreateSBO() override;
	void UploadSBO() override;
	void DestroySBO() override;
)";
	}

	h_pass_file_code += u8R"(
	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;
)";


	h_pass_file_code += GetPassShaderHeader();

	h_pass_file_code += u8R"(
};)";

	ct::replaceString(cpp_pass_file_code, "MODULE_XML_NAME", m_ModuleXmlName);
	ct::replaceString(h_pass_file_code, "MODULE_XML_NAME", m_ModuleXmlName);

	ct::replaceString(cpp_pass_file_code, "MODULE_DISPLAY_NAME", m_ModuleDisplayName);
	ct::replaceString(h_pass_file_code, "MODULE_DISPLAY_NAME", m_ModuleDisplayName);

	ct::replaceString(cpp_pass_file_code, "MODULE_CATEGORY_NAME", m_CategoryName);
	ct::replaceString(h_pass_file_code, "MODULE_CATEGORY_NAME", m_CategoryName);

	ct::replaceString(cpp_pass_file_code, "NODE_CLASS_NAME", pass_class_name);
	ct::replaceString(h_pass_file_code, "NODE_CLASS_NAME", pass_class_name);

	ct::replaceString(cpp_pass_file_code, "PASS_CLASS_NAME", pass_class_name);
	ct::replaceString(h_pass_file_code, "PASS_CLASS_NAME", pass_class_name);

	ct::replaceString(cpp_pass_file_code, "MODULE_RENDERER_INIT_FUNC", m_ModuleRendererInitFunc);
	ct::replaceString(h_pass_file_code, "MODULE_RENDERER_INIT_FUNC", m_ModuleRendererInitFunc);

	ct::replaceString(cpp_pass_file_code, "RENDERER_DISPLAY_TYPE", m_ModuleRendererDisplayType);
	ct::replaceString(h_pass_file_code, "RENDERER_DISPLAY_TYPE", m_ModuleRendererDisplayType);

	FileHelper::Instance()->SaveStringToFile(cpp_pass_file_code, cpp_pass_file_name);
	FileHelper::Instance()->SaveStringToFile(h_pass_file_code, h_pass_file_name);
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
		if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD)
		{
			res = "Quad_";
			m_ModuleRendererDisplayType = "Quad";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH)
		{
			res = "Mesh_";
			m_ModuleRendererDisplayType = "Mesh";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX)
		{
			res = "Vertex_";
			m_ModuleRendererDisplayType = "Vertex";
		}
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D)
	{
		res = "Comp_1D_";
		m_ModuleRendererDisplayType = "Comp";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_2D)
	{
		res = "Comp_2D_";
		m_ModuleRendererDisplayType = "Comp";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_3D)
	{
		res = "Comp_3D_";
		m_ModuleRendererDisplayType = "Comp";
	}
	else if (m_RendererType == RENDERER_TYPE_RTX)
	{
		res = "Rtx_";
		m_ModuleRendererDisplayType = "Rtx";
	}

	return res;
}

std::string GeneratorNode::GetPassRendererFunctionHeader()
{
	std::string res;

	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD)
		{
			// nothing because derived
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH)
		{
			res += u8R"(
void PASS_CLASS_NAME::DrawModel(vk::CommandBuffer * vCmdBuffer, const int& vIterationNumber)
{
	ZoneScoped;

	if (!m_Loaded) return;

	if (vCmdBuffer)
	{
		auto modelPtr = m_SceneModel.getValidShared();
		if (!modelPtr || modelPtr->empty()) return;

		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipelines[0].m_Pipeline);
		{
			VKFPScoped(*vCmdBuffer, "MODULE_DISPLAY_NAME", "DrawModel");

			vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);

			for (auto meshPtr : *modelPtr)
			{
				if (meshPtr)
				{
					vk::DeviceSize offsets = 0;
					vCmdBuffer->bindVertexBuffers(0, meshPtr->GetVerticesBuffer(), offsets);

					if (meshPtr->GetIndicesCount())
					{
						vCmdBuffer->bindIndexBuffer(meshPtr->GetIndicesBuffer(), 0, vk::IndexType::eUint32);
						vCmdBuffer->drawIndexed(meshPtr->GetIndicesCount(), 1, 0, 0, 0);
					}
					else
					{
						vCmdBuffer->draw(meshPtr->GetVerticesCount(), 1, 0, 0);
					}
				}
			}
		}
	}
}
)";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX)
		{
			// nothing because derived
		}
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D ||
		m_RendererType == RENDERER_TYPE_COMPUTE_2D ||
		m_RendererType == RENDERER_TYPE_COMPUTE_3D)
	{
		res += u8R"(
void PASS_CLASS_NAME::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
		{
			VKFPScoped(*vCmdBuffer, "MODULE_DISPLAY_NAME", "Compute");

			vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);

			for (uint32_t iter = 0; iter < m_CountIterations.w; iter++)
			{
				Dispatch(vCmdBuffer);
			}
		}
	}
}
)";
	}
	else if (m_RendererType == RENDERER_TYPE_RTX)
	{
		// nothing because derived
	}

	return res;
}

std::string GeneratorNode::GetPassUpdateLayoutBindingInRessourceDescriptorHeader()
{
	std::string res;

	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD)
		{
			res += u8R"()";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH)
		{
			res += u8R"()";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX)
		{
			res += u8R"()";
		}
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D)
	{
		res += u8R"()";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_2D)
	{
		res += u8R"(
	m_DescriptorSets[0].m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute);)";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_3D)
	{
		res += u8R"()";
	}
	else if (m_RendererType == RENDERER_TYPE_RTX)
	{
		res += u8R"(
	m_DescriptorSets[0].m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eStorageImage, 1, 
		vk::ShaderStageFlagBits::eRaygenKHR); // output
	m_DescriptorSets[0].m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eAccelerationStructureKHR, 1, 
		vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR); // accel struct
	m_DescriptorSets[0].m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eUniformBuffer, 1, 
		vk::ShaderStageFlagBits::eRaygenKHR); // camera
	m_DescriptorSets[0].m_LayoutBindings.emplace_back(3U, vk::DescriptorType::eStorageBuffer, 1,
		vk::ShaderStageFlagBits::eClosestHitKHR); // model device address
	m_DescriptorSets[0].m_LayoutBindings.emplace_back(4U, vk::DescriptorType::eStorageBuffer, 1, 
		vk::ShaderStageFlagBits::eClosestHitKHR);)";
	}

	return res;
}

std::string GeneratorNode::GetPassUpdateBufferInfoInRessourceDescriptorHeader()
{
	std::string res;

	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD)
		{
			res += u8R"()";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH)
		{
			res += u8R"()";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX)
		{
			res += u8R"()";
		}
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D)
	{
		res += u8R"()";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_2D)
	{
		res += u8R"(
	m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eStorageImage, 
		m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U), nullptr); // output
)";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_3D)
	{
		res += u8R"()";
	}
	else if (m_RendererType == RENDERER_TYPE_RTX)
	{
		res += u8R"(
	auto accelStructurePtr = m_SceneAccelStructure.getValidShared();
	if (accelStructurePtr && 
		accelStructurePtr->GetTLASInfo() && 
		accelStructurePtr->GetBufferAddressInfo())
	{
		m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eStorageImage,
			m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U), nullptr); // output
		// The acceleration structure descriptor has to be chained via pNext
		m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eAccelerationStructureKHR,
			nullptr, nullptr, nullptr, accelStructurePtr->GetTLASInfo()); // accel struct
		m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eUniformBuffer,
			nullptr, CommonSystem::Instance()->GetBufferInfo()); // camera
		m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 3U, 0, 1, vk::DescriptorType::eStorageBuffer,
			nullptr, accelStructurePtr->GetBufferAddressInfo()); // model device address
		m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 4U, 0, 1, vk::DescriptorType::eStorageBuffer, 
			nullptr, m_SceneLightGroupDescriptorInfoPtr);

		return true; // pas de maj si pas de structure acceleratrice
	})";
	}

	return res;
}

std::string GeneratorNode::GetPassShaderCode()
{
	std::string res;

	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD)
		{
			res += u8R"(
std::string PASS_CLASS_NAME::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PASS_CLASS_NAME_Vertex";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 vertPosition;
layout(location = 1) in vec2 vertUv;
layout(location = 0) out vec2 v_uv;

void main() 
{
	v_uv = vertUv;
	gl_Position = vec4(vertPosition, 0.0, 1.0);
}
))"; res += u8R"(";
}

std::string PASS_CLASS_NAME::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PASS_CLASS_NAME_Fragment";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 v_uv;

layout (std140, binding = 0) uniform UBO_Frag 
{ 
	//float u_param_0;
};

void main() 
{
	fragColor = vec4(0);
}
))"; res += u8R"(";
})";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH)
		{
			res += u8R"(
std::string PASS_CLASS_NAME::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PASS_CLASS_NAME_Vertex";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBiTangent;
layout(location = 4) in vec2 aUv;
layout(location = 5) in vec4 aColor;

layout(location = 0) out vec4 vertColor;
))"; res += u8R"(
+ CommonSystem::GetBufferObjectStructureHeader(0U) +
u8R"(
layout (std140, binding = 1) uniform UBO_Vert 
{ 
	mat4 transform;
};

void main() 
{
	vertColor = aColor;
	gl_Position = cam * transform * vec4(aPosition, 1.0);
}
))"; res += u8R"(";
}

std::string PASS_CLASS_NAME::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PASS_CLASS_NAME_Fragment";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec4 vertColor;
)"; res += u8R"(
+ CommonSystem::GetBufferObjectStructureHeader(0U) +
u8R"(
layout(std140, binding = 2) uniform UBO_Frag 
{ 
	//uint channel_idx;	// 0..3
	//uint color_count;	// 0..N
};

void main() 
{
	fragColor = vec4(0);
}
))"; res += u8R"(";
})";
		}
		else if (m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX)
		{
			res += u8R"(
std::string PASS_CLASS_NAME::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PASS_CLASS_NAME_Vertex";

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 vertColor;
) "
+ CommonSystem::GetBufferObjectStructureHeader(0U) +
u8R"(
layout(std140, binding = 1) uniform UBOStruct {
	
	float axisSize;
};

void main() 
{
	float vertexId = float(gl_VertexIndex);
	
	float astep = 3.14159 * 2.0 / 70.;
	float a = astep * float(vertexId) * (uTime+10.) * .3;
	vec2 d = a  * vec2(cos(a), sin(a)) / 100.;
	d.x *= screenSize.y/screenSize.x;
	if (vertexId<150.) gl_Position = cam * vec4(d,0,1);
	else gl_Position = cam * vec4(0,0,0,1);
	vertColor = vec4(1,1,1,1);
))"; res += u8R"(";
		}

		std::string PASS_CLASS_NAME::GetFragmentShaderCode(std::string& vOutShaderName)
		{
			vOutShaderName = "PASS_CLASS_NAME_Fragment";

			return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec4 vertColor;

void main() 
{
	fragColor = vertColor; 
}
))";
	res += u8R"(";
		})";
		}
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D)
	{
		res += u8R"()";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_2D)
	{
		res += u8R"(
std::string PASS_CLASS_NAME::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PASS_CLASS_NAME_Compute";

	SetLocalGroupSize(ct::uvec3(1U, 1U, 1U));

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform image2D colorBuffer;

layout(std140, binding = 0) uniform UBO_Comp
{
	float mouse_radius;
	ivec2 image_size;
};

layout(binding = 1) uniform sampler2D input_mask;

void main()
{
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

	vec4 color = vec4(coords, 0, 1);

	imageStore(colorBuffer, coords, color); 
}
))";
		res += u8R"(";
		})";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_3D)
	{
		res += u8R"()";
	}
	else if (m_RendererType == RENDERER_TYPE_RTX)
	{
	res += u8R"(
std::string PbrRenderer_Rtx_Pass::GetRayGenerationShaderCode(std::string& vOutShaderName)
{
	ZoneScoped;

	vOutShaderName = "PbrRenderer_Rtx_Pass";
	return u8R"(
#version 460
#extension GL_EXT_ray_tracing : enable

layout(binding = 0, rgba32f) uniform writeonly image2D out_color;
layout(binding = 1) uniform accelerationStructureEXT tlas;
) "
+ CommonSystem::GetBufferObjectStructureHeader(2U) +
u8R"(

struct hitPayload
{
	vec4 color;
	vec3 ro;
	vec3 rd;
};

layout(location = 0) rayPayloadEXT hitPayload prd;

vec3 getRayOrigin()
{
	vec3 ro = view[3].xyz + model[3].xyz;
	ro *= mat3(model);
	return -ro;
}

vec3 getRayDirection(vec2 uv)
{
	uv = uv * 2.0 - 1.0;
	vec4 ray_clip = vec4(uv.x, uv.y, -1.0, 0.0);
	vec4 ray_eye = inverse(proj) * ray_clip;
	vec3 rd = normalize(vec3(ray_eye.x, ray_eye.y, -1.0));
	rd *= mat3(view * model);
	return rd;
}

void main()
{
	const vec2 p = vec2(gl_LaunchIDEXT.xy);
	const vec2 s = vec2(gl_LaunchSizeEXT.xy);

	const vec2 pc = p + 0.5; // pixel center
	const vec2 uv = pc / s;
	const vec2 uvc = uv * 2.0 - 1.0;
	
	mat4 imv = inverse(model * view);
	mat4 ip = inverse(proj);

	vec4 origin    = imv * vec4(0, 0, 0, 1);
	vec4 target    = ip * vec4(uvc.x, uvc.y, 1, 1);
	vec4 direction = imv * vec4(normalize(target.xyz), 0);

	float tmin = 0.001;
	float tmax = 1e32;

	prd.ro = getRayOrigin();		// origin.xyz;
	prd.rd = getRayDirection(uv);	// direction.xyz;
	prd.color = vec4(0.0);

	traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, prd.ro, tmin, prd.rd, tmax, 0);
	
	imageStore(out_color, ivec2(gl_LaunchIDEXT.xy), prd.color);
}
)"; res += u8R"(";
}

std::string PbrRenderer_Rtx_Pass::GetRayIntersectionShaderCode(std::string& vOutShaderName)
{
	ZoneScoped;

	vOutShaderName = "PbrRenderer_Rtx_Pass";
	return u8R"()"; res += u8R"(";
}

std::string PbrRenderer_Rtx_Pass::GetRayMissShaderCode(std::string& vOutShaderName)
{
	ZoneScoped;

	vOutShaderName = "PbrRenderer_Rtx_Pass";
	return u8R"(
#version 460
#extension GL_EXT_ray_tracing : enable

struct hitPayload
{
	vec4 color;
	vec3 ro;
	vec3 rd;
	float ao;
	float diff;
	float spec;
	float sha;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;
layout(location = 1) rayPayloadEXT bool isShadowed;

void main()
{
	prd.color = vec4(0.0, 0.0, 0.0, 1.0);
	prd.diff = 0.0;
	prd.spec = 0.0;
	prd.sha = 0.0;
	isShadowed = false;
}
)"; res += u8R"(";
}

std::string PbrRenderer_Rtx_Pass::GetRayAnyHitShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PbrRenderer_Rtx_Pass";
	return u8R"()"; res += u8R"(";
}

std::string PbrRenderer_Rtx_Pass::GetRayClosestHitShaderCode(std::string& vOutShaderName)
{
	ZoneScoped;

	vOutShaderName = "PbrRenderer_Rtx_Pass";
	return u8R"(

#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

struct hitPayload
{
	vec4 color;
	vec3 ro;
	vec3 rd;
	float ao;
	float diff;
	float spec;
	float sha;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;
layout(location = 1) rayPayloadEXT bool isShadowed;

hitAttributeEXT vec3 attribs;

struct V3N3T3B3T2C4 
{
	float px, py, pz;
	float nx, ny, nz;
	float tax, tay, taz;
	float btax, btay, btaz;
	float tx, ty;
	float cx, cy, cz, cw;
};

layout(buffer_reference, scalar) readonly buffer Vertices
{
	V3N3T3B3T2C4 vdatas[];
};

layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Indices
{
	uvec3 idatas[];
};

layout(binding = 1) uniform accelerationStructureEXT tlas; // same as raygen shader

struct SceneMeshBuffers
{
	uint64_t vertices;
	uint64_t indices;
};

layout(binding = 3) buffer ModelAddresses 
{ 
	SceneMeshBuffers datas[]; 
} sceneMeshBuffers;
)"; res += u8R"(";
+
SceneLightGroup::GetBufferObjectStructureHeader(4U)
+
u8R"(
float ShadowTest(vec3 p, vec3 n, vec3 ld)
{
	if (dot(n, ld) > 0.0)
	{
		p += n * 0.1;
		uint flags  = 
			gl_RayFlagsTerminateOnFirstHitEXT | 
			gl_RayFlagsOpaqueEXT | 
			gl_RayFlagsSkipClosestHitShaderEXT;
		isShadowed = true; 
		traceRayEXT(tlas,        // acceleration structure
		            flags,             // rayFlags
		            0xFF,              // cullMask
		            0,                 // sbtRecordOffset
		            0,                 // sbtRecordStride
		            0,                 // missIndex
		            p,            	   // ray origin
		            0.1,              // ray min range
		            ld,            // ray direction
		            1e32,              // ray max range
		            1                  // payload (location = 1)
		);
		if (isShadowed)
			return 0.5;
	}
	
	return 1.0;
}

void main()
{
	// When contructing the TLAS, we stored the model id in InstanceCustomIndexEXT, so the
	// the instance can quickly have access to the data

	// Object data
	SceneMeshBuffers meshRes = sceneMeshBuffers.datas[gl_InstanceCustomIndexEXT];
	Indices indices = Indices(meshRes.indices);
	Vertices vertices = Vertices(meshRes.vertices);

	// Indices of the triangle
	uvec3 ind = indices.idatas[gl_PrimitiveID];

	// Vertex of the triangle
	V3N3T3B3T2C4 v0 = vertices.vdatas[ind.x];
	V3N3T3B3T2C4 v1 = vertices.vdatas[ind.y];
	V3N3T3B3T2C4 v2 = vertices.vdatas[ind.z];

	// Barycentric coordinates of the triangle
	const vec3 barycentrics = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);

	vec3 normal = 
		vec3(v0.nx, v0.ny, v0.nz) * barycentrics.x + 
		vec3(v1.nx, v1.ny, v1.nz) * barycentrics.y + 
		vec3(v2.nx, v2.ny, v2.nz) * barycentrics.z;
    
	// Transforming the normal to world space
	normal = normalize(vec3(normal * gl_WorldToObjectEXT)); 
	
	prd.color = vec4(normal * 0.5 + 0.5, 1.0); // return normal
}
)"; res += u8R"(;
})";
	}

	return res;
}

std::string GeneratorNode::GetPassShaderHeader()
{
	std::string res;

	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		res += u8R"(
	std::string GetVertexShaderCode(std::string& vOutShaderName) override;
	std::string GetFragmentShaderCode(std::string& vOutShaderName) override;)";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D)
	{
		res += u8R"(
	std::string GetComputeShaderCode(std::string& vOutShaderName) override;)";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_2D)
	{
		res += u8R"(
	std::string GetComputeShaderCode(std::string& vOutShaderName) override;)";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_3D)
	{
		res += u8R"(
	std::string GetComputeShaderCode(std::string& vOutShaderName) override;)";
	}
	else if (m_RendererType == RENDERER_TYPE_RTX)
	{
		res += u8R"(
	std::string GetRayGenerationShaderCode(std::string& vOutShaderName) override;
	std::string GetRayIntersectionShaderCode(std::string& vOutShaderName) override;
	std::string GetRayMissShaderCode(std::string& vOutShaderName) override;
	std::string GetRayAnyHitShaderCode(std::string& vOutShaderName) override;
	std::string GetRayClosestHitShaderCode(std::string& vOutShaderName) override;)";
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// GENERATOR SLOTS DICO ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SlotDico GeneratorNode::GetSlotDico()
{
	SlotDico res;

	m_ShowInputWidgets = false;
	for (const auto& inputSlot : m_Inputs)
	{
		if (inputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(inputSlot.second);
			if (slotDatasPtr)
			{
				m_InputSlotCounter[(BaseTypeEnum)slotDatasPtr->editorSlotTypeIndex]++;
			}

			m_ShowInputWidgets |= inputSlot.second->showWidget;
		}
	}

	for (const auto& inputSlot : m_Inputs)
	{
		if (inputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(inputSlot.second);
			if (slotDatasPtr)
			{
				switch(slotDatasPtr->editorSlotTypeIndex)
				{
				case BaseTypeEnum::BASE_TYPE_None: // None
					res[BASE_TYPE_None][NodeSlot::PlaceEnum::INPUT].push_back(GetSlotNoneInput(inputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_AccelStructure: // AccelStructure
					res[BASE_TYPE_AccelStructure][NodeSlot::PlaceEnum::INPUT].push_back(GetSlotAccelStructureInput(inputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_LightGroup: // LightGroup
					res[BASE_TYPE_LightGroup][NodeSlot::PlaceEnum::INPUT].push_back(GetSlotLightGroupInput(inputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_Model: // Model
					res[BASE_TYPE_Model][NodeSlot::PlaceEnum::INPUT].push_back(GetSlotModelInput(inputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_StorageBuffer: // StorageBuffer
					res[BASE_TYPE_StorageBuffer][NodeSlot::PlaceEnum::INPUT].push_back(GetSlotStorageBufferInput(inputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_TexelBuffer: // TexelBuffer
					res[BASE_TYPE_TexelBuffer][NodeSlot::PlaceEnum::INPUT].push_back(GetSlotTexelBufferInput(inputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_Texture: // Texture
					res[BASE_TYPE_Texture][NodeSlot::PlaceEnum::INPUT].push_back(GetSlotTextureInput(inputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_TextureCube: // TextureCube
					res[BASE_TYPE_TextureCube][NodeSlot::PlaceEnum::INPUT].push_back(GetSlotTextureCubeInput(inputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_TextureGroup: // TextureGroup
					res[BASE_TYPE_TextureGroup][NodeSlot::PlaceEnum::INPUT].push_back(GetSlotTextureGroupInput(inputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_Variable: // Variable
					res[BASE_TYPE_Variable][NodeSlot::PlaceEnum::INPUT].push_back(GetSlotVariableInput(inputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_Custom: // Custom
					res[BASE_TYPE_Custom][NodeSlot::PlaceEnum::INPUT].push_back(GetSlotCustomInput(inputSlot.second));
					break;
				}
				
			}
		}
	}

	m_ShowOutputWidgets = false;
	for (const auto& outputSlot : m_Outputs)
	{
		if (outputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(outputSlot.second);
			if (slotDatasPtr)
			{
				m_OutputSlotCounter[(BaseTypeEnum)slotDatasPtr->editorSlotTypeIndex]++;
			}

			m_ShowOutputWidgets |= outputSlot.second->showWidget;
		}
	}

	for (const auto& outputSlot : m_Outputs)
	{
		if (outputSlot.second)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(outputSlot.second);
			if (slotDatasPtr)
			{
				switch (slotDatasPtr->editorSlotTypeIndex)
				{
				case BaseTypeEnum::BASE_TYPE_None: // None
					res[BASE_TYPE_None][NodeSlot::PlaceEnum::OUTPUT].push_back(GetSlotNoneOutput(outputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_AccelStructure: // AccelStructure
					res[BASE_TYPE_AccelStructure][NodeSlot::PlaceEnum::OUTPUT].push_back(GetSlotAccelStructureOutput(outputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_LightGroup: // LightGroup
					res[BASE_TYPE_LightGroup][NodeSlot::PlaceEnum::OUTPUT].push_back(GetSlotLightGroupOutput(outputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_Model: // Model
					res[BASE_TYPE_Model][NodeSlot::PlaceEnum::OUTPUT].push_back(GetSlotModelOutput(outputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_StorageBuffer: // StorageBuffer
					res[BASE_TYPE_StorageBuffer][NodeSlot::PlaceEnum::OUTPUT].push_back(GetSlotStorageBufferOutput(outputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_TexelBuffer: // TexelBuffer
					res[BASE_TYPE_TexelBuffer][NodeSlot::PlaceEnum::OUTPUT].push_back(GetSlotTexelBufferOutput(outputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_Texture: // Texture
					res[BASE_TYPE_Texture][NodeSlot::PlaceEnum::OUTPUT].push_back(GetSlotTextureOutput(outputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_TextureCube: // TextureCube
					res[BASE_TYPE_TextureCube][NodeSlot::PlaceEnum::OUTPUT].push_back(GetSlotTextureCubeOutput(outputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_TextureGroup: // TextureGroup
					res[BASE_TYPE_TextureGroup][NodeSlot::PlaceEnum::OUTPUT].push_back(GetSlotTextureGroupOutput(outputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_Variable: // Variable
					res[BASE_TYPE_Variable][NodeSlot::PlaceEnum::OUTPUT].push_back(GetSlotVariableOutput(outputSlot.second));
					break;
				case BaseTypeEnum::BASE_TYPE_Custom: // Custom
					res[BASE_TYPE_Custom][NodeSlot::PlaceEnum::OUTPUT].push_back(GetSlotCustomOutput(outputSlot.second));
					break;
				}
			}
		}
	}

	
	return res;
}

SlotStringStruct GeneratorNode::GetSlotNoneInput(NodeSlotInputPtr vSlot)
{
	CTOOL_DEBUG_BREAK;

	SlotStringStruct res;

	res.cpp_node_func = u8R"()";
	res.cpp_module_func = u8R"()";
	res.cpp_pass_func = u8R"()";
	res.h_func = u8R"()";
	res.include_interface = u8R"()";
	res.include_slot = u8R"()";
	res.node_module_public_interface = u8R"()";
	res.pass_public_interface = u8R"()";
	res.node_slot_func += u8R"()";

	return res;
}

SlotStringStruct GeneratorNode::GetSlotNoneOutput(NodeSlotOutputPtr vSlot)
{
	CTOOL_DEBUG_BREAK;

	SlotStringStruct res;

	res.cpp_node_func = u8R"()";
	res.cpp_module_func = u8R"()";
	res.cpp_pass_func = u8R"()";
	res.h_func = u8R"()";
	res.include_interface = u8R"()";
	res.include_slot = u8R"()";
	res.node_module_public_interface = u8R"()";
	res.pass_public_interface = u8R"()";
	res.node_slot_func += u8R"()";

	return res;
}

SlotStringStruct GeneratorNode::GetSlotAccelStructureInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// ACCEL STRUCTURE INPUT ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetAccelStructure(SceneAccelStructureWeak vSceneAccelStructure)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->SetAccelStructure(vSceneAccelStructure);
	})";
	}
	else
	{
		res.cpp_node_func += u8R"(
)";
	}

	res.cpp_node_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// ACCEL STRUCTURE INPUT ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void MODULE_CLASS_NAME::SetAccelStructure(SceneAccelStructureWeak vSceneAccelStructure)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		m_PASS_CLASS_NAME_Ptr->SetAccelStructure(vSceneAccelStructure);
	})";
	}
	else
	{
		res.cpp_module_func += u8R"(
)";
	}

	res.cpp_module_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// ACCEL STRUCTURE INPUT ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void PASS_CLASS_NAME::SetAccelStructure(SceneAccelStructureWeak vSceneAccelStructure)
{	
	ZoneScoped;

	m_SceneAccelStructure = vSceneAccelStructure;
}
)";

	res.h_func = u8R"(
	void SetAccelStructure(SceneAccelStructureWeak vSceneAccelStructure) override;)";

	res.include_interface = u8R"(
#include <Interfaces/AccelStructureInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotAccelStructureInput.h>)";

	res.node_module_public_interface = u8R"(
	public AccelStructureInputInterface,)";

	res.pass_public_interface = u8R"(
	public AccelStructureInputInterface,)";

	res.node_slot_func += ct::toStr(u8R"(
	AddInput(NodeSlotAccelStructureInput::Create("%s"), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotAccelStructureOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// ACCEL STRUCTURE OUTPUT //////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneAccelStructureWeak NODE_CLASS_NAME::GetAccelStruct()
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		return m_MODULE_CLASS_NAMEPtr->GetAccelStruct();
	}
)";
	}

	res.cpp_node_func += u8R"(
	return SceneAccelStructureWeak();
}

vk::WriteDescriptorSetAccelerationStructureKHR* NODE_CLASS_NAME::GetTLASInfo()
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		return m_MODULE_CLASS_NAMEPtr->GetTLASInfo();
	}
)";
	}

	res.cpp_node_func += u8R"(
	return nullptr;
}

vk::DescriptorBufferInfo* NODE_CLASS_NAME::GetBufferAddressInfo()
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		return m_MODULE_CLASS_NAMEPtr->GetBufferAddressInfo();
	}
)";
	}

	res.cpp_node_func += u8R"(
	return nullptr;
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// ACCEL STRUCTURE OUTPUT //////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneAccelStructureWeak MODULE_CLASS_NAME::GetAccelStruct()
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		return m_PASS_CLASS_NAME_Ptr->GetAccelStruct();
	}
)";
	}

	res.cpp_node_func += u8R"(
	return SceneAccelStructureWeak();
}

vk::WriteDescriptorSetAccelerationStructureKHR* MODULE_CLASS_NAME::GetTLASInfo()
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		return m_PASS_CLASS_NAME_Ptr->GetTLASInfo();
	}
)";
	}

	res.cpp_node_func += u8R"(
	return nullptr;
}

vk::DescriptorBufferInfo* MODULE_CLASS_NAME::GetBufferAddressInfo()
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		return m_PASS_CLASS_NAME_Ptr->GetBufferAddressInfo();
	}
)";
	}

	res.cpp_node_func += u8R"(
	return nullptr;
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// ACCEL STRUCTURE OUTPUT //////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneAccelStructureWeak PASS_CLASS_NAME::GetAccelStruct()
{	
	ZoneScoped;

	return SceneAccelStructureWeak();
}

vk::WriteDescriptorSetAccelerationStructureKHR* PASS_CLASS_NAME::GetTLASInfo()
{	
	ZoneScoped;

	return nullptr;
}

vk::DescriptorBufferInfo* PASS_CLASS_NAME::GetBufferAddressInfo()
{	
	ZoneScoped;

	return nullptr;
}
)";

	res.h_func = u8R"(
	SceneAccelStructureWeak GetAccelStruct() override;
	vk::WriteDescriptorSetAccelerationStructureKHR* GetTLASInfo() override;
	vk::DescriptorBufferInfo* GetBufferAddressInfo() override;)";

	res.include_interface = u8R"(
#include <Interfaces/AccelStructureOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotAccelStructureOutput.h>)";

	res.node_module_public_interface = u8R"(
	public AccelStructureOutputInterface,)";

	res.pass_public_interface = u8R"(
	public AccelStructureOutputInterface,)";

	res.node_slot_func += ct::toStr(u8R"(
	AddOutput(NodeSlotAccelStructureOutput::Create("%s"), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotLightGroupInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// LIGHT GROUP INPUT ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->SetLightGroup(vSceneLightGroup);
	})";
	}
	else
	{
		res.cpp_node_func += u8R"(
)";
	}

	res.cpp_node_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// LIGHT GROUP INPUT ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void MODULE_CLASS_NAME::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		m_PASS_CLASS_NAME_Ptr->SetLightGroup(vSceneLightGroup);
	})";
	}
	else
	{
		res.cpp_module_func += u8R"(
)";
	}

	res.cpp_module_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// LIGHT GROUP INPUT ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{	
	ZoneScoped;

	m_SceneLightGroup = vSceneLightGroup;

	m_SceneLightGroupDescriptorInfoPtr = &m_SceneEmptyLightGroupDescriptorInfo;

	auto lightGroupPtr = m_SceneLightGroup.getValidShared();
	if (lightGroupPtr && 
		lightGroupPtr->GetBufferInfo())
	{
		m_SceneLightGroupDescriptorInfoPtr = lightGroupPtr->GetBufferInfo();
	}

	UpdateBufferInfoInRessourceDescriptor();
}
)";

	res.h_func = u8R"(
	void SetLightGroup(SceneLightGroupWeak vSceneLightGroup) override;)";

	res.include_interface = u8R"(
#include <Interfaces/LightGroupInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotLightGroupInput.h>)";

	res.node_module_public_interface = u8R"(
	public LightGroupInputInterface,)";

	res.pass_public_interface = u8R"(
	public LightGroupInputInterface,)";

	res.node_slot_func += ct::toStr(u8R"(
	AddInput(NodeSlotLightGroupInput::Create("%s"), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotLightGroupOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// LIGHT GROUP OUTPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneLightGroupWeak NODE_CLASS_NAME::GetLightGroup()
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		return m_MODULE_CLASS_NAMEPtr->GetLightGroup();
	}
)";
	}

	res.cpp_node_func += u8R"(
	return SceneLightGroupWeak();
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// LIGHT GROUP OUTPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneLightGroupWeak MODULE_CLASS_NAME::GetLightGroup()
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		return m_PASS_CLASS_NAME_Ptr->GetLightGroup();
	}
)";
	}

	res.cpp_module_func += u8R"(
	return SceneLightGroupWeak();
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// LIGHT GROUP OUTPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneLightGroupWeak NODE_CLASS_NAME::GetLightGroup()
{	
	ZoneScoped;

	return m_SceneLightGroup;
}
)";

	res.h_func = u8R"(
	SceneLightGroupWeak GetLightGroup() override;)";

	res.include_interface = u8R"(
#include <Interfaces/LightGroupOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotLightGroupOutput.h>)";

	res.node_module_public_interface = u8R"(
	public LightGroupOutputInterface,)";

	res.pass_public_interface = u8R"(
	public LightGroupOutputInterface,)";

	res.node_slot_func += ct::toStr(u8R"(
	AddOutput(NodeSlotLightGroupOutput::Create("%s"), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotModelInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL INPUT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetModel(SceneModelWeak vSceneModel)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->SetModel(vSceneModel);
	})";
	}
	else
	{
		res.cpp_node_func += u8R"(
)";
	}

	res.cpp_node_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL INPUT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void MODULE_CLASS_NAME::SetModel(SceneModelWeak vSceneModel)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		m_PASS_CLASS_NAME_Ptr->SetModel(vSceneModel);
	})";
	}
	else
	{
		res.cpp_module_func += u8R"(
)";
	}

	res.cpp_module_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL INPUT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetModel(SceneModelWeak vSceneModel)
{	
	ZoneScoped;

	m_SceneModel = vSceneModel;
}
)";

	res.h_func = u8R"(
	void SetModel(SceneModelWeak vSceneModel) override;)";

	res.include_interface = u8R"(
#include <Interfaces/ModelInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotModelInput.h>)";

	res.node_module_public_interface = u8R"(
	public ModelInputInterface,)";

	res.pass_public_interface = u8R"(
	public ModelInputInterface,)";

	res.node_slot_func += ct::toStr(u8R"(
	AddInput(NodeSlotModelInput::Create("%s"), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotModelOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL OUTPUT ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneModelWeak NODE_CLASS_NAME::GetModel()
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		return m_MODULE_CLASS_NAMEPtr->GetModel();
	}
)";
	}

	res.cpp_node_func += u8R"(
	return SceneModelWeak();
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL OUTPUT ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneModelWeak MODULE_CLASS_NAME::GetModel()
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		return m_PASS_CLASS_NAME_Ptr->GetModel();
	}
)";
	}

	res.cpp_module_func += u8R"(
	return SceneModelWeak();
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL OUTPUT ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneModelWeak NODE_CLASS_NAME::GetModel()
{	
	ZoneScoped;

	return m_SceneModelPtr;
}
)";

	res.h_func = u8R"(
	SceneModelWeak GetModel() override;)";

	res.include_interface = u8R"(
#include <Interfaces/ModelOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotModelOutput.h>)";

	res.node_module_public_interface = u8R"(
	public ModelOutputInterface,)";

	res.pass_public_interface = u8R"(
	public ModelOutputInterface,)";

	res.node_slot_func += ct::toStr(u8R"(
	AddOutput(NodeSlotModelOutput::Create("%s"), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotStorageBufferInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// STORAGE BUFFER INPUT ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetStorageBuffer(const uint32_t& vBindingPoint, vk::DescriptorBufferInfo* vStorageBuffer, uint32_t* vStorageBufferSize)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->SetStorageBuffer(vBindingPoint, vStorageBuffer, vStorageBufferSize);
	})";
	}
	else
	{
		res.cpp_node_func += u8R"(
)";
	}

	res.cpp_node_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// STORAGE BUFFER INPUT ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void MODULE_CLASS_NAME::SetStorageBuffer(const uint32_t& vBindingPoint, vk::DescriptorBufferInfo* vStorageBuffer, uint32_t* vStorageBufferSize)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		m_PASS_CLASS_NAME_Ptr->SetStorageBuffer(vBindingPoint, vStorageBuffer, vStorageBufferSize);
	})";
	}
	else
	{
		res.cpp_module_func += u8R"(
)";
	}

	res.cpp_module_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// STORAGE BUFFER INPUT ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetStorageBuffer(const uint32_t& vBindingPoint, vk::DescriptorBufferInfo* vStorageBuffer, uint32_t* vStorageBufferSize)
{	
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBindingPoint < m_StorageBuffers.size())
		{
			if (vStorageBuffer)
			{
				if (vStorageBufferSize)
				{
					m_ImageInfosSize[vBindingPoint] = *vStorageBufferSize;
				}

				m_StorageBufferSize[vBindingPoint] = *vStorageBuffer;
			}
			else
			{
				m_StorageBuffers[vBindingPoint] = m_VulkanCorePtr->getEmptyDescriptorBufferInfo();
			}
		}
	}
}
)";

	res.h_func = u8R"(
	void SetStorageBuffer(const uint32_t& vBindingPoint, vk::DescriptorBufferInfo* vStorageBuffer, uint32_t* vStorageBufferSize = nullptr) override;)";

	res.include_interface = u8R"(
#include <Interfaces/StorageBufferInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotStorageBufferInput.h>)";

	res.node_module_public_interface = u8R"(
	public StorageBufferInputInterface<0U>,)";

	res.pass_public_interface = ct::toStr(u8R"(
	public StorageBufferInputInterface,)", m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_StorageBuffer]);

	res.node_slot_func += ct::toStr(u8R"(
	AddInput(NodeSlotStorageBufferInput::Create("%s"), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotStorageBufferOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// STORAGE BUFFER OUTPUT ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorBufferInfo* NODE_CLASS_NAME::GetStorageBuffer(const uint32_t& vBindingPoint, uint32_t* vOutSize)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		return m_MODULE_CLASS_NAMEPtr->GetStorageBuffer(vBindingPoint, vOutSize);
	}
)";
	}

	res.cpp_node_func += u8R"(
	return nullptr;
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// STORAGE BUFFER OUTPUT ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorBufferInfo* MODULE_CLASS_NAME::GetStorageBuffer(const uint32_t& vBindingPoint, uint32_t* vOutSize)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		return m_PASS_CLASS_NAME_Ptr->GetStorageBuffer(vBindingPoint, vOutSize);
	}
)";
	}

	res.cpp_module_func += u8R"(
	return nullptr;
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// STORAGE BUFFER OUTPUT ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorBufferInfo* NODE_CLASS_NAME::GetStorageBuffer(const uint32_t& vBindingPoint, uint32_t* vOutSize)
{	
	ZoneScoped;

	return nullptr;
}
)";

	res.h_func = u8R"(
	vk::DescriptorBufferInfo* GetStorageBuffer(const uint32_t& vBindingPoint, uint32_t* vOutSize = nullptr) override;)";

	res.include_interface = u8R"(
#include <Interfaces/StorageBufferOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotStorageBufferOutput.h>)";

	res.node_module_public_interface = u8R"(
	public StorageBufferOutputInterface,)";

	res.pass_public_interface = u8R"(
	public StorageBufferOutputInterface,)";

	res.node_slot_func += ct::toStr(u8R"(
	AddOutput(NodeSlotStorageBufferOutput::Create("%s"), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotTexelBufferInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXEL BUFFER INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetTexelBuffer(const uint32_t& vBindingPoint, vk::Buffer* vTexelBuffer, ct::uvec2* vTexelBufferSize)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->SetTexelBuffer(vBindingPoint, vTexelBuffer, vTexelBufferSize);
	})";
	}
	else
	{
		res.cpp_node_func += u8R"(
)";
	}

	res.cpp_node_func += u8R"(
}

void NODE_CLASS_NAME::SetTexelBufferView(const uint32_t& vBindingPoint, vk::BufferView* vTexelBufferView, ct::uvec2* vTexelBufferSize)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->SetTexelBufferView(vBindingPoint, vTexelBufferView, vTexelBufferSize);
	})";
	}
	else
	{
		res.cpp_node_func += u8R"(
)";
	}

	res.cpp_node_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXEL BUFFER INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void MODULE_CLASS_NAME::SetTexelBuffer(const uint32_t& vBindingPoint, vk::Buffer* vTexelBuffer, ct::uvec2* vTexelBufferSize)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		m_PASS_CLASS_NAME_Ptr->SetTexelBuffer(vBindingPoint, vTexelBuffer, vTexelBufferSize);
	})";
	}
	else
	{
		res.cpp_module_func += u8R"(
)";
	}

	res.cpp_module_func += u8R"(
}

void MODULE_CLASS_NAME::SetTexelBufferView(const uint32_t& vBindingPoint, vk::BufferView* vTexelBufferView, ct::uvec2* vTexelBufferSize)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		m_PASS_CLASS_NAME_Ptr->SetTexelBufferView(vBindingPoint, vTexelBufferView, vTexelBufferSize);
	})";
	}
	else
	{
		res.cpp_module_func += u8R"(
)";
	}

	res.cpp_module_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXEL BUFFER INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetTexelBuffer(const uint32_t& vBindingPoint, vk::Buffer* vTexelBuffer, ct::uvec2* vTexelBufferSize)
{	
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBindingPoint < m_StorageBuffers.size())
		{
			if (vTexelBuffer)
			{
				if (vTexelBufferSize)
				{
					m_TexelBufferViewsSize[vBindingPoint] = *vTexelBufferSize;
				}

				m_TexelBuffers[vBindingPoint] = *vTexelBuffer;
			}
			else
			{
				m_TexelBuffers[vBindingPoint] = m_VulkanCorePtr->getEmptyDescriptorBufferInfo();
			}
		}
	}
}

void NODE_CLASS_NAME::SetTexelBufferView(const uint32_t& vBindingPoint, vk::BufferView* vTexelBufferView, ct::uvec2* vTexelBufferSize)
{	
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBindingPoint < m_StorageBuffers.size())
		{
			if (vTexelvTexelBufferViewuffer)
			{
				if (vTexelBufferSize)
				{
					m_TexelBufferViewsSize[vBindingPoint] = *vTexelBufferSize;
				}

				m_TexelBufferViews[vBindingPoint] = *vTexelBufferView;
			}
			else
			{
				m_TexelBufferViews[vBindingPoint] = m_VulkanCorePtr->getEmptyDescriptorBufferInfo();
			}
		}
	}
}
)";

	res.h_func = u8R"(
	void SetTexelBuffer(const uint32_t& vBindingPoint, vk::Buffer* vTexelBuffer, ct::uvec2* vTexelBufferSize = nullptr) override;
	void SetTexelBufferView(const uint32_t& vBindingPoint, vk::BufferView* vTexelBufferView, ct::uvec2* vTexelBufferSize = nullptr) override;)";

	res.include_interface = u8R"(
#include <Interfaces/TexelBufferInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotTexelBufferInput.h>)";

	res.node_module_public_interface = u8R"(
	public TexelBufferInputInterface<0U>,)";

	res.pass_public_interface = ct::toStr(u8R"(
	public TexelBufferInputInterface,)", m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_TexelBuffer]);

	res.node_slot_func += ct::toStr(u8R"(
	AddInput(NodeSlotTexelBufferInput::Create("%s"), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotTexelBufferOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXEL BUFFER OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::Buffer* NODE_CLASS_NAME::GetTexelBuffer(const uint32_t& vBindingPoint, ct::uvec2* vOutSize = nullptr)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		return m_MODULE_CLASS_NAMEPtr->GetTexelBuffer(vBindingPoint, vOutSize);
	}
)";
	}

	res.cpp_node_func += u8R"(
	return nullptr;
}

vk::BufferView* NODE_CLASS_NAME::GetTexelBufferView(const uint32_t& vBindingPoint, ct::uvec2* vOutSize = nullptr)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		return m_MODULE_CLASS_NAMEPtr->GetTexelBufferView(vBindingPoint, vOutSize);
	}
)";
	}

	res.cpp_node_func += u8R"(
	return nullptr;
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXEL BUFFER OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::Buffer* MODULE_CLASS_NAME::GetTexelBuffer(const uint32_t& vBindingPoint, ct::uvec2* vOutSize = nullptr)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		return m_PASS_CLASS_NAME_Ptr->GetTexelBuffer(vBindingPoint, vOutSize);
	}
)";
	}

	res.cpp_module_func += u8R"(
	return nullptr;
}

vk::BufferView* MODULE_CLASS_NAME::GetTexelBufferView(const uint32_t& vBindingPoint, ct::uvec2* vOutSize = nullptr)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		return m_PASS_CLASS_NAME_Ptr->GetTexelBufferView(vBindingPoint, vOutSize);
	}
)";
	}

	res.cpp_module_func += u8R"(
	return nullptr;
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXEL BUFFER OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::Buffer* NODE_CLASS_NAME::GetTexelBuffer(const uint32_t& vBindingPoint, ct::uvec2* vOutSize = nullptr)
{	
	ZoneScoped;

	return nullptr;
}

vk::BufferView* NODE_CLASS_NAME::GetTexelBufferView(const uint32_t& vBindingPoint, ct::uvec2* vOutSize = nullptr)
{	
	ZoneScoped;

	return nullptr;
}
)";

	res.h_func = u8R"(
	vk::Buffer* GetTexelBuffer(const uint32_t& vBindingPoint, ct::uvec2* vOutSize = nullptr) override;
	vk::BufferView* GetTexelBufferView(const uint32_t& vBindingPoint, ct::uvec2* vOutSize = nullptr) override;)";

	res.include_interface = u8R"(
#include <Interfaces/TexelBufferOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotTexelBufferOutput.h>)";

	res.node_module_public_interface = u8R"(
	public TexelBufferOutputInterface,)";

	res.pass_public_interface = u8R"(
	public TexelBufferOutputInterface,)";

	res.node_slot_func += ct::toStr(u8R"(
	AddOutput(NodeSlotTexelBufferOutput::Create("%s"), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotTextureInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	})";
	}
	else
	{
		res.cpp_node_func += u8R"(
)";
	}

	res.cpp_node_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void MODULE_CLASS_NAME::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		m_PASS_CLASS_NAME_Ptr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	})";
	}
	else
	{
		res.cpp_module_func += u8R"(
)";
	}

	res.cpp_module_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{	
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBindingPoint < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				if (vTextureSize)
				{
					m_ImageInfosSize[vBindingPoint] = *vTextureSize;
				}

				m_ImageInfos[vBindingPoint] = *vImageInfo;
			}
			else
			{
				m_ImageInfos[vBindingPoint] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
			}
		}
	}
}
)";

	res.h_func = u8R"(
	void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize = nullptr) override;)";

	res.include_interface = u8R"(
#include <Interfaces/TextureInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotTextureInput.h>)";

	res.node_module_public_interface = u8R"(
	public TextureInputInterface<0U>,)";

	res.pass_public_interface = ct::toStr(u8R"(
	public TextureInputInterface<%u>,)", m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_Texture]);

	res.node_slot_func += ct::toStr(u8R"(
	AddInput(NodeSlotTextureInput::Create("%s", %u), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(),
		vSlot->descriptorBinding,
		vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotTextureOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* NODE_CLASS_NAME::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		return m_MODULE_CLASS_NAMEPtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}
)";
	}

	res.cpp_node_func += u8R"(
	return nullptr;
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* MODULE_CLASS_NAME::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		return m_PASS_CLASS_NAME_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}
)";
	}

	res.cpp_module_func += u8R"(
	return nullptr;
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* NODE_CLASS_NAME::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;)";

	if (m_RendererType == RENDERER_TYPE_PIXEL_2D)
	{
		res.cpp_pass_func += u8R"(
	if (m_FrameBufferPtr)
	{
		if (vOutSize)
		{
			*vOutSize = m_FrameBufferPtr->GetOutputSize();
		}

		return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	})";
	}
	else if (m_RendererType == RENDERER_TYPE_COMPUTE_1D ||
		m_RendererType == RENDERER_TYPE_COMPUTE_2D ||
		m_RendererType == RENDERER_TYPE_COMPUTE_3D ||
		m_RendererType == RENDERER_TYPE_RTX)
	{
		res.cpp_pass_func += u8R"(
	if (m_ComputeBufferPtr)
	{
		if (vOutSize)
		{
			*vOutSize = m_ComputeBufferPtr->GetOutputSize();
		}

		return m_ComputeBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	})";
	}

	res.cpp_pass_func += u8R"(
	return nullptr;
}
)";

	res.h_func = u8R"(
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;)";

	res.include_interface = u8R"(
#include <Interfaces/TextureOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotTextureOutput.h>)";

	res.node_module_public_interface = u8R"(
	public TextureOutputInterface,)";

	res.pass_public_interface = u8R"(
	public TextureOutputInterface,)";

	res.node_slot_func += ct::toStr(u8R"(
	AddOutput(NodeSlotTextureOutput::Create("%s", %u), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(), vSlot->descriptorBinding, vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotTextureCubeInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetTextureCube(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageCubeInfo, ct::fvec2* vTextureSize)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->SetTextureCube(vBindingPoint, vImageCubeInfo, vTextureSize);
	})";
	}
	else
	{
		res.cpp_node_func += u8R"(
)";
	}

	res.cpp_node_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void MODULE_CLASS_NAME::SetTextureCube(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageCubeInfo, ct::fvec2* vTextureSize)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		m_PASS_CLASS_NAME_Ptr->SetTextureCube(vBindingPoint, vImageCubeInfo, vTextureSize);
	})";
	}
	else
	{
		res.cpp_module_func += u8R"(
)";
	}

	res.cpp_module_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetTextureCube(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageCubeInfo, ct::fvec2* vTextureSize)
{	
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBindingPoint < m_ImageCubeInfos.size())
		{
			if (vImageCubeInfo)
			{
				if (vTextureSize)
				{
					m_ImageCubeInfosSize[vBindingPoint] = *vTextureSize;
				}

				m_ImageCubeInfos[vBindingPoint] = *vImageCubeInfo;
			}
			else
			{
				m_ImageCubeInfos[vBindingPoint] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
			}
		}
	}
}
)";

	res.h_func = u8R"(
	void SetTextureCube(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageCubeInfo, ct::fvec2* vTextureSize = nullptr) override;)";

	res.include_interface = u8R"(
#include <Interfaces/TextureCubeInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotTextureCubeInput.h>)";

	res.node_module_public_interface = u8R"(
	public TextureCubeInputInterface<0U>,)";

	res.pass_public_interface = ct::toStr(u8R"(
	public TextureCubeInputInterface<%u>,)", m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_TextureCube]);

	res.node_slot_func += ct::toStr(u8R"(
	AddInput(NodeSlotTextureCubeInput::Create("%s", %u), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(),
		vSlot->descriptorBinding,
		vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotTextureCubeOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE CUBE SLOT OUTPUT ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* NODE_CLASS_NAME::GetTextureCube(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		return m_MODULE_CLASS_NAMEPtr->GetTextureCube(vBindingPoint, vOutSize);
	}
)";
	}

	res.cpp_node_func += u8R"(
	return nullptr;
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE CUBE SLOT OUTPUT ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* MODULE_CLASS_NAME::GetTextureCube(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		return m_PASS_CLASS_NAME_Ptr->GetTextureCube(vBindingPoint, vOutSize);
	}
)";
	}

	res.cpp_module_func += u8R"(
	return nullptr;
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE CUBE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* NODE_CLASS_NAME::GetTextureCube(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;

	return nullptr;
}
)";

	res.h_func = u8R"(
	vk::DescriptorImageInfo* GetTextureCube(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;)";

	res.include_interface = u8R"(
#include <Interfaces/TextureCubeOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotTextureCubeOutput.h>)";

	res.node_module_public_interface = u8R"(
	public TextureCubeOutputInterface,)";

	res.pass_public_interface = u8R"(
	public TextureCubeOutputInterface,)";

	res.node_slot_func += ct::toStr(u8R"(
	AddOutput(NodeSlotTextureCubeOutput::Create("%s", %u), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(), vSlot->descriptorBinding, vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotTextureGroupInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE GROUP SLOT INPUT ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetTextures(const uint32_t& vBindingPoint, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->SetTextures(vBindingPoint, vImageInfos, vOutSizes);
	})";
	}
	else
	{
		res.cpp_node_func += u8R"(
)";
	}

	res.cpp_node_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE GROUP SLOT INPUT ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void MODULE_CLASS_NAME::SetTextures(const uint32_t& vBindingPoint, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		m_PASS_CLASS_NAME_Ptr->SetTextures(vBindingPoint, vImageInfos, vOutSizes);
	})";
	}
	else
	{
		res.cpp_module_func += u8R"(
)";
	}

	res.cpp_module_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE GROUP SLOT INPUT ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetTextures(const uint32_t& vBindingPoint, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes)
{	
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBindingPoint < m_ImageInfos.size())
		{
			if (vImageInfos)
			{
				if (vOutSizes)
				{
					m_ImageGroupSizes[vBindingPoint] = *vOutSizes;
				}

				m_ImageGroups[vBindingPoint] = *vImageInfos;
			}
			else
			{
				m_ImageGroups[vBindingPoint] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
			}
		}
	}
}
)";

	res.h_func = u8R"(
	void SetTextures(const uint32_t& vBindingPoint, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes = nullptr) override;)";

	res.include_interface = u8R"(
#include <Interfaces/TextureGroupInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotTextureGroupInput.h>)";

	res.node_module_public_interface = u8R"(
	public TextureGroupInputInterface<0U>,)";

	res.pass_public_interface = ct::toStr(u8R"(
	public TextureGroupInputInterface,)", m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_TextureGroup]);

	res.node_slot_func += ct::toStr(u8R"(
	AddInput(NodeSlotTextureGroupInput::Create("%s"), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotTextureGroupOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE GROUP SLOT OUTPUT ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

DescriptorImageInfoVector* NODE_CLASS_NAME::GetDescriptorImageInfos(const uint32_t& vBindingPoint, fvec2Vector* vOutSizes)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		return m_MODULE_CLASS_NAMEPtr->GetDescriptorImageInfos(vBindingPoint, vOutSizes);
	}
)";
	}

	res.cpp_node_func += u8R"(
	return nullptr;
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE GROUP SLOT OUTPUT ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

DescriptorImageInfoVector* MODULE_CLASS_NAME::GetDescriptorImageInfos(const uint32_t& vBindingPoint, fvec2Vector* vOutSizes)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		return m_PASS_CLASS_NAME_Ptr->GetDescriptorImageInfos(vBindingPoint, vOutSizes);
	}
)";
	}

	res.cpp_module_func += u8R"(
	return nullptr;
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE GROUP SLOT OUTPUT ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

DescriptorImageInfoVector* NODE_CLASS_NAME::GetDescriptorImageInfos(const uint32_t& vBindingPoint, fvec2Vector* vOutSizes)
{	
	ZoneScoped;

	return nullptr;
}
)";

	res.h_func = u8R"(
	DescriptorImageInfoVector* GetDescriptorImageInfos(const uint32_t& vBindingPoint, fvec2Vector* vOutSizes = nullptr) override;)";

	res.include_interface = u8R"(
#include <Interfaces/TextureGroupOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotTextureGroupOutput.h>)";

	res.node_module_public_interface = u8R"(
	public TextureGroupOutputInterface,)";

	res.pass_public_interface = u8R"(
	public TextureGroupOutputInterface,)";

	res.node_slot_func += ct::toStr(u8R"(
	AddOutput(NodeSlotTextureGroupOutput::Create("%s"), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(), vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotVariableInput(NodeSlotInputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// VARIABLE SLOT INPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetVariable(const uint32_t& vVarIndex, SceneVariableWeak vSceneVariable)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->SetVariable(vVarIndex, vSceneVariable);
	})";
	}
	else
	{
		res.cpp_node_func += u8R"(
)";
	}

	res.cpp_node_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// VARIABLE SLOT INPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void MODULE_CLASS_NAME::SetVariable(const uint32_t& vVarIndex, SceneVariableWeak vSceneVariable)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		m_PASS_CLASS_NAME_Ptr->SetVariable(vVarIndex, vSceneVariable);
	})";
	}
	else
	{
		res.cpp_module_func += u8R"(
)";
	}

	res.cpp_module_func += u8R"(
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// VARIABLE SLOT INPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetVariable(const uint32_t& vVarIndex, SceneVariableWeak vSceneVariable)
{	
	ZoneScoped;

	if (vVarIndex < m_SceneVariables.size()))
	{
		m_SceneVariables[vVarIndex] = vSceneVariable;
	}
}
)";

	res.h_func = u8R"(
	void SetVariable(const uint32_t& vVarIndex, SceneVariableWeak vSceneVariable = SceneVariableWeak()) override;)";

	res.include_interface = u8R"(
#include <Interfaces/VariableInputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotVariableInput.h>)";

	res.node_module_public_interface = u8R"(
	public VariableInputInterface<0U>,)";

	res.pass_public_interface = ct::toStr(u8R"(
	public VariableInputInterface,)", m_InputSlotCounter[BaseTypeEnum::BASE_TYPE_Variable]);

	res.node_slot_func += ct::toStr(u8R"(
	AddInput(NodeSlotVariableInput::Create("%s", %s, %u), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(),
		vSlot->slotType.c_str(),
		vSlot->variableIndex,
		vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotVariableOutput(NodeSlotOutputPtr vSlot)
{
	SlotStringStruct res;

	////////////////////////////////////////////////////
	////// NODE ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_node_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// VARIABLE SLOT OUTPUT ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneVariableWeak NODE_CLASS_NAME::GetVariable(const uint32_t& vVariableIndex)
{	
	ZoneScoped;)";

	if (m_GenerateAModule)
	{
		res.cpp_node_func += u8R"(
	if (m_MODULE_CLASS_NAMEPtr)
	{
		return m_MODULE_CLASS_NAMEPtr->GetVariable(vVariableIndex)
	}
)";
	}

	res.cpp_node_func += u8R"(
	return SceneVariableWeak();
}
)";

	////////////////////////////////////////////////////
	////// MODULE //////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_module_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// VARIABLE SLOT OUTPUT ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneVariableWeak MODULE_CLASS_NAME::GetVariable(const uint32_t& vVariableIndex)
{	
	ZoneScoped;)";

	if (m_GenerateAPass)
	{
		res.cpp_module_func += u8R"(
	if (m_PASS_CLASS_NAME_Ptr)
	{
		return m_PASS_CLASS_NAME_Ptr->GetVariable(vVariableIndex)
	}
)";
	}

	res.cpp_module_func += u8R"(
	return SceneVariableWeak();
}
)";

	////////////////////////////////////////////////////
	////// PASS ////////////////////////////////////////
	////////////////////////////////////////////////////

	res.cpp_pass_func = u8R"(
//////////////////////////////////////////////////////////////////////////////////////////////
//// VARIABLE SLOT OUTPUT ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneVariableWeak NODE_CLASS_NAME::GetVariable(const uint32_t& vVariableIndex)
{	
	ZoneScoped;

	return SceneVariableWeak();
}
)";

	res.h_func = u8R"(
	SceneVariableWeak GetVariable(const uint32_t& vVariableIndex) override;)";

	res.include_interface = u8R"(
#include <Interfaces/VariableOutputInterface.h>)";

	res.include_slot = u8R"(
#include <Graph/Slots/NodeSlotVariableOutput.h>)";

	res.node_module_public_interface = u8R"(
	public VariableOutputInterface,)";

	res.pass_public_interface = u8R"(
	public VariableOutputInterface,)";

	res.node_slot_func += ct::toStr(u8R"(
	AddOutput(NodeSlotVariableOutput::Create("%s", %s, %u), false, %s);)",
		vSlot->hideName ? "" : vSlot->name.c_str(),
		vSlot->slotType.c_str(),
		vSlot->variableIndex,
		vSlot->hideName ? "true" : "false");

	return res;
}

SlotStringStruct GeneratorNode::GetSlotCustomInput(NodeSlotInputPtr vSlot)
{
	CTOOL_DEBUG_BREAK;

	SlotStringStruct res;

	res.cpp_node_func = u8R"()";
	res.cpp_module_func = u8R"()";
	res.cpp_pass_func = u8R"()";
	res.h_func = u8R"()";
	res.include_interface = u8R"()";
	res.include_slot = u8R"()";
	res.node_module_public_interface = u8R"()";
	res.pass_public_interface = u8R"()";
	res.node_slot_func += u8R"()";

	return res;
}

SlotStringStruct GeneratorNode::GetSlotCustomOutput(NodeSlotOutputPtr vSlot)
{
	CTOOL_DEBUG_BREAK;

	SlotStringStruct res;

	res.cpp_node_func = u8R"()";
	res.cpp_module_func = u8R"()";
	res.cpp_pass_func = u8R"()";
	res.h_func = u8R"()";
	res.include_interface = u8R"()";
	res.include_slot = u8R"()";
	res.node_module_public_interface = u8R"()";
	res.pass_public_interface = u8R"()";
	res.node_slot_func += u8R"()";

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// GENERATOR NODE SLOTS SUMMARY CODE ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string GeneratorNode::GetNodeSlotsInputFuncs(const SlotDico& vDico)
{
	std::string res;

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::INPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							for (auto item : vDico.at(type).at(NodeSlot::PlaceEnum::INPUT))
							{
								res += item.node_slot_func;
							}
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
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsOutputFuncs(const SlotDico& vDico)
{
	std::string res;

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::OUTPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							for (auto item : vDico.at(type).at(NodeSlot::PlaceEnum::OUTPUT))
							{
								res += item.node_slot_func;
							}
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
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsInputPublicInterfaces(const SlotDico& vDico)
{
	std::string res;

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::INPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							res += vDico.at(type).at(NodeSlot::PlaceEnum::INPUT)[0].node_module_public_interface;
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
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsOutputPublicInterfaces(const SlotDico& vDico)
{
	std::string res;

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::OUTPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							res += vDico.at(type).at(NodeSlot::PlaceEnum::OUTPUT)[0].node_module_public_interface;
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
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsInputIncludesSlots(const SlotDico& vDico)
{
	std::string res;

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::INPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							res += vDico.at(type).at(NodeSlot::PlaceEnum::INPUT)[0].include_slot;
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
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsOutputIncludesSlots(const SlotDico& vDico)
{
	std::string res;

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::OUTPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							res += vDico.at(type).at(NodeSlot::PlaceEnum::OUTPUT)[0].include_slot;
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
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsInputIncludesInterfaces(const SlotDico& vDico)
{
	std::string res;

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::INPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							res += vDico.at(type).at(NodeSlot::PlaceEnum::INPUT)[0].include_interface;
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
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsOutputIncludesInterfaces(const SlotDico& vDico)
{
	std::string res;

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::OUTPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							res += vDico.at(type).at(NodeSlot::PlaceEnum::OUTPUT)[0].include_interface;
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
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsInputCppFuncs(const SlotDico& vDico)
{
	std::string res;

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::INPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							res += vDico.at(type).at(NodeSlot::PlaceEnum::INPUT)[0].cpp_node_func;
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
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsOutputCppFuncs(const SlotDico& vDico)
{
	std::string res;

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::OUTPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							res += vDico.at(type).at(NodeSlot::PlaceEnum::OUTPUT)[0].cpp_node_func;
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
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsInputHFuncs(const SlotDico& vDico)
{
	std::string res;

	if (!m_Inputs.empty())
	{
		res += u8R"(
	// Interfaces Setters)";
	}

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::INPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							res += vDico.at(type).at(NodeSlot::PlaceEnum::INPUT)[0].h_func;
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
	}

	if (!m_Inputs.empty())
	{
		res += u8R"(
)";
	}

	return res;
}

std::string GeneratorNode::GetNodeSlotsOutputHFuncs(const SlotDico& vDico)
{
	std::string res;

	if (!m_Outputs.empty())
	{
		res += u8R"(
	// Interfaces Getters)";
	}

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::OUTPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							res += vDico.at(type).at(NodeSlot::PlaceEnum::OUTPUT)[0].h_func;
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
	}

	if (!m_Outputs.empty())
	{
		res += u8R"(
)";
	}

	return res;
}

std::string GeneratorNode::GetModuleInputCppFuncs(const SlotDico& vDico)
{
	std::string res;

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::INPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							res += vDico.at(type).at(NodeSlot::PlaceEnum::INPUT)[0].cpp_module_func;
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
	}

	return res;
}

std::string GeneratorNode::GetModuleOutputCppFuncs(const SlotDico& vDico)
{
	std::string res;

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::OUTPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							res += vDico.at(type).at(NodeSlot::PlaceEnum::OUTPUT)[0].cpp_module_func;
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
	}

	return res;
}

std::string GeneratorNode::GetPassInputPublicInterfaces(const SlotDico& vDico)
{
	std::string res;

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::INPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							res += vDico.at(type).at(NodeSlot::PlaceEnum::INPUT)[0].pass_public_interface;
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
	}

	return res;
}

std::string GeneratorNode::GetPassInputCppFuncs(const SlotDico& vDico)
{
	std::string res;

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::INPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							res += vDico.at(type).at(NodeSlot::PlaceEnum::INPUT)[0].cpp_pass_func;
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
	}

	return res;
}

std::string GeneratorNode::GetPassOutputCppFuncs(const SlotDico& vDico)
{
	std::string res;

	std::set<BaseTypeEnum> _alreadyKnowTypes;

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

				if (_alreadyKnowTypes.find(type) == _alreadyKnowTypes.end())
				{
					if (vDico.find(type) != vDico.end())
					{
						if (vDico.at(type).find(NodeSlot::PlaceEnum::OUTPUT) != vDico.at(type).end())
						{
							_alreadyKnowTypes.emplace(type);

							res += vDico.at(type).at(NodeSlot::PlaceEnum::OUTPUT)[0].cpp_pass_func;
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
	}

	return res;
}
