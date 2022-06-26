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