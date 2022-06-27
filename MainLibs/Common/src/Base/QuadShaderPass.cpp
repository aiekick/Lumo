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

#include "QuadShaderPass.h"

#include <vkFramework/VulkanRessource.h>

QuadShaderPass::QuadShaderPass(
	vkApi::VulkanCore* vVulkanCore,
	const MeshShaderPassType& vMeshShaderPassType)
	: MeshShaderPass<VertexStruct::P2_T2>(
		vVulkanCore,
		vMeshShaderPassType)
{

}

QuadShaderPass::QuadShaderPass(
	vkApi::VulkanCore* vVulkanCore,
	const MeshShaderPassType& vMeshShaderPassType,
	vk::CommandPool* vCommandPool,
	vk::DescriptorPool* vDescriptorPool)
	: MeshShaderPass<VertexStruct::P2_T2>(
		vVulkanCore,
		vMeshShaderPassType,
		vCommandPool,
		vDescriptorPool)
{

}

bool QuadShaderPass::BuildModel()
{
	ZoneScoped;

	m_Vertices.m_Array = {
		VertexStruct::P2_T2(ct::fvec2(-1.0f, -1.0f), ct::fvec2(0.0f, 0.0f)),
		VertexStruct::P2_T2(ct::fvec2(1.0f, -1.0f), ct::fvec2(1.0f, 0.0f)),
		VertexStruct::P2_T2(ct::fvec2(1.0f, 1.0f), ct::fvec2(1.0f, 1.0f)),
		VertexStruct::P2_T2(ct::fvec2(-1.0f, 1.0f), ct::fvec2(0.0f, 1.0f)),
	};
	m_Indices.m_Array = {
		0U, 1U, 2U, 0U, 2U, 3U
	};

	m_Vertices.m_Buffer = vkApi::VulkanRessource::createVertexBufferObject(m_VulkanCore, m_Vertices.m_Array);
	m_Vertices.m_Count = (uint32_t)m_Vertices.m_Array.size();
	m_Vertices.m_BufferInfo.buffer = m_Vertices.m_Buffer->buffer;
	m_Vertices.m_BufferInfo.range = m_Vertices.m_Count * sizeof(VertexStruct::P3_N3_TA3_BTA3_T2_C4);
	m_Vertices.m_BufferInfo.offset = 0;

	m_Indices.m_Buffer = vkApi::VulkanRessource::createIndexBufferObject(m_VulkanCore, m_Indices.m_Array);
	m_Indices.m_Count = (uint32_t)m_Indices.m_Array.size();
	m_Indices.m_BufferInfo.buffer = m_Indices.m_Buffer->buffer;
	m_Indices.m_BufferInfo.range = m_Indices.m_Count * sizeof(uint32_t);
	m_Indices.m_BufferInfo.offset = 0;

	return true;
}

void QuadShaderPass::DestroyModel(const bool& vReleaseDatas)
{
	ZoneScoped;

	m_VulkanCore->getDevice().waitIdle();

	m_Vertices.m_Buffer.reset();
	m_Indices.m_BufferInfo = vk::DescriptorBufferInfo{};

	m_Indices.m_Buffer.reset();
	m_Indices.m_BufferInfo = vk::DescriptorBufferInfo{};
}

std::string QuadShaderPass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "QuadShaderPass_Vertex";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 vertPosition;
layout(location = 1) in vec2 vertUv;

layout(location = 0) out vec2 uv_map;

void main() 
{
	uv_map = vertUv;
	gl_Position = vec4(vertPosition, 0.0, 1.0);
}
)";
}

std::string QuadShaderPass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "QuadShaderPass_Fragment";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec2 uv_map;

void main() 
{
	fragColor = vec4(uv_map, 0.0, 1.0);
}
)";
}