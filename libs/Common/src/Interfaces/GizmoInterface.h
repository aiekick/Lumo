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

#include <string>
#include <ctools/cTools.h>
#include <Common/Globals.h>

class GizmoInterface;
typedef ct::cWeak<GizmoInterface> GizmoInterfaceWeak;
typedef std::shared_ptr<GizmoInterface> GizmoInterfacePtr;

class COMMON_API GizmoInterface
{
public:
	std::string name = "";
	ImVec4 pressed_color = ImVec4(1.0f, 1.f, 0.2f, 1.0f);
	ImVec4 hovered_color = ImVec4(0.2f, 1.0f, 1.0f, 1.0f);
	ImVec4 idle_color = ImVec4(0.2f, 0.2f, 1.0f, 1.0f);
	bool showIcon = true;
	bool showText = true;
	bool wasChanged = true; // true, for have a first update

public:
	virtual float* GetGizmoFloatPtr() = 0;
};