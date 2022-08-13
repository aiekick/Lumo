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
#include <Helper/Messaging.h>
#include <ctools/FileHelper.h>
#include <Systems/CommonSystem.h>
#include <Gui/MainFrame.h>

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
	}
}