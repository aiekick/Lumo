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
#include <SceneGraph/SceneMesh.hpp>
#include <Common/Globals.h>

class SceneModel;
typedef std::shared_ptr<SceneModel> SceneModelPtr;
typedef ct::cWeak<SceneModel> SceneModelWeak;

class COMMON_API SceneModel
{
public:
	static SceneModelPtr Create();

private:
	SceneModelWeak m_This;
	std::vector<SceneMeshPtr> m_Meshs;
	ct::fAABBCC m_AABBCC;

public:
	void clear();
	bool empty();
	size_t size();
	SceneMeshWeak at(const size_t& vIdx) const;
	std::vector<SceneMeshPtr>::iterator begin();
	std::vector<SceneMeshPtr>::iterator end();
	void Add(const SceneMeshPtr& vSubMesh);
	SceneMeshWeak Get(const size_t& vIndex);
	void CombinePointInBoundingBox(const ct::fvec3& vPoint);
	void CenterCameraToModel();
	ct::fvec3 GetCenter();
};