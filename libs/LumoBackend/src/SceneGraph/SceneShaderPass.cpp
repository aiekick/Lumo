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

#include <LumoBackend/SceneGraph/SceneShaderPass.h>
#include <LumoBackend/Systems/CommonSystem.h>

SceneShaderPassPtr SceneShaderPass::Create()
{
	auto res = std::make_shared<SceneShaderPass>();
	res->m_This = res;
	return res;
}

ShaderPassWeak SceneShaderPass::at(const size_t& vIdx) const
{
	if (vIdx < m_ShaderPasses.size())
	{
		return m_ShaderPasses.at(vIdx);
	}

	return ShaderPassWeak();
}

std::vector<ShaderPassWeak>::iterator SceneShaderPass::begin()
{
	return m_ShaderPasses.begin();
}

std::vector<ShaderPassWeak>::iterator SceneShaderPass::end()
{
	return m_ShaderPasses.end();
}

size_t SceneShaderPass::size()
{
	return m_ShaderPasses.size();
}

void SceneShaderPass::clear()
{
	m_ShaderPasses.clear();
}

bool SceneShaderPass::empty()
{
	return m_ShaderPasses.empty();
}

void SceneShaderPass::Add(const ShaderPassWeak& vMesh)
{
	m_ShaderPasses.push_back(vMesh);
}

ShaderPassWeak SceneShaderPass::Get(const size_t& vIndex)
{
	if (m_ShaderPasses.size() > vIndex)
	{
		return m_ShaderPasses.at(vIndex);
	}

	return ShaderPassWeak();
}
