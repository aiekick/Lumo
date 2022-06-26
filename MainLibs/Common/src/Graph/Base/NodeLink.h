/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <Graph/Graph.h>
#include <ctools/cTools.h>
#include <uTypes/uTypes.h>
#include <imgui/imgui.h>
#include <string>
#include <set>
#include <memory>

#include <imgui_node_editor/NodeEditor/Include/imgui_node_editor.h>
namespace nd = ax::NodeEditor;

class BaseNode;
class NodeSlot;
class NodeLink
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