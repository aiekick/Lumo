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
#include <Graph/Base/NodeSlotOutput.h>

class NodeSlotTextureGroupOutput;
typedef ct::cWeak<NodeSlotTextureGroupOutput> NodeSlotTextureGroupOutputWeak;
typedef std::shared_ptr<NodeSlotTextureGroupOutput> NodeSlotTextureGroupOutputPtr;

class NodeSlotTextureGroupOutput : 
	public NodeSlotOutput
{
public:
	static NodeSlotTextureGroupOutputPtr Create(NodeSlotTextureGroupOutput vSlot);
	static NodeSlotTextureGroupOutputPtr Create(const std::string& vName);
	static NodeSlotTextureGroupOutputPtr Create(const std::string& vName, const bool& vHideName);
	static NodeSlotTextureGroupOutputPtr Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget);

public:
	explicit NodeSlotTextureGroupOutput();
	explicit NodeSlotTextureGroupOutput(const std::string& vName);
	explicit NodeSlotTextureGroupOutput(const std::string& vName, const bool& vHideName);
	explicit NodeSlotTextureGroupOutput(const std::string& vName, const bool& vHideName, const bool& vShowWidget);
	~NodeSlotTextureGroupOutput();

	void Init();
	void Unit();

	void SendFrontNotification(const NotifyEvent& vEvent) override;

	void DrawDebugInfos();
};