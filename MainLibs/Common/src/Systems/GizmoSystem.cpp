/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <Systems/GizmoSystem.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <ctools/Logger.h>
#include <glm/gtc/type_ptr.hpp>

static bool useGizmoCulling = false;

GizmoSystem::GizmoSystem()
{
	Init();
}

GizmoSystem::~GizmoSystem()
{

}

bool GizmoSystem::Init()
{
	puActivated = false;
	puCurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	puCurrentGizmoMode = ImGuizmo::MODE::WORLD;
	puNeedOneUniformUpdate = false;

	ImGuizmo::AllowAxisFlip(false);

	return true;
}

void GizmoSystem::Unit()
{

}

bool GizmoSystem::Use()
{
	return puActivated || puNeedOneUniformUpdate;
}

bool GizmoSystem::DrawTooltips(GizmoInterfaceWeak vGizmo, ct::frect vRect)
{
	bool change = false;

	auto gizmoPtr = vGizmo.getValidShared();
	if (gizmoPtr)
	{
		auto camPtr = CommonSystem::Instance();

		glm::mat4 m = camPtr->uModel * camPtr->uView;
		glm::mat4 p = camPtr->uProj;
		p[1][1] *= -1.0f;

		if (ImGuizmo::DrawPoint(
			glm::value_ptr(m),
			glm::value_ptr(p),
			gizmoPtr->GetGizmoFloatPtr(),
			gizmoPtr->name.c_str(),
			10.0f,
			gizmoPtr->showIcon,
			gizmoPtr->showText))
		{
			m_CurrentGizmo = gizmoPtr->GetGizmoFloatPtr();
		}

		if (gizmoPtr->GetGizmoFloatPtr() == m_CurrentGizmo)
		{
			static bool useSnap = false;
			static float snap[3] = { 1.f, 1.f, 1.f };
			static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
			static float boundsSnap[] = { 0.1f, 0.1f, 0.1f };
			static bool boundSizing = false;
			static bool boundSizingSnap = false;
			change |= ImGuizmo::Manipulate(
				glm::value_ptr(m),
				glm::value_ptr(p),
				puCurrentGizmoOperation,
				puCurrentGizmoMode,
				m_CurrentGizmo, 
				NULL, 
				useSnap ? &snap[0] : NULL, 
				boundSizing ? bounds : NULL, 
				boundSizingSnap ? boundsSnap : NULL);

			change |= ImGuizmo::IsUsing();
		}

		if (change)
		{
			gizmoPtr->wasChanged = true;
		}
	}

	return change;
}

bool GizmoSystem::DrawGizmoTransformDialog(GizmoInterfaceWeak vGizmo)
{
	bool change = false;

	auto gizmoPtr = vGizmo.getValidShared();
	if (gizmoPtr)
	{
		float matrixTranslation[3], matrixRotation[3], matrixScale[3], matrixRotationEmpty[3] = { 0.0f,0.0f,0.0f };
		ImGuizmo::DecomposeMatrixToComponents(gizmoPtr->GetGizmoFloatPtr(), matrixTranslation, matrixRotation, matrixScale);

		ImGui::BeginGroup();

		if (ImGui::ContrastedButton("UnSelect"))
		{
			m_CurrentGizmo = nullptr;
		}

		ImGui::SameLine();

		if (ImGui::ContrastedButton("Center"))
		{
			CommonSystem::Instance()->SetTargetXYZ(-ct::fvec3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]), true);
			change = true;
		}

		ImGui::Text("Reset :");

		////////////////////////////////

		if (ImGui::ContrastedButton("Matrix"))
		{
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

		if (ImGui::ContrastedButton("Pos"))
		{
			matrixTranslation[0] = 0.0f;
			matrixTranslation[1] = 0.0f;
			matrixTranslation[2] = 0.0f;
			change = true;
		}

		ImGui::SameLine();

		//if (useRot)
		{
			if (ImGui::ContrastedButton("Rot"))
			{
				matrixRotation[0] = 0.0f;
				matrixRotation[1] = 0.0f;
				matrixRotation[2] = 0.0f;
				change = true;
			}

			ImGui::SameLine();
		}

		if (ImGui::ContrastedButton("Scale"))
		{
			matrixScale[0] = 1.0f;
			matrixScale[1] = 1.0f;
			matrixScale[2] = 1.0f;
			change = true;
		}

		////////////////////////////////

		if (ImGui::RadioButtonLabeled(0.0f, "Local", "", (puCurrentGizmoMode == ImGuizmo::MODE::LOCAL)))
		{
			puCurrentGizmoMode = ImGuizmo::MODE::LOCAL;
			change = true;
		}

		ImGui::SameLine();

		if (ImGui::RadioButtonLabeled(0.0f, "World", "", (puCurrentGizmoMode == ImGuizmo::MODE::WORLD)))
		{
			puCurrentGizmoMode = ImGuizmo::MODE::WORLD;
			change = true;
		}

		ImGui::SameLine();

		////////////////////////////////

		if (ImGui::RadioButtonLabeled(0.0f, "Trans", "", (puCurrentGizmoOperation == ImGuizmo::OPERATION::TRANSLATE)))
		{
			puCurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
			change = true;
		}

		ImGui::SameLine();

		if (ImGui::RadioButtonLabeled(0.0f, "Rot", "", (puCurrentGizmoOperation == ImGuizmo::OPERATION::ROTATE)))
		{
			puCurrentGizmoOperation = ImGuizmo::OPERATION::ROTATE;
			change = true;
		}

		ImGui::SameLine();

		if (ImGui::RadioButtonLabeled(0.0f, "Scale", "", (puCurrentGizmoOperation == ImGuizmo::OPERATION::SCALE)))
		{
			puCurrentGizmoOperation = ImGuizmo::OPERATION::SCALE;
			change = true;
		}

		ImGui::SameLine();

		if (ImGui::RadioButtonLabeled(0.0f, "All", "", (puCurrentGizmoOperation == ImGuizmo::OPERATION::UNIVERSAL)))
		{
			puCurrentGizmoOperation = ImGuizmo::OPERATION::UNIVERSAL;
			change = true;
		}

		////////////////////////////////

		if (puCurrentGizmoOperation == ImGuizmo::OPERATION::TRANSLATE)
		{
			change |= ImGui::InputFloatDefault(150.0f, "tx", &matrixTranslation[0], 0.0f);
			change |= ImGui::InputFloatDefault(150.0f, "ty", &matrixTranslation[1], 0.0f);
			change |= ImGui::InputFloatDefault(150.0f, "tz", &matrixTranslation[2], 0.0f);
		}

		//if (useRot)
		{
			if (puCurrentGizmoOperation == ImGuizmo::OPERATION::ROTATE)
			{
				change |= ImGui::InputFloatDefault(150.0f, "rx", &matrixRotation[0], 0.0f);
				change |= ImGui::InputFloatDefault(150.0f, "ry", &matrixRotation[1], 0.0f);
				change |= ImGui::InputFloatDefault(150.0f, "rz", &matrixRotation[2], 0.0f);
			}
		}

		if (puCurrentGizmoOperation == ImGuizmo::OPERATION::SCALE)
		{
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

		if (change)
		{
			gizmoPtr->wasChanged = true;

			//if (useRot)
			{
				ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, gizmoPtr->GetGizmoFloatPtr());
			}
			/*else
			{
				// comme cela on ne nique pas la rotation pour le cube on n'en tient jute pas ;compte pour la sphere le temps de trouver une fonction glsl
				// qui permette de faire tourner l'ellipsoide
				ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotationEmpty, matrixScale, glm::value_ptr(vUni->mat4));
			}*/
		}
	}

	return change;
}
