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

#include "VertexShaderPass.h"

VertexShaderPass::VertexShaderPass(vkApi::VulkanCore* vVulkanCore)
	: ShaderPass(vVulkanCore, GenericType::PIXEL)
{

}

VertexShaderPass::VertexShaderPass(vkApi::VulkanCore* vVulkanCore,	
	vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool)
	: ShaderPass(vVulkanCore, GenericType::PIXEL, vCommandPool, vDescriptorPool)
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
