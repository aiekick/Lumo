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
#pragma warning(disable : 4251)

#include <cstdint>
#include <memory>
#include <ctools/cTools.h>
#include <LumoBackend/Graph/Graph.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

class SceneVariable;
typedef std::shared_ptr<SceneVariable> SceneVariablePtr;
typedef std::weak_ptr<SceneVariable> SceneVariableWeak;

class LUMO_BACKEND_API SceneVariable
{
public:
	static SceneVariablePtr Create(const std::string& vType);
	static bool IsAllowedType(const std::string& vType);

public:
	union VariableUnion
	{
		bool m_Boolean;
		uint32_t m_Uint32;
		int32_t m_Int32;
		float m_Float;
	};

private:
	VariableUnion datas;
	std::string type;

public:
	SceneVariable() = default;
	SceneVariable(const std::string& vType);
	void SetType(const std::string& vType);
	std::string GetType();
	VariableUnion& GetDatas();
};

	

