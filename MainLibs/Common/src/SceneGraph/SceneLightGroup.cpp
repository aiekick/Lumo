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

SceneLightGroupPtr SceneLightGroup::Create()
{
	auto res = std::make_shared<SceneLightGroup>();
	res->m_This = res;
	return res;
}

///////////////////////////////////////////////////////
//// STATIC : SBO//////////////////////////////////////
///////////////////////////////////////////////////////

std::string SceneLightGroup::GetBufferObjectStructureHeader(const uint32_t& vBinding)
{
	return ct::toStr(u8R"(
%s

layout(std140, binding = %u) buffer SBO_Lights
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
			vVulkanCorePtr, size_in_bytes, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY);
	}

	return nullptr;
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

void SceneLightGroup::Add(const SceneLightPtr& vLight)
{
	ZoneScoped;

	m_Lights.push_back(vLight);
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

	if (vVulkanCorePtr && m_BufferObjectIsDirty)
	{
		auto size_in_bytes = sizeof(SceneLight::lightDatas) * m_Lights.size();
		vkApi::VulkanRessource::upload(vVulkanCorePtr, *m_BufferObjectPtr, m_Lights.data(), size_in_bytes);
		m_BufferObjectIsDirty = false;
	}
}

bool SceneLightGroup::CreateBufferObject(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	if (vVulkanCorePtr)
	{
		vVulkanCorePtr->getDevice().waitIdle();
		auto size_in_bytes = sizeof(SceneLight::lightDatas) * m_Lights.size();
		m_BufferObjectPtr = vkApi::VulkanRessource::createStorageBufferObject(
			vVulkanCorePtr, size_in_bytes, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU);
		m_BufferObjectIsDirty = true;

		return true;
	}

	return false;
}

void SceneLightGroup::DestroyBufferObject()
{
	ZoneScoped;

	m_BufferObjectPtr.reset();
}

