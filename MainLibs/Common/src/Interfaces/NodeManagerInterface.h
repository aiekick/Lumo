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
#include <Graph/Base/BaseNode.h>

class NodeManagerInterface
{
public:
	virtual void Select(BaseNodeWeak) = 0;
	virtual BaseNodeWeak ShowNewNodeMenu(BaseNodeWeak, BaseNodeStateStruct*) = 0;
	virtual bool LoadNodeFromXML(BaseNodeWeak, tinyxml2::XMLElement*, tinyxml2::XMLElement*,
		const std::string&, const NodeTypeEnum&, const ct::fvec2&, const size_t&) = 0;
};