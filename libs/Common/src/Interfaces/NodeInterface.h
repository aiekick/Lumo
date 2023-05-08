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

#include <vector>
#include <Graph/Graph.h>
#include <ctools/cTools.h>

class NodeInterface
{
protected:
	BaseNodeWeak m_ParentNode; // node parent dans le cas d'un ndoe enfant d'un graph

public:
	void SetParentNode(BaseNodeWeak vBaseNodeWeak = BaseNodeWeak())
	{
		m_ParentNode = vBaseNodeWeak;
	}
	
	BaseNodeWeak GetParentNode() 
	{
		return m_ParentNode;
	}

	// draw the widget of the node (so not related to input or output widgets)
	virtual bool DrawNodeWidget(const uint32_t& /*vCurrentFrame*/, ImGuiContext* vContext)
	{
		assert(vContext);
		ImGui::SetCurrentContext(vContext);
		return false; 
	}

	// when XML loading of the node will began
	virtual void BeforeNodeXmlLoading() {}

	// when XML loading of the node is finished
	virtual void AfterNodeXmlLoading() {}
};