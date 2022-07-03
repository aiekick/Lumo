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

#include "VertexShaderPass.h"

VertexShaderPass::VertexShaderPass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr, GenericType::PIXEL)
{

}

VertexShaderPass::VertexShaderPass(vkApi::VulkanCorePtr vVulkanCorePtr,	
	vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool)
	: ShaderPass(vVulkanCorePtr, GenericType::PIXEL, vCommandPool, vDescriptorPool)
{

}

void VertexShaderPass::DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (!m_Loaded) return;

	if (vCmdBuffer)
	{
		vCmdBuffer->setLineWidth(m_LineWidth.w);
		//vCmdBuffer->setPrimitiveTopologyEXT(m_PrimitiveTopology);
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout, 0, m_DescriptorSet, nullptr);
		vCmdBuffer->draw(m_CountVertexs.w, m_CountInstances.w, 0, 0);
	}
}

void VertexShaderPass::SetInputStateBeforePipelineCreation()
{
	m_InputState.state = vk::PipelineVertexInputStateCreateInfo(
		vk::PipelineVertexInputStateCreateFlags(), // flags
		0, nullptr, // vertexBindingDescriptionCount
		0, nullptr // vertexAttributeDescriptions
	);
}
