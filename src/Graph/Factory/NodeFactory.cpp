/*
Copyright 2022-2022 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http"//www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "NodeFactory.h"

#include <Graph/Nodes/Assets/MeshNode.h>
#include <Graph/Nodes/Assets/Texture2DNode.h>

#include <Graph/Nodes/Breaks/BreakTexturesGroupNode.h>

#include <Graph/Nodes/Divers/GridNode.h>

#include <Graph/Nodes/Simulation/GrayScottNode.h>

#include <Graph/Nodes/Lighting/LightGroupNode.h>
#include <Graph/Nodes/Lighting/DiffuseNode.h>
#include <Graph/Nodes/Lighting/SpecularNode.h>
#include <Graph/Nodes/Lighting/ShadowMapNode.h>
#include <Graph/Nodes/Lighting/ModelShadowNode.h>

#include <Graph/Nodes/Modifiers/SmoothNormalNode.h>

#include <Graph/Nodes/Output/Output3DNode.h>
#include <Graph/Nodes/Output/Output2DNode.h>

#include <Graph/Nodes/PostPro/SSAONode.h>
#include <Graph/Nodes/PostPro/BlurNode.h>
#include <Graph/Nodes/PostPro/LaplacianNode.h>
#include <Graph/Nodes/PostPro/ToneMapNode.h>

#include <Graph/Nodes/Renderers/MatcapRendererNode.h>
#include <Graph/Nodes/Renderers/ChannelRendererNode.h>
#include <Graph/Nodes/Renderers/HeatmapRendererNode.h>
#include <Graph/Nodes/Renderers/DeferredRendererNode.h>
#include <Graph/Nodes/Renderers/PBRRendererNode.h>

#include <Graph/Nodes/Utils/MathNode.h>
#include <Graph/Nodes/Utils/DepthToPosNode.h>
#include <Graph/Nodes/Utils/PosToDepthNode.h>
#include <Graph/Nodes/Utils/MeshAttributesNode.h>

#include <Graph/Nodes/Variables/VariableNode.h>

BaseNodePtr NodeFactory::CreateNode(BaseNodeWeak vNodeGraph, const std::string& vNodeType)
{
	auto graphPtr = vNodeGraph.getValidShared();
	if (graphPtr)
	{
		auto corePtr = graphPtr->m_VulkanCorePtr;
		if (corePtr)
		{
			// assets
			if (vNodeType == "MESH")								return MeshNode::Create(corePtr);
			else if (vNodeType == "TEXTURE_2D")						return Texture2DNode::Create(corePtr);

			// Divers
			else if (vNodeType == "BREAK_TEXTURE_2D_GROUP")			return BreakTexturesGroupNode::Create(corePtr);

			// Divers
			else if (vNodeType == "GRID_AXIS")						return GridNode::Create(corePtr);

			// Lighting
			else if (vNodeType == "LIGHT_GROUP")					return LightGroupNode::Create(corePtr);
			else if (vNodeType == "SHADOW_MAPPING")					return ShadowMapNode::Create(corePtr);
			else if (vNodeType == "MODEL_SHADOW")					return ModelShadowNode::Create(corePtr);
			else if (vNodeType == "DIFFUSE")						return DiffuseNode::Create(corePtr);
			else if (vNodeType == "SPECULAR")						return SpecularNode::Create(corePtr);
			else if (vNodeType == "SSAO")							return SSAONode::Create(corePtr);

			// Modifiers
			else if (vNodeType == "SMOOTH_NORMAL")					return SmoothNormalNode::Create(corePtr);

			// graph output
			else if (vNodeType == "OUTPUT_3D")						return Output3DNode::Create(corePtr);
			else if (vNodeType == "OUTPUT_2D")						return Output2DNode::Create(corePtr);

			// Post Processing
			else if (vNodeType == "BLUR")							return BlurNode::Create(corePtr);
			else if (vNodeType == "LAPLACIAN")						return LaplacianNode::Create(corePtr);
			else if (vNodeType == "TONE_MAP")						return ToneMapNode::Create(corePtr);

			// renderers
			else if (vNodeType == "CHANNEL_RENDERER")				return ChannelRendererNode::Create(corePtr);
			else if (vNodeType == "DEFERRED_RENDERER")				return DeferredRendererNode::Create(corePtr);
			else if (vNodeType == "HEATMAP_RENDERER")				return HeatmapRendererNode::Create(corePtr);
			else if (vNodeType == "MATCAP_RENDERER")				return MatcapRendererNode::Create(corePtr);
			else if (vNodeType == "PBR_RENDERER")					return PBRRendererNode::Create(corePtr);

			// Simulations
			else if (vNodeType == "GRAY_SCOTT_SIMULATION")			return GrayScottNode::Create(corePtr);

			// Utils
			else if (vNodeType == "DEPTH_TO_POS")					return DepthToPosNode::Create(corePtr);
			else if (vNodeType == "MESH_ATTRIBUTES")				return MeshAttributesNode::Create(corePtr);
			else if (vNodeType == "POS_TO_DEPTH")					return PosToDepthNode::Create(corePtr);
			else if (vNodeType == "MATH")							return MathNode::Create(corePtr);

			// Variables
			else if (vNodeType == "TYPE_BOOLEAN")					return VariableNode::Create(corePtr, vNodeType);
			else if (vNodeType == "TYPE_FLOAT")						return VariableNode::Create(corePtr, vNodeType);
			else if (vNodeType == "TYPE_INT")						return VariableNode::Create(corePtr, vNodeType);
			else if (vNodeType == "TYPE_UINT")						return VariableNode::Create(corePtr, vNodeType);
		}
	}

	return nullptr;
}