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
	bool Init(vkApi::VulkanCorePtr vVulkanCorePtr);
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
