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

#include <vector>
#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <SceneGraph/SceneMesh.h>
#include <Interfaces/GizmoInterface.h>

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

class SceneLight : public GizmoInterface
{
public:
	static SceneLightPtr Create();

private:
	SceneLightWeak m_This; 

public:
	struct LightDatasStruct {
		alignas(16) glm::mat4x4 lightGizmo = glm::mat4x4(1.0f);
		alignas(16) glm::mat4x4 lightView = glm::mat4x4(1.0f);
		alignas(16) ct::fvec4 lightColor = 1.0f;
		alignas(4) float lightIntensity = 1.0f;
		alignas(4) int lightType = (int)LightTypeEnum::DIRECTIONNAL;
		alignas(4) float orthoSideSize = 30.0f;
		alignas(4) float orthoRearSize = 1000.0f;
		alignas(4) float orthoDeepSize = 1000.0f;
		alignas(4) float perspectiveAngle = 45.0f;
	} lightDatas;
	
public:
	float* GetGizmoFloatPtr() override;

	bool NeedUpdateCamera();
};