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

#include <SceneGraph/SceneLightGroup.h>

///////////////////////////////////////////////////////
//// STATIC ///////////////////////////////////////////
///////////////////////////////////////////////////////

SceneLightGroupPtr SceneLightGroup::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<SceneLightGroup>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

///////////////////////////////////////////////////////
//// STATIC : SBO//////////////////////////////////////
///////////////////////////////////////////////////////

std::string SceneLightGroup::GetBufferObjectStructureHeader(const uint32_t& vBinding)
{
	return ct::toStr(u8R"(
%s

layout(std430, binding = %u) readonly buffer SBO_Lights
{
	LightDatas lightDatas[];
};
)", SceneLight::GetStructureHeader().c_str(), vBinding);
}

// will create a empty sbo for default sbo when no slot are connected
VulkanBufferObjectPtr SceneLightGroup::CreateEmptyBuffer(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	if (vVulkanCorePtr)
	{
		auto size_in_bytes = sizeof(SceneLight::lightDatas);
		//gpu only since no udpate will be done
		return vkApi::VulkanRessource::createStorageBufferObject(
			vVulkanCorePtr, size_in_bytes, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);
	}

	return nullptr;
}

///////////////////////////////////////////////////////
//// PUBLIC : CTOR / DTOR /////////////////////////////
///////////////////////////////////////////////////////

SceneLightGroup::SceneLightGroup()
{

}

SceneLightGroup::~SceneLightGroup()
{
	Unit();
}

///////////////////////////////////////////////////////
//// PUBLIC : INIT / UNIT /////////////////////////////
///////////////////////////////////////////////////////

bool SceneLightGroup::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	m_VulkanCorePtr = vVulkanCorePtr;

	if (m_VulkanCorePtr)
	{
		if (empty())
		{
			Add(SceneLight::Create());
		}

		return CreateBufferObject(m_VulkanCorePtr);
	}

	return false;
}

void SceneLightGroup::Unit()
{
	DestroyBufferObject();
}

///////////////////////////////////////////////////////
//// PUBLIC ///////////////////////////////////////////
///////////////////////////////////////////////////////

std::vector<SceneLightPtr>::iterator SceneLightGroup::begin()
{
	ZoneScoped;

	return m_Lights.begin();
}

std::vector<SceneLightPtr>::iterator SceneLightGroup::end()
{
	ZoneScoped;

	return m_Lights.end();
}

size_t SceneLightGroup::size()
{
	ZoneScoped;

	return m_Lights.size();
}

void SceneLightGroup::clear()
{
	ZoneScoped;

	m_Lights.clear();
}

bool SceneLightGroup::empty()
{
	ZoneScoped;

	return m_Lights.empty();
}

void SceneLightGroup::Add(const SceneLightPtr& vLightPtr)
{
	ZoneScoped;

	if (vLightPtr)
	{
		/*
		alignas(16) glm::mat4x4 lightGizmo = glm::mat4x4(1.0f);
		alignas(16) glm::mat4x4 lightView = glm::mat4x4(1.0f);
		alignas(16) ct::fvec4 lightColor = 1.0f;
		alignas(4) float lightIntensity = 1.0f;
		alignas(4) int lightType = (int)LightTypeEnum::DIRECTIONNAL;
		alignas(4) float orthoSideSize = 30.0f;
		alignas(4) float orthoRearSize = 1000.0f;
		alignas(4) float orthoDeepSize = 1000.0f;
		alignas(4) float perspectiveAngle = 45.0f;
		*/

		uint32_t idx = (uint32_t)m_Lights.size();

		m_SBO430.RegisterVar(ct::toStr("lightGizmo_%u", idx), vLightPtr->lightDatas.lightGizmo);
		m_SBO430.RegisterVar(ct::toStr("lightView_%u", idx), vLightPtr->lightDatas.lightView);
		m_SBO430.RegisterVar(ct::toStr("lightColor_%u", idx), vLightPtr->lightDatas.lightColor);
		m_SBO430.RegisterVar(ct::toStr("lightIntensity_%u", idx), vLightPtr->lightDatas.lightIntensity);
		m_SBO430.RegisterVar(ct::toStr("lightType_%u", idx), vLightPtr->lightDatas.lightType);
		m_SBO430.RegisterVar(ct::toStr("orthoSideSize_%u", idx), vLightPtr->lightDatas.orthoSideSize);
		m_SBO430.RegisterVar(ct::toStr("orthoRearSize_%u", idx), vLightPtr->lightDatas.orthoRearSize);
		m_SBO430.RegisterVar(ct::toStr("orthoDeepSize_%u", idx), vLightPtr->lightDatas.orthoDeepSize);
		m_SBO430.RegisterVar(ct::toStr("perspectiveAngle_%u", idx), vLightPtr->lightDatas.perspectiveAngle);

		m_Lights.push_back(vLightPtr);
	}
}

SceneLightWeak SceneLightGroup::Get(const size_t& vIndex)
{
	ZoneScoped;

	if (m_Lights.size() > (size_t)vIndex)
	{
		return m_Lights[(size_t)vIndex];
	}

	return SceneLightWeak();
}

///////////////////////////////////////////////////////
//// SBO //////////////////////////////////////////////
///////////////////////////////////////////////////////

void SceneLightGroup::UploadBufferObjectIfDirty(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	m_SBO430.Upload(vVulkanCorePtr, true);
}

bool SceneLightGroup::CreateBufferObject(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	if (vVulkanCorePtr && !m_Lights.empty())
	{
		vVulkanCorePtr->getDevice().waitIdle();

		return m_SBO430.CreateSBO(vVulkanCorePtr, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU);
	}

	return false;
}

void SceneLightGroup::DestroyBufferObject()
{
	ZoneScoped;

	m_SBO430.DestroySBO();
}

vk::DescriptorBufferInfo* SceneLightGroup::GetBufferInfo()
{
	if (m_SBO430.bufferObjectPtr && m_SBO430.bufferObjectPtr->buffer)
	{
		return &m_SBO430.bufferObjectPtr->bufferInfo;
	}

	return nullptr;
}

