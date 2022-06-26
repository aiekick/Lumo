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
#include <Graph/Base/BaseNode.h>
#include <ctools/ConfigAbstract.h>
#include <Interfaces/GuiInterface.h>
#include <Interfaces/TaskInterface.h>

class NodeManager : 
	public GuiInterface, 
	public conf::ConfigAbstract,
	public TaskInterface
{
public:
	BaseNodePtr m_RootNodePtr = nullptr;
	// we msut clear the graph after the rendering
	bool m_NeedToClearTheGraph = false;

public:
	// init / unit
	bool Init(vkApi::VulkanCore* vVulkanCore);
	void Unit();
	
	void Clear();

	void PrepareToLoadGraph();

	bool Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer *vCmd = nullptr) override;

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	
	void FinalizeGraphLoading();

	bool LoadNodeFromXML(
		BaseNodeWeak vNodeGraphWeak,
		tinyxml2::XMLElement* vElem, 
		tinyxml2::XMLElement* vParent,
		const std::string& vNodeName,
		const std::string& vNodeType,
		const ct::fvec2& vPos,
		const size_t& vNodeId);

	void UpdateShaders(const std::set<std::string>& vFiles) const;

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

public: // singleton
	static NodeManager *Instance()
	{
		static NodeManager _instance;
		return &_instance;
	}

protected:
	NodeManager(); // Prevent construction
	NodeManager(const NodeManager&) = default; // Prevent construction by copying
	NodeManager& operator =(const NodeManager&) { return *this; }; // Prevent assignment
	~NodeManager() = default; // Prevent unwanted destruction
};
