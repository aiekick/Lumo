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

#pragma once
#pragma warning(disable : 4251)

#include <LumoBackend/Graph/Graph.h>
#include <ctools/cTools.h>
#include <imgui.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

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

struct LUMO_BACKEND_API ColumnContainerStruct {
    std::map<int, BaseNodeWeak> nodes;
    ImVec2 size;
    ImVec2 offset;

    void AddNode(BaseNodeWeak vNode);
    void Clear();
};

class LUMO_BACKEND_API GraphLayout {
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

private:  // security
    std::map<uintptr_t, int> m_InfLoopNodeDetector;
    bool IsThereAnInfiniteLoopForNode(BaseNodeWeak vNode);  // recursive func SetColumnOfNodesRecurs

private:  // final
    void ApplyPositionsInGraph();

public:  // singleton
    static GraphLayout *Instance() {
        static GraphLayout _instance;
        return &_instance;
    }

protected:
    GraphLayout() = default;                     // Prevent construction
    GraphLayout(const GraphLayout &) = default;  // Prevent construction by copying
    GraphLayout &operator=(const GraphLayout &) {
        return *this;
    };                         // Prevent assignment
    ~GraphLayout() = default;  // Prevent unwanted destruction
};