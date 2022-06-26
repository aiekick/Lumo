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

	bool Init(vkApi::VulkanCore* vVulkanCore, const std::string& vName, const std::string& vFilePathName);
	void Unit();

	PluginInterfaceWeak Get();
};

class PluginManager
{
private:
	std::map<std::string, PluginInstancePtr> m_Plugins;

public:
	void LoadPlugins(vkApi::VulkanCore* vVulkanCore);
	PluginInstanceWeak LoadPlugin(vkApi::VulkanCore* vVulkanCore, const std::string& vPluginName);
	PluginInstanceWeak Get(const std::string& vPluginName);
	std::vector<LibraryEntry> GetLibraryEntrys();
	BaseNodePtr CreatePluginNode(const std::string& vPluginNodeName);
	void ResetImGuiID(int vWidgetId);
	void Clear();

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