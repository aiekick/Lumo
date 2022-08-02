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

class NodeSlotTextureInput;
typedef ct::cWeak<NodeSlotTextureInput> NodeSlotTextureInputWeak;
typedef std::shared_ptr<NodeSlotTextureInput> NodeSlotTextureInputPtr;

class NodeSlotTextureInput : 
	public NodeSlotInput
{
public:
	static NodeSlotTextureInputPtr Create(NodeSlotTextureInput vSlot);
	static NodeSlotTextureInputPtr Create(const std::string& vName);
	static NodeSlotTextureInputPtr Create(const std::string& vName, const uint32_t& vBindingPoint);
	static NodeSlotTextureInputPtr Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName);
	static NodeSlotTextureInputPtr Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget);

public:
	explicit NodeSlotTextureInput();
	explicit NodeSlotTextureInput(const std::string& vName);
	explicit NodeSlotTextureInput(const std::string& vName, const uint32_t& vBindingPoint);
	explicit NodeSlotTextureInput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName);
	explicit NodeSlotTextureInput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget);
	~NodeSlotTextureInput();

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