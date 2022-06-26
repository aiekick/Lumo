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
#include <imgui/imgui.h>

#include <string>
#include <unordered_map>
#include <map>

#include <future>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include <list>

class BaseNode;
class NodeSlot;

struct ColumnContainerStruct
{
	std::map<int, BaseNodeWeak> nodes;
	ImVec2 size;
	ImVec2 offset;

	void AddNode(BaseNodeWeak vNode);
	void Clear();
};

class GraphLayout
{
public:
	static ImVec2 s_NodeSpacing;
	static float s_NodeCentering;

private:
	std::unordered_map<uint32_t, BaseNodePtr> *m_Nodes = nullptr;
	std::map<int, ColumnContainerStruct> m_Columns;
	
public:
	void ApplyLayout(BaseNodeWeak vGraphNode);
	void Clear();
	static bool DrawSettings();

private: 
	void CalcLayout(BaseNodeWeak vGraphNode);
	void ResetNodeStates();
	void ClassifyNodes(std::string vRootFunction);
	void SetColumnOfNodesRecurs(BaseNodeWeak vNode, ct::ivec2 vNodeCell);
	void AddNodesInCells();
	void AddNodeInCell(BaseNodeWeak vNode);
	void DefinePositionsOfNodes();

private: // security
	std::map<uintptr_t, int> m_InfLoopNodeDetector;
	bool IsThereAnInfiniteLoopForNode(BaseNodeWeak vNode); // recursive func SetColumnOfNodesRecurs

private: // final
	void ApplyPositionsInGraph();
	
public: // singleton
	static GraphLayout *Instance()
	{
		static GraphLayout _instance;
		return &_instance;
	}

protected:
	GraphLayout() = default; // Prevent construction
	GraphLayout(const GraphLayout&) = default; // Prevent construction by copying
	GraphLayout& operator =(const GraphLayout&) { return *this; }; // Prevent assignment
	~GraphLayout() = default; // Prevent unwanted destruction
};