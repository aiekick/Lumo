/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

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

#include <string>
#include <memory>
#include <LumoBackend/Graph/Graph.h>
#include <ctools/cTools.h>
#include <Modules/GeneratorNode.h>

class FilesSaver
{
public: // to save

	std::string m_GenerationRootPath;

public:
    void GenerateGraphFiles(const GeneratorNodeWeak& vRootNode, const std::string& vRootPath);

private:
    void CustomSceneGraphItem(const std::string& vRootPath, const std::string& vSceneGraphItemName);

public:
	static FilesSaver* Instance()
	{
		static FilesSaver _instance;
		return &_instance;
	}
};

