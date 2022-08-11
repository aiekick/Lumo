#include "NodeGenerator.h"

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Graph/Base/NodeSlot.h>
#include <Graph/Base/BaseNode.h>
#include <Graph/GeneratorCommon.h>
#include <Graph/Base/NodeSlotInput.h>
#include <Graph/Base/NodeSlotOutput.h>
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

void NodeGenerator::GenerateNodeClasses(const std::string& vPath, const ProjectFile& vDatas)
{
	auto nodePtr = vDatas.m_SelectedNode.getValidShared();
	if (nodePtr)
	{
		std::string node_class_name = nodePtr->m_ClassName + "Node";
		std::string cpp_node_file_name = node_class_name + ".cpp";
		std::string h_node_file_name = node_class_name + ".h";

		std::string module_class_name = nodePtr->m_ClassName + "Module";
		std::string cpp_module_file_name = module_class_name + ".cpp";
		std::string h_module_file_name = module_class_name + ".h";
		
		std::string cpp_node_file_code;
		std::string h_node_file_code;
		std::string h_node_interfaces_code;

		cpp_node_file_code += GetLicenceHeader();
		h_node_file_code += GetLicenceHeader();
		cpp_node_file_code += GetPVSStudioHeader();

		h_node_file_code += u8R"(
#include <Graph/Graph.h>
#include <Graph/Base/BaseNode.h>
)";

		cpp_node_file_code += ct::toStr(u8R"(#include "%s.h"
)", node_class_name.c_str());

		if (nodePtr->m_GenerateAModule)
		{
			cpp_node_file_code += ct::toStr(u8R"(#include <Modules/%s/%s.h>
)", nodePtr->m_Category.c_str(), module_class_name.c_str());
			GenerateModules(vPath, vDatas);
		}

		auto inputNodePtr = vDatas.m_SelectedNode.getValidShared();

		for (const auto& inputSlot : nodePtr->m_Inputs)
		{
			if (inputSlot.second)
			{
				auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(inputSlot.second);
				if (slotDatasPtr)
				{
					auto typeString = m_BaseTypes.m_TypeArray[slotDatasPtr->editorSlotTypeIndex];
					if (typeString == "None" ||
						typeString == "Custom")
					{
						continue;
					}

					cpp_node_file_code += ct::toStr(u8R"(#include <Graph/Slots/NodeSlot%sInput.h>
)", typeString.c_str());

					h_node_file_code += ct::toStr(u8R"(#include <Interfaces/%sInputInterface.h>
)", typeString.c_str());
				}
			}
		}
		for (const auto& outputSlot : nodePtr->m_Outputs)
		{
			if (outputSlot.second)
			{
				auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(outputSlot.second);
				if (slotDatasPtr)
				{
					auto typeString = m_BaseTypes.m_TypeArray[slotDatasPtr->editorSlotTypeIndex];
					if (typeString == "None" ||
						typeString == "Custom")
					{
						continue;
					}

					cpp_node_file_code += ct::toStr(u8R"(#include <Graph/Slots/NodeSlot%sOutput.h>
)", typeString.c_str());

					h_node_file_code += ct::toStr(u8R"(#include <Interfaces/%sOutputInterface.h>
)", typeString.c_str());
				}
			}
		}

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
	m_NodeTypeString = "NODE_CREATION_NAME";
}

NODE_CLASS_NAME::~NODE_CLASS_NAME()
{
	Unit();
}		

//////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool NODE_CLASS_NAME::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "NODE_DISPLAY_NAME";
)";

		std::map<uint32_t, uint32_t> _inputCounter;

		for (const auto& inputSlot : nodePtr->m_Inputs)
		{
			if (inputSlot.second)
			{
				auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(inputSlot.second);
				if (slotDatasPtr)
				{
					_inputCounter[slotDatasPtr->editorSlotTypeIndex]++;
					switch (slotDatasPtr->editorSlotTypeIndex)
					{
					case BaseTypeEnum::BASE_TYPE_None: // None
						break;
					case BaseTypeEnum::BASE_TYPE_LightGroup: // LightGroup
						cpp_node_file_code += ct::toStr(u8R"(
	AddInput(NodeSlotLightGroupInput::Create("%s"), false, %s);)",
							inputSlot.second->name.c_str(), inputSlot.second->hideName ? "true" : "false");
						break;
					case BaseTypeEnum::BASE_TYPE_Model: // Model
						cpp_node_file_code += ct::toStr(u8R"(
	AddInput(NodeSlotModelInput::Create("%s"), false, %s);)",
							inputSlot.second->name.c_str(), inputSlot.second->hideName ? "true" : "false");
						break;
					case BaseTypeEnum::BASE_TYPE_StorageBuffer: // StorageBuffer
						cpp_node_file_code += ct::toStr(u8R"(
	AddInput(NodeSlotStorageBufferInput::Create("%s"), false, %s);)",
							inputSlot.second->name.c_str(), inputSlot.second->hideName ? "true" : "false");
						break;
					case BaseTypeEnum::BASE_TYPE_TexelBuffer: // TexelBuffer
						cpp_node_file_code += ct::toStr(u8R"(
	AddInput(NodeSlotTexelBufferInput::Create("%s"), false, %s);)",
							inputSlot.second->name.c_str(), inputSlot.second->hideName ? "true" : "false");
						break;
					case BaseTypeEnum::BASE_TYPE_Texture: // Texture
						cpp_node_file_code += ct::toStr(u8R"(
	AddInput(NodeSlotTextureInput::Create("%s", %u), false, %s);)",
							inputSlot.second->name.c_str(),
							inputSlot.second->descriptorBinding,
							inputSlot.second->hideName ? "true" : "false");
						break;
					case BaseTypeEnum::BASE_TYPE_TextureGroup: // TextureGroup
						cpp_node_file_code += ct::toStr(u8R"(
	AddInput(NodeSlotTextureGroupInput::Create("%s"), false, %s);)",
							inputSlot.second->name.c_str(), inputSlot.second->hideName ? "true" : "false");
						break;
					case BaseTypeEnum::BASE_TYPE_Variable: // Variable
						cpp_node_file_code += ct::toStr(u8R"(
	AddInput(NodeSlotVariableInput::Create("%s", %s, %u), false, %s);)",
							inputSlot.second->name.c_str(),
							inputSlot.second->slotType.c_str(),
							inputSlot.second->variableIndex,
							inputSlot.second->hideName ? "true" : "false");
						break;
					case BaseTypeEnum::BASE_TYPE_Custom: // Custom
						break;
					}
				}
			}
		}

		cpp_node_file_code += u8R"(

)";

		if (_inputCounter[BaseTypeEnum::BASE_TYPE_LightGroup] > 0)
			h_node_file_code += u8R"(
	public LightGroupInputInterface,)";
		if (_inputCounter[BaseTypeEnum::BASE_TYPE_Model] > 0)
			h_node_file_code += u8R"(
	public ModelInputInterface,)";
		if (_inputCounter[BaseTypeEnum::BASE_TYPE_StorageBuffer] > 0)
			h_node_file_code += u8R"(
	public StorageBufferInputInterface,)";
		if (_inputCounter[BaseTypeEnum::BASE_TYPE_TexelBuffer] > 0)
			h_node_file_code += u8R"(
	public TexelBufferInputInterface,)";
		if (_inputCounter[BaseTypeEnum::BASE_TYPE_Texture] > 0)
			h_node_file_code += ct::toStr(u8R"(
	public TextureInputInterface<%u>,)", _inputCounter[BaseTypeEnum::BASE_TYPE_Texture]);
		if (_inputCounter[BaseTypeEnum::BASE_TYPE_TextureGroup] > 0)
			h_node_file_code += u8R"(
	public TextureGroupInputInterface,)";
		if (_inputCounter[BaseTypeEnum::BASE_TYPE_Variable] > 0)
			h_node_file_code += u8R"(
	public VariableInputInterface,)";

		std::map<uint32_t, uint32_t> _outputCounter;
		for (const auto& outputSlot : nodePtr->m_Outputs)
		{
			if (outputSlot.second)
			{
				auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(outputSlot.second);
				if (slotDatasPtr)
				{
					_outputCounter[slotDatasPtr->editorSlotTypeIndex]++;
					switch (slotDatasPtr->editorSlotTypeIndex)
					{
					case BaseTypeEnum::BASE_TYPE_None: // None
						break;
					case BaseTypeEnum::BASE_TYPE_LightGroup: // LightGroup
						cpp_node_file_code += ct::toStr(u8R"(
	AddOutput(NodeSlotLightGroupOutput::Create("%s"), false, %s);)",
							outputSlot.second->name.c_str(), outputSlot.second->hideName ? "true" : "false");
						break;
					case BaseTypeEnum::BASE_TYPE_Model: // Model
						cpp_node_file_code += ct::toStr(u8R"(
	AddOutput(NodeSlotModelOutput::Create("%s"), false, %s);)",
							outputSlot.second->name.c_str(), outputSlot.second->hideName ? "true" : "false");
						break;
					case BaseTypeEnum::BASE_TYPE_StorageBuffer: // StorageBuffer
						cpp_node_file_code += ct::toStr(u8R"(
	AddOutput(NodeSlotStorageBufferOutput::Create("%s"), false, %s);)",
							outputSlot.second->name.c_str(), outputSlot.second->hideName ? "true" : "false");
						break;
					case BaseTypeEnum::BASE_TYPE_TexelBuffer: // TexelBuffer
						cpp_node_file_code += ct::toStr(u8R"(
	AddOutput(NodeSlotTexelBufferOutput::Create("%s"), false, %s);)",
							outputSlot.second->name.c_str(), outputSlot.second->hideName ? "true" : "false");
						break;
					case BaseTypeEnum::BASE_TYPE_Texture: // Texture
						cpp_node_file_code += ct::toStr(u8R"(
	AddOutput(NodeSlotTextureOutput::Create("%s", %u), false, %s);)",
							outputSlot.second->name.c_str(),
							outputSlot.second->descriptorBinding,
							outputSlot.second->hideName ? "true" : "false");
						break;
					case BaseTypeEnum::BASE_TYPE_TextureGroup: // TextureGroup
						cpp_node_file_code += ct::toStr(u8R"(
	AddOutput(NodeSlotTextureGroupOutput::Create("%s"), false, %s);)",
							outputSlot.second->name.c_str(), outputSlot.second->hideName ? "true" : "false");
						break;
					case BaseTypeEnum::BASE_TYPE_Variable: // Variable
						cpp_node_file_code += ct::toStr(u8R"(
	AddOutput(NodeSlotVariableOutput::Create("%s", %s, %u), false, %s);)",
							outputSlot.second->name.c_str(),
							outputSlot.second->slotType.c_str(),
							outputSlot.second->variableIndex,
							outputSlot.second->hideName ? "true" : "false");
						h_node_file_code += u8R"(public VariableOutputInterface,)";
						break;
					case BaseTypeEnum::BASE_TYPE_Custom: // Custom
						break;
					}
				}
			}
		}

		if (_outputCounter[BaseTypeEnum::BASE_TYPE_LightGroup] > 0)
			h_node_file_code += u8R"(
	public LightGroupOutputInterface,)";
		if (_outputCounter[BaseTypeEnum::BASE_TYPE_Model] > 0)
			h_node_file_code += u8R"(
	public ModelOutputInterface,)";
		if (_outputCounter[BaseTypeEnum::BASE_TYPE_StorageBuffer] > 0)
			h_node_file_code += u8R"(
	public StorageBufferOutputInterface,)";
		if (_outputCounter[BaseTypeEnum::BASE_TYPE_TexelBuffer] > 0)
			h_node_file_code += u8R"(
	public TexelBufferOutputInterface,)";
		if (_outputCounter[BaseTypeEnum::BASE_TYPE_Texture] > 0)
			h_node_file_code += u8R"(
	public TextureOutputInterface,)";
		if (_outputCounter[BaseTypeEnum::BASE_TYPE_TextureGroup] > 0)
			h_node_file_code += u8R"(
	public TextureGroupOutputInterface,)";
		if (_outputCounter[BaseTypeEnum::BASE_TYPE_Variable] > 0)
			h_node_file_code += u8R"(
	public VariableOutputInterface,)";


	cpp_node_file_code += u8R"(
	bool res = false;

	m_MODULE_CLASS_NAMEPtr = MODULE_CLASS_NAME::Create(vVulkanCorePtr);
	if (m_MODULE_CLASS_NAMEPtr)
	{
		res = true;
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TASK EXECUTE ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool NODE_CLASS_NAME::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	bool res = false;

	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureInputDescriptorImageInfos(m_Inputs);

	if (m_MODULE_CLASS_NAMEPtr)
	{
		res = m_MODULE_CLASS_NAMEPtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool NODE_CLASS_NAME::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_MODULE_CLASS_NAMEPtr)
	{
		return m_MODULE_CLASS_NAMEPtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void NODE_CLASS_NAME::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW NODE ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
{
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
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* NODE_CLASS_NAME::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_MODULE_CLASS_NAMEPtr)
	{
		return m_MODULE_CLASS_NAMEPtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string NODE_CLASS_NAME::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_MODULE_CLASS_NAMEPtr)
		{
			res += m_MODULE_CLASS_NAMEPtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool NODE_CLASS_NAME::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SHADER UPDATE ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NODE_CLASS_NAME::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_MODULE_CLASS_NAMEPtr)
	{
		m_MODULE_CLASS_NAMEPtr->UpdateShaders(vFiles);
	}
}
		)";

	h_node_file_code += u8R"(
{
public:
	static std::shared_ptr<NODE_CLASS_NAME> Create(vkApi::VulkanCorePtr vVulkanCorePtr);

private:
	std::shared_ptr<MODULE_CLASS_NAME> m_MODULE_CLASS_NAMEPtr = nullptr;

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

	h_node_file_code += u8R"(

	// Interface Setters
)";
	if (_inputCounter[BaseTypeEnum::BASE_TYPE_LightGroup] > 0)
		h_node_file_code += u8R"(
	void SetLightGroup(SceneLightGroupWeak vSceneLightGroup) override;)";
	if (_inputCounter[BaseTypeEnum::BASE_TYPE_Model] > 0)
		h_node_file_code += u8R"(
	void SetModel(SceneModelWeak vSceneModel) override;)";
	if (_inputCounter[BaseTypeEnum::BASE_TYPE_StorageBuffer] > 0)
		h_node_file_code += u8R"(
	void SetStorageBuffer(const uint32_t& vBindingPoint, vk::DescriptorBufferInfo* vStorageBuffer, uint32_t* vStorageBufferSize) override;)";
	if (_inputCounter[BaseTypeEnum::BASE_TYPE_TexelBuffer] > 0)
		h_node_file_code += u8R"(
	void SetTexelBuffer(const uint32_t& vBindingPoint, vk::Buffer* vTexelBuffer, ct::uvec2* vTexelBufferSize) override;
	void SetTexelBufferView(const uint32_t& vBindingPoint, vk::BufferView* vTexelBufferView, ct::uvec2* vTexelBufferSize) override)";
	if (_inputCounter[BaseTypeEnum::BASE_TYPE_Texture] > 0)
		h_node_file_code += u8R"(
	void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;)";
	if (_inputCounter[BaseTypeEnum::BASE_TYPE_TextureGroup] > 0)
		h_node_file_code += u8R"(
	void SetTextures(const uint32_t& vBindingPoint, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes) override;)";
	if (_inputCounter[BaseTypeEnum::BASE_TYPE_Variable] > 0)
		h_node_file_code += u8R"(
	void SetVariable(const uint32_t& vVarIndex, SceneVariableWeak vSceneVariable = SceneVariableWeak()) override;)";

	h_node_file_code += u8R"(

	// Interface Getters
)";

	if (_outputCounter[BaseTypeEnum::BASE_TYPE_LightGroup] > 0)
		h_node_file_code += u8R"(
	SceneLightGroupWeak GetLightGroup() override;)";
	if (_outputCounter[BaseTypeEnum::BASE_TYPE_Model] > 0)
		h_node_file_code += u8R"(
	SceneModelWeak GetModel() override;)";
	if (_outputCounter[BaseTypeEnum::BASE_TYPE_StorageBuffer] > 0)
		h_node_file_code += u8R"(
	vk::DescriptorBufferInfo* GetStorageBuffer(const uint32_t& vBindingPoint, uint32_t* vOutSize = nullptr) override;)";
	if (_outputCounter[BaseTypeEnum::BASE_TYPE_TexelBuffer] > 0)
		h_node_file_code += u8R"(
	vk::Buffer* GetTexelBuffer(const uint32_t& vBindingPoint, ct::uvec2* vOutSize = nullptr) override;
	vk::BufferView* GetTexelBufferView(const uint32_t& vBindingPoint, ct::uvec2* vOutSize = nullptr) override)";
	if (_outputCounter[BaseTypeEnum::BASE_TYPE_Texture] > 0)
		h_node_file_code += u8R"(
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;)";
	if (_outputCounter[BaseTypeEnum::BASE_TYPE_TextureGroup] > 0)
		h_node_file_code += u8R"(
	DescriptorImageInfoVector* GetDescriptorImageInfos(const uint32_t& vBindingPoint, fvec2Vector* vOutSizes) override;)";
	if (_outputCounter[BaseTypeEnum::BASE_TYPE_Variable] > 0)
		h_node_file_code += u8R"(
	SceneVariableWeak GetVariable(const uint32_t& vVariableIndex) override;)";

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
		
		ct::replaceString(cpp_node_file_code, "NODE_CREATION_NAME", nodePtr->m_NodeCreationName);
		ct::replaceString(h_node_file_code, "NODE_CREATION_NAME", nodePtr->m_NodeCreationName);
		
		ct::replaceString(cpp_node_file_code, "NODE_DISPLAY_NAME", nodePtr->m_NodeDisplayName);
		ct::replaceString(h_node_file_code, "NODE_DISPLAY_NAME", nodePtr->m_NodeDisplayName);
		
		ct::replaceString(cpp_node_file_code, "MODULE_CLASS_NAME", module_class_name);
		ct::replaceString(h_node_file_code, "MODULE_CLASS_NAME", module_class_name);

		FileHelper::Instance()->SaveStringToFile(cpp_node_file_code, "test.cpp");
		FileHelper::Instance()->SaveStringToFile(h_node_file_code, "test.h");
	}
}

void NodeGenerator::GenerateModules(const std::string& vPath, const ProjectFile& vDatas)
{

}

void NodeGenerator::GeneratePasses(const std::string& vPath, const ProjectFile& vDatas)
{

}

std::string NodeGenerator::GetLicenceHeader()
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

std::string NodeGenerator::GetPVSStudioHeader()
{
	return 
u8R"(
// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
)";
}