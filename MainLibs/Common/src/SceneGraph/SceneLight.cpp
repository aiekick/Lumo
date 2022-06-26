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

#include <SceneGraph/SceneLight.h>
#include <Systems/CommonSystem.h>

SceneLightPtr SceneLight::Create()
{
	auto res = std::make_shared<SceneLight>();
	res->m_This = res;
	res->name = "Light";
	return res;
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
				/*glm::mat4 view = glm::mat4(1.0f);
				memcpy(glm::value_ptr(view), glm::value_ptr(m_LightGizmo), 16 * sizeof(float));*/
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
				/*glm::mat4 view = glm::mat4(1.0f);
				memcpy(glm::value_ptr(view), glm::value_ptr(m_LightGizmo), 16 * sizeof(float));*/
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
