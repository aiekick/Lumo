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

class NodeSlotTexelBufferOutput;
typedef ct::cWeak<NodeSlotTexelBufferOutput> NodeSlotTexelBufferOutputWeak;
typedef std::shared_ptr<NodeSlotTexelBufferOutput> NodeSlotTexelBufferOutputPtr;

class NodeSlotTexelBufferOutput : 
	public NodeSlotOutput
{
public:
	static NodeSlotTexelBufferOutputPtr Create(NodeSlotTexelBufferOutput vSlot);
	static NodeSlotTexelBufferOutputPtr Create(const std::string& vName, const std::string& vType);
	static NodeSlotTexelBufferOutputPtr Create(const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint);
	static NodeSlotTexelBufferOutputPtr Create(const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint, const bool& vHideName);
	static NodeSlotTexelBufferOutputPtr Create(const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget);

public:
	explicit NodeSlotTexelBufferOutput();
	explicit NodeSlotTexelBufferOutput(const std::string& vName, const std::string& vType);
	explicit NodeSlotTexelBufferOutput(const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint);
	explicit NodeSlotTexelBufferOutput(const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint, const bool& vHideName);
	explicit NodeSlotTexelBufferOutput(const std::string& vName, const std::string& vType, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget);
	~NodeSlotTexelBufferOutput();

	void Init();
	void Unit();

	void SendFrontNotification(const NotifyEvent& vEvent) override;

	void DrawDebugInfos();

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};