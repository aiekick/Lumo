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

#include <vulkan/vulkan.hpp>

#include <glm/mat4x4.hpp>

#include <string>
#include <memory>

#ifdef BUILD_SHARED
#ifdef _WIN32
#define COMMON_API   __declspec( dllimport )
#endif // _WIN32
#endif // BUILD_SHARED
#ifndef COMMON_API
#define COMMON_API
#endif // COMMON_API

class COMMON_API CameraInterface
{
public:
	glm::mat4 uView = glm::mat4(1.0f);
	glm::mat4 uProj = glm::mat4(1.0f);
	glm::mat4 uModel = glm::mat4(1.0f);
	glm::mat4 uCam = glm::mat4(1.0f);
	glm::mat4 uInvCam = glm::mat4(1.0f);
	glm::mat4 uNormalMatrix = glm::mat4(1.0f);

public:
	virtual void NeedCamChange() = 0;
};