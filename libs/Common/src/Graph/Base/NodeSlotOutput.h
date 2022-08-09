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

#include <Graph/Graph.h>
#include <Graph/Base/NodeSlot.h>

class NodeSlotOutput : public NodeSlot
{
public:
	static NodeSlotOutputPtr Create();
	static NodeSlotOutputPtr Create(std::string vName);
	static NodeSlotOutputPtr Create(std::string vName, std::string vType);
	static NodeSlotOutputPtr Create(std::string vName, std::string vType, bool vHideName);
	static NodeSlotOutputPtr Create(std::string vName, std::string vType, bool vHideName, bool vShowWidget);

public:
	explicit NodeSlotOutput();
	explicit NodeSlotOutput(std::string vName);
	explicit NodeSlotOutput(std::string vName, std::string vType);
	explicit NodeSlotOutput(std::string vName, std::string vType, bool vHideName);
	explicit NodeSlotOutput(std::string vName, std::string vType, bool vHideName, bool vShowWidget);
	~NodeSlotOutput();

	void Init();
	void Unit();

	void DrawDebugInfos();

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};