/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <Generic/ShaderPass.h>
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
	MeshShaderPass(vkApi::VulkanCore* vVulkanCore, const MeshShaderPassType& vMeshShaderPassType);
	MeshShaderPass(vkApi::VulkanCore* vVulkanCore, const MeshShaderPassType& vMeshShaderPassType,
		vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool);

	void DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber) override;
	void SetInputStateBeforePipelineCreation() override;
};

////////////////////////////////////////////////////////////////////
///// IMPLEMTNATION ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

template<typename T_VertexType>
MeshShaderPass<T_VertexType>::MeshShaderPass(
	vkApi::VulkanCore* vVulkanCore,
	const MeshShaderPassType& vMeshShaderPassType)
	: ShaderPass(
		vVulkanCore,
		(GenericType)vMeshShaderPassType)
{
	
}

template<typename T_VertexType>
MeshShaderPass<T_VertexType>::MeshShaderPass(
	vkApi::VulkanCore* vVulkanCore,
	const MeshShaderPassType& vMeshShaderPassType,
	vk::CommandPool* vCommandPool,
	vk::DescriptorPool* vDescriptorPool)
	: ShaderPass(
		vVulkanCore,
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
