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
#pragma warning(disable : 4251)

#include <vector>
#include <ctools/cTools.h>
#include <Gaia/gaia.h>
#include <LumoBackend/Interfaces/BufferObjectInterface.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

/*
- group of lights
- for the moment max 8 lights

*/

class SceneTextureGroup;
typedef std::shared_ptr<SceneTextureGroup> SceneTextureGroupPtr;
typedef std::weak_ptr<SceneTextureGroup> SceneTextureGroupWeak;

class LUMO_BACKEND_API SceneTextureGroup :
	public BufferObjectInterface
{
public:
	static constexpr uint32_t sMaxTextureCount = 8U;

public:
	static SceneTextureGroupPtr Create(GaiApi::VulkanCorePtr vVulkanCorePtr);
	static std::string GetBufferObjectStructureHeader(const uint32_t& vBindingPoint, const char* vTextureName, const uint32_t& vCount);
	static VulkanBufferObjectPtr CreateEmptyBuffer(GaiApi::VulkanCorePtr vVulkanCorePtr);

private:
	GaiApi::VulkanCorePtr m_VulkanCorePtr = nullptr;
	SceneTextureGroupWeak m_This;
	DescriptorImageInfoVector m_Textures;
	vk::DescriptorBufferInfo m_EmptyBufferInfo = { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

public:
	SceneTextureGroup();
	~SceneTextureGroup();

	bool Init(GaiApi::VulkanCorePtr vVulkanCorePtr);
	void Unit();

	void clear();
	bool empty();
	size_t size();
	DescriptorImageInfoVector::iterator begin();
	DescriptorImageInfoVector::iterator end();
	void erase(uint32_t vIdx);
	vk::DescriptorImageInfo* Add();
	vk::DescriptorImageInfo* Get(const size_t& vIndex);

	bool CanAddTexture() const;
	bool CanRemoveTexture() const;

	bool IsOk();
	void UploadBufferObjectIfDirty(GaiApi::VulkanCorePtr vVulkanCorePtr) override;
	bool CreateBufferObject(GaiApi::VulkanCorePtr vVulkanCorePtr) override;
	void DestroyBufferObject() override;
	vk::DescriptorBufferInfo* GetBufferInfo() override;

};