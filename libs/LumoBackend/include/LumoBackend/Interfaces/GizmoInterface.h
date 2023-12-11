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

#include <string>
#include <ctools/cTools.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>
#include <ImGuiPack.h>

class LUMO_BACKEND_API GizmoInterface {
public:
    std::string gizmo_name = "";
    ImVec4 gizmo_pressed_color = ImVec4(1.0f, 1.f, 0.2f, 1.0f);
    ImVec4 gizmo_hovered_color = ImVec4(0.2f, 1.0f, 1.0f, 1.0f);
    ImVec4 gizmo_idle_color = ImVec4(0.2f, 0.2f, 1.0f, 1.0f);
    bool gizmo_show_icon = true;
    bool gizmo_show_text = true;
    bool gizmo_was_changed = true;  // true, for have a first update
    ImGuizmo::OPERATION gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
    ImGuizmo::MODE gizmo_mode = ImGuizmo::MODE::WORLD;
    virtual float* GetGizmoFloatPtr() = 0;
};