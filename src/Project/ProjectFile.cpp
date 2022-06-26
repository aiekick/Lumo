/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "ProjectFile.h"
#include <Panes/GraphPane.h>
#include <Helper/Messaging.h>
#include <ctools/FileHelper.h>
#include <Systems/CommonSystem.h>
#include <Graph/Manager/NodeManager.h>
#include <Panes/Manager/LayoutManager.h>

ProjectFile::ProjectFile() = default;

ProjectFile::ProjectFile(const std::string& vFilePathName)
{
	m_ProjectFilePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
	auto ps = FileHelper::Instance()->ParsePathFileName(m_ProjectFilePathName);
	if (ps.isOk)
	{
		m_ProjectFileName = ps.name;
		m_ProjectFilePath = ps.path;
	}
}

ProjectFile::~ProjectFile() = default;

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
	NodeManager::Instance()->FinalizeGraphLoading();
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
		NodeManager::Instance()->Clear();
		NodeManager::Instance()->PrepareToLoadGraph();

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

	str += CommonSystem::Instance()->getXml(vOffset + "\t\t", "project");
	str += NodeManager::Instance()->getXml(vOffset + "\t\t", "project");

	str += vOffset + "\t</scene>\n"; 
	
	str += LayoutManager::Instance()->getXml(vOffset + "\t", "project");

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

	LayoutManager::Instance()->setFromXml(vElem, vParent, "project");

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
	else if (strName == "CommonSystem")
	{
		CommonSystem::Instance()->RecursParsingConfigChilds(vElem, "project");
	}
	else if (strName == "graph")
	{
		NodeManager::Instance()->RecursParsingConfigChilds(vElem, "project");
	}

	return false;
}

