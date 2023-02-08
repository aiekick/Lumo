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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "ProjectFile.h"

#include <filesystem>
#include <Gui/MainFrame.h>
#include <Helper/Messaging.h>
#include <ctools/FileHelper.h>
#include <Systems/CommonSystem.h>

namespace fs = std::filesystem;

ProjectFile::ProjectFile(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	m_RootNodePtr = GeneratorNode::Create(vVulkanCorePtr);
}

ProjectFile::ProjectFile(vkApi::VulkanCorePtr vVulkanCorePtr, const std::string& vFilePathName)
{
	m_RootNodePtr = GeneratorNode::Create(vVulkanCorePtr);

	m_ProjectFilePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
	auto ps = FileHelper::Instance()->ParsePathFileName(m_ProjectFilePathName);
	if (ps.isOk)
	{
		m_ProjectFileName = ps.name;
		m_ProjectFilePath = ps.path;
	}
}

ProjectFile::~ProjectFile()
{
	m_RootNodePtr.reset();
}

void ProjectFile::Clear()
{
	m_ProjectFilePathName.clear();
	m_ProjectFileName.clear();
	m_ProjectFilePath.clear();
	m_IsLoaded = false;
	m_IsThereAnyNotSavedChanged = false;
	Messaging::Instance()->Clear();
}

void ProjectFile::New()
{
	Clear();
	m_IsLoaded = true;
	m_NeverSaved = true;
	SetProjectChange(true);
}

void ProjectFile::New(const std::string& vFilePathName)
{
	Clear();
	m_ProjectFilePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
	auto ps = FileHelper::Instance()->ParsePathFileName(m_ProjectFilePathName);
	if (ps.isOk)
	{
		m_ProjectFileName = ps.name;
		m_ProjectFilePath = ps.path;
	}
	m_IsLoaded = true;
	SetProjectChange(false);

	// then load
	m_RootNodePtr->FinalizeGraphLoading();
}

bool ProjectFile::Load()
{
	return LoadAs(m_ProjectFilePathName);
}

// ils wanted to not pass the adress for re open case
// elwse, the clear will set vFilePathName to empty because with re open, target m_ProjectFilePathName
bool ProjectFile::LoadAs(const std::string vFilePathName)  
{
	if (!vFilePathName.empty())
	{
		m_RootNodePtr->ClearGraph();
		nd::SetCurrentEditor(
			m_RootNodePtr->m_BaseNodeState.m_NodeGraphContext);

		std::string filePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
		tinyxml2::XMLError xmlError = LoadConfigFile(filePathName);
		if (xmlError == tinyxml2::XMLError::XML_SUCCESS)
		{
			New(filePathName);
		}
		else
		{
			Clear();

			auto errMsg = getTinyXml2ErrorMessage(xmlError);
			Messaging::Instance()->AddError(true, nullptr, nullptr,
				"The project file %s cant be loaded, Error : %s", filePathName.c_str(), errMsg.c_str());
		}
	}

	return m_IsLoaded;
}

bool ProjectFile::Save()
{
	if (m_NeverSaved) 
		return false;

	if (SaveConfigFile(m_ProjectFilePathName))
	{
		SetProjectChange(false);
		return true;
	}
	
	return false;
}

bool ProjectFile::SaveTemporary()
{
	if (m_NeverSaved)
		return false;

	auto ps = FileHelper::Instance()->ParsePathFileName(m_ProjectFilePathName);
	if (ps.isOk)
	{
		auto ps_tmp = ps.GetFPNE_WithName(ps.name + "_tmp");
		if (SaveConfigFile(ps_tmp))
		{
			SetProjectChange(false);
			return true;
		}
	}

	return false;
}

bool ProjectFile::SaveAs(const std::string& vFilePathName)
{
	std::string filePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
	auto ps = FileHelper::Instance()->ParsePathFileName(filePathName);
	if (ps.isOk)
	{
		m_ProjectFilePathName = FileHelper::Instance()->ComposePath(ps.path, ps.name, "blt");
		m_ProjectFilePath = ps.path;
		m_NeverSaved = false;
		return Save();
	}
	return false;
}

bool ProjectFile::IsLoaded() const
{
	return m_IsLoaded;
}

bool ProjectFile::IsNeverSaved() const
{
	return m_NeverSaved;
}

bool ProjectFile::IsThereAnyNotSavedChanged() const
{
	return m_IsThereAnyNotSavedChanged;
}

void ProjectFile::SetProjectChange(bool vChange)
{
	m_IsThereAnyNotSavedChanged = vChange;
}

std::string ProjectFile::GetAbsolutePath(const std::string& vFilePathName) const
{
	std::string res = vFilePathName;

	if (!vFilePathName.empty())
	{
		if (!FileHelper::Instance()->IsAbsolutePath(vFilePathName)) // relative
		{
			res = FileHelper::Instance()->SimplifyFilePath(
				m_ProjectFilePath + FileHelper::Instance()->puSlashType + vFilePathName);
		}
	}

	return res;
}

std::string ProjectFile::GetRelativePath(const std::string& vFilePathName) const
{
	std::string res = vFilePathName;

	if (!vFilePathName.empty())
	{
		res = FileHelper::Instance()->GetRelativePathToPath(vFilePathName, m_ProjectFilePath);
	}

	return res;
}

std::string ProjectFile::GetProjectFilepathName() const
{
	return m_ProjectFilePathName;
}

std::string ProjectFile::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<project>\n";

	str += vOffset + "\t<scene>\n";

	str += m_RootNodePtr->getXml(vOffset + "\t\t", "project");

	str += vOffset + "\t</scene>\n";

	str += vOffset + ct::toStr("\t<root_path>%s</root_path>\n", m_GenerationRootPath.c_str());

	str += vOffset + "</project>\n";

	return str;
}

bool ProjectFile::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strName == "config")
	{
		return true;
	}
	else if (strName == "project")
	{
		return true;
	}
	else if (strName == "scene")
	{
		return true;
	}
	else if (strName == "generation")
	{
		return true;
	}
	else if (strName == "graph")
	{
		m_RootNodePtr->RecursParsingConfigChilds(vElem, "project");
	}
	else if (strParentName == "project")
	{
		if (strName == "root_path")
			m_GenerationRootPath = strValue;
	}

	return false;
}

void ProjectFile::GenerateGraphFiles(const std::string& vRootPath)
{
	if (m_RootNodePtr)
	{
		m_GenerationRootPath = vRootPath;

		for (auto node : m_RootNodePtr->m_ChildNodes)
		{
			if (node.second)
			{
				auto genNodePtr = std::dynamic_pointer_cast<GeneratorNode>(node.second);
				if (genNodePtr)
				{
					genNodePtr->GenerateNodeClasses(m_GenerationRootPath, this);
				}
			}
		}

		std::string scene_graph_class_name = MainFrame::Instance()->GetCustomTypeInputText().GetText();
		if (!scene_graph_class_name.empty())
		{
			CustomSceneGraphItem(vRootPath, scene_graph_class_name);
		}
	}
}

void ProjectFile::CustomSceneGraphItem(const std::string& vRootPath, const std::string& vSceneGraphItemName)
{
	fs::path scene_graph_path = vRootPath + "/SceneGraph/";
	if (!std::filesystem::exists(scene_graph_path))
		fs::create_directory(scene_graph_path);

	std::string maj_scene_graph_item_name = ct::toUpper(vSceneGraphItemName);

	//////////////////////////////////////
	//// SCENEGRAPH //////////////////////
	//////////////////////////////////////

	std::string scene_graph_cpp_file_name = scene_graph_path.string() + vSceneGraphItemName + ".cpp";
	std::string scene_graph_cpp_file_code = GeneratorNode::GetLicenceHeader();

	std::string scene_graph_h_file_name = scene_graph_path.string() + vSceneGraphItemName + ".h";
	std::string scene_graph_h_file_code = GeneratorNode::GetLicenceHeader();

	scene_graph_cpp_file_code += GeneratorNode::GetPVSStudioHeader();

	scene_graph_cpp_file_code += u8R"(

#include <SceneGraph/%s.h>

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

%sPtr %s::Create()
{
	auto res = std::make_shared<%s>();
	res->m_This = res;
	return res;
}

//////////////////////////////////////////////////////////////
//// PUBLIC : BUILD / CLEAR //////////////////////////////////
//////////////////////////////////////////////////////////////

%s::%s()
{

}

%s::~%s()
{
	Clear();
}

void %s::Clear()
{

}

bool %s::IsOk() const
{
	return true;
}

)";
	ct::replaceString(scene_graph_cpp_file_code, "%s", vSceneGraphItemName);
	ct::replaceString(scene_graph_cpp_file_code, "%S", maj_scene_graph_item_name);
	FileHelper::Instance()->SaveStringToFile(scene_graph_cpp_file_code, scene_graph_cpp_file_name);

	scene_graph_h_file_code += u8R"(

#pragma once

#include <vector>
#include <ctools/cTools.h>

class %s;
typedef std::shared_ptr<%s> %sPtr;
typedef ct::cWeak<%s> %sWeak;

// NotifyEvent : need to update the accel structure
#define %sUpdateDone "%sUpdateDone"

class %s
{
public:
	static %sPtr Create();

private:
	%sWeak m_This;

public:
	%s();
	~%s();
	void Clear();
	bool IsOk() const;
};
)";
	ct::replaceString(scene_graph_h_file_code, "%s", vSceneGraphItemName);
	ct::replaceString(scene_graph_h_file_code, "%S", maj_scene_graph_item_name);
	FileHelper::Instance()->SaveStringToFile(scene_graph_h_file_code, scene_graph_h_file_name);

	//////////////////////////////////////
	//// INTERFACES //////////////////////
	//////////////////////////////////////

	fs::path scene_interface_path = vRootPath + "/Interfaces/";
	if (!std::filesystem::exists(scene_interface_path))
		fs::create_directory(scene_interface_path);

	std::string scene_input_interface_h_file_name = scene_interface_path.string() + vSceneGraphItemName + "InputInterface.h";
	std::string scene_input_interface_h_file_code = GeneratorNode::GetLicenceHeader();
	
	std::string scene_output_interface_h_file_name = scene_interface_path.string() + vSceneGraphItemName + "OutputInterface.h";
	std::string scene_output_interface_h_file_code = GeneratorNode::GetLicenceHeader();

	scene_input_interface_h_file_code += u8R"(
#pragma once

#include <SceneGraph/%s.h>

#include <map>
#include <string>

class %sInputInterface
{
protected:
	std::map<std::string, %sWeak> m_%ss;

public:
	virtual void Set%s(const std::string& vName, %sWeak v%s) = 0;
};
)";
	ct::replaceString(scene_input_interface_h_file_code, "%s", vSceneGraphItemName);
	ct::replaceString(scene_input_interface_h_file_code, "%S", maj_scene_graph_item_name);
	FileHelper::Instance()->SaveStringToFile(scene_input_interface_h_file_code, scene_input_interface_h_file_name);

	scene_output_interface_h_file_code += u8R"(
#pragma once

#include <SceneGraph/%s.h>
#include <string>

class %sOutputInterface
{
public:
	virtual %sWeak Get%s(const std::string& vName) = 0;
};
)";
	ct::replaceString(scene_output_interface_h_file_code, "%s", vSceneGraphItemName);
	ct::replaceString(scene_output_interface_h_file_code, "%S", maj_scene_graph_item_name);
	FileHelper::Instance()->SaveStringToFile(scene_output_interface_h_file_code, scene_output_interface_h_file_name);

	//////////////////////////////////////
	//// SLOTS ///////////////////////////
	//////////////////////////////////////

	fs::path scene_slot_path = vRootPath + "/Slots/";
	if (!std::filesystem::exists(scene_slot_path))
		fs::create_directory(scene_slot_path);

	std::string scene_input_slot_cpp_file_name = scene_slot_path.string() + "NodeSlot" + vSceneGraphItemName + "Input.cpp";
	std::string scene_input_slot_h_file_name = scene_slot_path.string() + "NodeSlot" + vSceneGraphItemName + "Input.h";
	
	std::string scene_output_slot_cpp_file_name = scene_slot_path.string() + "NodeSlot" + vSceneGraphItemName + "Output.cpp";
	std::string scene_output_slot_h_file_name = scene_slot_path.string() + "NodeSlot" + vSceneGraphItemName + "Output.h";
	
	std::string scene_input_slot_cpp_file_code = GeneratorNode::GetLicenceHeader() + GeneratorNode::GetPVSStudioHeader();
	scene_input_slot_cpp_file_code += u8R"(
#include "NodeSlot%sInput.h"

#include <utility>
#include <SceneGraph/%s.h>
#include <Graph/Base/BaseNode.h>
#include <Interfaces/%sInputInterface.h>
#include <Interfaces/%sOutputInterface.h>

static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlot%sInputPtr NodeSlot%sInput::Create(NodeSlot%sInput vSlot)
{
	auto res = std::make_shared<NodeSlot%sInput>(vSlot);
	res->m_This = res;
	return res;
}

NodeSlot%sInputPtr NodeSlot%sInput::Create(const std::string& vName)
{
	auto res = std::make_shared<NodeSlot%sInput>(vName);
	res->m_This = res;
	return res;
}

NodeSlot%sInputPtr NodeSlot%sInput::Create(const std::string& vName, const bool& vHideName)
{
	auto res = std::make_shared<NodeSlot%sInput>(vName, vHideName);
	res->m_This = res;
	return res;
}

NodeSlot%sInputPtr NodeSlot%sInput::Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget)
{
	auto res = std::make_shared<NodeSlot%sInput>(vName, vHideName, vShowWidget);
	res->m_This = res;
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC CLASS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlot%sInput::NodeSlot%sInput()
	: NodeSlotInput("", "%S")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlot%sInput::NodeSlot%sInput(const std::string& vName)
	: NodeSlotInput(vName, "%S")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlot%sInput::NodeSlot%sInput(const std::string& vName, const bool& vHideName)
	: NodeSlotInput(vName, "%S", vHideName)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlot%sInput::NodeSlot%sInput(const std::string& vName, const bool& vHideName, const bool& vShowWidget)
	: NodeSlotInput(vName, "%S", vHideName, vShowWidget)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlot%sInput::~NodeSlot%sInput() = default;

void NodeSlot%sInput::Init()
{
	
}

void NodeSlot%sInput::Unit()
{
	// ici pas besoin du assert sur le m_This 
	// car NodeSlot%sInput peut etre isntancié à l'ancienne en copie local donc sans shared_ptr
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
						graphPtr->DisConnectSlot(m_This);
					}
				}
			}
		}
	}
}

void NodeSlot%sInput::Connect(NodeSlotWeak vOtherSlot)
{
	auto endSlotPtr = vOtherSlot.getValidShared();
	if (endSlotPtr)
	{
		auto parentNodePtr = dynamic_pointer_cast<%sInputInterface>(parentNode.getValidShared());
		if (parentNodePtr)
		{
			auto otherCodeNodePtr = dynamic_pointer_cast<%sOutputInterface>(endSlotPtr->parentNode.getValidShared());
			if (otherCodeNodePtr)
			{
				parentNodePtr->Set%s(name,
					otherCodeNodePtr->Get%s(endSlotPtr->name));
			}
		}
	}
}

void NodeSlot%sInput::DisConnect(NodeSlotWeak vOtherSlot)
{
	auto endSlotPtr = vOtherSlot.getValidShared();
	if (endSlotPtr)
	{
		auto parentNodePtr = dynamic_pointer_cast<%sInputInterface>(parentNode.getValidShared());
		if (parentNodePtr)
		{
			parentNodePtr->Set%s(name, %sWeak());
		}
	}
}

void NodeSlot%sInput::TreatNotification(
	NotifyEvent vEvent,
	const NodeSlotWeak& vEmitterSlot,
	const NodeSlotWeak& vReceiverSlot)
{
	if (vEvent == %sUpdateDone)
	{
		auto emiterSlotPtr = vEmitterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto parentCodeInputNodePtr = dynamic_pointer_cast<%sInputInterface>(parentNode.getValidShared());
				if (parentCodeInputNodePtr)
				{
					auto otherNodePtr = dynamic_pointer_cast<%sOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
					if (otherNodePtr)
					{
						auto receiverSlotPtr = vReceiverSlot.getValidShared();
						if (receiverSlotPtr)
						{
							parentCodeInputNodePtr->Set%s(receiverSlotPtr->name,
								otherNodePtr->Get%s(emiterSlotPtr->name));
						}
					}
				}
			}
		}
	}
}

void NodeSlot%sInput::DrawDebugInfos()
{
	ImGui::Text("--------------------");
	ImGui::Text("Slot %s", name.c_str());
	ImGui::Text(IsAnInput() ? "Input" : "Output");
	ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}
)";
	ct::replaceString(scene_input_slot_cpp_file_code, "%s", vSceneGraphItemName);
	ct::replaceString(scene_input_slot_cpp_file_code, "%S", maj_scene_graph_item_name);
	FileHelper::Instance()->SaveStringToFile(scene_input_slot_cpp_file_code, scene_input_slot_cpp_file_name);

	std::string scene_input_slot_h_file_code = GeneratorNode::GetLicenceHeader();
	scene_input_slot_h_file_code += u8R"(
#pragma once

#include <Graph/Graph.h>
#include <Graph/Base/NodeSlotInput.h>

class NodeSlot%sInput;
typedef ct::cWeak<NodeSlot%sInput> NodeSlot%sInputWeak;
typedef std::shared_ptr<NodeSlot%sInput> NodeSlot%sInputPtr;

class NodeSlot%sInput : 
	public NodeSlotInput
{
public:
	static NodeSlot%sInputPtr Create(NodeSlot%sInput vSlot);
	static NodeSlot%sInputPtr Create(const std::string& vName);
	static NodeSlot%sInputPtr Create(const std::string& vName, const bool& vHideName);
	static NodeSlot%sInputPtr Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget);

public:
	explicit NodeSlot%sInput();
	explicit NodeSlot%sInput(const std::string& vName);
	explicit NodeSlot%sInput(const std::string& vName, const bool& vHideName);
	explicit NodeSlot%sInput(const std::string& vName, const bool& vHideName, const bool& vShowWidget);
	~NodeSlot%sInput();

	void Init();
	void Unit();

	void Connect(NodeSlotWeak vOtherSlot) override;
	void DisConnect(NodeSlotWeak vOtherSlot) override;

	void TreatNotification(
		NotifyEvent vEvent,
		const NodeSlotWeak& vEmitterSlot = NodeSlotWeak(),
		const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());

	void DrawDebugInfos();
};
)";
	ct::replaceString(scene_input_slot_h_file_code, "%s", vSceneGraphItemName);
	ct::replaceString(scene_input_slot_h_file_code, "%S", maj_scene_graph_item_name);
	FileHelper::Instance()->SaveStringToFile(scene_input_slot_h_file_code, scene_input_slot_h_file_name);

	std::string scene_output_slot_cpp_file_code = GeneratorNode::GetLicenceHeader() + GeneratorNode::GetPVSStudioHeader();
	scene_output_slot_cpp_file_code += u8R"(
#include "NodeSlot%sOutput.h"

#include <utility>
#include <SceneGraph/%s.h>
#include <Graph/Base/BaseNode.h>

static const float slotIconSize = 15.0f;

//////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlot%sOutputPtr NodeSlot%sOutput::Create(NodeSlot%sOutput vSlot)
{
	auto res = std::make_shared<NodeSlot%sOutput>(vSlot);
	res->m_This = res;
	return res;
}

NodeSlot%sOutputPtr NodeSlot%sOutput::Create(const std::string& vName)
{
	auto res = std::make_shared<NodeSlot%sOutput>(vName);
	res->m_This = res;
	return res;
}

NodeSlot%sOutputPtr NodeSlot%sOutput::Create(const std::string& vName, const bool& vHideName)
{
	auto res = std::make_shared<NodeSlot%sOutput>(vName, vHideName);
	res->m_This = res;
	return res;
}

NodeSlot%sOutputPtr NodeSlot%sOutput::Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget)
{
	auto res = std::make_shared<NodeSlot%sOutput>(vName, vHideName, vShowWidget);
	res->m_This = res;
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// PUBIC CLASS /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

NodeSlot%sOutput::NodeSlot%sOutput()
	: NodeSlotOutput("", "%S")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlot%sOutput::NodeSlot%sOutput(const std::string& vName)
	: NodeSlotOutput(vName, "%S")
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlot%sOutput::NodeSlot%sOutput(const std::string& vName, const bool& vHideName)
	: NodeSlotOutput(vName, "%S", vHideName)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlot%sOutput::NodeSlot%sOutput(const std::string& vName, const bool& vHideName, const bool& vShowWidget)
	: NodeSlotOutput(vName, "%S", vHideName, vShowWidget)
{
	pinID = sGetNewSlotId();
	color = sGetSlotColors()->GetSlotColor(slotType);
	colorIsSet = true;
}

NodeSlot%sOutput::~NodeSlot%sOutput() = default;

void NodeSlot%sOutput::Init()
{
	
}

void NodeSlot%sOutput::Unit()
{
	// ici pas besoin du assert sur le m_This 
	// car NodeSlot%sOutput peut etre instancié à l'ancienne en copie local donc sans shared_ptr
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
						graphPtr->DisConnectSlot(m_This);
					}
				}
			}
		}
	}
}

void NodeSlot%sOutput::SendFrontNotification(const NotifyEvent& vEvent)
{
	if (vEvent == %sUpdateDone)
	{
		SendNotification(slotType, vEvent);
	}
}

void NodeSlot%sOutput::DrawDebugInfos()
{
	ImGui::Text("--------------------");
	ImGui::Text("Slot %s", name.c_str());
	ImGui::Text(IsAnInput() ? "Input" : "Output");
	ImGui::Text("Count connections : %u", (uint32_t)linkedSlots.size());
}
)";
	ct::replaceString(scene_output_slot_cpp_file_code, "%s", vSceneGraphItemName);
	ct::replaceString(scene_output_slot_cpp_file_code, "%S", maj_scene_graph_item_name);
	FileHelper::Instance()->SaveStringToFile(scene_output_slot_cpp_file_code, scene_output_slot_cpp_file_name);

	std::string scene_output_slot_h_file_code = GeneratorNode::GetLicenceHeader();
	scene_output_slot_h_file_code += u8R"(
#pragma once

#include <Graph/Graph.h>
#include <Graph/Base/NodeSlotOutput.h>

class NodeSlot%sOutput;
typedef ct::cWeak<NodeSlot%sOutput> NodeSlot%sOutputWeak;
typedef std::shared_ptr<NodeSlot%sOutput> NodeSlot%sOutputPtr;

class NodeSlot%sOutput : 
	public NodeSlotOutput
{
public:
	static NodeSlot%sOutputPtr Create(NodeSlot%sOutput vSlot);
	static NodeSlot%sOutputPtr Create(const std::string& vName);
	static NodeSlot%sOutputPtr Create(const std::string& vName, const bool& vHideName);
	static NodeSlot%sOutputPtr Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget);

public:
	explicit NodeSlot%sOutput();
	explicit NodeSlot%sOutput(const std::string& vName);
	explicit NodeSlot%sOutput(const std::string& vName, const bool& vHideName);
	explicit NodeSlot%sOutput(const std::string& vName, const bool& vHideName, const bool& vShowWidget);
	~NodeSlot%sOutput();

	void Init();
	void Unit();

	void SendFrontNotification(const NotifyEvent& vEvent) override;

	void DrawDebugInfos();
};

)";
	ct::replaceString(scene_output_slot_h_file_code, "%s", vSceneGraphItemName);
	ct::replaceString(scene_output_slot_h_file_code, "%S", maj_scene_graph_item_name);
	FileHelper::Instance()->SaveStringToFile(scene_output_slot_h_file_code, scene_output_slot_h_file_name);
}