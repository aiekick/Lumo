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

#include <SceneGraph/SceneLight.h>
#include <Systems/CommonSystem.h>

SceneLightPtr SceneLight::Create()
{
	auto res = std::make_shared<SceneLight>();
	res->m_This = res;
	res->name = "Light";
	return res;
}

std::string SceneLight::GetStructureHeader()
{
	return u8R"(
struct LightDatas
{
	mat4 lightGizmo;
	mat4 lightView;
	vec4 lightColor;
	float lightIntensity;
	int lightType;
	float orthoSideSize;
	float orthoRearSize;
	float orthoDeepSize;
	float perspectiveAngle;
};)";
}

float* SceneLight::GetGizmoFloatPtr()
{
	return glm::value_ptr(lightDatas.lightGizmo);
}

bool SceneLight::NeedUpdateCamera()
{
	ZoneScoped;

	if (wasChanged)
	{
		const glm::mat4 bias(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.5f, 0.5f, 0.5f, 1.0f);

		switch ((LightTypeEnum)lightDatas.lightType)
		{
			case LightTypeEnum::POINT:
			{
				break;
			}
			case LightTypeEnum::DIRECTIONNAL:
			{
				glm::mat4 proj = glm::ortho<float>(
					-lightDatas.orthoSideSize, lightDatas.orthoSideSize, 
					-lightDatas.orthoSideSize, lightDatas.orthoSideSize, 
					-lightDatas.orthoRearSize, lightDatas.orthoDeepSize);
				proj[1][1] *= -1;
				glm::mat4 view = glm::lookAt(glm::vec3(lightDatas.lightGizmo[3]), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				lightDatas.lightView = proj * view;
				break;
			}
			case LightTypeEnum::SPOT:
			{
				const float persp_ratio = 1.0f;
				const float persp_near = CommonSystem::Instance()->m_UBOCamera.cam_near;
				const float persp_far = CommonSystem::Instance()->m_UBOCamera.cam_far;
				glm::mat4 proj = glm::perspective(glm::radians(lightDatas.perspectiveAngle), persp_ratio, persp_near, persp_far);
				proj[1][1] *= -1;
				glm::mat4 view = glm::lookAt(glm::vec3(lightDatas.lightGizmo[3]), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				lightDatas.lightView = proj * view;
				break;
			}
			case LightTypeEnum::AREA:
			{
				break;
			}
		}

		wasChanged = false;

		return true;
	}

	return false;
}
