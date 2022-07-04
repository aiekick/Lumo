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
}

ct::cWeak<PluginInstance> PluginManager::LoadPlugin(vkApi::VulkanCoreWeak vVulkanCoreWeak, const std::string& vPluginName)
{
	ct::cWeak<PluginInstance> res;

	if (!vPluginName.empty())
	{
		PathStruct ps;
		auto file_path_name = ps.GetFPNE_WithPathNameExt(FileHelper::Instance()->GetAppPath(), vPluginName, GetDLLExtention());

		if (fs::exists(file_path_name))
		{
			auto resPtr = std::make_shared<PluginInstance>();
			if (!resPtr->Init(vVulkanCoreWeak, vPluginName, file_path_name))
			{
				resPtr.reset();
				LogVarDebug("Plugin %s fail to load",
					vPluginName.c_str());
			}
			else
			{
				auto pluginInstancePtr = resPtr->Get().getValidShared();
				if (pluginInstancePtr)
				{
					LogVarLightInfo("Plugin %s %s loaded - (%s)",
						vPluginName.c_str(),
						pluginInstancePtr->GetVersion().c_str(),
						pluginInstancePtr->GetDescription().c_str());
				}

				m_Plugins[vPluginName] = resPtr;
				res = resPtr;
			}
		}
	}
	
	return res;
}

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

std::vector<LibraryEntry> PluginManager::GetLibraryEntrys()
{
	std::vector<LibraryEntry> res;

	for (auto plugin : m_Plugins)
	{
		if (plugin.second)
		{
			auto pluginInstancePtr = plugin.second->Get().getValidShared();
			if (pluginInstancePtr)
			{
				auto lib_entrys = pluginInstancePtr->GetLibrary();
				if (!lib_entrys.empty())
				{
					res.insert(res.end(), lib_entrys.begin(), lib_entrys.end());
				}
			}
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
				auto pluginInstancePtr = plugin.second->Get().getValidShared();
				if (pluginInstancePtr)
				{
					auto nodePtr = pluginInstancePtr->CreatePluginNode(vPluginNodeName);
					if (nodePtr)
					{
						return nodePtr;
					}
				}
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
			auto pluginInstancePtr = plugin.second->Get().getValidShared();
			if (pluginInstancePtr)
			{
				id += pluginInstancePtr->ResetImGuiID(id);
			}
		}
	}
}