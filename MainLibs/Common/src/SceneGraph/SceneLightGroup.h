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
#include <ctools/cTools.h>
#include <SceneGraph/SceneLight.h>

class SceneLightGroup;
typedef std::shared_ptr<SceneLightGroup> SceneLightGroupPtr;
typedef ct::cWeak<SceneLightGroup> SceneLightGroupWeak;

class SceneLightGroup
{
public:
	static SceneLightGroupPtr Create();

private:
	SceneLightGroupWeak m_This;
	std::vector<SceneLightPtr> m_Lights;

public:
	void clear();
	bool empty();
	size_t size();
	std::vector<SceneLightPtr>::iterator begin();
	std::vector<SceneLightPtr>::iterator end();
	void Add(const SceneLightPtr& vLight);
	SceneLightWeak Get(const size_t& vIndex);
};