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

#pragma once

#include <ctools/cTools.h>
#include <ImGuizmo/ImGuizmo.h>
#include <Interfaces/GizmoInterface.h>

class CommonSystem;
class GizmoSystem
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