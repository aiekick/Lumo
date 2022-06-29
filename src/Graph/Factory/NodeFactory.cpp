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

#include "NodeFactory.h"

#include <Graph/Nodes/Assets/MeshNode.h>
#include <Graph/Nodes/Assets/Texture2DNode.h>

#include <Graph/Nodes/Divers/GridNode.h>

#include <Graph/Nodes/Lighting/LightNode.h>
#include <Graph/Nodes/Lighting/ShadowMapNode.h>
#include <Graph/Nodes/Lighting/ModelShadowNode.h>

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
		auto vulkanCore = graphPtr->m_VulkanCore;
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
		case NodeTypeEnum::MODEL_SHADOW:				return ModelShadowNode::Create(vulkanCore);

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