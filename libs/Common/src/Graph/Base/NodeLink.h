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
#include <uTypes/uTypes.h>
#include <Common/Globals.h>
#include <imgui/imgui.h>
#include <string>
#include <set>
#include <memory>

#include <imgui_node_editor/NodeEditor/Include/imgui_node_editor.h>
namespace nd = ax::NodeEditor;

class BaseNode;
class NodeSlot;
class COMMON_API NodeLink
{
public:
	NodeSlotWeak in;
	NodeSlotWeak out;
	uint32_t linkId = 0;
	ImColor color = ImColor(255, 255, 0, 255);
	float thick = 2.0f;

public:
	NodeLink();
	~NodeLink();
};