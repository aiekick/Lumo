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

#include "SceneMesh.h"
using namespace vkApi;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC ////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

SceneMeshPtr SceneMesh::Create(vkApi::VulkanCore* vVulkanCore)
{
	SceneMeshPtr res = std::make_shared<SceneMesh>(vVulkanCore);
	res->m_This = res;
	return res;
}
SceneMeshPtr SceneMesh::Create(vkApi::VulkanCore* vVulkanCore, VerticeArray vVerticeArray, IndiceArray vIndiceArray)
{
	SceneMeshPtr res = std::make_shared<SceneMesh>(vVulkanCore, vVerticeArray, vIndiceArray);
	res->m_This = res;
	if (!res->Init())
		res.reset();
	return res;
}

SceneMeshPtr SceneMesh::Copy(SceneMeshWeak vSceneMeshToCopy)
{
	auto fromPtr = vSceneMeshToCopy.getValidShared();
	if (fromPtr)
	{
		SceneMeshPtr res = std::make_shared<SceneMesh>(*fromPtr);
		res->m_This = res;
		return res;
	}
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CTOR / DTOR ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

SceneMesh::SceneMesh(vkApi::VulkanCore* vVulkanCore) 
	: m_VulkanCore(vVulkanCore)
{

}

SceneMesh::SceneMesh(vkApi::VulkanCore* vVulkanCore, VerticeArray vVerticeArray, IndiceArray vIndiceArray)
	: m_VulkanCore(vVulkanCore)
{
	m_Vertices.m_Array = vVerticeArray;
	m_Indices.m_Array = vIndiceArray;
}

SceneMesh::~SceneMesh()
{
	Unit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMesh::Init()
{
	return Build(true);
}

void SceneMesh::Unit()
{
	Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// TESTS /////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMesh::empty()
{
	return m_Vertices.m_Array.empty() || m_Indices.m_Array.empty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// GETTERS ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

vk::Buffer SceneMesh::GetVerticesBuffer()
{
	return m_Vertices.m_Buffer->buffer;
}

vk::DescriptorBufferInfo* SceneMesh::GetVerticesBufferInfo()
{
	return &m_Vertices.m_BufferInfo;
}

VerticeArray* SceneMesh::GetVertices()
{
	return &m_Vertices.m_Array;
}

uint32_t SceneMesh::GetVerticesCount()
{
	return (uint32_t)m_Vertices.m_Array.size();
}

vk::Buffer SceneMesh::GetIndicesBuffer()
{
	return m_Indices.m_Buffer->buffer;
}

vk::DescriptorBufferInfo* SceneMesh::GetIndicesBufferInfo()
{
	return &m_Indices.m_BufferInfo;
}

IndiceArray* SceneMesh::GetIndices()
{
	return &m_Indices.m_Array;
}

uint32_t SceneMesh::GetIndicesCount()
{
	return (uint32_t)m_Indices.m_Array.size();
}

bool SceneMesh::HasNormals()
{
	return m_HaveNormals;
}

void SceneMesh::HaveNormals()
{
	m_HaveNormals = true;
}

bool SceneMesh::HasTangeants()
{
	return m_HaveTangeants;
}

void SceneMesh::HaveTangeants()
{
	m_HaveTangeants = true;
}

bool SceneMesh::HasBiTangeants()
{
	return m_HaveBiTangeants;
}

void SceneMesh::HaveBiTangeants()
{
	m_HaveBiTangeants = true;
}

bool SceneMesh::HasTextureCoords()
{
	return m_HaveTextureCoords;
}

void SceneMesh::HaveTextureCoords()
{
	m_HaveTextureCoords = true;
}

bool SceneMesh::HasVertexColors()
{
	return m_HaveVertexColors;
}

void SceneMesh::HaveVertexColors()
{
	m_HaveVertexColors = true;
}

bool SceneMesh::HasIndices()
{
	return m_HaveIndices;
}

void SceneMesh::HaveIndices()
{
	m_HaveIndices = true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMesh::Build(bool vUseSBO)
{
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

void SceneMesh::Destroy()
{
	DestroyVBO();
	DestroyIBO();
}

bool SceneMesh::BuildVBO(bool vUseSBO)
{
	DestroyVBO();

	m_Vertices.m_Buffer = VulkanRessource::createVertexBufferObject(m_VulkanCore, m_Vertices.m_Array, vUseSBO);
	m_Vertices.m_Count = (uint32_t)m_Vertices.m_Array.size();

	m_Vertices.m_BufferInfo.buffer = m_Vertices.m_Buffer->buffer;
	m_Vertices.m_BufferInfo.range = m_Vertices.m_Count * sizeof(VertexStruct::P3_N3_TA3_BTA3_T2_C4);
	m_Vertices.m_BufferInfo.offset = 0;

	return true;
}

void SceneMesh::DestroyVBO()
{
	m_VulkanCore->getDevice().waitIdle();

	m_Vertices.m_Buffer.reset();
	m_Vertices.m_BufferInfo = vk::DescriptorBufferInfo();
}

void SceneMesh::BuildIBO(bool vUseSBO)
{
	DestroyIBO();

	m_Indices.m_Buffer = VulkanRessource::createIndexBufferObject(m_VulkanCore, m_Indices.m_Array, vUseSBO);
	m_Indices.m_Count = (uint32_t)m_Indices.m_Array.size();

	m_Indices.m_BufferInfo.buffer = m_Indices.m_Buffer->buffer;
	m_Indices.m_BufferInfo.range = m_Indices.m_Count * sizeof(uint32_t);
	m_Indices.m_BufferInfo.offset = 0;
}

void SceneMesh::DestroyIBO()
{
	m_VulkanCore->getDevice().waitIdle();

	m_Indices.m_Buffer.reset();
	m_Indices.m_BufferInfo = vk::DescriptorBufferInfo();
}