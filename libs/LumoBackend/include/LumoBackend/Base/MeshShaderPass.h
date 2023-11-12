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

#include <LumoBackend/Base/ShaderPass.h>
#include <LumoBackend/SceneGraph/SceneMesh.hpp>
#include <LumoBackend/Headers/LumoBackendDefs.h>

enum class MeshShaderPassType : uint8_t
{
	NONE = 0,
	PIXEL,			// vertex + fragment (shader + m_Pipelines[0] + ubo + fbo + renderpass + command buffer)
	PIXEL_MERGER,	// pas de shader ni m_Pipelines[0] ni ubo, mais command buffer, fbo et renderpass
	Count
};

template<typename T_VertexType>
class MeshShaderPass : public ShaderPass
{
protected:
	MeshInfo<T_VertexType> m_Vertices;
	MeshInfo<VertexStruct::I1> m_Indices;

public:
	MeshShaderPass(GaiApi::VulkanCorePtr vVulkanCorePtr, const MeshShaderPassType& vMeshShaderPassType);
	MeshShaderPass(GaiApi::VulkanCorePtr vVulkanCorePtr, const MeshShaderPassType& vMeshShaderPassType,
		vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool);

	void DrawModel(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) override;
	void SetInputStateBeforePipelineCreation() override;

	bool Build(bool vUseSBO = false);

protected:
	bool BuildVBO(bool vUseSBO);
	void DestroyVBO();
	void BuildIBO(bool vUseSBO);
	void DestroyIBO();
};

////////////////////////////////////////////////////////////////////
///// IMPLEMTNATION ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

template<typename T_VertexType>
MeshShaderPass<T_VertexType>::MeshShaderPass(
	GaiApi::VulkanCorePtr vVulkanCorePtr,
	const MeshShaderPassType& vMeshShaderPassType)
	: ShaderPass(
		vVulkanCorePtr,
		(GenericType)vMeshShaderPassType) {
    ZoneScoped;
	
}

template<typename T_VertexType>
MeshShaderPass<T_VertexType>::MeshShaderPass(
	GaiApi::VulkanCorePtr vVulkanCorePtr,
	const MeshShaderPassType& vMeshShaderPassType,
	vk::CommandPool* vCommandPool,
	vk::DescriptorPool* vDescriptorPool)
	: ShaderPass(
		vVulkanCorePtr,
		(GenericType)vMeshShaderPassType,
		vCommandPool,
		vDescriptorPool) {
    ZoneScoped;

}

template<typename T_VertexType>
void MeshShaderPass<T_VertexType>::DrawModel(vk::CommandBuffer* vCmdBufferPtr, const int& /*vIterationNumber*/) {
    ZoneScoped;
	if (!m_Loaded) return;

	if (vCmdBufferPtr && m_Vertices.m_Count)
	{
		vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipelines[0].m_Pipeline);

		vCmdBufferPtr->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);

		vk::DeviceSize offsets = 0;
		vCmdBufferPtr->bindVertexBuffers(0, m_Vertices.m_Buffer->buffer, offsets);

		if (m_Indices.m_Count)
		{
			vCmdBufferPtr->bindIndexBuffer(m_Indices.m_Buffer->buffer, 0, vk::IndexType::eUint32);
			vCmdBufferPtr->drawIndexed(m_Indices.m_Count, m_CountInstances.w, 0, 0, 0);
		}
		else
		{
			vCmdBufferPtr->draw(m_Vertices.m_Count, m_CountInstances.w, 0, 0);
		}
	}
}

template<typename T_VertexType>
void MeshShaderPass<T_VertexType>::SetInputStateBeforePipelineCreation() {
    ZoneScoped;
	T_VertexType::GetInputState(m_InputState);
}

template<typename T_VertexType>
bool MeshShaderPass<T_VertexType>::Build(bool vUseSBO) {
    ZoneScoped;
	if (!BuildVBO(vUseSBO))
	{
		return false;
	}
	else
	{
		BuildIBO(vUseSBO);
	}

	return true;
}

////////////////////////////////////////////////////////////////////
///// PROTECTED ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

template<typename T_VertexType>
bool MeshShaderPass<T_VertexType>::BuildVBO(bool vUseSBO) {
    ZoneScoped;
	DestroyVBO();

	m_Vertices.m_Buffer = GaiApi::VulkanRessource::createVertexBufferObject(m_VulkanCorePtr, m_Vertices.m_Array, vUseSBO, false, m_VulkanCorePtr->GetSupportedFeatures().is_RTX_Supported);
	m_Vertices.m_Count = (uint32_t)m_Vertices.m_Array.size();

	m_Vertices.m_BufferInfo.buffer = m_Vertices.m_Buffer->buffer;
	m_Vertices.m_BufferInfo.range = m_Vertices.m_Count * (uint32_t)sizeof(VertexStruct::P3_N3_TA3_BTA3_T2_C4);
	m_Vertices.m_BufferInfo.offset = 0;

	return true;
}

template<typename T_VertexType>
void MeshShaderPass<T_VertexType>::DestroyVBO() {
    ZoneScoped;
	m_VulkanCorePtr->getDevice().waitIdle();

	m_Vertices.m_Buffer.reset();
	m_Vertices.m_BufferInfo = vk::DescriptorBufferInfo();
}

template<typename T_VertexType>
void MeshShaderPass<T_VertexType>::BuildIBO(bool vUseSBO) {
    ZoneScoped;
	DestroyIBO();

	auto devicePtr = m_VulkanCorePtr->getFrameworkDevice().lock();
	if (devicePtr)
	{
        m_Indices.m_Buffer = GaiApi::VulkanRessource::createIndexBufferObject(m_VulkanCorePtr, m_Indices.m_Array, vUseSBO, false, devicePtr->GetRTXUse());  // the last true is for RTX
		m_Indices.m_Count = (uint32_t)m_Indices.m_Array.size();

		m_Indices.m_BufferInfo.buffer = m_Indices.m_Buffer->buffer;
		m_Indices.m_BufferInfo.range = m_Indices.m_Count * (uint32_t)sizeof(uint32_t);
		m_Indices.m_BufferInfo.offset = 0;
	}
}

template<typename T_VertexType>
void MeshShaderPass<T_VertexType>::DestroyIBO() {
    ZoneScoped;
	m_VulkanCorePtr->getDevice().waitIdle();

	m_Indices.m_Buffer.reset();
	m_Indices.m_BufferInfo = vk::DescriptorBufferInfo();
}
