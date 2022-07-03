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

#include "NodeFactory.h"

#include <Graph/Nodes/Assets/MeshNode.h>
#include <Graph/Nodes/Assets/Texture2DNode.h>

#include <Graph/Nodes/Divers/GridNode.h>

#include <Graph/Nodes/Lighting/LightNode.h>
#include <Graph/Nodes/Lighting/ShadowMapNode.h>
#include <Graph/Nodes/Lighting/SSSMapNode.h>
#include <Graph/Nodes/Lighting/ModelShadowNode.h>
#include <Graph/Nodes/Lighting/ModelSSSNode.h>

#include <Graph/Nodes/Modifiers/ComputeSmoothMeshNormalNode.h>

#include <Graph/Nodes/Output/OutputNode.h>

#include <Graph/Nodes/PostPro/SSAONode.h>
#include <Graph/Nodes/PostPro/BlurNode.h>
#include <Graph/Nodes/PostPro/LaplacianNode.h>

#include <Graph/Nodes/Renderers/ChannelRendererNode.h>
#include <Graph/Nodes/Renderers/DeferredRendererNode.h>
#include <Graph/Nodes/Renderers/HeatmapRendererNode.h>
#include <Graph/Nodes/Renderers/MatcapRendererNode.h>

#include <Graph/Nodes/Utils/MeshAttributesNode.h>
#include <Graph/Nodes/Utils/DepthToPosNode.h>
#include <Graph/Nodes/Utils/PosToDepthNode.h>

#include <Graph/Nodes/Variables/VariableNode.h>

BaseNodePtr NodeFactory::CreateNode(BaseNodeWeak vNodeGraph, const std::string& vNodeType)
{
	return CreateNode(vNodeGraph, Graph::GetNodeTypeEnumFromString(vNodeType));
}

BaseNodePtr NodeFactory::CreateNode(BaseNodeWeak vNodeGraph, const NodeTypeEnum& vNodeType)
{
	auto graphPtr = vNodeGraph.getValidShared();
	if (graphPtr)
	{
		auto vulkanCore = graphPtr->m_VulkanCorePtr;
		switch (vNodeType)
		{
		// assets
		case NodeTypeEnum::MESH:						return MeshNode::Create(vulkanCore);
		case NodeTypeEnum::TEXTURE_2D:					return Texture2DNode::Create(vulkanCore);

		// Divers
		case NodeTypeEnum::GRID_AXIS:					return GridNode::Create(vulkanCore);

		// Lighting
		case NodeTypeEnum::LIGHT:						return LightNode::Create(vulkanCore);
		case NodeTypeEnum::SHADOW_MAPPING:				return ShadowMapNode::Create(vulkanCore);
		case NodeTypeEnum::SSS_MAPPING:					return SSSMapNode::Create(vulkanCore);
		case NodeTypeEnum::MODEL_SHADOW:				return ModelShadowNode::Create(vulkanCore);
		case NodeTypeEnum::MODEL_SSS:					return ModelSSSNode::Create(vulkanCore);

		// Modifiers
		case NodeTypeEnum::COMPUTE_SMOOTH_MESH_NORMAL:	return ComputeSmoothMeshNormalNode::Create(vulkanCore);

			// graph output
		case NodeTypeEnum::OUTPUT:						return OutputNode::Create(vulkanCore);

		// Post Processing
		case NodeTypeEnum::SSAO:						return SSAONode::Create(vulkanCore);
		case NodeTypeEnum::BLUR:						return BlurNode::Create(vulkanCore);
		case NodeTypeEnum::LAPLACIAN:					return LaplacianNode::Create(vulkanCore);

		// renderers
		case NodeTypeEnum::CHANNEL_RENDERER:			return ChannelRendererNode::Create(vulkanCore);
		case NodeTypeEnum::DEFERRED_RENDERER:			return DeferredRendererNode::Create(vulkanCore);
		case NodeTypeEnum::HEATMAP_RENDERER:			return HeatmapRendererNode::Create(vulkanCore);
		case NodeTypeEnum::MATCAP_RENDERER:				return MatcapRendererNode::Create(vulkanCore);

		// Utils
		case NodeTypeEnum::DEPTH_TO_POS:				return DepthToPosNode::Create(vulkanCore);
		case NodeTypeEnum::MESH_ATTRIBUTES:				return MeshAttributesNode::Create(vulkanCore);
		case NodeTypeEnum::POS_TO_DEPTH:				return PosToDepthNode::Create(vulkanCore);

		// Variables
		case NodeTypeEnum::TYPE_BOOLEAN:
		case NodeTypeEnum::TYPE_FLOAT:
		case NodeTypeEnum::TYPE_INT:
		case NodeTypeEnum::TYPE_UINT:
			return VariableNode::Create(vulkanCore, vNodeType);

		default:
			break;
		};
	}

	return nullptr;
}