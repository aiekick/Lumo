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

#include <LumoBackend/Graph/Graph.h>
#include <ctools/cTools.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>
#include <string>

class LUMO_BACKEND_API NodeStamp
{
public:
	std::string typeStamp; // float(vec3)
	std::string nameStamp; // float map(vec3)
	std::string fullStamp; // float map(vec3 a)
	
public:
	NodeStamp();
	~NodeStamp();

	void DrawImGui();
};