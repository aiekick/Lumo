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

#include <map>
#include <set>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <Graph/Graph.h>
#include <ctools/cTools.h>
#include <Graph/Base/NodeSlot.h>
#include <Graph/Library/LibraryCategory.h>

class UserNodeLibrary
{
private:
	LibraryCategory m_RootLibraryCategory;

public:
	BaseNodeWeak ShowNewNodeMenu(
		BaseNodeWeak vNodeGraph, BaseNodeState *vBaseNodeState);
	void AnalyseRootDirectory();

private:
	BaseNodeWeak CreateNode(BaseNodeWeak vNodeGraph, const LibraryEntry& vLibraryEntry);
	void AnalyseRootDirectoryRecurs(const char *name, int indent, LibraryCategory *vLibraryCategory);

public: // singleton
	static UserNodeLibrary *Instance()
	{
		static UserNodeLibrary _instance;
		return &_instance;
	}

protected:
	UserNodeLibrary(); // Prevent construction
	UserNodeLibrary(const UserNodeLibrary&) = default; // Prevent construction by copying
	UserNodeLibrary& operator =(const UserNodeLibrary&) { return *this; }; // Prevent assignment
	~UserNodeLibrary() = default; // Prevent unwanted destruction
};