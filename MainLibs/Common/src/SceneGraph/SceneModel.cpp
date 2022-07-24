// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include <SceneGraph/SceneModel.h>
#include <Systems/CommonSystem.h>

SceneModelPtr SceneModel::Create()
{
	auto res = std::make_shared<SceneModel>();
	res->m_This = res;
	return res;
}

std::vector<SceneMeshPtr>::iterator SceneModel::begin()
{
	return m_Meshs.begin();
}

std::vector<SceneMeshPtr>::iterator SceneModel::end()
{
	return m_Meshs.end();
}

size_t SceneModel::size()
{
	return m_Meshs.size();
}

void SceneModel::clear()
{
	m_Meshs.clear();
	m_AABBCC.lowerBound = 1e7;
	m_AABBCC.upperBound = -1e7;
}

bool SceneModel::empty()
{
	return m_Meshs.empty();
}

void SceneModel::Add(const SceneMeshPtr& vMesh)
{
	m_Meshs.push_back(vMesh);
}

SceneMeshWeak SceneModel::Get(const size_t& vIndex)
{
	if (m_Meshs.size() > (size_t)vIndex)
	{
		return m_Meshs[(size_t)vIndex];
	}

	return SceneMeshWeak();
}

void SceneModel::CombinePointInBoundingBox(const ct::fvec3& vPoint)
{
	m_AABBCC.Combine(vPoint);
}

void SceneModel::CenterCameraToModel()
{
	auto centerPos = m_AABBCC.GetCenter();
	CommonSystem::Instance()->SetTargetXYZ(-centerPos, true);
}

ct::fvec3 SceneModel::GetCenter()
{
	return m_AABBCC.GetCenter();
}
