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

#include <Panes/Abstract/AbstractPane.h>

#include <imgui/imgui.h>

#include <stdint.h>
#include <string>
#include <map>

#include <Graph/Base/BaseNode.h>
#include <ctools/ConfigAbstract.h>

class ProjectFile;
class GraphPane : public conf::ConfigAbstract, public AbstractPane
{
private:
	// nodeId, visibility
	typedef std::pair<BaseNodeWeak, bool> GraphVisibilityPair;
	std::vector<GraphVisibilityPair> m_GraphPanes;
	BaseNodeWeak m_LastFocusedGraph;

public:
	bool Init() override;
	void Unit() override;
	int DrawPanes(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas) override;
	void DrawDialogsAndPopups(const uint32_t& vCurrentFrame, std::string vUserDatas) override;
	int DrawWidgets(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas) override;

	void DrawDebugInfos();
	void DrawProperties();

	void Clear();

	void DeleteNodesIfAnys();

	void AddGraphPane(BaseNodeWeak vNodeGraphToShow);
	void RemoveGraphPane(BaseNodeWeak vNodeGraphToShow);
	void ClearGraphPanes();

private:
	bool DrawGraph(BaseNodeWeak vNode, bool &vCanShow, bool vRootNode, size_t vInitialPanesCount);

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

public: // singleton
	static GraphPane *Instance()
	{
		static GraphPane _instance;
		return &_instance;
	}

protected:
	GraphPane(); // Prevent construction
	GraphPane(const GraphPane&) = default; // Prevent construction by copying
	GraphPane& operator =(const GraphPane&) { return *this; }; // Prevent assignment
	~GraphPane(); // Prevent unwanted destruction};
};
