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

#include <Base/ShaderPass.h>
#include <SceneGraph/SceneModel.h>

class RtxShaderPass : public ShaderPass
{
private:
	std::vector<VulkanAccelStructObjectPtr> m_AccelStructure_Bottom_Ptrs;
	VulkanAccelStructObjectPtr m_AccelStructure_Top_Ptr = nullptr;

public:
	RtxShaderPass(vkApi::VulkanCorePtr vVulkanCorePtr);
	RtxShaderPass(vkApi::VulkanCorePtr vVulkanCorePtr,
		vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool);

	bool BuildModel() override;
	void DestroyModel(const bool& vReleaseDatas = false) override;

	std::string GetVertexShaderCode(std::string& vOutShaderName) override;
	std::string GetFragmentShaderCode(std::string& vOutShaderName) override;

protected:
	// https://developer.nvidia.com/blog/vulkan-raytracing/

	bool CreateRtxPipeline() override;

	bool CreateBottomLevelAccelerationStructureForMesh(SceneMeshWeak vMesh);
	void DestroyBottomLevelAccelerationStructureForMesh();

	bool CreateTopLevelAccelerationStructure(std::vector<vk::AccelerationStructureInstanceKHR>& vBlasInstances);
	void DestroyTopLevelAccelerationStructure();

	vk::AccelerationStructureInstanceKHR CreateBlasInstance(const uint32_t& blas_id, glm::mat4& mat);

	bool CreateShaderBindingTable();
	void DestroyShaderBindingTable();
};