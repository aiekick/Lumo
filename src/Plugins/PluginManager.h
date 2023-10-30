/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

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

#include <string>
#include <memory>
#include <filesystem>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>
#include <ImGuiPack.h>

#ifdef WIN32
#include <IDLLoader/Windows/DLLoader.h>
#else
#include <IDLLoader/Unix/DLLoader.h>
#endif

#include <LumoBackend/Interfaces/PluginInterface.h>

class PluginInstance;
typedef std::weak_ptr<PluginInstance> PluginInstanceWeak;
typedef std::shared_ptr<PluginInstance> PluginInstancePtr;

enum class PluginReturnMsg { LOADING_SUCCEED = 1, LOADING_FAILED = 0, NOT_A_PLUGIN = -1 };

class PluginInstance
{
private:
	dlloader::DLLoader<PluginInterface> m_Loader;
	PluginInterfacePtr m_PluginInstance = nullptr;
	std::string m_Name;

public:
	PluginInstance();
	~PluginInstance();

	PluginReturnMsg Init(
        GaiApi::VulkanCoreWeak vVulkanCoreWeak, const std::string& vName, const std::string& vFilePathName);
	void Unit();

	PluginInterfaceWeak Get();
};

class PluginManager : public conf::ConfigAbstract
{
private:
	std::map<std::string, PluginInstancePtr> m_Plugins;

public:
	void LoadPlugins(GaiApi::VulkanCoreWeak vVulkanCore);
	std::vector<LibraryEntry> GetLibraryEntrys();
	BaseNodePtr CreatePluginNode(const std::string& vPluginNodeName);
	std::vector<PluginPaneConfig> GetPluginsPanes();
	void ResetImGuiID(int vWidgetId);
    void Clear();
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
    

private:
    void m_LoadPlugin(const std::filesystem::directory_entry& vEntry, GaiApi::VulkanCoreWeak vVulkanCore);
    void m_DisplayLoadedPlugins();

public:
	static PluginManager* Instance()
	{
		static PluginManager _instance;
		return &_instance;
	}

protected:
	PluginManager() = default; // Prevent construction
	PluginManager(const PluginManager&) = default; // Prevent construction by copying
	PluginManager& operator =(const PluginManager&) { return *this; }; // Prevent assignment
	~PluginManager() = default; // Prevent unwanted destruction
};