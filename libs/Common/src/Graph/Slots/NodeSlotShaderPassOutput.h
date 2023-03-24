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
#include <Graph/Base/NodeSlotOutput.h>

class NodeSlotShaderPassOutput;
typedef ct::cWeak<NodeSlotShaderPassOutput> NodeSlotShaderPassOutputWeak;
typedef std::shared_ptr<NodeSlotShaderPassOutput> NodeSlotShaderPassOutputPtr;

class NodeSlotShaderPassOutput : 
	public NodeSlotOutput
{
public:
	static NodeSlotShaderPassOutputPtr Create(NodeSlotShaderPassOutput vSlot);
	static NodeSlotShaderPassOutputPtr Create(const std::string& vName, const uint32_t& vBindingPoint);
	static NodeSlotShaderPassOutputPtr Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName);
	static NodeSlotShaderPassOutputPtr Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget);

public:
	explicit NodeSlotShaderPassOutput();
	explicit NodeSlotShaderPassOutput(const std::string& vName, const uint32_t& vBindingPoint);
	explicit NodeSlotShaderPassOutput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName);
	explicit NodeSlotShaderPassOutput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget);
	~NodeSlotShaderPassOutput();

	void Init();
	void Unit();

	void SendFrontNotification(const NotifyEvent& vEvent) override;

	void MouseDoubleClickedOnSlot(const ImGuiMouseButton& vMouseButton) override;

	void DrawDebugInfos();
};