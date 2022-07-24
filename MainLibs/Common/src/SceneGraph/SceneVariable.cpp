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

#include <SceneGraph/SceneVariable.h>

SceneVariablePtr SceneVariable::Create(const std::string& vType)
{
	if (vType == "TYBE_BOOLEAN" ||
		vType == "TYBE_FLOAT" ||
		vType == "TYBE_INT" ||
		vType == "TYBE_UINT")
	{
		return std::make_shared<SceneVariable>(vType);
	}
	
	return nullptr;
}

SceneVariable::SceneVariable(const std::string& vType)
{
	type = vType;

	if (type == "TYBE_BOOLEAN")
	{
		datas.m_Boolean = false;
	}
	else if (type == "TYBE_FLOAT")
	{
		datas.m_Float = 0.0f;
	}
	else if (type == "TYBE_INT")
	{
		datas.m_Int32 = 0;
	}
	else if (type == "TYBE_UINT")
	{
		datas.m_Uint32 = 0U;
	}
}

void SceneVariable::SetType(const std::string& vType)
{
	type = vType;
}

std::string SceneVariable::GetType() 
{ 
	return type; 
}

SceneVariable::VariableUnion& SceneVariable::GetDatas()
{ 
	return datas; 
}