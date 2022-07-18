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
#include <vulkan/vulkan.hpp>
#include <vkFramework/VulkanRessource.h>
#include <SceneGraph/SceneModel.h>

class SceneAccelStructure;
typedef std::shared_ptr<SceneAccelStructure> SceneAccelStructurePtr;
typedef ct::cWeak<SceneAccelStructure> SceneAccelStructureWeak;

class SceneAccelStructure
{
public:
	static SceneAccelStructurePtr Create(vkApi::VulkanCorePtr vVulkanCorePtr);

private:
	SceneAccelStructureWeak m_This;
	vk::Device m_Device = nullptr;
	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;
	std::vector<VulkanAccelStructObjectPtr> m_AccelStructure_Bottom_Ptrs;
	VulkanAccelStructObjectPtr m_AccelStructure_Top_Ptr = nullptr;
	vk::WriteDescriptorSetAccelerationStructureKHR m_AccelStructureTopDescriptorInfo;
	VulkanBufferObjectPtr m_ModelAdressesPtr = nullptr;
	vk::DescriptorBufferInfo m_ModelAdressesBufferInfo = { VK_NULL_HANDLE, 0U, VK_WHOLE_SIZE };

	bool m_SuccessfullyBuilt = false;

public:
	SceneAccelStructure(vkApi::VulkanCorePtr vVulkanCorePtr);
	void Clear();
	bool BuildForModel(SceneModelWeak vSceneModelWeak);
	bool IsOk() const { return m_SuccessfullyBuilt; }
	vk::WriteDescriptorSetAccelerationStructureKHR* GetTLASInfo();
	vk::DescriptorBufferInfo* GetBufferAddressInfo();

private:
	bool CreateBottomLevelAccelerationStructureForMesh(SceneMeshWeak vMesh);
	void DestroyBottomLevelAccelerationStructureForMesh();

	bool CreateTopLevelAccelerationStructure(std::vector<vk::AccelerationStructureInstanceKHR>& vBlasInstances);
	void DestroyTopLevelAccelerationStructure();

	vk::AccelerationStructureInstanceKHR CreateBlasInstance(const uint32_t& blas_id, glm::mat4& mat);

};