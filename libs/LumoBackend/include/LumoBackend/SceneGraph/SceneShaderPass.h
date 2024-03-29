/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

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
#pragma warning(disable : 4251)

#include <vector>
#include <ctools/cTools.h>
#include <LumoBackend/Base/Base.h>
#include <LumoBackend/Base/ShaderPass.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

class SceneShaderPass;
typedef std::shared_ptr<SceneShaderPass> SceneShaderPassPtr;
typedef std::weak_ptr<SceneShaderPass> SceneShaderPassWeak;
typedef std::unordered_map<uint32_t, SceneShaderPassWeak> SceneShaderPassContainer;

// class qui va contenir un enesemble de shaderpass
// cat un merger peut etre connect� a d'autre merger, 
// et donc un ensemble de pluieurs passes peut etre mit sur un slot input
// au lieu de juste 1, donc c'est un objet simple qui contient des passes en provenance d'autres nodes

class LUMO_BACKEND_API SceneShaderPass
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