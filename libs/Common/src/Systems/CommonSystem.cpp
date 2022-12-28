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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "CommonSystem.h"
#include <ImWidgets/ImWidgets.h>
#include <FontIcons/CustomFont2.h>

#include <vkFramework/VulkanCore.h>

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

void CommonSystem::SetCameraMode(const CAMERA_MODE_Enum& vCameraMode)
{
	ZoneScoped;

	if (m_CameraSettings.m_CameraMode != vCameraMode)
	{
		if (vCameraMode == CAMERA_MODE_Enum::CAMERA_MODE_FREE) // from roty to filght
		{
			// il faut calculer m_PosXYZ
			auto currentPoint = glm::inverse(uModel) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			m_CameraSettings.m_PosXYZ = glm::vec3(-currentPoint.x, -currentPoint.y, -currentPoint.z);
		}
		else if (vCameraMode == CAMERA_MODE_Enum::CAMERA_MODE_TURNTABLE_Y) // from flight to roty
		{
			// il faut calculer m_TransXYZ & m_Zoom
			// on va l'extraire de la matrice uModel
			m_CameraSettings.m_TransXY.x = uModel[3][0];
			m_CameraSettings.m_TransXY.y = uModel[3][1];
			m_CameraSettings.m_Zoom = -uModel[3][2];
			m_CameraSettings.m_TargetXYZ = glm::vec3(0.0f);
		}
	}

	m_CameraSettings.m_CameraMode = vCameraMode;
	m_NeedCamChange = true;
}

void CommonSystem::SetCameraType(const CAMERA_TYPE_Enum& vCameraType)
{
	ZoneScoped;

	m_CameraSettings.m_CameraType = vCameraType;
	m_NeedCamChange = true;
}

void CommonSystem::SetPerspectiveAngle(const float& vPerspAngle)
{
	ZoneScoped;

	m_CameraSettings.m_PerspAngle = vPerspAngle;
	m_NeedCamChange = true;
}

void CommonSystem::SetPerspectiveLimitZ(const float& vNearPlane, const float& vFarPlane)
{
	ZoneScoped;

	m_UBOCamera.cam_near = vNearPlane;
	m_UBOCamera.cam_far = vFarPlane;
	m_NeedCamChange = true;
}

void CommonSystem::SetOrthographicLimitZ(const float& vLeftPlane, const float& vRightPlane, const float& vBottomPlane, const float& vTopPlane)
{
	ZoneScoped;

	lrbtPlanes = glm::vec4(vLeftPlane, vRightPlane, vBottomPlane, vTopPlane);
	m_NeedCamChange = true;
}

void CommonSystem::SetTargetXYZ(const ct::fvec3& vTarget, bool vForce)
{
	ZoneScoped;

	if (!m_CameraSettings.m_TargetXYZLock.x || vForce)
		m_CameraSettings.m_TargetXYZ.x = vTarget.x;
	if (!m_CameraSettings.m_TargetXYZLock.y || vForce)
		m_CameraSettings.m_TargetXYZ.y = vTarget.y;
	if (!m_CameraSettings.m_TargetXYZLock.z || vForce)
		m_CameraSettings.m_TargetXYZ.z = vTarget.z;

	m_NeedCamChange = true;
}

void CommonSystem::IncTranslateXY(const ct::fvec2& vMouseOffset)
{
	ZoneScoped;

	if (vMouseOffset.emptyAND())
		return;

	if (!m_CameraSettings.m_TransXYLock.x)
		m_CameraSettings.m_TransXY.x += vMouseOffset.x * m_CameraSettings.m_TransFactor;
	if (!m_CameraSettings.m_TransXYLock.y)
		m_CameraSettings.m_TransXY.y -= vMouseOffset.y * m_CameraSettings.m_TransFactor;

	m_NeedCamChange = true;
}

void CommonSystem::SetTranslateXY(const ct::fvec2& vTranlation, bool vForce)
{
	ZoneScoped;

	if (!m_CameraSettings.m_TransXYLock.x || vForce)
		m_CameraSettings.m_TransXY.x = vTranlation.x;
	if (!m_CameraSettings.m_TransXYLock.y || vForce)
		m_CameraSettings.m_TransXY.y = vTranlation.y;

	m_NeedCamChange = true;
}

void CommonSystem::SetTranslateFactor(const float& vTranslateFactor)
{
	ZoneScoped;

	m_CameraSettings.m_TransFactor = vTranslateFactor;
}

void CommonSystem::IncRotateXYZ(const ct::fvec3& vMouseOffset)
{
	ZoneScoped;

	if (vMouseOffset.emptyAND())
		return;

	ct::fvec3 mouseOffset = vMouseOffset;
	if (!m_CameraSettings.m_RotationXYZLock.x)
	{
		m_CameraSettings.m_RotationXYZ.x +=
			mouseOffset.x * m_CameraSettings.m_RotationFactor;

		if (m_CameraSettings.m_RotationXYZ.x > 3.14159f)
			m_CameraSettings.m_RotationXYZ.x = -3.14158f;
		if (m_CameraSettings.m_RotationXYZ.x < -3.14159f)
			m_CameraSettings.m_RotationXYZ.x = 3.14158f;
	}

	if (!m_CameraSettings.m_RotationXYZLock.y)
	{
		m_CameraSettings.m_RotationXYZ.y +=
			mouseOffset.y * m_CameraSettings.m_RotationFactor;

		if (m_CameraSettings.m_RotationXYZ.y > 3.14159f)
			m_CameraSettings.m_RotationXYZ.y = -3.14158f;
		if (m_CameraSettings.m_RotationXYZ.y < -3.14159f)
			m_CameraSettings.m_RotationXYZ.y = 3.14158f;
	}

	if (!m_CameraSettings.m_RotationXYZLock.z)
	{
		m_CameraSettings.m_RotationXYZ.z +=
			mouseOffset.z * m_CameraSettings.m_RotationFactor;

		if (m_CameraSettings.m_RotationXYZ.z > 3.14159f)
			m_CameraSettings.m_RotationXYZ.z = -3.14158f;
		if (m_CameraSettings.m_RotationXYZ.z < -3.14159f)
			m_CameraSettings.m_RotationXYZ.z = 3.14158f;
	}

	m_NeedCamChange = true;
}

void CommonSystem::SetRotateXYZ(const ct::fvec3& vRotate)
{
	ZoneScoped;

	if (!m_CameraSettings.m_RotationXYZLock.x)
		m_CameraSettings.m_RotationXYZ.x = vRotate.x;
	if (!m_CameraSettings.m_RotationXYZLock.y)
		m_CameraSettings.m_RotationXYZ.y = vRotate.y;
	if (!m_CameraSettings.m_RotationXYZLock.z)
		m_CameraSettings.m_RotationXYZ.z = vRotate.z;

	m_NeedCamChange = true;
}

void CommonSystem::SetRotateFactor(const float& vRotateFactor)
{
	ZoneScoped;

	m_CameraSettings.m_RotationFactor = vRotateFactor;
}

void CommonSystem::IncZoom(const float& vMouseOffset)
{
	ZoneScoped;

	if (!m_CameraSettings.m_ZoomLock)
		m_CameraSettings.m_Zoom = ct::maxi(m_CameraSettings.m_Zoom + vMouseOffset * m_CameraSettings.m_ZoomFactor, 0.0000001f);

	m_NeedCamChange = true;
}

void CommonSystem::SetZoom(const float& vZoom)
{
	ZoneScoped;

	if (!m_CameraSettings.m_ZoomLock)
		m_CameraSettings.m_Zoom = vZoom;

	m_NeedCamChange = true;
}

void CommonSystem::SetZoomFactor(const float& vZoomFactor)
{
	ZoneScoped;

	m_CameraSettings.m_ZoomFactor = vZoomFactor;
}

/// <summary>
/// Free Fly mode, move the cam pos of vMovementOffset
/// </summary>
/// <param name="vMove">offset for movement</param>
void CommonSystem::IncFlyingPosition(const float& vMovementOffset)
{
	// le but du jeu est juste de calculer la nouvelle m_PosXYZ apres vMovementOffset dans la direction camOrientation
	auto mat = glm::mat4(1.0f);
	mat = glm::translate(mat, glm::vec3(m_CameraSettings.m_PosXYZ.x, m_CameraSettings.m_PosXYZ.y, m_CameraSettings.m_PosXYZ.z));
	camOrientation = glm::quat(glm::vec3(-m_CameraSettings.m_RotationXYZ.y, -m_CameraSettings.m_RotationXYZ.x, -m_CameraSettings.m_RotationXYZ.z));
	mat *= glm::mat4_cast(camOrientation);
	mat = glm::translate(mat, glm::vec3(0.0f, 0.0f, vMovementOffset));
	m_CameraSettings.m_PosXYZ = mat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	m_NeedCamChange = true;
}

void CommonSystem::SetViewMatrix(const glm::mat4& vMatrix)
{
	uView = vMatrix;
}

void CommonSystem::SetProjMatrix(const glm::mat4& vMatrix)
{
	uProj = vMatrix;
}

void CommonSystem::NeedCamChange()
{
	m_NeedCamChange = true;
}

bool CommonSystem::DrawImGui()
{
	ZoneScoped;

	static CameraSettingsStruct s_CameraSettingsStruct_Default;

	bool change = false;

	if (ImGui::CollapsingHeader("Camera"))
	{
		ImGui::FramedGroupSeparator();

		const float aw = (ImGui::GetContentRegionMax().x - ImGui::GetCursorPosX() - ImGui::GetStyle().ItemInnerSpacing.x) * 0.5f;

		if (ImGui::RadioButtonLabeled(aw, "Perspective", "Use Perspective Camera", m_CameraSettings.m_CameraType == CAMERA_TYPE_Enum::CAMERA_TYPE_PERSPECTIVE))
		{
			SetCameraType(CAMERA_TYPE_Enum::CAMERA_TYPE_PERSPECTIVE);
			change = true;
		}

#ifdef _DEBUG
		ImGui::SameLine();

		if (ImGui::RadioButtonLabeled(aw, "Orthographic", "Use Orthographic Camera", m_CameraSettings.m_CameraType == CAMERA_TYPE_Enum::CAMERA_TYPE_ORTHOGRAPHIC))
		{
			SetCameraType(CAMERA_TYPE_Enum::CAMERA_TYPE_ORTHOGRAPHIC);
			change = true;
		}
#endif

		if (m_CameraSettings.m_CameraType == CAMERA_TYPE_Enum::CAMERA_TYPE_PERSPECTIVE)
		{
			change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Perspective Angle##Persp", &m_CameraSettings.m_PerspAngle, 0.0f, 180.0f, 45.0f);
			//change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Cam Near##Persp", &m_UBOCamera.cam_near, 0.0f, CAM_FAR * 2.0f, 0.001f);
			change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Cam Far##Persp", &m_UBOCamera.cam_far, 0.0f, CAM_FAR * 2.0f, CAM_FAR);
		}

		ImGui::FramedGroupSeparator();

		if (ImGui::RadioButtonLabeled(aw, "Turntable Y", "Turntable Y Mode", m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_TURNTABLE_Y))
		{
			SetCameraMode(CAMERA_MODE_Enum::CAMERA_MODE_TURNTABLE_Y);
			change = true;
		}

		ImGui::SameLine();

		if (ImGui::RadioButtonLabeled(aw, "Free", "Free Camera Mode", m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_FREE))
		{
			SetCameraMode(CAMERA_MODE_Enum::CAMERA_MODE_FREE);
			change = true;
		}

		ImGui::FramedGroupSeparator();

		if (ImGui::ContrastedButton("Reset Cam to 2D", nullptr, nullptr, aw))
		{
			if (m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_TURNTABLE_Y)
			{
				m_CameraSettings.m_TargetXYZ = glm::vec3(0.0f);
				m_CameraSettings.m_TransXY = glm::vec2(0.0f);
				m_CameraSettings.m_RotationXYZ.x = 0.0f;
				m_CameraSettings.m_RotationXYZ.y = 0.0f;
				m_CameraSettings.m_Zoom = 5.0f;
				m_CameraSettings.m_TargetXYZ = glm::vec3(0.0f);
				m_UBOCamera.cam_near = 0.001f;
				m_UBOCamera.cam_far = CAM_FAR;
			}
			else if (m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_FREE)
			{
				m_CameraSettings.m_PosXYZ = glm::vec3(0.0f, 0.0f, -5.0f);
				m_CameraSettings.m_RotationXYZ = glm::vec3(0.0f);
				m_CameraSettings.m_TargetXYZ = glm::vec3(0.0f);
				m_UBOCamera.cam_near = 0.001f;
				m_UBOCamera.cam_far = CAM_FAR;
			}

			change |= true;
		}

		ImGui::SameLine();

		if (ImGui::ContrastedButton("Reset Cam to 3D", nullptr, nullptr, aw))
		{
			if (m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_TURNTABLE_Y)
			{
				m_CameraSettings.m_TransXY = glm::vec2(0.0f);
				m_CameraSettings.m_RotationXYZ.x = -0.785f;
				m_CameraSettings.m_RotationXYZ.y = 0.612f;
				m_CameraSettings.m_Zoom = 5.0f;
				m_CameraSettings.m_TargetXYZ = glm::vec3(0.0f);
				m_UBOCamera.cam_near = 0.001f;
				m_UBOCamera.cam_far = CAM_FAR;

				change = true;
			}
			else if (m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_FREE)
			{
				m_CameraSettings.m_PosXYZ = glm::vec3(0.0f, 0.0f, -5.0f);
				m_CameraSettings.m_SpeedFactor = 0.05f;
				m_CameraSettings.m_RotationXYZ = glm::vec3(0.0f);
				m_CameraSettings.m_RotationFactor = 0.05f;
				m_CameraSettings.m_TargetXYZ = glm::vec3(0.0f);
				m_UBOCamera.cam_near = 0.001f;
				m_UBOCamera.cam_far = CAM_FAR;

				change = true;
			}
		}

		if (m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_FREE)
		{
			ImGui::FramedGroupSeparator();

			change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Cam Pos X##campx",
				&m_CameraSettings.m_PosXYZ.x, -10.0f, 10.0f, s_CameraSettingsStruct_Default.m_PosXYZ.x);
			change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Cam Pos Y##campy",
				&m_CameraSettings.m_PosXYZ.y, -10.0f, 10.0f, s_CameraSettingsStruct_Default.m_PosXYZ.y);
			change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Cam Pos Z##campz",
				&m_CameraSettings.m_PosXYZ.z, -10.0f, 10.0f, s_CameraSettingsStruct_Default.m_PosXYZ.z);
			change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Speed##camspeed",
				&m_CameraSettings.m_SpeedFactor, 0.0f, 100.0f, s_CameraSettingsStruct_Default.m_SpeedFactor);
		}

		if (m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_TURNTABLE_Y)
		{
			ImGui::FramedGroupSeparator();

			ImGui::CheckBoxIcon("##lock_target_x", ICON_NDP2_LOCK, &m_CameraSettings.m_TargetXYZLock.x); ImGui::SameLine(); if (ImGui::IsItemHovered()) ImGui::SetTooltip("lock camera target x\nfrom modification with mouse or gizmo");
			change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Target X##tgtx", &m_CameraSettings.m_TargetXYZ.x, -10.0f, 10.0f, 0.0f);
			ImGui::CheckBoxIcon("##lock_target_y", ICON_NDP2_LOCK, &m_CameraSettings.m_TargetXYZLock.y); ImGui::SameLine(); if (ImGui::IsItemHovered()) ImGui::SetTooltip("lock camera target y\nfrom modification with mouse or gizmo");
			change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Target Y##tgty", &m_CameraSettings.m_TargetXYZ.y, -10.0f, 10.0f, 0.0f);
			ImGui::CheckBoxIcon("##lock_target_z", ICON_NDP2_LOCK, &m_CameraSettings.m_TargetXYZLock.z); ImGui::SameLine(); if (ImGui::IsItemHovered()) ImGui::SetTooltip("lock camera target z\nfrom modification with mouse or gizmo");
			change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Target Z##tgtz", &m_CameraSettings.m_TargetXYZ.z, -10.0f, 10.0f, 0.0f);

			ImGui::FramedGroupSeparator();

			ImGui::CheckBoxIcon("##lock_trans_x", ICON_NDP2_LOCK, &m_CameraSettings.m_TransXYLock.x); ImGui::SameLine(); if (ImGui::IsItemHovered()) ImGui::SetTooltip("lock translation x");
			change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Trans X##transx", &m_CameraSettings.m_TransXY.x, -10.0f, 10.0f, 0.0f);
			ImGui::CheckBoxIcon("##lock_trans_y", ICON_NDP2_LOCK, &m_CameraSettings.m_TransXYLock.y); ImGui::SameLine(); if (ImGui::IsItemHovered()) ImGui::SetTooltip("lock translation y");
			change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Trans Y##transy", &m_CameraSettings.m_TransXY.y, -10.0f, 10.0f, 0.0f);
			change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Factor##trans", &m_CameraSettings.m_TransFactor, 0.0f, 100.0f, s_CameraSettingsStruct_Default.m_TransFactor);
		}

		ImGui::FramedGroupSeparator();

		ImGui::CheckBoxIcon("##lock_rot_x", ICON_NDP2_LOCK, &m_CameraSettings.m_RotationXYZLock.x); ImGui::SameLine(); if (ImGui::IsItemHovered()) ImGui::SetTooltip("lock rotation x\nfrom modification with mouse");
		change |= ImGui::SliderFloatDefaultCompact(-1.0f, "RotX##rotx", &m_CameraSettings.m_RotationXYZ.x, -3.14159f, 3.14159f, 0.0f);
		ImGui::CheckBoxIcon("##lock_rot_y", ICON_NDP2_LOCK, &m_CameraSettings.m_RotationXYZLock.y); ImGui::SameLine(); if (ImGui::IsItemHovered()) ImGui::SetTooltip("lock rotation y\nfrom modification with mouse");
		change |= ImGui::SliderFloatDefaultCompact(-1.0f, "RotY##roty", &m_CameraSettings.m_RotationXYZ.y, -3.14159f, 3.14159f, 0.0f);
		ImGui::CheckBoxIcon("##lock_rot_z", ICON_NDP2_LOCK, &m_CameraSettings.m_RotationXYZLock.z); ImGui::SameLine(); if (ImGui::IsItemHovered()) ImGui::SetTooltip("lock rotation z\nfrom modification with mouse");
		change |= ImGui::SliderFloatDefaultCompact(-1.0f, "RotZ##rotz", &m_CameraSettings.m_RotationXYZ.z, -3.14159f, 3.14159f, 0.0f);
		change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Factor##rotxyz",
			&m_CameraSettings.m_RotationFactor, 0.0f, 6.28318f, s_CameraSettingsStruct_Default.m_RotationFactor);

		//ImGui::FramedGroupSeparator();
		//ImGui::CheckBoxIcon("##lock_uni_scale", ICON_NDP2_LOCK, &m_CameraSettings.m_UniformScaleLock); ImGui::SameLine(); if (ImGui::IsItemHovered()) ImGui::SetTooltip("lock scale");
		//change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Scale##scale", &m_CameraSettings.m_UniformScale, 0.00001f, 2.0f, 1.0f);

		if (m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_TURNTABLE_Y)
		{
			ImGui::FramedGroupSeparator();

			ImGui::CheckBoxIcon("##lock_zoom", ICON_NDP2_LOCK, &m_CameraSettings.m_ZoomLock); ImGui::SameLine(); if (ImGui::IsItemHovered()) ImGui::SetTooltip("lock zoom");
			change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Zoom##Zoom",
				&m_CameraSettings.m_Zoom, 0.00001f, 100.0f, s_CameraSettingsStruct_Default.m_Zoom, 0.0f, "%.5f");
			change |= ImGui::SliderFloatDefaultCompact(-1.0f, "Factor##Zoom",
				&m_CameraSettings.m_ZoomFactor, 0.0f, 100.0f, s_CameraSettingsStruct_Default.m_ZoomFactor);
		}

#ifdef USE_VR
		if (VRBackend::Instance()->IsLoaded())
		{
			ImGui::FramedGroupSeparator();

			ImGui::CheckBoxBoolDefault("Use VR Controller for control Camera",
				&m_CameraSettings.m_UseVRControllerForControlCamera,
				s_CameraSettingsStruct_Default.m_UseVRControllerForControlCamera,
				"Only in VR :\nUse left thumstick XY for control rotation around Z and zoom\nUse right thumbstick XY for control rotations\nOnly if left or right Squeeze are pressed");
			if (m_CameraSettings.m_UseVRControllerForControlCamera)
			{
				change |= ImGui::SliderFloatDefaultCompact(-1.0f, "VR Step Rot X##vrrotx",
					&m_CameraSettings.m_VRRotStepFactor.x, 0.00001f, 2.0f, s_CameraSettingsStruct_Default.m_VRRotStepFactor.x, 0.0f, "%.5f");
				change |= ImGui::SliderFloatDefaultCompact(-1.0f, "VR Step Rot Y##vrroty",
					&m_CameraSettings.m_VRRotStepFactor.y, 0.00001f, 2.0f, s_CameraSettingsStruct_Default.m_VRRotStepFactor.y, 0.0f, "%.5f");
				change |= ImGui::SliderFloatDefaultCompact(-1.0f, "VR Step Rot Z##vrrotz",
					&m_CameraSettings.m_VRRotStepFactor.z, 0.00001f, 2.0f, s_CameraSettingsStruct_Default.m_VRRotStepFactor.z, 0.0f, "%.5f");
				change |= ImGui::SliderFloatDefaultCompact(-1.0f, "VR Step Zoom##vrzoom",
					&m_CameraSettings.m_VRZoomStepFactor, 0.00001f, 5.0f, s_CameraSettingsStruct_Default.m_VRZoomStepFactor, 0.0f, "%.5f");
			}
		}
#endif

		ImGui::FramedGroupSeparator();

		//if (m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_TURNTABLE_Y)
		{
			const float aw2 = (ImGui::GetContentRegionMax().x - ImGui::GetCursorPosX() - ImGui::GetStyle().ItemInnerSpacing.x) * 0.15f;

			if (ImGui::ContrastedButton("X+", nullptr, nullptr, aw2))
			{
				m_CameraSettings.m_RotationXYZ.x = -_pi * 0.5f;
				m_CameraSettings.m_RotationXYZ.y = 0.0f;
				change |= true;
			}

			ImGui::SameLine();

			if (ImGui::ContrastedButton("X-", nullptr, nullptr, aw2))
			{
				m_CameraSettings.m_RotationXYZ.x = _pi * 0.5f;
				m_CameraSettings.m_RotationXYZ.y = 0.0f;
				change |= true;
			}

			ImGui::SameLine();

			if (ImGui::ContrastedButton("Y+", nullptr, nullptr, aw2))
			{
				m_CameraSettings.m_RotationXYZ.x = 0.0f;
				m_CameraSettings.m_RotationXYZ.y = _pi * 0.5f;
				change |= true;
			}

			ImGui::SameLine();

			if (ImGui::ContrastedButton("Y-", nullptr, nullptr, aw2))
			{
				m_CameraSettings.m_RotationXYZ.x = 0.0f;
				m_CameraSettings.m_RotationXYZ.y = -_pi * 0.5f;
				change |= true;
			}

			ImGui::SameLine();

			if (ImGui::ContrastedButton("Z+", nullptr, nullptr, aw2))
			{
				m_CameraSettings.m_RotationXYZ.x = 0.0f;
				m_CameraSettings.m_RotationXYZ.y = 0.0f;
				change |= true;
			}

			ImGui::SameLine();

			if (ImGui::ContrastedButton("Z-", nullptr, nullptr, aw2))
			{
				m_CameraSettings.m_RotationXYZ.x = _pi;
				m_CameraSettings.m_RotationXYZ.y = 0.0f;
				change |= true;
			}
		}

		const float aw3 = (ImGui::GetContentRegionMax().x - ImGui::GetCursorPosX() - ImGui::GetStyle().ItemInnerSpacing.x) * 0.25f;

		//if (m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_TURNTABLE_Y)
		{
			if (ImGui::ContrastedButton(u8"Y+1�", nullptr, nullptr, aw3))
			{
				m_CameraSettings.m_RotationXYZ.x -= DEGTORAD * 1.0f;
				change |= true;
			}

			ImGui::SameLine();

			if (ImGui::ContrastedButton(u8"Y-1�", nullptr, nullptr, aw3))
			{
				m_CameraSettings.m_RotationXYZ.x += DEGTORAD * 1.0f;
				change |= true;
			}

			ImGui::SameLine();

			if (ImGui::ContrastedButton(u8"XZ+1�", nullptr, nullptr, aw3))
			{
				m_CameraSettings.m_RotationXYZ.y += DEGTORAD * 1.0f;
				change |= true;
			}

			ImGui::SameLine();

			if (ImGui::ContrastedButton(u8"XZ-1�", nullptr, nullptr, aw3))
			{
				m_CameraSettings.m_RotationXYZ.y -= DEGTORAD * 1.0f;
				change |= true;
			}
		}

		//if (m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_TURNTABLE_Y)
		{
			if (ImGui::ContrastedButton(u8"Y+10�", nullptr, nullptr, aw3))
			{
				m_CameraSettings.m_RotationXYZ.x -= DEGTORAD * 10.0f;
				change |= true;
			}

			ImGui::SameLine();

			if (ImGui::ContrastedButton(u8"Y-10�", nullptr, nullptr, aw3))
			{
				m_CameraSettings.m_RotationXYZ.x += DEGTORAD * 10.0f;
				change |= true;
			}

			ImGui::SameLine();

			if (ImGui::ContrastedButton(u8"XZ+10�", nullptr, nullptr, aw3))
			{
				m_CameraSettings.m_RotationXYZ.y += DEGTORAD * 10.0f;
				change |= true;
			}

			ImGui::SameLine();

			if (ImGui::ContrastedButton(u8"XZ-10�", nullptr, nullptr, aw3))
			{
				m_CameraSettings.m_RotationXYZ.y -= DEGTORAD * 10.0f;
				change |= true;
			}
		}
	}

	if (change)
	{
		NeedCamChange();
	}

	return change;
}

bool CommonSystem::ForceUpdate(const ct::uvec2& vScreenSize)
{
	return ComputeCameras(vScreenSize);
}

bool CommonSystem::UpdateIfNeeded(const ct::uvec2& vScreenSize)
{
	if (m_NeedCamChange)
	{
		m_NeedCamChange = false;
		return ComputeCameras(vScreenSize);
	}

	return false;
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////

std::string CommonSystem::toStrFromGLMVec2(const glm::vec2& v, char delimiter)
{
	return ct::toStrFromArray(&v.x, 2, delimiter);
}

glm::vec2 CommonSystem::toGLMVec2FromStr(const std::string& vStr, char delimiter)
{
	glm::vec2 p = glm::vec2(0.0f);
	std::vector<float> result = ct::StringToNumberVector<float>(vStr, delimiter);
	const size_t s = result.size();
	if (s > 0U) p.x = result[0];
	if (s > 1U) p.y = result[1];
	return p;
}

std::string CommonSystem::toStrFromGLMVec3(const glm::vec3& v, char delimiter)
{
	return ct::toStrFromArray(&v.x, 3, delimiter);
}

glm::vec3 CommonSystem::toGLMVec3FromStr(const std::string& vStr, char delimiter)
{
	glm::vec3 p = glm::vec3(0.0f);
	std::vector<float> result = ct::StringToNumberVector<float>(vStr, delimiter);
	const size_t s = result.size();
	if (s > 0U) p.x = result[0];
	if (s > 1U) p.y = result[1];
	if (s > 2U) p.z = result[2];
	return p;
}

std::string CommonSystem::toStrFromGLMVec4(const glm::vec4& v, char delimiter)
{
	return ct::toStrFromArray(&v.x, 4, delimiter);
}

glm::vec4 CommonSystem::toGLMVec4FromStr(const std::string& vStr, char delimiter)
{
	glm::vec4 p = glm::vec4(0.0f);
	std::vector<float> result = ct::StringToNumberVector<float>(vStr, delimiter);
	const size_t s = result.size();
	if (s > 0U) p.x = result[0];
	if (s > 1U) p.y = result[1];
	if (s > 2U) p.z = result[2];
	if (s > 3U) p.w = result[3];
	return p;
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string CommonSystem::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	ZoneScoped;

	std::string str;

	str += vOffset + "<CommonSystem>\n";

	str += vOffset + "\t<TargetXYZ>" + toStrFromGLMVec3(m_CameraSettings.m_TargetXYZ) + "</TargetXYZ>\n";
	str += vOffset + "\t<TargetXYZLock>" + m_CameraSettings.m_TargetXYZLock.string() + "</TargetXYZLock>\n";
	str += vOffset + "\t<TransXY>" + toStrFromGLMVec2(m_CameraSettings.m_TransXY) + "</TransXY>\n";
	str += vOffset + "\t<TransXYLock>" + m_CameraSettings.m_TransXYLock.string() + "</TransXYLock>\n";
	str += vOffset + "\t<TransFactor>" + ct::toStr(m_CameraSettings.m_TransFactor) + "</TransFactor>\n";
	str += vOffset + "\t<RotXYZ>" + toStrFromGLMVec3(m_CameraSettings.m_RotationXYZ) + "</RotXYZ>\n";
	str += vOffset + "\t<RotXYZLock>" + m_CameraSettings.m_RotationXYZLock.string() + "</RotXYZLock>\n";
	str += vOffset + "\t<RotXYZFactor>" + ct::toStr(m_CameraSettings.m_RotationFactor) + "</RotXYZFactor>\n";
	str += vOffset + "\t<Zoom>" + ct::toStr(m_CameraSettings.m_Zoom) + "</Zoom>\n";
	str += vOffset + "\t<ZoomLock>" + ct::toStr(m_CameraSettings.m_ZoomLock) + "</ZoomLock>\n";
	str += vOffset + "\t<ZoomFactor>" + ct::toStr(m_CameraSettings.m_ZoomFactor) + "</ZoomFactor>\n";
	str += vOffset + "\t<PerspectiveAngle>" + ct::toStr(m_CameraSettings.m_PerspAngle) + "</PerspectiveAngle>\n";
	//str += vOffset + "\t<CamNear>" + ct::toStr(m_UBOCamera.cam_near) + "</CamNear>\n";
	str += vOffset + "\t<CamFar>" + ct::toStr(m_UBOCamera.cam_far) + "</CamFar>\n";
	str += vOffset + "\t<CameraType>" + ct::toStr((int)m_CameraSettings.m_CameraType) + "</CameraType>\n";
	str += vOffset + "\t<CameraType>" + ct::toStr((int)m_CameraSettings.m_CameraType) + "</CameraType>\n";
	str += vOffset + "\t<CameraMode>" + ct::toStr((int)m_CameraSettings.m_CameraMode) + "</CameraMode>\n";
#ifdef USE_VR
	str += vOffset + "\t<vrCamera>" + ct::toStr(m_CameraSettings.m_UseVRControllerForControlCamera) + "</vrCamera>\n";
	str += vOffset + "\t<vrCameraRotStepFactor>" + toStrFromGLMVec3(m_CameraSettings.m_VRRotStepFactor) + "</vrCameraRotStepFactor>\n";
	str += vOffset + "\t<vrCameraZoomStepFactor>" + ct::toStr(m_CameraSettings.m_VRZoomStepFactor) + "</vrCameraZoomStepFactor>\n";
#endif

	str += vOffset + "</CommonSystem>\n";

	return str;
}

bool CommonSystem::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	ZoneScoped;

	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strParentName == "CommonSystem")
	{
		if (strName == "TargetXYZ") {
			m_CameraSettings.m_TargetXYZ = toGLMVec3FromStr(strValue);
		}
		else if (strName == "TransXYLock") {
			m_CameraSettings.m_TargetXYZLock = (ct::bvec3)ct::ivariant(strValue).GetV3();
		}
		else if (strName == "TransXY") {
			m_CameraSettings.m_TransXY = toGLMVec2FromStr(strValue);
		}
		else if (strName == "TransXYZLock") {
			m_CameraSettings.m_TransXYLock = (ct::bvec2)ct::ivariant(strValue).GetV2();
		}
		else if (strName == "TransFactor") {
			m_CameraSettings.m_TransFactor = ct::fvariant(strValue).GetF();
		}
		else if (strName == "RotXYZ") {
			m_CameraSettings.m_RotationXYZ = toGLMVec3FromStr(strValue);
		}
		else if (strName == "RotXYZLock") {
			m_CameraSettings.m_RotationXYZLock = (ct::bvec3)ct::ivariant(strValue).GetV3();
		}
		else if (strName == "RotXYZFactor") {
			m_CameraSettings.m_RotationFactor = ct::fvariant(strValue).GetF();
		}
		else if (strName == "Zoom") {
			m_CameraSettings.m_Zoom = ct::fvariant(strValue).GetF();
		}
		else if (strName == "ZoomLock") {
			m_CameraSettings.m_ZoomLock = ct::fvariant(strValue).GetB();
		}
		else if (strName == "ZoomFactor") {
			m_CameraSettings.m_ZoomFactor = ct::fvariant(strValue).GetF();
		}
		else if (strName == "PerspectiveAngle") {
			m_CameraSettings.m_PerspAngle = ct::fvariant(strValue).GetF();
		}
		else if (strName == "CamNear") {
			m_UBOCamera.cam_near = ct::fvariant(strValue).GetF();
		}
		//else if (strName == "CamFar") {
		//	m_UBOCamera.cam_far = ct::fvariant(strValue).GetF();
		//}
		else if (strName == "CameraType") {
			m_CameraSettings.m_CameraType = (CAMERA_TYPE_Enum)ct::ivariant(strValue).GetI();
		}
		else if (strName == "CameraMode") {
			m_CameraSettings.m_CameraMode = (CAMERA_MODE_Enum)ct::ivariant(strValue).GetI();
		}
#ifdef USE_VR
		else if (strName == "vrCamera") {
			m_CameraSettings.m_UseVRControllerForControlCamera = ct::ivariant(strValue).GetB();
		}
		else if (strName == "vrCameraRotStepFactor") {
			m_CameraSettings.m_VRRotStepFactor = toGLMVec3FromStr(strValue);
		}
		else if (strName == "vrCameraZoomStepFactor") {
			m_CameraSettings.m_VRZoomStepFactor = ct::fvariant(strValue).GetF();
		}
#endif
		return false;
	}

	return true;
}

void CommonSystem::SetMousePos(ct::fvec2 vNormalizedPos, ct::fvec2 /*vBufferSize*/, bool vDownButton[5])
{
	ZoneScoped;

	m_UBOCamera.left_mouse.x = vNormalizedPos.x;
	m_UBOCamera.left_mouse.y = vNormalizedPos.y;

	if (vDownButton[0])
	{
		m_UBOCamera.left_mouse.z = vNormalizedPos.x;
		m_UBOCamera.left_mouse.w = vNormalizedPos.y;
	}
	else
	{
		m_UBOCamera.left_mouse.z = 0.0f;
		m_UBOCamera.left_mouse.w = 0.0f;
	}

	m_BufferObjectIsDirty = true;
}

void CommonSystem::SetScreenSize(ct::uvec2 vScreenSize)
{
	ZoneScoped;

	m_UBOCamera.screenSize = vScreenSize;

	m_BufferObjectIsDirty = true;
}

void CommonSystem::SetViewportSize(ct::uvec2 vViewportSize)
{
	ZoneScoped;

	m_UBOCamera.viewportSize = vViewportSize;

	m_BufferObjectIsDirty = true;
}

void CommonSystem::UploadBufferObjectIfDirty(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	if (m_BufferObjectIsDirty)
	{
		vkApi::VulkanRessource::upload(vVulkanCorePtr, m_BufferObjectPtr, &m_UBOCamera, sizeof(UBOCamera));

		m_BufferObjectIsDirty = false;
	}
}

bool CommonSystem::CreateBufferObject(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	vVulkanCorePtr->getDevice().waitIdle();

	m_BufferObjectPtr = vkApi::VulkanRessource::createUniformBufferObject(vVulkanCorePtr, sizeof(UBOCamera));
	if (m_BufferObjectPtr && m_BufferObjectPtr->buffer)
	{
		m_DescriptorBufferInfo = vk::DescriptorBufferInfo { m_BufferObjectPtr->buffer, 0, sizeof(UBOCamera) };
	}
	else
	{
		m_DescriptorBufferInfo = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	}

	m_BufferObjectIsDirty = true;

	return true;
}

void CommonSystem::DestroyBufferObject()
{
	ZoneScoped;

	m_BufferObjectPtr.reset();
}

std::string CommonSystem::GetBufferObjectStructureHeader(const uint32_t& vBindingPoint)
{
	return ct::toStr(u8R"(
layout(std140, binding = %u) uniform UBO_Camera 
{
	mat4 cam;			// the MVP matrix
	mat4 model;			// the model matrix
	mat4 view;			// the view matrix
	mat4 proj;			// the proj matrix
	mat4 normalMat;		// the normal matrix
	vec4 left_mouse;	// 2pos_2click_normalized
	vec4 middle_mouse;	// 2pos_2click_normalized
	vec4 right_mouse;	// 2pos_2click_normalized
	vec2 screenSize;	// the screensize
	vec2 viewportSize;	// the viewportSize
	float cam_near;		// the cam near
	float cam_far;		// the cam far
};

float LinearizeDepth(float vDepth)
{
	return (cam_near * cam_far) / (cam_far + cam_near - vDepth * (cam_far - cam_near));
}

float DeLinearizeDepth(float vDepth)
{
	return (cam_near + cam_far) * vDepth - cam_far * cam_near / (cam_near - cam_far) * vDepth;
}
)", vBindingPoint);
}

///////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////
///////////////////////////////////////////////////////

glm::mat4 CommonSystem::ComputeProjectionMatrix(const ct::uvec2& vScreenSize)
{
	ZoneScoped;

	glm::mat4 res = glm::mat4(1.0f);
	
	if (!vScreenSize.emptyOR())
	{
		camSize = glm::vec2((float)vScreenSize.x, (float)vScreenSize.y);

		if (m_CameraSettings.m_CameraType == CAMERA_TYPE_Enum::CAMERA_TYPE_ORTHOGRAPHIC)
		{
			const ct::fvec2 s = ct::fvec2(m_CameraSettings.m_Zoom, m_CameraSettings.m_Zoom * camSize.y / camSize.x) * 0.5f;
			res = glm::ortho(-s.x, s.x, -s.y, s.y, m_UBOCamera.cam_near, m_UBOCamera.cam_far);
		}
		else if (m_CameraSettings.m_CameraType == CAMERA_TYPE_Enum::CAMERA_TYPE_PERSPECTIVE)
		{
			res = glm::perspective(glm::radians(m_CameraSettings.m_PerspAngle), camSize.x / camSize.y, m_UBOCamera.cam_near, m_UBOCamera.cam_far);
		}
	}

	res[1][1] *= -1;
	return res;
}

glm::mat4 CommonSystem::ComputeModelMatrix()
{
	ZoneScoped;

	uView = glm::mat4(1.0f);

	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);

	glm::quat rx = glm::angleAxis(m_CameraSettings.m_RotationXYZ.x, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::quat ry = glm::angleAxis(m_CameraSettings.m_RotationXYZ.y, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::quat rz = glm::angleAxis(m_CameraSettings.m_RotationXYZ.z, glm::vec3(0.0f, 0.0f, 1.0f));
	camOrientation = glm::normalize(rz * ry * rx);

	if (m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_TURNTABLE_Y)
	{
		Model = glm::translate(glm::vec3(m_CameraSettings.m_TransXY.x, m_CameraSettings.m_TransXY.y, -m_CameraSettings.m_Zoom));
		Model *= glm::mat4_cast(camOrientation);
		Model = glm::translate(Model, glm::vec3(m_CameraSettings.m_TargetXYZ.x, m_CameraSettings.m_TargetXYZ.y, m_CameraSettings.m_TargetXYZ.z));
	}
	else if (m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_FREE)
	{
		Model = glm::translate(
			glm::mat4_cast(camOrientation),
			glm::vec3(m_CameraSettings.m_PosXYZ.x, m_CameraSettings.m_PosXYZ.y, m_CameraSettings.m_PosXYZ.z));
	}

	Model = glm::scale(Model, glm::vec3(m_CameraSettings.m_UniformScale));

	return Model;
}

// voir le code de la camera ici : https://github.com/g-truc/glm
glm::mat4 CommonSystem::ComputeCameraMatrix()
{
	ZoneScoped;

	// Our ModelViewProjection : multiplication of our 3 matrices
	// mvp Projection * View * Model; // Remember, matrix multiplication is the other way around

	return uProj * uView * uModel;
}

// voir le code de la camera ici : https://github.com/g-truc/glm
glm::mat4 CommonSystem::ComputeInvCameraMatrix()
{
	ZoneScoped;

	// Our ModelViewProjection : multiplication of our 3 matrices
	// mvp Projection * View * Model; // Remember, matrix multiplication is the other way around

	return glm::inverse(ComputeCameraMatrix());
}
glm::mat4 CommonSystem::ComputeCameraMatrix(const ct::uvec2& vScreenSize)
{
	ZoneScoped;

	ComputeProjectionMatrix(vScreenSize);
	ComputeModelMatrix();
	return ComputeCameraMatrix();
}

glm::mat4 CommonSystem::ComputeNormalMatrix()
{
	ZoneScoped;

	return glm::transpose(glm::inverse(uView * uModel));
}

bool CommonSystem::ComputeCameras(const ct::uvec2& vScreenSize)
{
	ZoneScoped;

	if (m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_TURNTABLE_Y)
	{
		uModel = ComputeModelMatrix();
	}
	else if (m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_FREE)
	{
		/*
		// voir aussi ce lien https://gamedev.stackexchange.com/questions/158050/what-am-i-missing-for-this-flycam-to-work-correctly?rq=1

		m_CameraSettings.m_RotationXYZ.x += GamePadSystem::Instance()->m_LeftThumb.y * m_CameraSettings.m_RotFactor; // pitch => x
		m_CameraSettings.m_RotationXYZ.y += GamePadSystem::Instance()->m_RightThumb.x * m_CameraSettings.m_RotFactor; // yaw => Y
		m_CameraSettings.m_RotationXYZ.z += GamePadSystem::Instance()->m_LeftThumb.x * m_CameraSettings.m_RotFactor; // roll => z

		glm::quat q = glm::quat(glm::vec3(m_CameraSettings.m_RotationXYZ.x, m_CameraSettings.m_RotationXYZ.y, m_CameraSettings.m_RotationXYZ.z)); // pitch, yaw, roll

		m_CameraSettings.m_PosXYZ.x = uModel[3].x;
		m_CameraSettings.m_PosXYZ.y = uModel[3].y;
		m_CameraSettings.m_PosXYZ.z = uModel[3].z;

		float st = m_CameraSettings.m_SpeedFactor * (GamePadSystem::Instance()->m_RightTrigger - GamePadSystem::Instance()->m_LeftTrigger);

		uModel = glm::mat4(1.0f);
		uModel = glm::rotate(uModel, m_CameraSettings.m_RotationXYZ.x, glm::vec3(0.0f, 1.0f, 0.0f)); // yaw   Y
		uModel = glm::rotate(uModel, m_CameraSettings.m_RotationXYZ.y, glm::vec3(1.0f, 0.0f, 0.0f)); // pitch X
		uModel = glm::rotate(uModel, m_CameraSettings.m_RotationXYZ.z, glm::vec3(0.0f, 0.0f, 1.0f)); // roll  Z
		glm::vec3 rd = glm::normalize(glm::vec3(0, 0, 1) * glm::mat3x3(uModel));

		m_CameraSettings.m_PosXYZ.x += rd.x * st;
		m_CameraSettings.m_PosXYZ.y += rd.y * st;
		m_CameraSettings.m_PosXYZ.z += rd.z * st;

		uModel[3].x = m_CameraSettings.m_PosXYZ.x;
		uModel[3].y = m_CameraSettings.m_PosXYZ.y;
		uModel[3].z = m_CameraSettings.m_PosXYZ.z;

		//uModel = GetModelMatrix(glm::vec3(m_PosXYZ.x, m_PosXYZ.y, m_PosXYZ.z), glm::vec3(m_RotationXYZ.x, m_RotationXYZ.y, m_RotationXYZ.z));
		uView = glm::inverse(uModel); // https://community.khronos.org/t/glm-inaccurate-camera-rotations/65263
		*/

		uModel = ComputeModelMatrix();
	}

	uProj = ComputeProjectionMatrix(vScreenSize);
	uCam = ComputeCameraMatrix();
	uNormalMatrix = ComputeNormalMatrix();
	uInvCam = ComputeInvCameraMatrix();

	m_UBOCamera.cam = uCam;
	m_UBOCamera.model = uModel;
	m_UBOCamera.view = uView;
	m_UBOCamera.proj = uProj;
	m_UBOCamera.normal = uNormalMatrix;
	m_BufferObjectIsDirty = true;

	return true;
}
