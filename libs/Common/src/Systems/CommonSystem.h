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

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>
#include <Interfaces/CameraInterface.h>
#include <Interfaces/BufferObjectInterface.h>
#include <vkFramework/VulkanRessource.h>

#pragma warning(push)
#pragma warning(disable:4201)   // suppress even more warnings about nameless structs
#include <glm/glm/glm.hpp>
#include <glm/glm/vec3.hpp> // glm::vec3
#include <glm/glm/vec4.hpp> // glm::vec4
#include <glm/glm/gtc/type_ptr.hpp>
#include <glm/glm/gtc/quaternion.hpp>
#include <glm/glm/gtx/intersect.hpp>
#include <glm/glm/gtx/transform.hpp>
#pragma warning(pop)

#ifdef BUILD_SHARED
#ifdef _WIN32
#define COMMON_API   __declspec( dllimport )
#endif // _WIN32
#endif // BUILD_SHARED
#ifndef COMMON_API
#define COMMON_API
#endif // COMMON_API

#ifndef CAM_FAR
#define CAM_FAR 1000.0f
#endif

enum class CAMERA_TYPE_Enum : uint8_t
{
	CAMERA_TYPE_PERSPECTIVE = 0,
	CAMERA_TYPE_ORTHOGRAPHIC,
	CAMERA_TYPE_Count
};

enum class CAMERA_MODE_Enum : uint8_t
{
	CAMERA_MODE_TURNTABLE_Y = 0,
	CAMERA_MODE_FREE,
	CAMERA_MODE_Count
};

/*
unique class for :
 - camera
 - screen size
 - mouse pos
 - etc..

with a unique UBO who will be linked every where
*/
class COMMON_API CommonSystem :
	public conf::ConfigAbstract,
	public CameraInterface,
	public BufferObjectInterface
{
public:
	static std::string GetBufferObjectStructureHeader(const uint32_t& vBindingPoint);

public: // params to be serialized
	glm::vec2 camSize;
	//glm::vec2 nearFarPlanes = glm::vec2(0.01f, 10000.0f);
	glm::vec4 lrbtPlanes = glm::vec4(-10.0f, 10.0f, -10.0f, 10.0f);
	glm::quat camOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // direction of the camera (free fly mode), w is in first

	struct CameraSettingsStruct
	{
		// Rot Y mode
		glm::vec2 m_TransXY = glm::vec2(0.0f);
		ct::bvec2 m_TransXYLock = true;
		float m_TransFactor = 5.0f;
		float m_Zoom = 5.0f;
		bool m_ZoomLock = false;
		float m_ZoomFactor = 5.0f;

		// flight mode
		glm::vec3 m_PosXYZ = glm::vec3(0.0f);
		float m_SpeedFactor = 0.01f;

		// Rot Y mode and flight mode
		glm::vec3 m_TargetXYZ = glm::vec3(0.0f);
		ct::bvec3 m_TargetXYZLock = true;
		glm::vec3 m_RotationXYZ = glm::vec3(0.785f, -0.612f, 0.0f);
		ct::bvec3 m_RotationXYZLock = ct::bvec3(false, false, true);
		float m_RotationFactor = 1.0f;
		bool m_UniformScaleLock = true;
		float m_UniformScale = 1.0f;

		float m_PerspAngle = 45.0f;

		CAMERA_TYPE_Enum m_CameraType = CAMERA_TYPE_Enum::CAMERA_TYPE_PERSPECTIVE;
		CAMERA_MODE_Enum m_CameraMode = CAMERA_MODE_Enum::CAMERA_MODE_TURNTABLE_Y;

#ifdef USE_VR
		bool m_UseVRControllerForControlCamera = false;
		glm::vec3 m_VRRotStepFactor = glm::vec3(0.01f);
		float m_VRZoomStepFactor = 1.0f;
#endif

	} m_CameraSettings;

public:
	struct UBOCamera {
		alignas(16) glm::mat4x4 cam = glm::mat4x4(1.0f);
		alignas(16) glm::mat4x4 model = glm::mat4x4(1.0f);
		alignas(16) glm::mat4x4 view = glm::mat4x4(1.0f);
		alignas(16) glm::mat4x4 proj = glm::mat4x4(1.0f);
		alignas(16) glm::mat4x4 normal = glm::mat4x4(1.0f);
		alignas(16) ct::fvec4 left_mouse;
		alignas(16) ct::fvec4 middle_mouse;
		alignas(16) ct::fvec4 right_mouse;
		alignas(8) ct::fvec2 screenSize;
		alignas(8) ct::fvec2 viewportSize;
		alignas(4) float cam_near = 0.001f;
		alignas(4) float cam_far = CAM_FAR;
	} m_UBOCamera;
	bool m_NeedCamChange = true;

public:
	static CommonSystem* Instance(CommonSystem* vCopy = nullptr, bool vForce = false)
	{
		static CommonSystem _instance;
		static CommonSystem* _instance_copy = nullptr;
		if (vCopy || vForce)
			_instance_copy = vCopy;
		if (_instance_copy)
			return _instance_copy;
		return &_instance;
	}

protected:
	CommonSystem() = default; // Prevent construction
	CommonSystem(const CommonSystem&) = default; // Prevent construction by copying
	CommonSystem& operator =(const CommonSystem&) { return *this; }; // Prevent assignment
	~CommonSystem() = default; // Prevent unwanted destruction

public:
	void SetCameraMode(const CAMERA_MODE_Enum& vCameraMode);
	void SetCameraType(const CAMERA_TYPE_Enum& vCameraType);

	void SetPerspectiveAngle(const float& vAngle);
	void SetPerspectiveLimitZ(
		const float& vNearPlane,
		const float& vFarPlane);
	void SetOrthographicLimitZ(
		const float& vLeftPlane,
		const float& vRightPlane,
		const float& vBottomPlane,
		const float& vTopPlane);

	void SetTargetXYZ(const ct::fvec3& vTarget, bool vForce = false);

	void IncTranslateXY(const ct::fvec2& vMouseOffset);
	void SetTranslateXY(const ct::fvec2& vTranlation, bool vForce = false);
	void SetTranslateFactor(const float& vTranslateFactor);

	void IncRotateXYZ(const ct::fvec3& vMouseOffset);
	void SetRotateXYZ(const ct::fvec3& vRotate);
	void SetRotateFactor(const float& vRotateFactor);

	void IncZoom(const float& vMouseOffset);
	void SetZoom(const float& vZoom);
	void SetZoomFactor(const float& vZoomFactor);

	void IncFlyingPosition(const float& vMovementOffset);

	const CameraSettingsStruct& GetCameraSettings() const { return m_CameraSettings; }
	void SetViewMatrix(const glm::mat4x4& vMatrix);
	void SetProjMatrix(const glm::mat4x4& vMatrix);
	void NeedCamChange() override;
	bool DrawImGui();
	bool ForceUpdate(const ct::uvec2& vScreenSize);
	bool UpdateIfNeeded(const ct::uvec2& vScreenSize);

	///////////////////////////////////////////////////////
	///////////////////////////////////////////////////////
	///////////////////////////////////////////////////////

	std::string toStrFromGLMVec2(const glm::vec2& v, char delimiter = ';');
	glm::vec2 toGLMVec2FromStr(const std::string& vStr, char delimiter = ';');
	std::string toStrFromGLMVec3(const glm::vec3& v, char delimiter = ';');
	glm::vec3 toGLMVec3FromStr(const std::string& vStr, char delimiter = ';');
	std::string toStrFromGLMVec4(const glm::vec4& v, char delimiter = ';');
	glm::vec4 toGLMVec4FromStr(const std::string& vStr, char delimiter = ';');

	///////////////////////////////////////////////////////
	//// CONFIGURATION ////////////////////////////////////
	///////////////////////////////////////////////////////

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

	///////////////////////////////////////////////////////
	//// Infos ////////////////////////////////////////////
	///////////////////////////////////////////////////////

	void SetMousePos(ct::fvec2 vNormalizedPos, ct::fvec2 vBufferSize, bool vDownButton[5]);
	void SetScreenSize(ct::uvec2 vScreenSize);
	void SetViewportSize(ct::uvec2 vViewportSize);

	///////////////////////////////////////////////////////
	//// BUFFER OBJECTS ///////////////////////////////////
	///////////////////////////////////////////////////////

	void UploadBufferObjectIfDirty(vkApi::VulkanCorePtr vVulkanCorePtr) override;
	bool CreateBufferObject(vkApi::VulkanCorePtr vVulkanCorePtr) override;
	void DestroyBufferObject() override;

private:
	bool ComputeCameras(const ct::uvec2& vScreenSize);
	glm::mat4x4 ComputeCameraMatrix(const ct::uvec2& vScreenSize);
	glm::mat4x4 ComputeCameraMatrix();
	glm::mat4x4 ComputeProjectionMatrix(const ct::uvec2& vScreenSize);
	glm::mat4x4 ComputeModelMatrix();
	glm::mat4x4 ComputeNormalMatrix();
	glm::mat4x4 ComputeInvCameraMatrix();
};

