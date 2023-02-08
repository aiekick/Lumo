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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "PrimitiveModule.h"

#include <cinttypes>
#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <Graph/Base/BaseNode.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanSubmitter.h>
#include <utils/Mesh/VertexStruct.h>
#include <Base/FrameBuffer.h>
#include <SceneGraph/SceneModel.h>
#include <SceneGraph/SceneMesh.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<PrimitiveModule> PrimitiveModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<PrimitiveModule>(vVulkanCorePtr);
	res->SetParentNode(vParentNode);
	res->m_This = res;
	if (!res->Init())
	{
		res.reset();
	}

	return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

PrimitiveModule::PrimitiveModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: m_VulkanCorePtr(vVulkanCorePtr)
{
	ZoneScoped;
}

PrimitiveModule::~PrimitiveModule()
{
	ZoneScoped;

	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool PrimitiveModule::Init()
{
	ZoneScoped;

	m_SceneModelPtr = SceneModel::Create();

	m_PrimitiveTypes.resize(PrimitiveTypeEnum::PRIMITIVE_TYPE_Count);
	m_PrimitiveTypes[PrimitiveTypeEnum::PRIMITIVE_TYPE_PLANE] = "PLANE";
	m_PrimitiveTypes[PrimitiveTypeEnum::PRIMITIVE_TYPE_CUBE] = "CUBE";
	m_PrimitiveTypes[PrimitiveTypeEnum::PRIMITIVE_TYPE_ICO_SPHERE] = "ICO SPHERE";
	m_PrimitiveTypes[PrimitiveTypeEnum::PRIMITIVE_TYPE_UV_SPHERE] = "UV SPHERE";
	m_PrimitiveTypes[PrimitiveTypeEnum::PRIMITIVE_TYPE_TORUS] = "TORUS";

	return true;
}

void PrimitiveModule::Unit()
{
	ZoneScoped;

	m_SceneModelPtr.reset();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool PrimitiveModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (ImGui::ContrastedComboVectorDefault(0.0f, "Primitive Types", &m_PrimitiveTypeIndex, m_PrimitiveTypes, 0))
	{
		prUpdateMesh();
	}

	prDrawWidgets();

	return false;
}

void PrimitiveModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
}

void PrimitiveModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL OUTPUT ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneModelWeak PrimitiveModule::GetModel()
{	
	ZoneScoped;

	return m_SceneModelPtr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string PrimitiveModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += vOffset + "<primitive_module>\n";
	str += vOffset + "\t<primitive_type>" + ct::toStr(m_PrimitiveTypeIndex) + "</primitive_type>\n";
	str += vOffset + "\t<plane_centered>" + (m_PlaneCentered ? "true" : "false") + "</plane_centered>\n";
	str += vOffset + "\t<plane_subdivision_level>" + ct::toStr(m_PlaneSubdivisionLevel) + "</plane_subdivision_level>\n";
	str += vOffset + "\t<plane_size>" + m_PlaneSize.string() + "</plane_size>\n";
	str += vOffset + "\t<cube_centered>" + (m_CubeCentered ? "true" : "false") + "</cube_centered>\n";
	str += vOffset + "\t<cube_subdivision_level>" + ct::toStr(m_CubeSubdivisionLevel) + "</cube_subdivision_level>\n";
	str += vOffset + "\t<cube_size>" + m_CubeSize.string() + "</cube_size>\n";
	str += vOffset + "\t<icosaedre_radius>" + ct::toStr(m_IcosaedreRadius) + "</icosaedre_radius>\n";
	str += vOffset + "\t<icosaedre_subdivision_level>" + ct::toStr(m_IcosaedreSubdivisionLevel) + "</icosaedre_subdivision_level>\n";
	str += vOffset + "</primitive_module>\n";

	return str;
}

bool PrimitiveModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	ZoneScoped;

	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strParentName == "primitive_module")
	{
		if (strName == "primitive_type")
			m_PrimitiveTypeIndex = ct::ivariant(strValue).GetI();
		else if (strName == "plane_centered")
			m_PlaneCentered = ct::ivariant(strValue).GetB();
		else if (strName == "plane_subdivision_level")
			m_PlaneSubdivisionLevel = ct::uvariant(strValue).GetU();
		else if (strName == "plane_size")
			m_PlaneSize = ct::fvariant(strValue).GetV2();
		else if (strName == "cube_centered")
			m_CubeCentered = ct::ivariant(strValue).GetB();
		else if (strName == "cube_subdivision_level")
			m_CubeSubdivisionLevel = ct::uvariant(strValue).GetU();
		else if (strName == "cube_size")
			m_CubeSize = ct::fvariant(strValue).GetV3();
		else if (strName == "icosaedre_radius")
			m_IcosaedreRadius = ct::fvariant(strValue).GetF();
		else if (strName == "icosaedre_subdivision_level")
			m_IcosaedreSubdivisionLevel = ct::uvariant(strValue).GetU();
	}

	return true;
}

void PrimitiveModule::AfterNodeXmlLoading()
{
	ZoneScoped;

	prUpdateMesh();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrimitiveModule::prDrawWidgets()
{
	if (ImGui::CollapsingHeader("Primitives"))
	{
		bool change = false;

		switch (m_PrimitiveTypeIndex)
		{
		case PRIMITIVE_TYPE_PLANE:
		{
			change |= ImGui::CheckBoxBoolDefault("Centered ?", &m_PlaneCentered, true);
			change |= ImGui::SliderUIntDefaultCompact(0.0f, "Subdivision Level", &m_PlaneSubdivisionLevel, 0U, 5U, 0U);
			change |= ImGui::InputFloatDefault(0.0f, "Size X", &m_PlaneSize.x, 1.0f);
			change |= ImGui::InputFloatDefault(0.0f, "Size Y", &m_PlaneSize.y, 1.0f);
		}
		break;
		case PRIMITIVE_TYPE_CUBE:
		{
			/*change |= ImGui::CheckBoxBoolDefault("Centered ?", &m_PlaneCentered, true);
			change |= ImGui::SliderUIntDefaultCompact(0.0f, "Subdivision Level", &m_PlaneSubdivisionLevel, 0U, 5U, 0U);
			change |= ImGui::InputFloatDefault(0.0f, "Size X", &m_CubeSize.x, 1.0f);
			change |= ImGui::InputFloatDefault(0.0f, "Size Y", &m_CubeSize.y, 1.0f);
			change |= ImGui::InputFloatDefault(0.0f, "Size Z", &m_CubeSize.z, 1.0f);*/
		}
		break;
		case PRIMITIVE_TYPE_ICO_SPHERE:
		{
			change |= ImGui::InputFloatDefault(0.0f, "Radius", &m_IcosaedreRadius, 1.0f);
			change |= ImGui::SliderUIntDefaultCompact(0.0f, "Subdivision Level", &m_IcosaedreSubdivisionLevel, 0U, 5U, 0U);
		}
		break;
		case PRIMITIVE_TYPE_UV_SPHERE:
		{

		}
		break;
		case PRIMITIVE_TYPE_TORUS:
		{

		}
		break;
		case PRIMITIVE_TYPE_Count:
			break;
		default:
			break;
		}

		if (change)
		{
			prUpdateMesh();
		}
	}
}

void PrimitiveModule::prUpdateMesh()
{
	switch (m_PrimitiveTypeIndex)
	{
		case PRIMITIVE_TYPE_PLANE:
		{
			CreatePlane();
		}
		break;
		case PRIMITIVE_TYPE_CUBE:
		{
			//CreateCube();
		}
		break;
		case PRIMITIVE_TYPE_ICO_SPHERE:
		{
			CreateIcosaedre();
		}
		break;
		case PRIMITIVE_TYPE_UV_SPHERE:
		{
			CTOOL_DEBUG_BREAK;
		}
		break;
		case PRIMITIVE_TYPE_TORUS:
		{
			CTOOL_DEBUG_BREAK;
		}
		break;
		case PRIMITIVE_TYPE_Count:
		{
			CTOOL_DEBUG_BREAK;
		}
		break;
		default:
		{
			CTOOL_DEBUG_BREAK;
		}
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE : FRAMEWORK ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrimitiveModule::CreatePlane()
{
	m_Vertices.clear();
	m_Faces.clear();

	CreateQuad(m_PlaneSize, m_Origin, m_Target, m_Vertices, m_Faces);

	BuildMesh();
}

void PrimitiveModule::CreateCube()
{
	m_Vertices.clear();
	m_Faces.clear();

	BuildMesh();
}

void PrimitiveModule::CreateIcosaedre()
{
	m_Vertices.clear();
	m_Faces.clear();

	// create 12 vertices of a icosahedron
	float t = (1.0f + sqrtf(5.0f)) / 2.0f;

	AddIcosaedreVertex(ct::fvec3(-1.0f, t, 0.0f).GetNormalized(), m_Origin, m_IcosaedreRadius, m_Vertices);
	AddIcosaedreVertex(ct::fvec3(1.0f, t, 0.0f).GetNormalized(), m_Origin, m_IcosaedreRadius, m_Vertices);
	AddIcosaedreVertex(ct::fvec3(-1.0f, -t, 0.0f).GetNormalized(), m_Origin, m_IcosaedreRadius, m_Vertices);
	AddIcosaedreVertex(ct::fvec3(1.0f, -t, 0.0f).GetNormalized(), m_Origin, m_IcosaedreRadius, m_Vertices);

	AddIcosaedreVertex(ct::fvec3(0.0f, -1.0f, t).GetNormalized(), m_Origin, m_IcosaedreRadius, m_Vertices);
	AddIcosaedreVertex(ct::fvec3(0.0f, 1.0f, t).GetNormalized(), m_Origin, m_IcosaedreRadius, m_Vertices);
	AddIcosaedreVertex(ct::fvec3(0.0f, -1.0f, -t).GetNormalized(), m_Origin, m_IcosaedreRadius, m_Vertices);
	AddIcosaedreVertex(ct::fvec3(0.0f, 1.0f, -t).GetNormalized(), m_Origin, m_IcosaedreRadius, m_Vertices);

	AddIcosaedreVertex(ct::fvec3(t, 0.0f, -1.0f).GetNormalized(), m_Origin, m_IcosaedreRadius, m_Vertices);
	AddIcosaedreVertex(ct::fvec3(t, 0.0f, 1.0f).GetNormalized(), m_Origin, m_IcosaedreRadius, m_Vertices);
	AddIcosaedreVertex(ct::fvec3(-t, 0.0f, -1.0f).GetNormalized(), m_Origin, m_IcosaedreRadius, m_Vertices);
	AddIcosaedreVertex(ct::fvec3(-t, 0.0f, 1.0f).GetNormalized(), m_Origin, m_IcosaedreRadius, m_Vertices);

	// create 20 triangles of the icosahedron

	// 5 faces around point 0
	AddFace(0, 11, 5, m_Faces);
	AddFace(0, 5, 1, m_Faces);
	AddFace(0, 1, 7, m_Faces);
	AddFace(0, 7, 10, m_Faces);
	AddFace(0, 10, 11, m_Faces);

	// 5 adjacent faces
	AddFace(1, 5, 9, m_Faces);
	AddFace(5, 11, 4, m_Faces);
	AddFace(11, 10, 2, m_Faces);
	AddFace(10, 7, 6, m_Faces);
	AddFace(7, 1, 8, m_Faces);

	// 5 faces around point 3
	AddFace(3, 9, 4, m_Faces);
	AddFace(3, 4, 2, m_Faces);
	AddFace(3, 2, 6, m_Faces);
	AddFace(3, 6, 8, m_Faces);
	AddFace(3, 8, 9, m_Faces);

	// 5 adjacent faces
	AddFace(4, 9, 5, m_Faces);
	AddFace(2, 4, 11, m_Faces);
	AddFace(6, 2, 10, m_Faces);
	AddFace(8, 6, 7, m_Faces);
	AddFace(9, 8, 1, m_Faces);

	// subdivide if needed (vSubdivLevel > 0))
	Subdivide(m_IcosaedreSubdivisionLevel, m_Vertices, m_Faces);

	BuildMesh();
}

size_t PrimitiveModule::GetMiddlePoint_Icosaedre(const size_t& p1, const size_t& p2, VerticeArray& vVertices, CacheDB& vCache, const float& vRadius)
{
	if (p1 == p2)
	{
		CTOOL_DEBUG_BREAK;
	}

	std::tuple<size_t, size_t> _block(ct::mini(p1, p2), ct::maxi(p1, p2));

	if (vCache.find(_block) != vCache.end())
	{
		return vCache.at(_block);
	}

	// not in cache, calculate it
	const ct::fvec3 point1 = vVertices.at(p1).p;
	const ct::fvec3 point2 = vVertices.at(p2).p;
	const ct::fvec3 middle = (point1 + point2) * 0.5f;

	// add vertex makes sure point is on unit sphere
	size_t i = vVertices.size();
	vVertices.emplace_back(middle.GetNormalized() * vRadius, middle.GetNormalized());

	// store it, return index
	vCache[_block] = i;

	return i;
}

size_t PrimitiveModule::GetMiddlePoint_Plane(const size_t& p1, const size_t& p2, VerticeArray& vVertices, CacheDB& vCache, const ct::fvec3& vNormal)
{
	if (p1 == p2)
	{
		CTOOL_DEBUG_BREAK;
	}

	std::tuple<size_t, size_t> _block(ct::mini(p1, p2), ct::maxi(p1, p2));

	if (vCache.find(_block) != vCache.end())
	{
		return vCache.at(_block);
	}

	// not in cache, calculate it
	const ct::fvec3 point1 = vVertices.at(p1).p;
	const ct::fvec3 point2 = vVertices.at(p2).p;
	const ct::fvec3 middle = (point1 + point2) * 0.5f;

	// add vertex makes sure point is on unit sphere
	size_t i = vVertices.size();
	vVertices.emplace_back(middle, vNormal);

	// store it, return index
	vCache[_block] = i;

	return i;
}

void PrimitiveModule::CreateQuad(
	const ct::fvec2& vSize, 
	const ct::fvec3& vOrigin, 
	const ct::fvec3& vNormal, 
	std::vector<Vertice>& vVertices, 
	std::vector<Face>& vFaces)
{
	ct::fvec3 size = ct::fvec3(vSize.x, 0.0f, vSize.y);

	AddVertex(vOrigin + ct::fvec3(-0.5f, 0.0f, -0.5f) * size, vNormal, ct::fvec2(0.0f, 0.0f), vVertices);
	AddVertex(vOrigin + ct::fvec3(0.5f, 0.0f, -0.5f) * size, vNormal, ct::fvec2(1.0f, 0.0f), vVertices);
	AddVertex(vOrigin + ct::fvec3(0.5f, 0.0f, 0.5f) * size, vNormal, ct::fvec2(1.0f, 1.0f), vVertices);
	AddVertex(vOrigin + ct::fvec3(-0.5f, 0.0f, 0.5f) * size, vNormal, ct::fvec2(0.0f, 1.0f), vVertices);

	AddFace(0, 1, 2, vFaces);
	AddFace(0, 2, 3, vFaces);

	Subdivide(m_PlaneSubdivisionLevel, vVertices, vFaces);
}

void PrimitiveModule::AddVertex(const ct::fvec3& vP, const ct::fvec3& vN, const ct::fvec2& vUV, std::vector<Vertice>& vVertices)
{
	vVertices.emplace_back(vP, vN, 0.0f, 0.0f, vUV);
}

void PrimitiveModule::AddIcosaedreVertex(const ct::fvec3& vNormal, const ct::fvec3& vOrigin, const float& vRadius, std::vector<Vertice>& vVertices)
{
	vVertices.emplace_back(vOrigin + vNormal * vRadius, vNormal);
}

void PrimitiveModule::AddFace(const size_t& vV0, const size_t& vV1, const size_t& vV2, std::vector<Face>& vFaces)
{
	vFaces.emplace_back(vV0, vV1, vV2);
}

void PrimitiveModule::CalcNormal(Vertice& v0, Vertice& v1, Vertice& v2)
{
	const ct::fvec3 vec0 = (v2.p - v0.p).GetNormalized();
	const ct::fvec3 vec1 = (v1.p - v0.p).GetNormalized();
	const ct::fvec3 nor = cCross(vec1, vec0).GetNormalized();

	v0.n += nor; v1.n += nor; v2.n += nor;
}

void PrimitiveModule::Subdivide(const size_t& vSubdivLevel, std::vector<Vertice>& vVertices, std::vector<Face>& vFaces)
{
	if (vSubdivLevel > 0)
	{
		CacheDB middlePointIndexCache;

		ct::fvec3 planeNormal;

		if (m_PrimitiveTypeIndex == PrimitiveTypeEnum::PRIMITIVE_TYPE_PLANE)
		{
			planeNormal = (m_Origin - m_Target).GetNormalized();
		}

		// refine triangles

		try
		{
			for (int i = 0; i < vSubdivLevel; ++i)
			{
				size_t a = 0, b = 0, c = 0;

				std::vector<Face> subFaces;
				std::vector<Vertice> subVertices = m_Vertices;

				for (auto& face : m_Faces)
				{
					if (m_PrimitiveTypeIndex == PrimitiveTypeEnum::PRIMITIVE_TYPE_ICO_SPHERE)
					{
						a = GetMiddlePoint_Icosaedre(face.v1, face.v2, subVertices, middlePointIndexCache, m_IcosaedreRadius);
						b = GetMiddlePoint_Icosaedre(face.v2, face.v3, subVertices, middlePointIndexCache, m_IcosaedreRadius);
						c = GetMiddlePoint_Icosaedre(face.v3, face.v1, subVertices, middlePointIndexCache, m_IcosaedreRadius);
					}
					else if (m_PrimitiveTypeIndex == PrimitiveTypeEnum::PRIMITIVE_TYPE_PLANE)
					{
						a = GetMiddlePoint_Plane(face.v1, face.v2, subVertices, middlePointIndexCache, planeNormal);
						b = GetMiddlePoint_Plane(face.v2, face.v3, subVertices, middlePointIndexCache, planeNormal);
						c = GetMiddlePoint_Plane(face.v3, face.v1, subVertices, middlePointIndexCache, planeNormal);
					}

					if (m_PrimitiveTypeIndex == PrimitiveTypeEnum::PRIMITIVE_TYPE_ICO_SPHERE ||
						m_PrimitiveTypeIndex == PrimitiveTypeEnum::PRIMITIVE_TYPE_PLANE)
					{
						AddFace(face.v1, a, c, subFaces);
						AddFace(face.v2, b, a, subFaces);
						AddFace(face.v3, c, b, subFaces);
						AddFace(a, b, c, subFaces);
					}
				}

				m_Faces = subFaces;
				m_Vertices = subVertices;
			}
		}
		catch (std::exception& ex)
		{
			printf("error %s", std::string(ex.what()).c_str());
		}
	}
}

void PrimitiveModule::BuildMesh()
{
	VerticeArray vertices;
	IndiceArray indices;

	for (const auto& v : m_Vertices)
	{
		vertices.push_back(v);
	}

	for (auto& f : m_Faces)
	{
		indices.push_back(Indice(f.v1));
		indices.push_back(Indice(f.v2));
		indices.push_back(Indice(f.v3));

		CalcNormal(
			vertices.at(f.v1), 
			vertices.at(f.v2), 
			vertices.at(f.v3));

		printf("face : %u:%u:%u\n", (uint32_t)f.v1, (uint32_t)f.v2, (uint32_t)f.v3);
	}

	for (auto& v : vertices)
	{
		v.n = v.n.GetNormalized();
	}

	auto sceneMeshPtr = SceneMesh::Create(m_VulkanCorePtr, vertices, indices);
	m_SceneModelPtr->clear();
	m_SceneModelPtr->Add(sceneMeshPtr);

	auto parentNodePtr = GetParentNode().getValidShared();
	if (parentNodePtr)
	{
		parentNodePtr->SendFrontNotification(ModelUpdateDone);
	}
}
