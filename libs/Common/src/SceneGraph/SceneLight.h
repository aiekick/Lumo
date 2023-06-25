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
#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <SceneGraph/SceneMesh.hpp>
#include <Interfaces/GizmoInterface.h>
#include <Common/Globals.h>

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

class SceneLight;
typedef std::shared_ptr<SceneLight> SceneLightPtr;
typedef ct::cWeak<SceneLight> SceneLightWeak;

enum class LightTypeEnum : uint8_t
{
	NONE = 0,
	POINT,
	DIRECTIONNAL,
	SPOT,
	AREA,
	Count
};
inline static std::string GetStringFromLightTypeEnum(const LightTypeEnum& vLightTypeEnum)
{
	static std::array<std::string, (uint8_t)LightTypeEnum::Count> LightTypeString = {
		"NONE",
		"POINT",
		"DIRECTIONNAL",
		"SPOT",
		"AREA"
	};
	if (vLightTypeEnum != LightTypeEnum::Count)
		return LightTypeString[(int)vLightTypeEnum];
	LogVarDebug("Error, one LightTypeEnum have no corresponding string, return \"None\"");
	return "NONE";
}

inline static LightTypeEnum GetLightTypeEnumFromString(const std::string& vNodeTypeString)
{
	if (vNodeTypeString == "NONE") return LightTypeEnum::NONE;
	else if (vNodeTypeString == "POINT") return LightTypeEnum::POINT;
	else if (vNodeTypeString == "DIRECTIONNAL") return LightTypeEnum::DIRECTIONNAL;
	else if (vNodeTypeString == "SPOT") return LightTypeEnum::SPOT;
	else if (vNodeTypeString == "AREA") return LightTypeEnum::AREA;
	return LightTypeEnum::NONE;
}

class COMMON_API SceneLight : public GizmoInterface
{
public:
	static SceneLightPtr Create();
	static std::string GetStructureHeader();

private:
	SceneLightWeak m_This; 

public:
	// std430
	struct LightDatasStruct {
		glm::mat4x4 lightGizmo = glm::mat4x4(1.0f);
		glm::mat4x4 lightView = glm::mat4x4(1.0f);
		ct::fvec4 lightColor = 1.0f;
		float lightIntensity = 1.0f;
		int lightType = (int)LightTypeEnum::DIRECTIONNAL;
		float orthoSideSize = 30.0f;
		float orthoRearSize = 1000.0f;
		float orthoDeepSize = 1000.0f;
		float perspectiveAngle = 45.0f;
		float lightActive = 1.0f;
		float is_inside = 0.0f;
	} lightDatas;
	
public:
	float* GetGizmoFloatPtr() override;

	bool NeedUpdateCamera();

	void AdaptIconColor();
};