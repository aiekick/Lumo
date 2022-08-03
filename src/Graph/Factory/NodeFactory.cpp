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

#include <Graph/Nodes/Output/Output3DNode.h>
#include <Graph/Nodes/Output/Output2DNode.h>

BaseNodePtr NodeFactory::CreateNode(BaseNodeWeak vNodeGraph, const std::string& vNodeType)
{
	auto graphPtr = vNodeGraph.getValidShared();
	if (graphPtr)
	{
		auto corePtr = graphPtr->m_VulkanCorePtr;
		if (corePtr)
		{
			// graph output
			if (vNodeType == "OUTPUT_3D")						return Output3DNode::Create(corePtr);
			else if (vNodeType == "OUTPUT_2D")					return Output2DNode::Create(corePtr);
		}
	}

	return nullptr;
}