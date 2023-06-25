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
#include <Common/Globals.h>

class COMMON_API NodeSlotOutput : public NodeSlot
{
public:
	static NodeSlotOutputPtr Create();
	static NodeSlotOutputPtr Create(const std::string& vName);
	static NodeSlotOutputPtr Create(const std::string& vName, const std::string& vType);
	static NodeSlotOutputPtr Create(const std::string& vName, const std::string& vType, const bool& vHideName);
	static NodeSlotOutputPtr Create(const std::string& vName, const std::string& vType, const bool& vHideName, const bool& vShowWidget);

public:
	explicit NodeSlotOutput();
	explicit NodeSlotOutput(const std::string& vName);
	explicit NodeSlotOutput(const std::string& vName, const std::string& vType);
	explicit NodeSlotOutput(const std::string& vName, const std::string& vType, const bool& vHideName);
	explicit NodeSlotOutput(const std::string& vName, const std::string& vType, const bool& vHideName, const bool& vShowWidget);
	~NodeSlotOutput();

	void Init();
	void Unit();

	void DrawDebugInfos();
};