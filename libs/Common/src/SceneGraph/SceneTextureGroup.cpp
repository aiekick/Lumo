// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include <SceneGraph/SceneTextureGroup.h>

///////////////////////////////////////////////////////
//// STATIC ///////////////////////////////////////////
///////////////////////////////////////////////////////

SceneTextureGroupPtr SceneTextureGroup::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<SceneTextureGroup>();
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

std::string SceneTextureGroup::GetBufferObjectStructureHeader(const uint32_t& vBindingPoint, const char *vTextureName, const uint32_t& vCount)
{
	return ct::toStr("layout(binding = %u) uniform sampler2D %s[%u];\n", vBindingPoint, vTextureName, vCount);
}

// will create a empty sbo for default sbo when no slot are connected
VulkanBufferObjectPtr SceneTextureGroup::CreateEmptyBuffer(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	/*if (vVulkanCorePtr)
	{
		auto size_in_bytes = sizeof(SceneTexture::lightDatas);
		//gpu only since no udpate will be done
		return vkApi::VulkanRessource::createStorageBufferObject(
			vVulkanCorePtr, size_in_bytes, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);
	}*/

	return nullptr;
}

///////////////////////////////////////////////////////
//// PUBLIC : CTOR / DTOR /////////////////////////////
///////////////////////////////////////////////////////

SceneTextureGroup::SceneTextureGroup()
{
	
}

SceneTextureGroup::~SceneTextureGroup()
{
	Unit();
}

///////////////////////////////////////////////////////
//// PUBLIC : INIT / UNIT /////////////////////////////
///////////////////////////////////////////////////////

bool SceneTextureGroup::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	m_VulkanCorePtr = vVulkanCorePtr;

	if (m_VulkanCorePtr)
	{
		if (empty())
		{
			Add();
		}

		return CreateBufferObject(m_VulkanCorePtr);
	}

	return false;
}

void SceneTextureGroup::Unit()
{
	DestroyBufferObject();
}

///////////////////////////////////////////////////////
//// PUBLIC ///////////////////////////////////////////
///////////////////////////////////////////////////////

DescriptorImageInfoVector::iterator SceneTextureGroup::begin()
{
	ZoneScoped;

	return m_Textures.begin();
}

DescriptorImageInfoVector::iterator SceneTextureGroup::end()
{
	ZoneScoped;

	return m_Textures.end();
}

size_t SceneTextureGroup::size()
{
	ZoneScoped;

	return m_Textures.size();
}

void SceneTextureGroup::clear()
{
	ZoneScoped;

	m_Textures.clear();
}

bool SceneTextureGroup::empty()
{
	ZoneScoped;

	return m_Textures.empty();
}

vk::DescriptorImageInfo* SceneTextureGroup::Add()
{
	ZoneScoped;

	/*uint32_t idx = (uint32_t)m_Textures.size();

	if (idx <= sMaxTextureCount)
	{
		auto lightPtr = SceneTexture::Create();
		if (lightPtr)
		{
			m_SBO430.RegisterVar(ct::toStr("lightDatas_%u", idx), lightPtr->lightDatas);

			m_Textures.push_back(lightPtr);

			return Get(idx);
		}
	}*/

	return nullptr;
}

void SceneTextureGroup::erase(uint32_t vIndex)
{
	ZoneScoped;

	if ((size_t)vIndex < m_Textures.size())
	{
		m_Textures.erase(m_Textures.begin() + vIndex);
	}
}

vk::DescriptorImageInfo* SceneTextureGroup::Get(const size_t& vIndex)
{
	ZoneScoped;

	if (m_Textures.size() > (size_t)vIndex)
	{
		return &m_Textures.at((size_t)vIndex);
	}

	return nullptr;
}

bool SceneTextureGroup::CanAddTexture() const
{
	return (m_Textures.size() <= sMaxTextureCount);
}

bool SceneTextureGroup::CanRemoveTexture() const
{
	return (m_Textures.size() > 1U);
}

///////////////////////////////////////////////////////
//// SBO //////////////////////////////////////////////
///////////////////////////////////////////////////////

bool SceneTextureGroup::IsOk()
{
	return false;
}

void SceneTextureGroup::UploadBufferObjectIfDirty(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	//m_SBO430.Upload(vVulkanCorePtr, true);
}

bool SceneTextureGroup::CreateBufferObject(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	/*if (vVulkanCorePtr && !m_Textures.empty())
	{
		vVulkanCorePtr->getDevice().waitIdle();

		return m_SBO430.CreateSBO(vVulkanCorePtr, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU);
	}*/

	return false;
}

void SceneTextureGroup::DestroyBufferObject()
{
	ZoneScoped;

	//m_SBO430.DestroySBO();
}

vk::DescriptorBufferInfo* SceneTextureGroup::GetBufferInfo()
{
	/*if (m_SBO430.IsOk())
	{
		return &m_SBO430.descriptorBufferInfo;
	}*/
	return nullptr;
}

