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

#pragma once

#include <vector>
#include <ctools/cTools.h>
#include <SceneGraph/SceneMesh.h>

class SceneModel;
typedef std::shared_ptr<SceneModel> SceneModelPtr;
typedef ct::cWeak<SceneModel> SceneModelWeak;

class SceneModel
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
	std::vector<SceneMeshPtr>::iterator begin();
	std::vector<SceneMeshPtr>::iterator end();
	void Add(const SceneMeshPtr& vSubMesh);
	SceneMeshWeak Get(const size_t& vIndex);
	void CombinePointInBoundingBox(const ct::fvec3& vPoint);
	void CenterCameraToModel();
	ct::fvec3 GetCenter();
};