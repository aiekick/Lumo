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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <LumoBackend/Systems/GizmoSystem.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <ctools/Logger.h>
#include <glm/gtc/type_ptr.hpp>

static bool useGizmoCulling = false;

GizmoSystem::GizmoSystem() {
    Init();
}

GizmoSystem::~GizmoSystem() {
}

bool GizmoSystem::Init() {
    puActivated = false;
    puCurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
    puCurrentGizmoMode = ImGuizmo::MODE::WORLD;
    puNeedOneUniformUpdate = false;

    ImGuizmo::AllowAxisFlip(false);

    return true;
}

void GizmoSystem::Unit() {
}

bool GizmoSystem::Use() {
    return puActivated || puNeedOneUniformUpdate;
}

bool GizmoSystem::DrawTooltips(GizmoInterfaceWeak vGizmo, ct::frect /*vRect*/) {
    bool change = false;

    auto gizmoPtr = vGizmo.lock();
    if (gizmoPtr) {
        auto camPtr = CommonSystem::Instance();

        glm::mat4 m = camPtr->uModel * camPtr->uView;
        glm::mat4 p = camPtr->uProj;
        p[1][1] *= -1.0f;

        if (ImGuizmo::DrawPoint(glm::value_ptr(m), glm::value_ptr(p), gizmoPtr->GetGizmoFloatPtr(), gizmoPtr->gizmo_name.c_str(), 10.0f,
                gizmoPtr->gizmo_show_icon, gizmoPtr->gizmo_show_text, gizmoPtr->gizmo_pressed_color, gizmoPtr->gizmo_hovered_color,
                gizmoPtr->gizmo_idle_color)) {
            m_CurrentGizmo = gizmoPtr->GetGizmoFloatPtr();
        }

        if (gizmoPtr->GetGizmoFloatPtr() == m_CurrentGizmo) {
            static bool useSnap = false;
            static float snap[3] = {1.f, 1.f, 1.f};
            static float bounds[] = {-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};
            static float boundsSnap[] = {0.1f, 0.1f, 0.1f};
            static bool boundSizing = false;
            static bool boundSizingSnap = false;
            change |= ImGuizmo::Manipulate(glm::value_ptr(m), glm::value_ptr(p), puCurrentGizmoOperation, puCurrentGizmoMode, m_CurrentGizmo, NULL,
                useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);

            change |= ImGuizmo::IsUsing();
        }

        if (change) {
            gizmoPtr->gizmo_was_changed = true;
        }
    }

    return change;
}

bool GizmoSystem::DrawGizmoTransformDialog(GizmoInterfaceWeak vGizmo) {
    bool change = false;

    auto gizmoPtr = vGizmo.lock();
    if (gizmoPtr) {
        float matrixTranslation[3], matrixRotation[3], matrixScale[3], matrixRotationEmpty[3] = {0.0f, 0.0f, 0.0f};
        ImGuizmo::DecomposeMatrixToComponents(gizmoPtr->GetGizmoFloatPtr(), matrixTranslation, matrixRotation, matrixScale);

        ImGui::BeginGroup();

        if (ImGui::ContrastedButton("UnSelect")) {
            m_CurrentGizmo = nullptr;
        }

        ImGui::SameLine();

        if (ImGui::ContrastedButton("Center")) {
            CommonSystem::Instance()->SetTargetXYZ(-ct::fvec3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]), true);
            change = true;
        }

        ImGui::Text("Reset :");

        ////////////////////////////////

        if (ImGui::ContrastedButton("Matrix")) {
            matrixTranslation[0] = 0.0f;
            matrixTranslation[1] = 0.0f;
            matrixTranslation[2] = 0.0f;
            matrixRotation[0] = 0.0f;
            matrixRotation[1] = 0.0f;
            matrixRotation[2] = 0.0f;
            matrixScale[0] = 1.0f;
            matrixScale[1] = 1.0f;
            matrixScale[2] = 1.0f;
            change = true;
        }

        ImGui::SameLine();

        if (ImGui::ContrastedButton("Pos")) {
            matrixTranslation[0] = 0.0f;
            matrixTranslation[1] = 0.0f;
            matrixTranslation[2] = 0.0f;
            change = true;
        }

        ImGui::SameLine();

        // if (useRot)
        {
            if (ImGui::ContrastedButton("Rot")) {
                matrixRotation[0] = 0.0f;
                matrixRotation[1] = 0.0f;
                matrixRotation[2] = 0.0f;
                change = true;
            }

            ImGui::SameLine();
        }

        if (ImGui::ContrastedButton("Scale")) {
            matrixScale[0] = 1.0f;
            matrixScale[1] = 1.0f;
            matrixScale[2] = 1.0f;
            change = true;
        }

        ////////////////////////////////

        if (ImGui::RadioButtonLabeled(0.0f, "Local", "", (puCurrentGizmoMode == ImGuizmo::MODE::LOCAL))) {
            puCurrentGizmoMode = ImGuizmo::MODE::LOCAL;
            change = true;
        }

        ImGui::SameLine();

        if (ImGui::RadioButtonLabeled(0.0f, "World", "", (puCurrentGizmoMode == ImGuizmo::MODE::WORLD))) {
            puCurrentGizmoMode = ImGuizmo::MODE::WORLD;
            change = true;
        }

        ImGui::SameLine();

        ////////////////////////////////

        if (ImGui::RadioButtonLabeled(0.0f, "Trans", "", (puCurrentGizmoOperation == ImGuizmo::OPERATION::TRANSLATE))) {
            puCurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
            change = true;
        }

        ImGui::SameLine();

        if (ImGui::RadioButtonLabeled(0.0f, "Rot", "", (puCurrentGizmoOperation == ImGuizmo::OPERATION::ROTATE))) {
            puCurrentGizmoOperation = ImGuizmo::OPERATION::ROTATE;
            change = true;
        }

        ImGui::SameLine();

        if (ImGui::RadioButtonLabeled(0.0f, "Scale", "", (puCurrentGizmoOperation == ImGuizmo::OPERATION::SCALE))) {
            puCurrentGizmoOperation = ImGuizmo::OPERATION::SCALE;
            change = true;
        }

        ImGui::SameLine();

        if (ImGui::RadioButtonLabeled(0.0f, "All", "", (puCurrentGizmoOperation == ImGuizmo::OPERATION::UNIVERSAL))) {
            puCurrentGizmoOperation = ImGuizmo::OPERATION::UNIVERSAL;
            change = true;
        }

        ////////////////////////////////

        if (puCurrentGizmoOperation == ImGuizmo::OPERATION::TRANSLATE) {
            change |= ImGui::InputFloatDefault(150.0f, "tx", &matrixTranslation[0], 0.0f);
            change |= ImGui::InputFloatDefault(150.0f, "ty", &matrixTranslation[1], 0.0f);
            change |= ImGui::InputFloatDefault(150.0f, "tz", &matrixTranslation[2], 0.0f);
        }

        // if (useRot)
        {
            if (puCurrentGizmoOperation == ImGuizmo::OPERATION::ROTATE) {
                change |= ImGui::InputFloatDefault(150.0f, "rx", &matrixRotation[0], 0.0f);
                change |= ImGui::InputFloatDefault(150.0f, "ry", &matrixRotation[1], 0.0f);
                change |= ImGui::InputFloatDefault(150.0f, "rz", &matrixRotation[2], 0.0f);
            }
        }

        if (puCurrentGizmoOperation == ImGuizmo::OPERATION::SCALE) {
            change |= ImGui::InputFloatDefault(150.0f, "sx", &matrixScale[0], 1.0f);
            change |= ImGui::InputFloatDefault(150.0f, "sy", &matrixScale[1], 1.0f);
            change |= ImGui::InputFloatDefault(150.0f, "sz", &matrixScale[2], 1.0f);
        }

        ////////////////////////////////

        /*float *m = glm::value_ptr(vUni->mat4);
        ImGui::Text("Matrix 4x4\n 0 : %.2f %.2f %.2f %.2f\n1 : %.2f %.2f %.2f %.2f\n2 : %.2f %.2f %.2f %.2f\n3 : %.2f %.2f %.2f %.2f",
            m[0], m[1], m[2], m[3],
            m[4], m[5], m[6], m[7],
            m[8], m[9], m[10], m[11],
            m[12], m[13], m[14], m[15]);*/

        ImGui::EndGroup();

        if (change) {
            gizmoPtr->gizmo_was_changed = true;

            // if (useRot)
            { ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, gizmoPtr->GetGizmoFloatPtr()); }
            /*else
            {
                // comme cela on ne nique pas la rotation pour le cube on n'en tient jute pas ;compte pour la sphere le temps de trouver une fonction
            glsl
                // qui permette de faire tourner l'ellipsoide
                ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotationEmpty, matrixScale, glm::value_ptr(vUni->mat4));
            }*/
        }
    }

    return change;
}
