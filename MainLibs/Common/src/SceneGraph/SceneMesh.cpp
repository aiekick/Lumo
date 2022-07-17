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

#include "SceneMesh.h"
using namespace vkApi;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC ////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

SceneMeshPtr SceneMesh::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	if (!vVulkanCorePtr) return nullptr;
	SceneMeshPtr res = std::make_shared<SceneMesh>(vVulkanCorePtr);
	res->m_This = res;
	return res;
}
SceneMeshPtr SceneMesh::Create(vkApi::VulkanCorePtr vVulkanCorePtr, VerticeArray vVerticeArray, IndiceArray vIndiceArray)
{
	SceneMeshPtr res = std::make_shared<SceneMesh>(vVulkanCorePtr, vVerticeArray, vIndiceArray);
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

SceneMesh::SceneMesh(vkApi::VulkanCorePtr vVulkanCorePtr) 
	: m_VulkanCorePtr(vVulkanCorePtr)
{

}

SceneMesh::SceneMesh(vkApi::VulkanCorePtr vVulkanCorePtr, VerticeArray vVerticeArray, IndiceArray vIndiceArray)
	: m_VulkanCorePtr(vVulkanCorePtr)
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

uint64_t SceneMesh::GetVerticesDeviceAddress()
{
	return m_Vertices.m_Buffer->device_address;
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

uint64_t SceneMesh::GetIndiceDeviceAddress()
{
	return m_Indices.m_Buffer->device_address;
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

	m_Vertices.m_Buffer = VulkanRessource::createVertexBufferObject(m_VulkanCorePtr, m_Vertices.m_Array, vUseSBO, false, true); // the last true is for RTX
	m_Vertices.m_Count = (uint32_t)m_Vertices.m_Array.size();

	m_Vertices.m_BufferInfo.buffer = m_Vertices.m_Buffer->buffer;
	m_Vertices.m_BufferInfo.range = m_Vertices.m_Count * sizeof(VertexStruct::P3_N3_TA3_BTA3_T2_C4);
	m_Vertices.m_BufferInfo.offset = 0;

	m_SceneMeshBuffers.vertices_address = m_Vertices.m_Buffer->device_address;

	return true;
}

void SceneMesh::DestroyVBO()
{
	m_VulkanCorePtr->getDevice().waitIdle();

	m_Vertices.m_Buffer.reset();
	m_Vertices.m_BufferInfo = vk::DescriptorBufferInfo();
}

void SceneMesh::BuildIBO(bool vUseSBO)
{
	DestroyIBO();

	m_Indices.m_Buffer = VulkanRessource::createIndexBufferObject(m_VulkanCorePtr, m_Indices.m_Array, vUseSBO, false, true); // the last true is for RTX
	m_Indices.m_Count = (uint32_t)m_Indices.m_Array.size();

	m_Indices.m_BufferInfo.buffer = m_Indices.m_Buffer->buffer;
	m_Indices.m_BufferInfo.range = m_Indices.m_Count * sizeof(uint32_t);
	m_Indices.m_BufferInfo.offset = 0;

	m_SceneMeshBuffers.vertices_address = m_Indices.m_Buffer->device_address;
}

void SceneMesh::DestroyIBO()
{
	m_VulkanCorePtr->getDevice().waitIdle();

	m_Indices.m_Buffer.reset();
	m_Indices.m_BufferInfo = vk::DescriptorBufferInfo();
}