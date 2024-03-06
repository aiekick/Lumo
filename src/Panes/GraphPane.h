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

#include <ImGuiPack.h>

#include <stdint.h>
#include <string>
#include <map>

#include <LumoBackend/Graph/Base/BaseNode.h>
#include <ctools/ConfigAbstract.h>

class ProjectFile;
class GraphPane : public conf::ConfigAbstract, public AbstractPane {
private:
    // nodeId, visibility
    typedef std::pair<BaseNodeWeak, bool> GraphVisibilityPair;
    std::vector<GraphVisibilityPair> m_GraphPanes;
    BaseNodeWeak m_LastFocusedGraph;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(
        const uint32_t& vCurrentFrame, PaneFlags& vInOutPaneShown, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

    void DrawDebugInfos() override;
    void DrawProperties();

    void Clear();

    void AddGraphPane(BaseNodeWeak vNodeGraphToShow);
    void RemoveGraphPane(BaseNodeWeak vNodeGraphToShow);
    void ClearGraphPanes();

private:
    bool DrawGraph(BaseNodeWeak vNode, bool& vCanShow, bool vRootNode, size_t vInitialPanesCount, PaneFlags& vInOutPaneShown);

public:
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

public:  // singleton
    static std::shared_ptr<GraphPane> Instance() {
        static std::shared_ptr<GraphPane> _instance = std::make_shared<GraphPane>();
        return _instance;
    }

public:
    GraphPane();                            // Prevent construction
    GraphPane(const GraphPane&) = default;  // Prevent construction by copying
    GraphPane& operator=(const GraphPane&) {
        return *this;
    };             // Prevent assignment
    ~GraphPane();  // Prevent unwanted destruction};
};
