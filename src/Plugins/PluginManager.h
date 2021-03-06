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

#include <string>
#include <memory>

#include <ctools/cTools.h>

#ifdef WIN32
#include <IDLLoader/Windows/DLLoader.h>
#else
#include <IDLLoader/Unix/DLLoader.h>
#endif

#include <Interfaces/PluginInterface.h>

class PluginInstance;
typedef ct::cWeak<PluginInstance> PluginInstanceWeak;
typedef std::shared_ptr<PluginInstance> PluginInstancePtr;

class PluginInstance
{
private:
	dlloader::DLLoader<PluginInterface> m_Loader;
	PluginInterfacePtr m_PluginInstance = nullptr;
	std::string m_Name;

public:
	PluginInstance();
	~PluginInstance();

	bool Init(vkApi::VulkanCoreWeak vVulkanCoreWeak, const std::string& vName, const std::string& vFilePathName);
	void Unit();

	PluginInterfaceWeak Get();
};

class PluginManager
{
private:
#ifndef USE_PLUGIN_STATIC_LINKING
	std::map<std::string, PluginInstancePtr> m_Plugins;
#else // USE_PLUGIN_STATIC_LINKING
	std::map<std::string, PluginInterfacePtr> m_Plugins;
#endif // USE_PLUGIN_STATIC_LINKING

public:
	void LoadPlugins(vkApi::VulkanCoreWeak vVulkanCorePtr);
	std::vector<LibraryEntry> GetLibraryEntrys();
	BaseNodePtr CreatePluginNode(const std::string& vPluginNodeName);
	void ResetImGuiID(int vWidgetId);
	void Clear();

private:
#ifndef USE_PLUGIN_STATIC_LINKING
	PluginInstanceWeak Get(const std::string& vPluginName);
#else // USE_PLUGIN_STATIC_LINKING
	void AddPlugin(
		const std::string& vPluginName,
		PluginInterfacePtr vPluginPtr,
		vkApi::VulkanCoreWeak vVulkanCoreWeak);
#endif

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