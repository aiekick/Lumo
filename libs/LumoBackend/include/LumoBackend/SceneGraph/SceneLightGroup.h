/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

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
#pragma warning(disable : 4251)

#include <vector>
#include <ctools/cTools.h>
#include <LumoBackend/SceneGraph/SceneLight.h>
#include <LumoBackend/Interfaces/BufferObjectInterface.h>
#include <Gaia/Resources/StorageBufferStd430.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

/*
- group of lights
- for the moment max 8 lights

*/

class SceneLightGroup;
typedef std::shared_ptr<SceneLightGroup> SceneLightGroupPtr;
typedef std::weak_ptr<SceneLightGroup> SceneLightGroupWeak;

class LUMO_BACKEND_API SceneLightGroup :
	public BufferObjectInterface
{
public:
	static constexpr uint32_t sMaxLightCount = 8U;

public:
	static SceneLightGroupPtr Create(GaiApi::VulkanCoreWeak vVulkanCore);
	static std::string GetBufferObjectStructureHeader(const uint32_t& vBindingPoint);
	static VulkanBufferObjectPtr CreateEmptyBuffer(GaiApi::VulkanCoreWeak vVulkanCore);

private:
	GaiApi::VulkanCoreWeak m_VulkanCore;
	SceneLightGroupWeak m_This;
	uint32_t m_LightsCount = 0U;
	std::vector<SceneLightPtr> m_Lights;
	StorageBufferStd430 m_SBO430;
	vk::DescriptorBufferInfo m_SBO430BufferInfo = { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	vk::DescriptorBufferInfo m_EmptyBufferInfo = { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

public:
	SceneLightGroup();
	~SceneLightGroup();

	bool Init(GaiApi::VulkanCoreWeak vVulkanCore);
	void Unit();

	void clear();
	bool empty();
	size_t size();
	std::vector<SceneLightPtr>::iterator begin();
	std::vector<SceneLightPtr>::iterator end();
	void erase(uint32_t vIdx);
	SceneLightWeak Add();
	SceneLightWeak Get(const size_t& vIndex);
	StorageBufferStd430& GetSBO430() { return m_SBO430; }

	bool CanAddLight() const;
	bool CanRemoveLight() const;

	bool IsOk();
	void UploadBufferObjectIfDirty(GaiApi::VulkanCoreWeak vVulkanCore) override;
	bool CreateBufferObject(GaiApi::VulkanCoreWeak vVulkanCore) override;
	void DestroyBufferObject() override;
	vk::DescriptorBufferInfo* GetBufferInfo() override;

};