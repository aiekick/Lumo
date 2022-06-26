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