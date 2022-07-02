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
	SSS_MAPPING,
	MODEL_SHADOW,
	MODEL_SSS,
	SHADOW_MAPPING_OMNI,
	SSAO,
	BLUR,
	LAPLACIAN,
	ENVIRONMENT,
	GRID_AXIS,
	OUTPUT,
	SCENE_MERGER,
	DEFERRED_RENDERER,
	POST_PRO_RENDERER,
	POS_TO_DEPTH,
	DEPTH_TO_POS,
	TYPE_BOOLEAN,
	TYPE_UINT,
	TYPE_INT,
	TYPE_FLOAT,
	Count,
};

class Graph
{
public:
	static std::string GetStringFromNodeTypeEnum(const NodeTypeEnum& vNodeTypeEnum);
	static NodeTypeEnum GetNodeTypeEnumFromString(const std::string& vNodeTypeString);
};
