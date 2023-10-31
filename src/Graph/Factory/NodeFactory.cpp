/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

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

#include <Nodes/Assets/Load/MeshNode.h>
#include <Nodes/Assets/Load/Texture2DNode.h>
#include <Nodes/Assets/Load/CubeMapNode.h>

#include <Nodes/Assets/Save/ModelExporterNode.h>
#include <Nodes/Assets/Save/TextureExporterNode.h>

#include <Nodes/Misc/GridNode.h>
#include <Nodes/Misc/SceneMergerNode.h>
#include <Nodes/Misc/AnimatorNode.h>

#include <Nodes/Renderers/MatcapRendererNode.h>
#include <Nodes/Renderers/ChannelRendererNode.h>
#include <Nodes/Renderers/HeatmapRendererNode.h>
#include <Nodes/Renderers/ModelRendererNode.h>

#include <Nodes/Widgets/VariableNode.h>
#include <Nodes/Widgets/WidgetColorNode.h>

#include <Nodes/Utils/MathNode.h>

BaseNodePtr NodeFactory::CreateNode(BaseNodeWeak vNodeGraph, const std::string& vNodeType) {
    auto graphPtr = vNodeGraph.lock();
    if (graphPtr) {
        auto corePtr = graphPtr->m_VulkanCorePtr;
        if (corePtr) {
            // Assets
            if (vNodeType == "MESH")
                return MeshNode::Create(corePtr);
            else if (vNodeType == "TEXTURE_2D")
                return Texture2DNode::Create(corePtr);
            else if (vNodeType == "CUBE_MAP")
                return CubeMapNode::Create(corePtr);

            // Exporters
            else if (vNodeType == "MODEL_EXPORTER")
                return ModelExporterNode::Create(corePtr);
            else if (vNodeType == "TEXTURE_2D_EXPORTER")
                return TextureExporterNode::Create(corePtr);
            
            // Misc
            else if (vNodeType == "ANIMATOR")
                return AnimatorNode::Create(corePtr);
            else if (vNodeType == "GRID_AXIS")
                return GridNode::Create(corePtr);
            else if (vNodeType == "SCENE_MERGER")
                return SceneMergerNode::Create(corePtr);

            // renderers
            else if (vNodeType == "CHANNEL_RENDERER")
                return ChannelRendererNode::Create(corePtr);
            else if (vNodeType == "HEATMAP_RENDERER")
                return HeatmapRendererNode::Create(corePtr);
            else if (vNodeType == "MATCAP_RENDERER")
                return MatcapRendererNode::Create(corePtr);
            else if (vNodeType == "MODEL_RENDERER")
                return ModelRendererNode::Create(corePtr);

            // Variables
            else if (vNodeType == "WIDGET_BOOLEAN")
                return VariableNode::Create(corePtr, vNodeType);
            else if (vNodeType == "WIDGET_FLOAT")
                return VariableNode::Create(corePtr, vNodeType);
            else if (vNodeType == "WIDGET_INT")
                return VariableNode::Create(corePtr, vNodeType);
            else if (vNodeType == "WIDGET_UINT")
                return VariableNode::Create(corePtr, vNodeType);
            else if (vNodeType == "WIDGET_COLOR")
                return WidgetColorNode::Create(corePtr);

            // Utils
            else if (vNodeType == "MATH")
                return MathNode::Create(corePtr);
        }
    }

    return nullptr;
}