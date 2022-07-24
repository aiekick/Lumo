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

	m_RootLibraryCategory.AddCustom("Core/Breaks", "Break Textures 2D Group", "BREAK_TEXTURE_2D_GROUP");

	m_RootLibraryCategory.AddCustom("Core/Divers", "Grid / Axis", "GRID_AXIS");
	
	m_RootLibraryCategory.AddCustom("Core/Lighting", "Lights", "LIGHT_GROUP");
	m_RootLibraryCategory.AddCustom("Core/Lighting/Shadow", "Shadow Mapping", "SHADOW_MAPPING");
	m_RootLibraryCategory.AddCustom("Core/Lighting/Shadow", "Model Shadow", "MODEL_SHADOW");
	m_RootLibraryCategory.AddCustom("Core/Lighting", "Diffuse", "DIFFUSE");
	m_RootLibraryCategory.AddCustom("Core/Lighting", "Specular", "SPECULAR");
	m_RootLibraryCategory.AddCustom("Core/Lighting", "SSAO", "SSAO");
	
	m_RootLibraryCategory.AddCustom("Core/Modifiers", "Smooth Normals", "SMOOTH_NORMAL");

	m_RootLibraryCategory.AddCustom("Core/Outputs", "Output 3D", "OUTPUT_3D");
	m_RootLibraryCategory.AddCustom("Core/Outputs", "Output 2D", "OUTPUT_2D");

	m_RootLibraryCategory.AddCustom("Core/PostPro", "Blur", "BLUR");
	m_RootLibraryCategory.AddCustom("Core/PostPro", "Laplacian", "LAPLACIAN");
	m_RootLibraryCategory.AddCustom("Core/PostPro", "Tone Map", "TONE_MAP");
	
	m_RootLibraryCategory.AddCustom("Core/Renderers", "Channels", "CHANNEL_RENDERER");
	m_RootLibraryCategory.AddCustom("Core/Renderers", "Deferred", "DEFERRED_RENDERER");
	m_RootLibraryCategory.AddCustom("Core/Renderers", "Heatmap", "HEATMAP_RENDERER");
	m_RootLibraryCategory.AddCustom("Core/Renderers", "Matcap", "MATCAP_RENDERER");
	m_RootLibraryCategory.AddCustom("Core/Renderers", "PBR", "PBR_RENDERER");

	m_RootLibraryCategory.AddCustom("Core/Simulations", "Gray Scott Reaction Diffusion", "GRAY_SCOTT_SIMULATION");

	m_RootLibraryCategory.AddCustom("Core/Utils", "3D Model Attributes", "MESH_ATTRIBUTES");
	m_RootLibraryCategory.AddCustom("Core/Utils", "Depth to Pos", "DEPTH_TO_POS");
	m_RootLibraryCategory.AddCustom("Core/Utils", "Pos to Depth", "POS_TO_DEPTH");
	m_RootLibraryCategory.AddCustom("Core/Utils", "Math", "MATH");

	m_RootLibraryCategory.AddCustom("Core/Widgets", "Boolean", "WIDGET_BOOLEAN");
	m_RootLibraryCategory.AddCustom("Core/Widgets", "Color", "WIDGET_COLOR");

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

BaseNodeWeak UserNodeLibrary::ShowNewNodeMenu(BaseNodeWeak vNodeGraph, BaseNodeState *vBaseNodeState)
{
	if (vNodeGraph.expired()) return BaseNodeWeak();
	
	BaseNodeWeak createdNode;

	ImGui::SetNextWindowViewport(ImGui::GetWindowViewport()->ID);

	if (ImGui::BeginPopup("CreateNewNode"))
	{
		auto entry = m_RootLibraryCategory.ShowContent(vNodeGraph, vBaseNodeState, 0);
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
			if (vLibraryEntry.second.nodeType == "OUTPUT_3D")
			{
				graphPtr->m_Output3DNode = nodePtr;
			}
			else if (vLibraryEntry.second.nodeType == "OUTPUT_2D")
			{
				graphPtr->m_Output2DNode = nodePtr;
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