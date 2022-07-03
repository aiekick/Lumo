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
#include <SceneGraph/SceneMesh.h>

enum class MeshShaderPassType : uint8_t
{
	NONE = 0,
	PIXEL,			// vertex + fragment (shader + m_Pipeline + ubo + fbo + renderpass + command buffer)
	PIXEL_MERGER,		// pas de shader ni m_Pipeline ni ubo, mais command buffer, fbo et renderpass
	Count
};

template<typename T_VertexType>
class MeshShaderPass : public ShaderPass
{
protected:
	MeshInfo<T_VertexType> m_Vertices;
	MeshInfo<VertexStruct::I1> m_Indices;

public:
	MeshShaderPass(vkApi::VulkanCorePtr vVulkanCorePtr, const MeshShaderPassType& vMeshShaderPassType);
	MeshShaderPass(vkApi::VulkanCorePtr vVulkanCorePtr, const MeshShaderPassType& vMeshShaderPassType,
		vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool);

	void DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber) override;
	void SetInputStateBeforePipelineCreation() override;
};

////////////////////////////////////////////////////////////////////
///// IMPLEMTNATION ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

template<typename T_VertexType>
MeshShaderPass<T_VertexType>::MeshShaderPass(
	vkApi::VulkanCorePtr vVulkanCorePtr,
	const MeshShaderPassType& vMeshShaderPassType)
	: ShaderPass(
		vVulkanCorePtr,
		(GenericType)vMeshShaderPassType)
{
	
}

template<typename T_VertexType>
MeshShaderPass<T_VertexType>::MeshShaderPass(
	vkApi::VulkanCorePtr vVulkanCorePtr,
	const MeshShaderPassType& vMeshShaderPassType,
	vk::CommandPool* vCommandPool,
	vk::DescriptorPool* vDescriptorPool)
	: ShaderPass(
		vVulkanCorePtr,
		(GenericType)vMeshShaderPassType,
		vCommandPool,
		vDescriptorPool)
{

}

template<typename T_VertexType>
void MeshShaderPass<T_VertexType>::DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (!m_Loaded) return;

	if (vCmdBuffer && m_Vertices.m_Count)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);

		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout, 0, m_DescriptorSet, nullptr);

		vk::DeviceSize offsets = 0;
		vCmdBuffer->bindVertexBuffers(0, m_Vertices.m_Buffer->buffer, offsets);

		if (m_Indices.m_Count)
		{
			vCmdBuffer->bindIndexBuffer(m_Indices.m_Buffer->buffer, 0, vk::IndexType::eUint32);
			vCmdBuffer->drawIndexed(m_Indices.m_Count, 1, 0, 0, 0);
		}
		else
		{
			vCmdBuffer->draw(m_Vertices.m_Count, 1, 0, 0);
		}
	}
}

template<typename T_VertexType>
void MeshShaderPass<T_VertexType>::SetInputStateBeforePipelineCreation()
{
	T_VertexType::GetInputState(m_InputState);
}
