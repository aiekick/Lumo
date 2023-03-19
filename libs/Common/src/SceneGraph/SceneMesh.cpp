// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

/*
#include "SceneMesh.hpp"
using namespace vkApi;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC ////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T_VertexType>
std::shared_ptr<SceneMesh<T_VertexType>> SceneMesh<T_VertexType>::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<SceneMesh<T_VertexType>>(vVulkanCorePtr);
	res->m_This = res;
	return res;
}

template<typename T_VertexType>
std::shared_ptr<SceneMesh<T_VertexType>> SceneMesh<T_VertexType>::Create(vkApi::VulkanCorePtr vVulkanCorePtr, std::vector<T_VertexType> vVerticeArray, IndiceArray vIndiceArray)
{
	auto res = std::make_shared<SceneMesh>(vVulkanCorePtr, vVerticeArray, vIndiceArray);
	res->m_This = res;
	if (!res->Init())
		res.reset();
	return res;
}

template<typename T_VertexType>
std::shared_ptr<SceneMesh<T_VertexType>> SceneMesh<T_VertexType>::Copy(const ct::cWeak<SceneMesh<T_VertexType>>& vSceneMeshToCopy)
{
	auto fromPtr = vSceneMeshToCopy.getValidShared();
	if (fromPtr)
	{
		auto res = std::make_shared<SceneMesh>(*fromPtr);
		res->m_This = res;
		return res;
	}
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CTOR / DTOR ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T_VertexType>
SceneMesh<T_VertexType>::SceneMesh(vkApi::VulkanCorePtr vVulkanCorePtr) 
	: m_VulkanCorePtr(vVulkanCorePtr)
{

}

template<typename T_VertexType>
SceneMesh<T_VertexType>::SceneMesh(vkApi::VulkanCorePtr vVulkanCorePtr, std::vector<T_VertexType> vVerticeArray, IndiceArray vIndiceArray)
	: m_VulkanCorePtr(vVulkanCorePtr)
{
	m_Vertices.m_Array = vVerticeArray;
	m_Indices.m_Array = vIndiceArray;
}

template<typename T_VertexType>
SceneMesh<T_VertexType>::~SceneMesh()
{
	Unit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T_VertexType>
bool SceneMesh<T_VertexType>::Init()
{
	return Build(true);
}

template<typename T_VertexType>
void SceneMesh<T_VertexType>::Unit()
{
	Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// TESTS /////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T_VertexType>
bool SceneMesh<T_VertexType>::empty()
{
	return m_Vertices.m_Array.empty() || m_Indices.m_Array.empty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// GETTERS ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T_VertexType>
vk::Buffer SceneMesh<T_VertexType>::GetVerticesBuffer()
{
	return m_Vertices.m_Buffer->buffer;
}

template<typename T_VertexType>
vk::DescriptorBufferInfo* SceneMesh<T_VertexType>::GetVerticesBufferInfo()
{
	return &m_Vertices.m_BufferInfo;
}

template<typename T_VertexType>
uint64_t SceneMesh<T_VertexType>::GetVerticesDeviceAddress()
{
	return m_Vertices.m_Buffer->device_address;
}

template<typename T_VertexType>
std::vector<T_VertexType>* SceneMesh<T_VertexType>::GetVertices()
{
	return &m_Vertices.m_Array;
}

template<typename T_VertexType>
uint32_t SceneMesh<T_VertexType>::GetVerticesCount()
{
	return (uint32_t)m_Vertices.m_Array.size();
}

template<typename T_VertexType>
vk::Buffer SceneMesh<T_VertexType>::GetIndicesBuffer()
{
	return m_Indices.m_Buffer->buffer;
}

template<typename T_VertexType>
vk::DescriptorBufferInfo* SceneMesh<T_VertexType>::GetIndicesBufferInfo()
{
	return &m_Indices.m_BufferInfo;
}

template<typename T_VertexType>
uint64_t SceneMesh<T_VertexType>::GetIndiceDeviceAddress()
{
	return m_Indices.m_Buffer->device_address;
}

template<typename T_VertexType>
IndiceArray* SceneMesh<T_VertexType>::GetIndices()
{
	return &m_Indices.m_Array;
}

template<typename T_VertexType>
uint32_t SceneMesh<T_VertexType>::GetIndicesCount()
{
	return (uint32_t)m_Indices.m_Array.size();
}

template<typename T_VertexType>
bool SceneMesh<T_VertexType>::HasNormals()
{
	return m_HaveNormals;
}

template<typename T_VertexType>
void SceneMesh<T_VertexType>::HaveNormals()
{
	m_HaveNormals = true;
}

template<typename T_VertexType>
bool SceneMesh<T_VertexType>::HasTangeants()
{
	return m_HaveTangeants;
}

template<typename T_VertexType>
void SceneMesh<T_VertexType>::HaveTangeants()
{
	m_HaveTangeants = true;
}

template<typename T_VertexType>
bool SceneMesh<T_VertexType>::HasBiTangeants()
{
	return m_HaveBiTangeants;
}

template<typename T_VertexType>
void SceneMesh<T_VertexType>::HaveBiTangeants()
{
	m_HaveBiTangeants = true;
}

template<typename T_VertexType>
bool SceneMesh<T_VertexType>::HasTextureCoords()
{
	return m_HaveTextureCoords;
}

template<typename T_VertexType>
void SceneMesh<T_VertexType>::HaveTextureCoords()
{
	m_HaveTextureCoords = true;
}

template<typename T_VertexType>
bool SceneMesh<T_VertexType>::HasVertexColors()
{
	return m_HaveVertexColors;
}

template<typename T_VertexType>
void SceneMesh<T_VertexType>::HaveVertexColors()
{
	m_HaveVertexColors = true;
}

template<typename T_VertexType>
bool SceneMesh<T_VertexType>::HasIndices()
{
	return m_HaveIndices;
}

template<typename T_VertexType>
void SceneMesh<T_VertexType>::HaveIndices()
{
	m_HaveIndices = true;
}

template<typename T_VertexType>
void SceneMesh<T_VertexType>::SetPrimitiveType(const SceneModelPrimitiveType& vType)
{
	m_PrimitiveType = vType;
}

template<typename T_VertexType>
const SceneModelPrimitiveType& SceneMesh<T_VertexType>::GetPrimitiveType() const
{
	return m_PrimitiveType;
}

template<typename T_VertexType>
bool SceneMesh<T_VertexType>::Build(bool vUseSBO)
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

template<typename T_VertexType>
std::vector<T_VertexType> SceneMesh<T_VertexType>::GetVerticesFromGPU()
{
	std::vector<T_VertexType> res;

	CTOOL_DEBUG_BREAK;

	return res;
}

template<typename T_VertexType>
std::vector<VertexStruct::I1> SceneMesh<T_VertexType>::GetIndicesFromGPU()
{
	std::vector<VertexStruct::I1> res;

	CTOOL_DEBUG_BREAK;

	return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T_VertexType>
void SceneMesh<T_VertexType>::Destroy()
{
	DestroyVBO();
	DestroyIBO();
}

template<typename T_VertexType>
bool SceneMesh<T_VertexType>::BuildVBO(bool vUseSBO)
{
	DestroyVBO();

	m_Vertices.m_Buffer = VulkanRessource::createVertexBufferObject(m_VulkanCorePtr, m_Vertices.m_Array, vUseSBO, false, m_VulkanCorePtr->GetSupportedFeatures().is_RTX_Supported);
	m_Vertices.m_Count = (uint32_t)m_Vertices.m_Array.size();

	m_Vertices.m_BufferInfo.buffer = m_Vertices.m_Buffer->buffer;
	m_Vertices.m_BufferInfo.range = m_Vertices.m_Count * (uint32_t)sizeof(T_VertexType);
	m_Vertices.m_BufferInfo.offset = 0;

	m_SceneMeshBuffers.vertices_address = m_Vertices.m_Buffer->device_address;

	return true;
}

template<typename T_VertexType>
void SceneMesh<T_VertexType>::DestroyVBO()
{
	m_VulkanCorePtr->getDevice().waitIdle();

	m_Vertices.m_Buffer.reset();
	m_Vertices.m_BufferInfo = vk::DescriptorBufferInfo();
}

template<typename T_VertexType>
void SceneMesh<T_VertexType>::BuildIBO(bool vUseSBO)
{
	DestroyIBO();

	auto devicePtr = m_VulkanCorePtr->getFrameworkDevice().lock();
	if (devicePtr)
	{
		m_Indices.m_Buffer = VulkanRessource::createIndexBufferObject(m_VulkanCorePtr, m_Indices.m_Array, vUseSBO, false, devicePtr->GetRTXUse()); // the last true is for RTX
		m_Indices.m_Count = (uint32_t)m_Indices.m_Array.size();

		m_Indices.m_BufferInfo.buffer = m_Indices.m_Buffer->buffer;
		m_Indices.m_BufferInfo.range = m_Indices.m_Count * (uint32_t)sizeof(uint32_t);
		m_Indices.m_BufferInfo.offset = 0;

		m_SceneMeshBuffers.vertices_address = m_Indices.m_Buffer->device_address;
	}
}

template<typename T_VertexType>
void SceneMesh<T_VertexType>::DestroyIBO()
{
	m_VulkanCorePtr->getDevice().waitIdle();

	m_Indices.m_Buffer.reset();
	m_Indices.m_BufferInfo = vk::DescriptorBufferInfo();
}

*/