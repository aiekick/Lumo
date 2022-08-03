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
#include <Graph/Base/NodeSlotInput.h>

class NodeSlotVariableInput;
typedef ct::cWeak<NodeSlotVariableInput> NodeSlotVariableInputWeak;
typedef std::shared_ptr<NodeSlotVariableInput> NodeSlotVariableInputPtr;

class NodeSlotVariableInput : 
	public NodeSlotInput
{
public:
	static NodeSlotVariableInputPtr Create(NodeSlotVariableInput vSlot);
	static NodeSlotVariableInputPtr Create(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex);
	static NodeSlotVariableInputPtr Create(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName);
	static NodeSlotVariableInputPtr Create(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName, const bool& vShowWidget);

public:
	explicit NodeSlotVariableInput();
	explicit NodeSlotVariableInput(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex);
	explicit NodeSlotVariableInput(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName);
	explicit NodeSlotVariableInput(const std::string& vName, const std::string& vType, const uint32_t& vVariableIndex, const bool& vHideName, const bool& vShowWidget);
	~NodeSlotVariableInput();

	void Init();
	void Unit();

	void Connect(NodeSlotWeak vOtherSlot) override;
	void DisConnect(NodeSlotWeak vOtherSlot) override;

	void TreatNotification(
		const NotifyEvent& vEvent,
		const NodeSlotWeak& vEmitterSlot = NodeSlotWeak(),
		const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());

	void DrawDebugInfos();

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};