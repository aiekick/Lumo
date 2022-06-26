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

#include <string>
#include <memory>
#include <ctools/cTools.h>
#include <ctools/Logger.h>

class BaseNode;
typedef ct::cWeak<BaseNode> BaseNodeWeak;
typedef std::shared_ptr<BaseNode> BaseNodePtr;

class NodeSlot;
typedef ct::cWeak<NodeSlot> NodeSlotWeak;
typedef std::shared_ptr<NodeSlot> NodeSlotPtr;

enum class NodeTypeEnum : uint8_t
{
	NONE = 0,
	MESH,
	MESH_GROUP,
	MESH_ATTRIBUTES,
	TEXTURE_2D,
	COMPUTE_SMOOTH_MESH_NORMAL,
	PBR_RENDERER,
	CHANNEL_RENDERER,
	HEATMAP_RENDERER,
	MATCAP_RENDERER,
	LIGHT,
	SHADOW_MAPPING,
	MODEL_SHADOW,
	SHADOW_MAPPING_OMNI,
	SSAO,
	ENVIRONMENT,
	GRID_AXIS,
	OUTPUT,
	SCENE_MERGER,
	DEFERRED_RENDERER,
	POST_PRO_RENDERER,
	POS_TO_DEPTH,
	DEPTH_TO_POS,
	Count,
};

class Graph
{
public:
	static std::string GetStringFromNodeTypeEnum(const NodeTypeEnum& vNodeTypeEnum);
	static NodeTypeEnum GetNodeTypeEnumFromString(const std::string& vNodeTypeString);
};
