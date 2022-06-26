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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "UserNodeLibrary.h"

#include <imgui/imgui.h>
#include <Graph/Base/BaseNode.h>
#include <ctools/FileHelper.h>

#include <Graph/Manager/NodeManager.h>

#include <stdio.h>
#include <string.h>

#include <functional>

#include <Plugins/PluginManager.h>

#include <Graph/Factory/NodeFactory.h>

// for directory/files lister
#if defined(__WIN32__) || defined(_WIN32)
#include <ImGuiFileDialog/dirent/dirent.h>
#define PATH_SEP '\\'
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#include <dirent.h>
#define PATH_SEP '/'
#endif

#define LIBRARY_EFECT_ROOT_PATH "NodeLibrary"

//////////////////////////////////////////////////////////////////////////////////////////////
//// CUSTOM MENU /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void UserNodeLibrary::AnalyseRootDirectory()
{
	AnalyseRootDirectoryRecurs(LIBRARY_EFECT_ROOT_PATH, 0, &m_RootLibraryCategory);

	m_RootLibraryCategory.AddCustom("Core/Assets", "3D Model", "MESH");
	m_RootLibraryCategory.AddCustom("Core/Assets", "2D Texture", "TEXTURE_2D");

	m_RootLibraryCategory.AddCustom("Core/Divers", "Grid / Axis", "GRID_AXIS");
	
	m_RootLibraryCategory.AddCustom("Core/Lighting", "Light", "LIGHT");
	m_RootLibraryCategory.AddCustom("Core/Lighting", "Shadow Mapping", "SHADOW_MAPPING");
	m_RootLibraryCategory.AddCustom("Core/Lighting", "Model Shadow", "MODEL_SHADOW");

	m_RootLibraryCategory.AddCustom("Core/Modifiers", "Smooth Normals", "COMPUTE_SMOOTH_MESH_NORMAL");

	m_RootLibraryCategory.AddCustom("Core/Output", "Output", "OUTPUT");

	m_RootLibraryCategory.AddCustom("Core/PostPro", "SSAO", "SSAO");

	m_RootLibraryCategory.AddCustom("Core/Renderers", "Channels", "CHANNEL_RENDERER");
	m_RootLibraryCategory.AddCustom("Core/Renderers", "Deferred", "DEFERRED_RENDERER");
	m_RootLibraryCategory.AddCustom("Core/Renderers", "Heatmap", "HEATMAP_RENDERER");
	m_RootLibraryCategory.AddCustom("Core/Renderers", "Matcap", "MATCAP_RENDERER");

	m_RootLibraryCategory.AddCustom("Core/Utils", "3D Model Attributes", "MESH_ATTRIBUTES");
	m_RootLibraryCategory.AddCustom("Core/Utils", "Depth to Pos", "DEPTH_TO_POS");
	m_RootLibraryCategory.AddCustom("Core/Utils", "Pos to Depth", "POS_TO_DEPTH");

	// les plugins
	auto pluginEntrys = PluginManager::Instance()->GetLibraryEntrys();
	for (auto entry : pluginEntrys)
	{
		m_RootLibraryCategory.AddLibraryEntry(entry);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// INIT ////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

UserNodeLibrary::UserNodeLibrary()
{
	using namespace std::placeholders;
	BaseNode::sShowNewNodeMenuCallback = std::bind(&UserNodeLibrary::ShowNewNodeMenu, this, _1, _2);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// MENU ////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

BaseNodeWeak UserNodeLibrary::ShowNewNodeMenu(BaseNodeWeak vNodeGraph, BaseNodeStateStruct *vCanvasState)
{
	if (vNodeGraph.expired()) return BaseNodeWeak();
	
	BaseNodeWeak createdNode;

	if (ImGui::BeginPopup("CreateNewNode"))
	{
		auto entry = m_RootLibraryCategory.ShowContent(vNodeGraph, vCanvasState, 0);
		if (!entry.first.empty())
		{
			createdNode = CreateNode(vNodeGraph, entry);
			auto createdNodePtr = createdNode.getValidShared();
			if (createdNodePtr)
			{
				createdNodePtr->Notify(NotifyEvent::GraphIsLoaded);
			}
		}

		ImGui::EndPopup();
	}
	else
	{
		auto graphPtr = vNodeGraph.lock();
		if (graphPtr)
		{
			graphPtr->m_CreateNewNode = false;
		}
	}

	if (!createdNode.expired())
	{
		auto nodePtr = createdNode.lock();
		auto graphPtr = vNodeGraph.lock();
		if (nodePtr && graphPtr)
		{
			nd::SetNodePosition(nodePtr->nodeID, graphPtr->m_OpenPopupPosition);
		}
	}

	return createdNode;
}

BaseNodeWeak UserNodeLibrary::CreateNode(BaseNodeWeak vNodeGraph, const LibraryEntry& vLibraryEntry)
{
	if (vNodeGraph.expired()) return BaseNodeWeak();
	if (vLibraryEntry.first.empty()) return BaseNodeWeak();

	bool isOutputNode = false;
	BaseNodePtr nodePtr = nullptr;

	if (vLibraryEntry.first == "internal")
	{
		if (vLibraryEntry.second.type == LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_INTERNAL)
		{
			nodePtr = NodeFactory::CreateNode(vNodeGraph, vLibraryEntry.second.nodeType);
		}
	}
	else if (vLibraryEntry.first == "plugins")
	{
		if (vLibraryEntry.second.type == LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_PLUGIN)
		{
			nodePtr = PluginManager::Instance()->CreatePluginNode(vLibraryEntry.second.nodeType);
		}
	}

	if (nodePtr)
	{
		auto graphPtr = vNodeGraph.getValidShared();
		if (graphPtr)
		{
			if (vLibraryEntry.second.nodeType == "OUTPUT")
			{
				graphPtr->m_OutputNode = nodePtr;
			}

			graphPtr->AddChildNode(nodePtr);

			return nodePtr;
		}
	}

	return BaseNodeWeak();
}

void UserNodeLibrary::AnalyseRootDirectoryRecurs(const char *vRootPath, int vIndent, LibraryCategory *vLibraryCategory)
{
	if (!vLibraryCategory) return;

	DIR *dir = opendir(vRootPath);
	struct dirent *entry;

	if (!dir)
		return;

	while ((entry = readdir(dir)) != nullptr) 
	{
		char path[1024];
		
		if (entry->d_type == DT_DIR) // Directory
		{
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			snprintf(path, sizeof(path), "%s/%s", vRootPath, entry->d_name);

			printf("%*s[%s]\n", vIndent, "", entry->d_name);
			
			auto cat = vLibraryCategory->AddCategory(entry->d_name);

			AnalyseRootDirectoryRecurs(path, vIndent + 2, cat);
		}
		else if (entry->d_type == DT_REG) // File
		{
			snprintf(path, sizeof(path), "%s/%s", vRootPath, entry->d_name);
			
			vLibraryCategory->AddShader(entry->d_name, path);
			
			printf("%*s- %s\n", vIndent, "", entry->d_name);
		}
	}

	closedir(dir);
}