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
	vk::DescriptorBufferInfo m_BufferInfo = {};
	std::vector<T> m_Array;
	uint32_t m_Count = 0U;

public:
	~MeshInfo()
	{
		m_Buffer.reset();
		m_BufferInfo = vk::DescriptorBufferInfo();
	}
};

class SceneMesh
{
public:
	static SceneMeshPtr Create(vkApi::VulkanCore* vVulkanCore);
	static SceneMeshPtr Create(vkApi::VulkanCore* vVulkanCore, VerticeArray vVerticeArray, IndiceArray vIndiceArray);
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

	vkApi::VulkanCore* m_VulkanCore = nullptr;

public:
	SceneMesh(vkApi::VulkanCore* vVulkanCore);
	SceneMesh(vkApi::VulkanCore* vVulkanCore, VerticeArray vVerticeArray, IndiceArray vIndiceArray);
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
