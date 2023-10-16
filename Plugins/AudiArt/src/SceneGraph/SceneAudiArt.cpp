/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <SceneGraph/SceneAudiArt.h>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

SceneAudiArtPtr SceneAudiArt::Create()
{
	auto res = std::make_shared<SceneAudiArt>();
	res->m_This = res;
	return res;
}

//////////////////////////////////////////////////////////////
//// PUBLIC : BUILD / CLEAR //////////////////////////////////
//////////////////////////////////////////////////////////////

SceneAudiArt::SceneAudiArt()
{

}

SceneAudiArt::~SceneAudiArt()
{
	Clear();
}

void SceneAudiArt::Clear()
{

}

bool SceneAudiArt::IsOk() const
{
	return true;
}

