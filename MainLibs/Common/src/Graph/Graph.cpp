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

#include "Graph.h"

#include <array>

std::string Graph::GetStringFromNodeTypeEnum(const NodeTypeEnum& vNodeTypeEnum)
{
	static std::array<std::string, (uint8_t)NodeTypeEnum::Count> NodeTypeString = {
		"NONE",
		"MESH",
		"MESH_GROUP",
		"MESH_ATTRIBUTES",
		"TEXTURE_2D",
		"COMPUTE_SMOOTH_MESH_NORMAL",
		"PBR_RENDERER",
		"CHANNEL_RENDERER",
		"HEATMAP_RENDERER",
		"MATCAP_RENDERER",
		"LIGHT",
		"SHADOW_MAPPING",
		"MODEL_SHADOW",
		"SHADOW_MAPPING_OMNI",
		"SSAO",
		"BLUR",
		"LAPLACIAN",
		"ENVIRONMENT",
		"GRID_AXIS",
		"OUTPUT",
		"SCENE_MERGER",
		"DEFERRED_RENDERER",
		"POST_PRO_RENDERER",
		"POS_TO_DEPTH",
		"DEPTH_TO_POS",
		"TYPE_BOOLEAN",
	};
	if (vNodeTypeEnum != NodeTypeEnum::Count)
		return NodeTypeString[(int)vNodeTypeEnum];
	LogVarDebug("Error, one NoteTypeEnum have no corresponding string, return \"None\"");

	return "NONE";
}

NodeTypeEnum Graph::GetNodeTypeEnumFromString(const std::string& vNodeTypeString)
{
	if (vNodeTypeString == "NONE") return NodeTypeEnum::NONE;
	else if (vNodeTypeString == "MESH") return NodeTypeEnum::MESH;
	else if (vNodeTypeString == "MESH_GROUP") return NodeTypeEnum::MESH_GROUP;
	else if (vNodeTypeString == "MESH_ATTRIBUTES") return NodeTypeEnum::MESH_ATTRIBUTES;
	else if (vNodeTypeString == "TEXTURE_2D") return NodeTypeEnum::TEXTURE_2D;
	else if (vNodeTypeString == "COMPUTE_SMOOTH_MESH_NORMAL") return NodeTypeEnum::COMPUTE_SMOOTH_MESH_NORMAL;
	else if (vNodeTypeString == "PBR_RENDERER") return NodeTypeEnum::PBR_RENDERER;
	else if (vNodeTypeString == "CHANNEL_RENDERER") return NodeTypeEnum::CHANNEL_RENDERER;
	else if (vNodeTypeString == "HEATMAP_RENDERER") return NodeTypeEnum::HEATMAP_RENDERER;
	else if (vNodeTypeString == "MATCAP_RENDERER") return NodeTypeEnum::MATCAP_RENDERER;
	else if (vNodeTypeString == "LIGHT") return NodeTypeEnum::LIGHT;
	else if (vNodeTypeString == "SHADOW_MAPPING") return NodeTypeEnum::SHADOW_MAPPING;
	else if (vNodeTypeString == "MODEL_SHADOW") return NodeTypeEnum::MODEL_SHADOW;
	else if (vNodeTypeString == "SHADOW_MAPPING_OMNI") return NodeTypeEnum::SHADOW_MAPPING_OMNI;
	else if (vNodeTypeString == "SSAO") return NodeTypeEnum::SSAO;
	else if (vNodeTypeString == "BLUR") return NodeTypeEnum::BLUR;
	else if (vNodeTypeString == "LAPLACIAN") return NodeTypeEnum::LAPLACIAN;
	else if (vNodeTypeString == "ENVIRONMENT") return NodeTypeEnum::ENVIRONMENT;
	else if (vNodeTypeString == "GRID_AXIS") return NodeTypeEnum::GRID_AXIS;
	else if (vNodeTypeString == "OUTPUT") return NodeTypeEnum::OUTPUT;
	else if (vNodeTypeString == "SCENE_MERGER") return NodeTypeEnum::SCENE_MERGER;
	else if (vNodeTypeString == "DEFERRED_RENDERER") return NodeTypeEnum::DEFERRED_RENDERER;
	else if (vNodeTypeString == "POST_PRO_RENDERER") return NodeTypeEnum::POST_PRO_RENDERER;
	else if (vNodeTypeString == "POS_TO_DEPTH") return NodeTypeEnum::POS_TO_DEPTH;
	else if (vNodeTypeString == "DEPTH_TO_POS") return NodeTypeEnum::DEPTH_TO_POS;
	else if (vNodeTypeString == "TYPE_BOOLEAN") return NodeTypeEnum::TYPE_BOOLEAN;

	return NodeTypeEnum::NONE;
}