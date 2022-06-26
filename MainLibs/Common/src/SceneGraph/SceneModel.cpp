/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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
