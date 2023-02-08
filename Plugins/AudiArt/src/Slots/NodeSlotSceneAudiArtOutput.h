/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

#pragma once

#include <Graph/Graph.h>
#include <Graph/Base/NodeSlotOutput.h>

class NodeSlotSceneAudiArtOutput;
typedef ct::cWeak<NodeSlotSceneAudiArtOutput> NodeSlotSceneAudiArtOutputWeak;
typedef std::shared_ptr<NodeSlotSceneAudiArtOutput> NodeSlotSceneAudiArtOutputPtr;

class NodeSlotSceneAudiArtOutput : 
	public NodeSlotOutput
{
public:
	static NodeSlotSceneAudiArtOutputPtr Create(NodeSlotSceneAudiArtOutput vSlot);
	static NodeSlotSceneAudiArtOutputPtr Create(const std::string& vName);
	static NodeSlotSceneAudiArtOutputPtr Create(const std::string& vName, const std::string& vType);
	static NodeSlotSceneAudiArtOutputPtr Create(const std::string& vName, const std::string& vType, const bool& vHideName);
	static NodeSlotSceneAudiArtOutputPtr Create(const std::string& vName, const std::string& vType, const bool& vHideName, const bool& vShowWidget);

public:
	explicit NodeSlotSceneAudiArtOutput();
	explicit NodeSlotSceneAudiArtOutput(const std::string& vName);
	explicit NodeSlotSceneAudiArtOutput(const std::string& vName, const std::string& vType);
	explicit NodeSlotSceneAudiArtOutput(const std::string& vName, const std::string& vType, const bool& vHideName);
	explicit NodeSlotSceneAudiArtOutput(const std::string& vName, const std::string& vType, const bool& vHideName, const bool& vShowWidget);
	~NodeSlotSceneAudiArtOutput();

	void Init();
	void Unit();

	void SendFrontNotification(const NotifyEvent& vEvent) override;

	void DrawDebugInfos();

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};

