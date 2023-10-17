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

#include <LumoBackend/Graph/Graph.h>
#include <LumoBackend/Graph/Base/NodeSlotInput.h>

class NodeSlotParticlesInput;
typedef std::weak_ptr<NodeSlotParticlesInput> NodeSlotParticlesInputWeak;
typedef std::shared_ptr<NodeSlotParticlesInput> NodeSlotParticlesInputPtr;

class NodeSlotParticlesInput : 
	public NodeSlotInput
{
public:
	static NodeSlotParticlesInputPtr Create(NodeSlotParticlesInput vSlot);
	static NodeSlotParticlesInputPtr Create(const std::string& vName);
	static NodeSlotParticlesInputPtr Create(const std::string& vName, const bool& vHideName);
	static NodeSlotParticlesInputPtr Create(const std::string& vName, const bool& vHideName, const bool& vShowWidget);

public:
	explicit NodeSlotParticlesInput();
	explicit NodeSlotParticlesInput(const std::string& vName);
	explicit NodeSlotParticlesInput(const std::string& vName, const bool& vHideName);
	explicit NodeSlotParticlesInput(const std::string& vName, const bool& vHideName, const bool& vShowWidget);
	~NodeSlotParticlesInput();

	void Init();
	void Unit();

	void OnConnectEvent(NodeSlotWeak vOtherSlot) override;
	void OnDisConnectEvent(NodeSlotWeak vOtherSlot) override;

	void TreatNotification(
		NotifyEvent vEvent,
		const NodeSlotWeak& vEmitterSlot = NodeSlotWeak(),
		const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());

	void DrawDebugInfos();

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};