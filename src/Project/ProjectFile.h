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

#pragma once

#include <ctools/ConfigAbstract.h>
#include <LumoBackend/Interfaces/ProjectInterface.h>
#include <string>
#include <memory>

class ProjectFile : public ProjectInterface, public conf::ConfigAbstract {
private: // to save
	std::string m_ProjectFilePathName;
	std::string m_ProjectFileName;
	std::string m_ProjectFilePath;

private: // dont save
	bool m_IsLoaded = false;
	bool m_NeverSaved = false;
	bool m_IsThereAnyChanges = false;
    bool m_WasJustSaved = false;
    size_t m_WasJustSavedFrameCounter = 0U; // the state of m_WasJustSaved will be keeped during two frames

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

    bool IsProjectLoaded() const override;
    bool IsProjectNeverSaved() const override;
    bool IsThereAnyProjectChanges() const override;
    void SetProjectChange(bool vChange = true) override;

    void NewFrame();
    bool WasJustSaved() override;

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

