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

#pragma once

#include <ctools/ConfigAbstract.h>
#include <imgui/imgui.h>
#include <string>
#include <memory>

class ProjectFile : public conf::ConfigAbstract
{
private: // to save
	std::string m_ProjectFilePathName;
	std::string m_ProjectFileName;
	std::string m_ProjectFilePath;

private: // dont save
	bool m_IsLoaded = false; // jsute pour avancer
	bool m_NeverSaved = false;
	bool m_IsThereAnyNotSavedChanged = false;

public:
	ProjectFile();
	explicit ProjectFile(const std::string& vFilePathName);
	~ProjectFile();

	void Clear();
	void New();
	void New(const std::string& vFilePathName);
	bool Load();
	bool LoadAs(const std::string vFilePathName); // ils wanted to not pass the adress for re open case
	bool Save();
	bool SaveTemporary();
	bool SaveAs(const std::string& vFilePathName);
	bool IsLoaded() const;
	bool IsNeverSaved() const;

	bool IsThereAnyNotSavedChanged() const;
	void SetProjectChange(bool vChange = true);

	std::string GetAbsolutePath(const std::string& vFilePathName) const;
	std::string GetRelativePath(const std::string& vFilePathName) const;
	std::string GetProjectFilepathName() const;

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

public:
	static ProjectFile* Instance()
	{
		static ProjectFile _instance;
		return &_instance;
	}
};

