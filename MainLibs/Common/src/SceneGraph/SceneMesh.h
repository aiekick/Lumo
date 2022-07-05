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
#include <vector>
#include <glm/glm.hpp>
#include <ctools/cTools.h>
#include <vulkan/vulkan.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanRessource.h>
#include <Utils/Mesh/VertexStruct.h>

class SceneMesh;
typedef std::shared_ptr<SceneMesh> SceneMeshPtr;
typedef ct::cWeak<SceneMesh> SceneMeshWeak;
typedef std::vector<VertexStruct::P3_N3_TA3_BTA3_T2_C4> VerticeArray;
typedef std::vector<VertexStruct::I1> IndiceArray;

template<typename T>
class MeshInfo
{
public:
	VulkanBufferObjectPtr m_Buffer = nullptr;
	vk::DescriptorBufferInfo m_BufferInfo = vk::DescriptorBufferInfo { VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	std::vector<T> m_Array;
	uint32_t m_Count = 0U;

public:
	~MeshInfo()
	{
		m_Buffer.reset();
		m_BufferInfo = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	}
};

class SceneMesh
{
public:
	static SceneMeshPtr Create(vkApi::VulkanCorePtr vVulkanCorePtr);
	static SceneMeshPtr Create(vkApi::VulkanCorePtr vVulkanCorePtr, VerticeArray vVerticeArray, IndiceArray vIndiceArray);
	static SceneMeshPtr Copy(SceneMeshWeak vSceneMeshToCopy);

private:
	SceneMeshWeak m_This;

	MeshInfo<VertexStruct::P3_N3_TA3_BTA3_T2_C4> m_Vertices;
	MeshInfo<VertexStruct::I1> m_Indices;

	bool m_HaveNormals = false;
	bool m_HaveTangeants = false;
	bool m_HaveBiTangeants = false;
	bool m_HaveTextureCoords = false;
	bool m_HaveVertexColors = false;
	bool m_HaveIndices = false;

	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;

public:
	SceneMesh(vkApi::VulkanCorePtr vVulkanCorePtr);
	SceneMesh(vkApi::VulkanCorePtr vVulkanCorePtr, VerticeArray vVerticeArray, IndiceArray vIndiceArray);
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
	
	vk::Buffer GetVerticesBuffer();
	vk::DescriptorBufferInfo* GetVerticesBufferInfo();
	VerticeArray* GetVertices();
	uint32_t GetVerticesCount();

	vk::Buffer GetIndicesBuffer();
	vk::DescriptorBufferInfo* GetIndicesBufferInfo();
	IndiceArray* GetIndices();
	uint32_t GetIndicesCount();

	void HaveNormals();
	void HaveTangeants();
	void HaveBiTangeants();
	void HaveTextureCoords();
	void HaveVertexColors();
	void HaveIndices();

	bool Build(bool vUseSBO = false);

private:
	void Destroy();
	bool BuildVBO(bool vUseSBO);
	void DestroyVBO();
	void BuildIBO(bool vUseSBO);
	void DestroyIBO();
};
