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
#include <SceneGraph/SceneLight.h>
#include <Interfaces/BufferObjectInterface.h>
#include <vkFramework/StorageBufferStd430.h>

class SceneLightGroup;
typedef std::shared_ptr<SceneLightGroup> SceneLightGroupPtr;
typedef ct::cWeak<SceneLightGroup> SceneLightGroupWeak;

class SceneLightGroup :
	public BufferObjectInterface
{
public:
	static SceneLightGroupPtr Create(vkApi::VulkanCorePtr vVulkanCorePtr);
	static std::string GetBufferObjectStructureHeader(const uint32_t& vBinding);
	static VulkanBufferObjectPtr CreateEmptyBuffer(vkApi::VulkanCorePtr vVulkanCorePtr);

private:
	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;
	SceneLightGroupWeak m_This;
	std::vector<SceneLightPtr> m_Lights;
	StorageBufferStd430 m_SBO430;
	vk::DescriptorBufferInfo m_EmptyBufferInfo = { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

public:
	SceneLightGroup();
	~SceneLightGroup();

	bool Init(vkApi::VulkanCorePtr vVulkanCorePtr);
	void Unit();

	void clear();
	bool empty();
	size_t size();
	std::vector<SceneLightPtr>::iterator begin();
	std::vector<SceneLightPtr>::iterator end();
	void Add(const SceneLightPtr& vLight);
	SceneLightWeak Get(const size_t& vIndex);
	StorageBufferStd430& GetSBO430() { return m_SBO430; }

	bool IsOk();
	void UploadBufferObjectIfDirty(vkApi::VulkanCorePtr vVulkanCorePtr) override;
	bool CreateBufferObject(vkApi::VulkanCorePtr vVulkanCorePtr) override;
	void DestroyBufferObject() override;
	vk::DescriptorBufferInfo* GetBufferInfo() override;

};