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

#include <ctools/cTools.h>
#include <ImGuizmo.h>
#include <LumoBackend/Interfaces/GizmoInterface.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

class CommonSystem;
class LUMO_BACKEND_API GizmoSystem
{
public:
	bool puActivated = false;
	ImGuizmo::OPERATION puCurrentGizmoOperation = ImGuizmo::OPERATION::UNIVERSAL;
	ImGuizmo::MODE puCurrentGizmoMode = ImGuizmo::MODE::LOCAL;
	bool puNeedOneUniformUpdate = false;

private:
	ct::ActionTime puActionTime;
	float* m_CurrentGizmo = nullptr;

public:	
	bool Init();
	void Unit();
	bool Use();
	bool DrawTooltips(GizmoInterfaceWeak vGizmo, ct::frect vRect);
	bool DrawGizmoTransformDialog(GizmoInterfaceWeak vGizmo);

public:
	static GizmoSystem* Instance()
	{
		static GizmoSystem _instance;
		return &_instance;
	}

protected:
	GizmoSystem(); // Prevent construction
	GizmoSystem(const GizmoSystem&) {}; // Prevent construction by copying
	GizmoSystem& operator =(const GizmoSystem&) { return *this; }; // Prevent assignment
	~GizmoSystem(); // Prevent unwanted destruction
};