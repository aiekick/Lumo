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

#include "PluginManager.h"

#include <Gaia/gaia.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>

#include <ImGuiPack.h>

#include <ctools/FileHelper.h>

#include <LumoBackend/Systems/CommonSystem.h>

#ifdef _DEBUG
#define RuntimeType "_Debug_"
#else
#define RuntimeType "_Release_"
#endif

namespace fs = std::filesystem;

//////////////////////////////////////////////////////////////////////////////
////// PluginInstance ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

PluginInstance::PluginInstance() {}

PluginInstance::~PluginInstance() { Unit(); }

PluginReturnMsg PluginInstance::Init(
    GaiApi::VulkanCoreWeak vVulkanCoreWeak, const std::string& vName, const std::string& vFilePathName) {
    m_Name = vName;
    m_Loader = dlloader::DLLoader<PluginInterface>(vFilePathName);
    m_Loader.DLOpenLib();
    m_PluginInstance = m_Loader.DLGetInstance();
    if (m_Loader.IsAPlugin()) {
        if (m_Loader.IsValid()) {
            if (m_PluginInstance) {
                if (!m_PluginInstance->Init(vVulkanCoreWeak)) {
                    m_PluginInstance.reset();
                } else {
                    return PluginReturnMsg::LOADING_SUCCEED;
                }
            }
        }
        return PluginReturnMsg::LOADING_FAILED;
    }
    return PluginReturnMsg::NOT_A_PLUGIN;
}

void PluginInstance::Unit() {
    m_PluginInstance.reset();
    m_Loader.DLCloseLib();
}

PluginInterfaceWeak PluginInstance::Get() { return m_PluginInstance; }

//////////////////////////////////////////////////////////////////////////////
////// PluginLoader //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

static inline std::string GetDLLExtention() {
#ifdef WIN32
    return "dll";
#elif defined(__linux__)
    return "so";
#elif defined(__APPLE__)
    return "dylib";
#elif
    return "";
#endif
}

void PluginManager::Clear() { m_Plugins.clear(); }

void PluginManager::LoadPlugins(GaiApi::VulkanCoreWeak vVulkanCore) {
    printf("-----------\n");
    LogVarLightInfo("Availables Plugins :\n");
    auto plugin_directory = std::filesystem::path(FileHelper::Instance()->GetAppPath()).append("plugins");
    if (std::filesystem::exists(plugin_directory)) {
        const auto dir_iter = std::filesystem::directory_iterator(plugin_directory);
        for (const auto& file : dir_iter) {
            m_LoadPlugin(file, vVulkanCore);
        }
        m_DisplayLoadedPlugins();
    } else {
        LogVarLightInfo("Plugin directory %s not found !", plugin_directory.string().c_str());
    }
    printf("-----------\n");
}

std::vector<LibraryEntry> PluginManager::GetLibraryEntrys() {
    std::vector<LibraryEntry> res;

    for (auto plugin : m_Plugins) {
        if (plugin.second) {
            auto pluginInstancePtr = plugin.second->Get().lock();
            if (pluginInstancePtr) {
                auto lib_entrys = pluginInstancePtr->GetLibrary();
                if (!lib_entrys.empty()) {
                    res.insert(res.end(), lib_entrys.begin(), lib_entrys.end());
                }
            }
        }
    }

    return res;
}

BaseNodePtr PluginManager::CreatePluginNode(const std::string& vPluginNodeName) {
    if (!vPluginNodeName.empty()) {
        for (auto plugin : m_Plugins) {
            if (plugin.second) {
                auto pluginInstancePtr = plugin.second->Get().lock();
                if (pluginInstancePtr) {
                    auto nodePtr = pluginInstancePtr->CreatePluginNode(vPluginNodeName);
                    if (nodePtr) {
                        return nodePtr;
                    }
                }
            }
        }
    }
    return nullptr;
}

std::vector<PluginPaneConfig> PluginManager::GetPluginsPanes() {
    std::vector<PluginPaneConfig> pluginsPanes;
    for (auto plugin : m_Plugins) {
        if (plugin.second) {
            auto pluginInstancePtr = plugin.second->Get().lock();
            if (pluginInstancePtr) {
                auto _pluginPanes = pluginInstancePtr->GetPanes();
                if (!_pluginPanes.empty()) {
                    pluginsPanes.insert(pluginsPanes.end(), _pluginPanes.begin(), _pluginPanes.end());
                }
            }
        }
    }
    return pluginsPanes;
}

void PluginManager::ResetImGuiID(int vWidgetId) {
    int id = vWidgetId;
    for (auto plugin : m_Plugins) {
        id += 10000;
        if (plugin.second) {
            auto pluginInstancePtr = plugin.second->Get().lock();
            if (pluginInstancePtr) {
                id += pluginInstancePtr->ResetImGuiID(id);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// LOAD / SAVE /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string PluginManager::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;
    for (auto plugin : m_Plugins) {
        if (plugin.second) {
            auto pluginInstancePtr = plugin.second->Get().lock();
            if (pluginInstancePtr) {
                str += pluginInstancePtr->getXml(vOffset, vUserDatas);
            }
        }
    }
    return str;
}

bool PluginManager::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    for (auto plugin : m_Plugins) {
        if (plugin.second) {
            auto pluginInstancePtr = plugin.second->Get().lock();
            if (pluginInstancePtr) {
                pluginInstancePtr->RecursParsingConfig(vElem, vParent, vUserDatas);
            }
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////
//// PRIVATE /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void PluginManager::m_LoadPlugin(const fs::directory_entry& vEntry, GaiApi::VulkanCoreWeak vVulkanCore) {
    if (vEntry.is_directory()) {
        const auto dir_iter = std::filesystem::directory_iterator(vEntry);
        for (const auto& file : dir_iter) {
            m_LoadPlugin(file, vVulkanCore);
        }
    }
    else if (vEntry.is_regular_file()) {
        auto file_name = vEntry.path().filename().string();
        if (file_name.find(PLUGIN_PREFIX) == 0U) {
            if (file_name.find(RuntimeType) != std::string::npos) {
                auto file_path_name = vEntry.path().string();
                if (file_path_name.find(GetDLLExtention()) != std::string::npos) {
                    auto ps = FileHelper::Instance()->ParsePathFileName(file_path_name);
                    if (ps.isOk) {
                        auto resPtr = std::make_shared<PluginInstance>();
                        auto ret = resPtr->Init(vVulkanCore, ps.name, ps.GetFPNE());
                        if (ret != PluginReturnMsg::LOADING_SUCCEED) {
                            resPtr.reset();
                            if (ret == PluginReturnMsg::LOADING_FAILED) {
                                LogVarDebugError("Plugin %s fail to load", ps.name.c_str());
                            }
                        } else {
                            auto pluginInstancePtr = resPtr->Get().lock();
                            if (pluginInstancePtr) {
                                char spaceBuffer[40 + 1] = "";
                                spaceBuffer[0] = '\0';

                                std::string name = pluginInstancePtr->GetName();
                                if (name.size() < 15U) {
                                    size_t of = 15U - name.size();
                                    memset(spaceBuffer, 32, of);  // 32 is space code in ASCII table
                                    spaceBuffer[of] = '\0';
                                    name += spaceBuffer;
                                } else {
                                    name = name.substr(0, 15U);
                                }

                                std::string version = pluginInstancePtr->GetVersion();
                                if (version.size() < 10U) {
                                    size_t of = 10U - version.size();
                                    memset(spaceBuffer, 32, of);  // 32 is space code in ASCII table
                                    spaceBuffer[of] = '\0';
                                    version += spaceBuffer;
                                } else {
                                    version = version.substr(0, 10U);
                                }

                                std::string desc = pluginInstancePtr->GetDescription();
                            }

                            m_Plugins[ps.name] = resPtr;
                        }
                    }
                }
            }
        }
    }
}

void PluginManager::m_DisplayLoadedPlugins() {
    if (!m_Plugins.empty()) {
        size_t max_name_size = 0U;
        size_t max_vers_size = 0U;
        const size_t& minimal_space = 2U;
        for (auto plugin : m_Plugins) {
            if (plugin.second != nullptr) {
                auto plugin_instance_ptr = plugin.second->Get().lock();
                if (plugin_instance_ptr != nullptr) {
                    max_name_size = ct::maxi(max_name_size, plugin_instance_ptr->GetName().size() + minimal_space);
                    max_vers_size = ct::maxi(max_vers_size, plugin_instance_ptr->GetVersion().size() + minimal_space);
                }
            }
        }
        for (auto plugin : m_Plugins) {
            if (plugin.second != nullptr) {
                auto plugin_instance_ptr = plugin.second->Get().lock();
                if (plugin_instance_ptr != nullptr) {
                    const auto& name = plugin_instance_ptr->GetName();
                    const auto& name_space = std::string(max_name_size - name.size(), ' ');  // 32 is a space in ASCII
                    const auto& vers = plugin_instance_ptr->GetVersion();
                    const auto& vers_space = std::string(max_vers_size - vers.size(), ' ');  // 32 is a space in ASCII
                    const auto& desc = plugin_instance_ptr->GetDescription();
                    LogVarLightInfo("Plugin loaded : %s%sv%s%s(%s)",  //
                        name.c_str(), name_space.c_str(),             //
                        vers.c_str(), vers_space.c_str(),             //
                        desc.c_str());
                }
            }
        }
    }
}