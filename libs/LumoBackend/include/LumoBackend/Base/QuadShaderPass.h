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

#include <LumoBackend/Base/MeshShaderPass.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

class LUMO_BACKEND_API QuadShaderPass : public MeshShaderPass<VertexStruct::P2_T2>
{
public:
	QuadShaderPass(GaiApi::VulkanCorePtr vVulkanCorePtr, const MeshShaderPassType& vMeshShaderPassType);
	QuadShaderPass(GaiApi::VulkanCorePtr vVulkanCorePtr, const MeshShaderPassType& vMeshShaderPassType,
		vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool);

	bool BuildModel() override;
	void DestroyModel(const bool& vReleaseDatas = false) override;

	std::string GetVertexShaderCode(std::string& vOutShaderName) override;
	std::string GetFragmentShaderCode(std::string& vOutShaderName) override;
};