/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/


#pragma once

#include <set>
#include <array>
#include <string>
#include <memory>

#include <Headers/Globals.h>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <Base/BaseRenderer.h>
#include <Base/QuadShaderPass.h>

#include <vulkan/vulkan.hpp>
#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanDevice.h>
#include <vkFramework/vk_mem_alloc.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/ImGuiTexture.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/VulkanFrameBuffer.h>

#include <Interfaces/GuiInterface.h>
#include <Interfaces/NodeInterface.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/NodeInterface.h>
#include <Interfaces/ResizerInterface.h>

#include <Interfaces/ModelOutputInterface.h>

class PrimitiveModule :
	public NodeInterface,
	public conf::ConfigAbstract,
	public ModelOutputInterface,
	public GuiInterface
{
private:
	typedef std::map<std::tuple<size_t, size_t>, size_t> CacheDB;
	typedef VertexStruct::P3_N3_TA3_BTA3_T2_C4 Vertice;
	typedef VertexStruct::I1 Indice;
	enum PrimitiveTypeEnum : int32_t
	{
		PRIMITIVE_TYPE_PLANE = 0,
		PRIMITIVE_TYPE_CUBE,
		PRIMITIVE_TYPE_ICO_SPHERE,
		PRIMITIVE_TYPE_UV_SPHERE,
		PRIMITIVE_TYPE_TORUS,
		PRIMITIVE_TYPE_Count
	};
	struct Face
	{
		size_t v1 = 0U, v2 = 0U, v3 = 0U;

		Face(const size_t& vV1, const size_t& vV2, const size_t& vV3)
		{
			v1 = vV1;
			v2 = vV2;
			v3 = vV3;
		}
	};

public:
	static std::shared_ptr<PrimitiveModule> Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode);

private:
	ct::cWeak<PrimitiveModule> m_This;
	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;
	SceneModelPtr m_SceneModelPtr = nullptr;
	std::vector<std::string> m_PrimitiveTypes;
	int32_t m_PrimitiveTypeIndex = PrimitiveTypeEnum::PRIMITIVE_TYPE_PLANE;

private: // plane
	bool m_PlaneCentered = true;
	uint32_t m_PlaneSubdivisionLevel = 0U;
	ct::fvec2 m_PlaneSize = 1.0f;

private: // cube
	bool m_CubeCentered = true;
	uint32_t m_CubeSubdivisionLevel = 0U;
	ct::fvec3 m_CubeSize = 1.0f;

private: // icosaedre
	float m_IcosaedreRadius = 1.0f;
	uint32_t m_IcosaedreSubdivisionLevel = 0U;

private: // framework
	CacheDB m_CacheDB;
	std::vector<Vertice> m_Vertices;
	std::vector<Face> m_Faces;
	ct::fvec3 m_Origin;
	ct::fvec3 m_Target = ct::fvec3(0.0f, 1.0f, 0.0f);

public:
	PrimitiveModule(vkApi::VulkanCorePtr vVulkanCorePtr);
	~PrimitiveModule();

	bool Init();
	void Unit();

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;

	// Interfaces Getters
	SceneModelWeak GetModel() override;

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
	void AfterNodeXmlLoading() override;

private:
	void prDrawWidgets();
	void prUpdateMesh();

private: // framework
	void CreatePlane();
	void CreateCube();
	void CreateIcosaedre();

	size_t GetMiddlePoint_Icosaedre(const size_t& p1, const size_t& p2, VerticeArray& vVertices, CacheDB& vCache, const float& vRadius);
	size_t GetMiddlePoint_Plane(const size_t& p1, const size_t& p2, VerticeArray& vVertices, CacheDB& vCache, const ct::fvec3& vNormal);

	void CreateQuad(
		const ct::fvec2& vSize, 
		const ct::fvec3& vOrigin,
		const ct::fvec3& vNormal,
		std::vector<Vertice>& vVertices, 
		std::vector<Face>& vFaces);
	void AddVertex(const ct::fvec3& vP, const ct::fvec3& vN, const ct::fvec2& vUV, std::vector<Vertice>& vVertices);
	void AddIcosaedreVertex(const ct::fvec3& vNormal, const ct::fvec3& vOrigin, const float& vRadius, std::vector<Vertice>& vVertices);
	void AddFace(const size_t& vV0, const size_t& vV1, const size_t& vV2, std::vector<Face>& vFaces);
	void CalcNormal(Vertice& v0, Vertice& v1, Vertice& v2);
	void Subdivide(const size_t& vSubdivLevel, std::vector<Vertice>& vVertices, std::vector<Face>& vFaces);
	void BuildMesh();
};
