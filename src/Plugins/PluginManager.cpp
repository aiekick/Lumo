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

#include "PluginManager.h"
#include <ctools/FileHelper.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <ctools/FileHelper.h>
#include <Systems/CommonSystem.h>
#include <ImWidgets/ImWidgets.h>
#include <filesystem>

namespace fs = std::filesystem;

#ifdef USE_STATIC_LINKING_OF_PLUGINS
#ifdef PROJECT_PLUGINS_INCLUDES
#include PROJECT_PLUGINS_INCLUDES
#define LOAD_PLUGIN(PLUGIN_NAME, PLUGIN_CLASS) AddPlugin(PLUGIN_NAME, std::make_shared<PLUGIN_CLASS>(), vVulkanCoreWeak);
#endif // PROJECT_PLUGINS_INCLUDES
#endif // USE_STATIC_LINKING_OF_PLUGINS

//////////////////////////////////////////////////////////////////////////////
////// PluginInstance ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

PluginInstance::PluginInstance()
{
	
}

PluginInstance::~PluginInstance()
{
	Unit();
}

bool PluginInstance::Init(vkApi::VulkanCoreWeak vVulkanCoreWeak, const std::string& vName, const std::string& vFilePathName)
{
	m_Name = vName;
	m_Loader = dlloader::DLLoader<PluginInterface>(vFilePathName);
	m_Loader.DLOpenLib();
	if (m_Loader.IsValid())
	{
		m_PluginInstance = m_Loader.DLGetInstance();
		if (m_PluginInstance)
		{
			if (!m_PluginInstance->Init(
				vVulkanCoreWeak,
				FileHelper::Instance(), 
				CommonSystem::Instance(),
				ImGui::GetCurrentContext(),
				ImGui::CustomStyle::Instance()))
			{
				m_PluginInstance.reset();
			}
			else
			{
				return true;
			}
		}
	}
	return false;
}

void PluginInstance::Unit()
{
	m_PluginInstance.reset();
	m_Loader.DLCloseLib();
}

PluginInterfaceWeak PluginInstance::Get()
{
	return m_PluginInstance;
}

//////////////////////////////////////////////////////////////////////////////
////// PluginLoader //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

static inline std::string GetDLLExtention()
{
#ifdef WIN32
	return "dll";
#endif
#ifdef __linux__
	return "so";
#endif
#ifdef __APPLE__
	return "dylib";
#endif
	return "";
}

void PluginManager::Clear()
{
	m_Plugins.clear();
}

void PluginManager::LoadPlugins(vkApi::VulkanCoreWeak vVulkanCoreWeak)
{
#ifndef USE_STATIC_LINKING_OF_PLUGINS
	auto plugin_directory = std::filesystem::path(FileHelper::Instance()->GetAppPath());
#ifndef _DEBUG
	plugin_directory.append("plugins");
#endif
	if (std::filesystem::exists(plugin_directory))
	{
		printf("-----------\n");
		printf("Availables Plugins :\n");

		const auto dir_iter = std::filesystem::directory_iterator(plugin_directory);
		for (const auto& file : dir_iter)
		{
			if (file.is_regular_file())
			{
				auto file_path_name = file.path().string();
				if (file_path_name.find(GetDLLExtention()) != std::string::npos)
				{
					auto ps = FileHelper::Instance()->ParsePathFileName(file_path_name);
					if (ps.isOk)
					{
						auto resPtr = std::make_shared<PluginInstance>();

						if (!resPtr->Init(vVulkanCoreWeak, ps.name, ps.GetFPNE()))
						{
							resPtr.reset();
							//LogVarDebug("Plugin %s fail to load", ps.name.c_str());
						}
						else
						{
							auto pluginInstancePtr = resPtr->Get().getValidShared();
							if (pluginInstancePtr)
							{
								char spaceBuffer[40 + 1] = "";
								spaceBuffer[0] = '\0';

								std::string name = pluginInstancePtr->GetName();
								if (name.size() < 15U)
								{
									size_t of = 15U - name.size();
									memset(spaceBuffer, 32, of); // 32 is space code in ASCII table
									spaceBuffer[of] = '\0';
									name += spaceBuffer;
								}
								else
								{
									name = name.substr(0, 15U);
								}

								std::string version = pluginInstancePtr->GetVersion();
								if (version.size() < 10U)
								{
									size_t of = 10U - version.size();
									memset(spaceBuffer, 32, of); // 32 is space code in ASCII table
									spaceBuffer[of] = '\0';
									version += spaceBuffer;
								}
								else
								{
									version = version.substr(0, 10U);
								}

								std::string desc = pluginInstancePtr->GetDescription();

								printf("Plugin loaded : %s v%s (%s)\n",
									name.c_str(),
									version.c_str(),
									desc.c_str());
							}

							m_Plugins[ps.name] = resPtr;
						}
					}
				}
			}
		}
	}
	else
	{
		LogVarLightInfo("Plugin directory %s not found !", plugin_directory.string().c_str());
	}
	printf("-----------\n");
#else // USE_STATIC_LINKING_OF_PLUGINS
	#ifdef LOAD_STATIC_PLUGINS
		LOAD_STATIC_PLUGINS
	#endif
#endif // USE_STATIC_LINKING_OF_PLUGINS
}

std::vector<LibraryEntry> PluginManager::GetLibraryEntrys()
{
	std::vector<LibraryEntry> res;

	for (auto plugin : m_Plugins)
	{
		if (plugin.second)
		{
#ifndef USE_STATIC_LINKING_OF_PLUGINS
			auto pluginInstancePtr = plugin.second->Get().getValidShared();
			if (pluginInstancePtr)
			{
				auto lib_entrys = pluginInstancePtr->GetLibrary();
				if (!lib_entrys.empty())
				{
					res.insert(res.end(), lib_entrys.begin(), lib_entrys.end());
				}
			}
#else // USE_STATIC_LINKING_OF_PLUGINS
			auto lib_entrys = plugin.second->GetLibrary();
			if (!lib_entrys.empty())
			{
				res.insert(res.end(), lib_entrys.begin(), lib_entrys.end());
			}
#endif // USE_STATIC_LINKING_OF_PLUGINS
		}
	}

	return res;
}

BaseNodePtr PluginManager::CreatePluginNode(const std::string& vPluginNodeName)
{
	if (!vPluginNodeName.empty())
	{
		for (auto plugin : m_Plugins)
		{
			if (plugin.second)
			{
#ifndef USE_STATIC_LINKING_OF_PLUGINS
				auto pluginInstancePtr = plugin.second->Get().getValidShared();
				if (pluginInstancePtr)
				{
					auto nodePtr = pluginInstancePtr->CreatePluginNode(vPluginNodeName);
					if (nodePtr)
					{
						return nodePtr;
					}
				}
#else // USE_STATIC_LINKING_OF_PLUGINS
				auto nodePtr = plugin.second->CreatePluginNode(vPluginNodeName);
				if (nodePtr)
				{
					return nodePtr;
				}
#endif // USE_STATIC_LINKING_OF_PLUGINS
			}
		}
	}
	return nullptr;
}

void PluginManager::ResetImGuiID(int vWidgetId)
{
	int id = vWidgetId;
	for (auto plugin : m_Plugins)
	{
		id += 10000;
		if (plugin.second)
		{
#ifndef USE_STATIC_LINKING_OF_PLUGINS
			auto pluginInstancePtr = plugin.second->Get().getValidShared();
			if (pluginInstancePtr)
			{
				id += pluginInstancePtr->ResetImGuiID(id);
			}
#else // USE_STATIC_LINKING_OF_PLUGINS
			id += plugin.second->ResetImGuiID(id);
#endif // USE_STATIC_LINKING_OF_PLUGINS
		}
	}
}

//////////////////////////////////////////////////////////////
//// PRIVATE /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

#ifndef USE_STATIC_LINKING_OF_PLUGINS

ct::cWeak<PluginInstance> PluginManager::Get(const std::string& vPluginName)
{
	if (!vPluginName.empty())
	{
		if (m_Plugins.find(vPluginName) != m_Plugins.end())
		{
			return m_Plugins.at(vPluginName);
		}
	}
	return ct::cWeak<PluginInstance>();
}

#else // USE_STATIC_LINKING_OF_PLUGINS

void PluginManager::AddPlugin(
	const std::string& vPluginName,
	PluginInterfacePtr vPluginPtr,
	vkApi::VulkanCoreWeak vVulkanCoreWeak)
{
	if (!vPluginPtr->Init(
		vVulkanCoreWeak,
		nullptr,
		nullptr,
		nullptr,
		nullptr))
	{
		vPluginPtr.reset();
	}
	else
	{
		char spaceBuffer[40 + 1] = "";
		spaceBuffer[0] = '\0';

		std::string name = vPluginPtr->GetName();
		if (name.size() < 15U)
		{
			size_t of = 15U - name.size();
			memset(spaceBuffer, 32, of); // 32 is space code in ASCII table
			spaceBuffer[of] = '\0';
			name += spaceBuffer;
		}
		else
		{
			name = name.substr(0, 15U);
		}

		std::string version = vPluginPtr->GetVersion();
		if (version.size() < 10U)
		{
			size_t of = 10U - version.size();
			memset(spaceBuffer, 32, of); // 32 is space code in ASCII table
			spaceBuffer[of] = '\0';
			version += spaceBuffer;
		}
		else
		{
			version = version.substr(0, 10U);
		}

		std::string desc = vPluginPtr->GetDescription();

		printf("Plugin loaded : %s v%s (%s)\n",
			name.c_str(),
			version.c_str(),
			desc.c_str());

		m_Plugins[vPluginName] = vPluginPtr;
	}
}

#endif // USE_STATIC_LINKING_OF_PLUGINS