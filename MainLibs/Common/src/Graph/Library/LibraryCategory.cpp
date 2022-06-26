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

#include "LibraryCategory.h"

#include <imgui/imgui.h>
#include <Graph/Base/BaseNode.h>
#include <ctools/FileHelper.h>

#include <stdio.h>
#include <string.h>

#include <functional>

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
//// INIT ////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

LibraryCategory::LibraryCategory()
{
	
}

LibraryCategory::~LibraryCategory()
{
	
}

void LibraryCategory::Clear()
{
	m_CategoryName.clear();
	m_LibraryItems.clear();
	m_SubCategories.clear();
}

LibraryCategory* LibraryCategory::AddCategory(const std::string& vCategoryName)
{
	if (vCategoryName.empty()) return 0;

	m_SubCategories[vCategoryName].m_CategoryName = vCategoryName;

	return &m_SubCategories[vCategoryName];
}

void LibraryCategory::AddShader(const std::string& vShaderName, const std::string& vShaderPath)
{
	if (vShaderName.empty()) return;
	if (vShaderPath.empty()) return;

	auto ps = FileHelper::Instance()->ParsePathFileName(vShaderName);
	if (ps.isOk)
	{
		LibraryEntry entry;

		if (ps.ext == "comp") entry.first = "compute";
		if (ps.ext == "frag") entry.first = "fragment";
		if (ps.ext == "scen") entry.first = "scene";

		if (!entry.first.empty())
		{
			entry.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_SHADER;
			entry.second.shaderpath = vShaderPath;
			m_LibraryItems[ps.name] = entry;
		}
	}
}

void LibraryCategory::AddCustom(const std::string& vCategoryPath, const std::string& vNodeLabel, const std::string& vNodeType, const ct::fvec4& vColor)
{
	if (vCategoryPath.empty()) return;
	if (vNodeLabel.empty()) return;

	auto vec = ct::splitStringToVector(vCategoryPath, '/');
	if (!vec.empty())
	{
		auto cat = this;
		for (auto word : vec)
		{
			if (cat->m_SubCategories.find(word) != cat->m_SubCategories.end())
			{
				cat = &cat->m_SubCategories[word];
			}
			else
			{
				cat = cat->AddCategory(word);
			}
		}

		LibraryEntry entry;
		entry.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_INTERNAL;
		entry.first = "internal";
		entry.second.nodeLabel = vNodeLabel;
		entry.second.nodeType = vNodeType;
		entry.second.color = vColor;
		cat->m_LibraryItems[vNodeLabel] = entry;
	}
}

void LibraryCategory::AddLibraryEntry(const LibraryEntry& vLibraryEntry)
{
	if (vLibraryEntry.second.nodeLabel.empty()) return;

	auto vec = ct::splitStringToVector(vLibraryEntry.second.categoryPath, '/');
	if (!vec.empty())
	{
		auto cat = this;
		for (auto word : vec)
		{
			if (cat->m_SubCategories.find(word) != cat->m_SubCategories.end())
			{
				cat = &cat->m_SubCategories[word];
			}
			else
			{
				cat = cat->AddCategory(word);
			}
		}

		cat->m_LibraryItems[vLibraryEntry.second.nodeLabel] = vLibraryEntry;
	}
}

LibraryEntry LibraryCategory::ShowMenu(BaseNodeWeak vNodeGraph, BaseNodeStateStruct *vCanvasState, int vLevel)
{
	LibraryEntry entry;

	if (ImGui::BeginMenu(m_CategoryName.c_str()))
	{
		auto ent = ShowContent(vNodeGraph, vCanvasState, vLevel + 1);
		if (!ent.first.empty())
		{
			entry = ent;
		}

		ImGui::EndMenu();
	}

	return entry;
}

LibraryEntry LibraryCategory::ShowContent(BaseNodeWeak vNodeGraph, BaseNodeStateStruct *vCanvasState, int vLevel)
{
	LibraryEntry entry;
	
	if (vCanvasState && vLevel == 0)
	{
		if (!vCanvasState->linkFromSlot.expired())
		{
			if (ImGui::MenuItem("extract"))
			{
				auto slotPtr = vCanvasState->linkFromSlot.lock();
				if (slotPtr)
				{
					entry.second.type = LibraryItem::LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_INTERNAL;
					entry.first = "internal";
					entry.second.nodeLabel = "extract";
					entry.second.nodeType = "extract";
					entry.second.nodeSlot = *slotPtr;
				}
			}

			ImGui::Separator();
		}
	}

	for (auto &category : m_SubCategories)
	{
		auto ent = category.second.ShowMenu(vNodeGraph, vCanvasState, vLevel + 1);
		if (!ent.first.empty())
		{
			entry = ent;
		}
	}

	if (!m_LibraryItems.empty() &&
		!m_SubCategories.empty())
	{
		ImGui::Separator();
	}

	for (auto &item : m_LibraryItems)
	{
		if (ImGui::MenuItem(item.first.c_str()))
		{
			entry = item.second;

			break;
		}
	}

	return entry;
}
