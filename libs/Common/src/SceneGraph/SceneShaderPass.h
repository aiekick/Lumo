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
#include <Base/ShaderPass.h>
#include <Common/Globals.h>

class SceneShaderPass;
typedef std::shared_ptr<SceneShaderPass> SceneShaderPassPtr;
typedef ct::cWeak<SceneShaderPass> SceneShaderPassWeak;
typedef std::unordered_map<uint32_t, SceneShaderPassWeak> SceneShaderPassContainer;

// class qui va contenir un enesemble de shaderpass
// cat un merger peut etre connecté a d'autre merger, 
// et donc un ensemble de pluieurs passes peut etre mit sur un slot input
// au lieu de jsute 1, donc cet un objet simple qui cotient des passes en provenance d'autre nodes

class COMMON_API SceneShaderPass
{
public:
	static SceneShaderPassPtr Create();

private:
	SceneShaderPassWeak m_This;
	std::vector<ShaderPassWeak> m_ShaderPasses;

public:
	void clear();
	bool empty();
	size_t size();
	ShaderPassWeak at(const size_t& vIdx) const;
	std::vector<ShaderPassWeak>::iterator begin();
	std::vector<ShaderPassWeak>::iterator end();
	void Add(const ShaderPassWeak& vShaderPass);
	ShaderPassWeak Get(const size_t& vIndex);
};