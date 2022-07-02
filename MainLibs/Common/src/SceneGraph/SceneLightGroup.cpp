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

#include <SceneGraph/SceneLightGroup.h>

SceneLightGroupPtr SceneLightGroup::Create()
{
	auto res = std::make_shared<SceneLightGroup>();
	res->m_This = res;
	return res;
}

std::vector<SceneLightPtr>::iterator SceneLightGroup::begin()
{
	return m_Lights.begin();
}

std::vector<SceneLightPtr>::iterator SceneLightGroup::end()
{
	return m_Lights.end();
}

size_t SceneLightGroup::size()
{
	return m_Lights.size();
}

void SceneLightGroup::clear()
{
	m_Lights.clear();
}

bool SceneLightGroup::empty()
{
	return m_Lights.empty();
}

void SceneLightGroup::Add(const SceneLightPtr& vLight)
{
	m_Lights.push_back(vLight);
}

SceneLightWeak SceneLightGroup::Get(const size_t& vIndex)
{
	if (m_Lights.size() > (size_t)vIndex)
	{
		return m_Lights[(size_t)vIndex];
	}

	return SceneLightWeak();
}