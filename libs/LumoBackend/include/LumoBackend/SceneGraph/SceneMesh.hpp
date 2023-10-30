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

#include <vector>
#include <glm/glm.hpp>
#include <ctools/cTools.h>
#include <Gaia/gaia.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <LumoBackend/Headers/LumoBackendDefs.h>

template<typename T_VertexType> class SceneMesh;
typedef std::shared_ptr<SceneMesh<VertexStruct::P3_N3_TA3_BTA3_T2_C4>> SceneMeshPtr;
typedef std::weak_ptr<SceneMesh<VertexStruct::P3_N3_TA3_BTA3_T2_C4>> SceneMeshWeak;
typedef std::vector<VertexStruct::P3_N3_TA3_BTA3_T2_C4> VerticeArray;
typedef std::vector<VertexStruct::I1> IndiceArray;

enum class SceneModelPrimitiveType
{
	SCENE_MODEL_PRIMITIVE_TYPE_POINTS = 0,
	SCENE_MODEL_PRIMITIVE_TYPE_CURVES,
	SCENE_MODEL_PRIMITIVE_TYPE_FACES
};

template<typename T_VertexType>
class MeshInfo
{
public:
	VulkanBufferObjectPtr m_Buffer = nullptr;
	vk::DescriptorBufferInfo m_BufferInfo = vk::DescriptorBufferInfo { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	std::vector<T_VertexType> m_Array;
	uint32_t m_Count = 0U;

public:
	~MeshInfo()
	{
		m_Buffer.reset();
		m_BufferInfo = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	}
};

class SceneMeshBuffers {
public:
    vk::DeviceAddress vertices_address = vk::DeviceAddress{};
    vk::DeviceAddress indices_address = vk::DeviceAddress{};
};

template<typename T_VertexType>
class SceneMesh
{
public:
	static std::shared_ptr<SceneMesh<T_VertexType>> Create(GaiApi::VulkanCorePtr vVulkanCorePtr);
	static std::shared_ptr<SceneMesh<T_VertexType>> Create(GaiApi::VulkanCorePtr vVulkanCorePtr, std::vector<T_VertexType> vVerticeArray, IndiceArray vIndiceArray);
	static std::shared_ptr<SceneMesh<T_VertexType>> Copy(const std::weak_ptr<SceneMesh<T_VertexType>>& vSceneMeshToCopy);

private:
	std::weak_ptr<SceneMesh<VertexStruct::P3_N3_TA3_BTA3_T2_C4>> m_This;

	SceneModelPrimitiveType m_PrimitiveType =
		SceneModelPrimitiveType::SCENE_MODEL_PRIMITIVE_TYPE_FACES;

	MeshInfo<T_VertexType> m_Vertices;
	MeshInfo<VertexStruct::I1> m_Indices;

	SceneMeshBuffers m_SceneMeshBuffers = {};

	bool m_HaveNormals = false;
	bool m_HaveTangeants = false;
	bool m_HaveBiTangeants = false;
	bool m_HaveTextureCoords = false;
	bool m_HaveVertexColors = false;
	bool m_HaveIndices = false;

	GaiApi::VulkanCorePtr m_VulkanCorePtr = nullptr;

public:
	SceneMesh(GaiApi::VulkanCorePtr vVulkanCorePtr);
	SceneMesh(GaiApi::VulkanCorePtr vVulkanCorePtr, std::vector<T_VertexType> vVerticeArray, IndiceArray vIndiceArray);
	~SceneMesh();

	bool Init();
	void Unit();

	bool empty();
	bool HasNormals();
	bool HasTangeants();
	bool HasBiTangeants();
	bool HasTextureCoords();
	bool HasVertexColors();
	bool HasIndices();

	void SetPrimitiveType(const SceneModelPrimitiveType& vType);
	const SceneModelPrimitiveType& GetPrimitiveType() const;

	vk::Buffer GetVerticesBuffer();
	vk::DescriptorBufferInfo* GetVerticesBufferInfo();
	uint64_t GetVerticesDeviceAddress();
	std::vector<T_VertexType>* GetVertices();
	uint32_t GetVerticesCount();

	vk::Buffer GetIndicesBuffer();
	vk::DescriptorBufferInfo* GetIndicesBufferInfo();
	uint64_t GetIndiceDeviceAddress();
	IndiceArray* GetIndices();
	uint32_t GetIndicesCount();

	void HaveNormals();
	void HaveTangeants();
	void HaveBiTangeants();
	void HaveTextureCoords();
	void HaveVertexColors();
	void HaveIndices();

	bool Build(bool vUseSBO = false);

	std::vector<T_VertexType> GetVerticesFromGPU();
	IndiceArray GetIndicesFromGPU();

private:
	void Destroy();
	bool BuildVBO(bool vUseSBO);
	void DestroyVBO();
	void BuildIBO(bool vUseSBO);
	void DestroyIBO();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC ////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T_VertexType>
std::shared_ptr<SceneMesh<T_VertexType>> SceneMesh<T_VertexType>::Create(GaiApi::VulkanCorePtr vVulkanCorePtr)
{
	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<SceneMesh<T_VertexType>>(vVulkanCorePtr);
	res->m_This = res;
	return res;
}

template<typename T_VertexType>
std::shared_ptr<SceneMesh<T_VertexType>> SceneMesh<T_VertexType>::Create(GaiApi::VulkanCorePtr vVulkanCorePtr, std::vector<T_VertexType> vVerticeArray, IndiceArray vIndiceArray)
{
	auto res = std::make_shared<SceneMesh>(vVulkanCorePtr, vVerticeArray, vIndiceArray);
	res->m_This = res;
	if (!res->Init())
		res.reset();
	return res;
}

template<typename T_VertexType>
std::shared_ptr<SceneMesh<T_VertexType>> SceneMesh<T_VertexType>::Copy(const std::weak_ptr<SceneMesh<T_VertexType>>& vSceneMeshToCopy)
{
	auto fromPtr = vSceneMeshToCopy.lock();
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
SceneMesh<T_VertexType>::SceneMesh(GaiApi::VulkanCorePtr vVulkanCorePtr)
	: m_VulkanCorePtr(vVulkanCorePtr)
{

}

template<typename T_VertexType>
SceneMesh<T_VertexType>::SceneMesh(GaiApi::VulkanCorePtr vVulkanCorePtr, std::vector<T_VertexType> vVerticeArray, IndiceArray vIndiceArray)
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
    vk::BufferCreateInfo stagingBufferInfo = {};
    VmaAllocationCreateInfo stagingAllocInfo = {};
    stagingBufferInfo.size = m_Vertices.m_BufferInfo.range;
    stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst;
    stagingAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_TO_CPU;
    auto stagebufferPtr = GaiApi::VulkanRessource::createSharedBufferObject(m_VulkanCorePtr, stagingBufferInfo, stagingAllocInfo);
    if (stagebufferPtr) {
        vk::BufferCopy region = {};
        region.size = stagingBufferInfo.size;
        GaiApi::VulkanRessource::copy(m_VulkanCorePtr, stagebufferPtr->buffer, m_Vertices.m_Buffer->buffer, region);
        std::vector<T_VertexType> res;
        res.resize(m_Vertices.m_Count);
        if (GaiApi::VulkanRessource::download(m_VulkanCorePtr, stagebufferPtr, (void*)res.data(), stagingBufferInfo.size)) {
            return res;
        }
    }
    return {};
}

template<typename T_VertexType>
std::vector<VertexStruct::I1> SceneMesh<T_VertexType>::GetIndicesFromGPU()
{
	vk::BufferCreateInfo stagingBufferInfo = {};
    VmaAllocationCreateInfo stagingAllocInfo = {};
    stagingBufferInfo.size = m_Indices.m_BufferInfo.range;
    stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst;
    stagingAllocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_TO_CPU;
    auto stagebufferPtr = GaiApi::VulkanRessource::createSharedBufferObject(m_VulkanCorePtr, stagingBufferInfo, stagingAllocInfo);
    if (stagebufferPtr) {
        vk::BufferCopy region = {};
        region.size = stagingBufferInfo.size;
        GaiApi::VulkanRessource::copy(m_VulkanCorePtr, stagebufferPtr->buffer, m_Indices.m_Buffer->buffer, region);
        std::vector<VertexStruct::I1> res;
        res.resize(m_Indices.m_Count);
        if (GaiApi::VulkanRessource::download(m_VulkanCorePtr, stagebufferPtr, (void*)res.data(), stagingBufferInfo.size)) {
            return res;
        }
    }
    return {};
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

	m_Vertices.m_Buffer = GaiApi::VulkanRessource::createVertexBufferObject(m_VulkanCorePtr, m_Vertices.m_Array, vUseSBO, false, m_VulkanCorePtr->GetSupportedFeatures().is_RTX_Supported);
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
		m_Indices.m_Buffer = GaiApi::VulkanRessource::createIndexBufferObject(m_VulkanCorePtr, m_Indices.m_Array, vUseSBO, false, devicePtr->GetRTXUse()); // the last true is for RTX
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