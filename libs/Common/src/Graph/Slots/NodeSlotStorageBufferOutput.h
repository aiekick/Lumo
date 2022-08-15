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

class NodeSlotStorageBufferOutput;
typedef ct::cWeak<NodeSlotStorageBufferOutput> NodeSlotStorageBufferOutputWeak;
typedef std::shared_ptr<NodeSlotStorageBufferOutput> NodeSlotStorageBufferOutputPtr;

class NodeSlotStorageBufferOutput : 
	public NodeSlotOutput
{
public:
	static NodeSlotStorageBufferOutputPtr Create(NodeSlotStorageBufferOutput vSlot);
	static NodeSlotStorageBufferOutputPtr Create(const std::string& vName, const std::string& vType);
	static NodeSlotStorageBufferOutputPtr Create(const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint);
	static NodeSlotStorageBufferOutputPtr Create(const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint, const bool& vHideName);
	static NodeSlotStorageBufferOutputPtr Create(const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget);

public:
	explicit NodeSlotStorageBufferOutput();
	explicit NodeSlotStorageBufferOutput(const std::string& vName, const std::string& vType);
	explicit NodeSlotStorageBufferOutput(const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint);
	explicit NodeSlotStorageBufferOutput(const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint, const bool& vHideName);
	explicit NodeSlotStorageBufferOutput(const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget);
	~NodeSlotStorageBufferOutput();

	void Init();
	void Unit();

	void SendFrontNotification(const NotifyEvent& vEvent) override;

	void DrawDebugInfos();
};