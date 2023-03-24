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

class NodeSlotShaderPassInput;
typedef ct::cWeak<NodeSlotShaderPassInput> NodeSlotShaderPassInputWeak;
typedef std::shared_ptr<NodeSlotShaderPassInput> NodeSlotShaderPassInputPtr;

class NodeSlotShaderPassInput : 
	public NodeSlotInput
{
public:
	static NodeSlotShaderPassInputPtr Create(NodeSlotShaderPassInput vSlot);
	static NodeSlotShaderPassInputPtr Create(const std::string& vName, const uint32_t& vBindingPoint);
	static NodeSlotShaderPassInputPtr Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName);
	static NodeSlotShaderPassInputPtr Create(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget);

public:
	explicit NodeSlotShaderPassInput();
	explicit NodeSlotShaderPassInput(const std::string& vName, const uint32_t& vBindingPoint);
	explicit NodeSlotShaderPassInput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName);
	explicit NodeSlotShaderPassInput(const std::string& vName, const uint32_t& vBindingPoint, const bool& vHideName, const bool& vShowWidget);
	~NodeSlotShaderPassInput();

	void Init();
	void Unit();

	void Connect(NodeSlotWeak vOtherSlot) override;
	void DisConnect(NodeSlotWeak vOtherSlot) override;

	void TreatNotification(
		const NotifyEvent& vEvent,
		const NodeSlotWeak& vEmitterSlot = NodeSlotWeak(),
		const NodeSlotWeak& vReceiverSlot = NodeSlotWeak());

	void MouseDoubleClickedOnSlot(const ImGuiMouseButton& vMouseButton) override;

	void DrawDebugInfos();
};