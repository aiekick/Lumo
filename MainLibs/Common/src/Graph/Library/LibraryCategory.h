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

#include <Graph/Graph.h>
#include <ctools/cTools.h>
#include <Graph/Base/NodeSlot.h>
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <memory>

// shader type, shader filepathname

class LibraryItem
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
struct BaseNodeStateStruct;
class LibraryCategory
{
public:
	std::string m_CategoryName;

private:
	// shader name, ShaderEntry
	std::map<std::string, LibraryEntry> m_LibraryItems;
	// category name, sub category
	std::map<std::string, LibraryCategory> m_SubCategories;

private:
	LibraryEntry ShowMenu(BaseNodeWeak vNodeGraph, BaseNodeStateStruct *vCanvasState, int vLevel);

public:
	LibraryCategory();
	~LibraryCategory();
	void Clear();
	LibraryCategory* AddCategory(const std::string& vCategoryName);
	void AddShader(const std::string& vShaderName, const std::string& vShaderPath);
	void AddCustom(const std::string& vCategoryPath, const std::string& vNodeLabel, const std::string& vNodeType, const ct::fvec4& vColor = 0.0f);
	void AddLibraryEntry(const LibraryEntry& vLibraryEntry);
	LibraryEntry ShowContent(BaseNodeWeak vNodeGraph, BaseNodeStateStruct *vCanvasState, int vLevel);
};
