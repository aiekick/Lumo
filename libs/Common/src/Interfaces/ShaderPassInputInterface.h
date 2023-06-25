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
#include <SceneGraph/SceneShaderPass.h>
#include <Common/Globals.h>

// the shader passes ar not attached to shader so
// binding point use make no sense
// instead for the slot removing/adding it make more sens eto link it to SlotID's

class COMMON_API ShaderPassInputInterface
{
public:
	virtual void SetShaderPasses(const uint32_t& vSlotID, SceneShaderPassWeak vSceneShaderPassWeak) = 0;
};
