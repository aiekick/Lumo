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
#pragma warning(disable : 4251)

#include <Graph/Graph.h>
#include <ctools/cTools.h>
#include <Graph/Base/NodeSlot.h>
#include <Common/Globals.h>
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <memory>

// shader type, shader filepathname

class COMMON_API LibraryItem
{
public:
	enum class LibraryItemTypeEnum : uint8_t
	{
		LIBRARY_ITEM_TYPE_SHADER = 0,
		LIBRARY_ITEM_TYPE_BLUEPRINT,
		LIBRARY_ITEM_TYPE_INTERNAL,
		LIBRARY_ITEM_TYPE_PLUGIN,
		LIBRARY_ITEM_TYPE_Count
	} type = LibraryItemTypeEnum::LIBRARY_ITEM_TYPE_SHADER;

public:
	// SHADER
	std::string shaderpath;

	// INTERNAL
	std::string nodeLabel;
	std::string nodeType;

	ct::fvec4 color;
	std::string categoryPath;

	// func from slot
	NodeSlot nodeSlot;
};

typedef std::pair<std::string, LibraryItem> LibraryEntry;

class BaseNode;
struct BaseNodeState;
class COMMON_API LibraryCategory
{
public:
	std::string m_CategoryName;

private:
	// shader name, ShaderEntry
	std::map<std::string, LibraryEntry> m_LibraryItems;
	// category name, sub category
	std::map<std::string, LibraryCategory> m_SubCategories;

private:
	LibraryEntry ShowMenu(BaseNodeWeak vNodeGraph, BaseNodeState *vBaseNodeState, int vLevel);

public:
	LibraryCategory();
	~LibraryCategory();
	void Clear();
	LibraryCategory* AddCategory(const std::string& vCategoryName);
	void AddShader(const std::string& vShaderName, const std::string& vShaderPath);
	void AddCustom(const std::string& vCategoryPath, const std::string& vNodeLabel, const std::string& vNodeType, const ct::fvec4& vColor = 0.0f);
	void AddLibraryEntry(const LibraryEntry& vLibraryEntry);
	LibraryEntry ShowContent(BaseNodeWeak vNodeGraph, BaseNodeState *vBaseNodeState, int vLevel);
};
