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

#include <LumoBackend/SceneGraph/SceneLight.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

class LUMO_BACKEND_API LightInputInterface
{
protected:
	SceneLightWeak m_SceneLightWeak;

public:
	virtual void SetLight(SceneLightWeak vSceneLight) = 0;
};