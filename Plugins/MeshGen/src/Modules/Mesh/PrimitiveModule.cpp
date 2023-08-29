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
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <Gaia/VulkanCore.h>
#include <Gaia/VulkanShader.h>
#include <Gaia/VulkanSubmitter.h>
#include <utils/Mesh/VertexStruct.h>
#include <Base/FrameBuffer.h>
#include <SceneGraph/SceneModel.h>
#include <SceneGraph/SceneMesh.hpp>
#include <cmath>
#include <glm/gtx/euler_angles.hpp>

using namespace GaiApi;

#define TORUS_MODE_MAJOR_MINOR 0
#define TORUS_MODE_EXT_INT 1

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<PrimitiveModule> PrimitiveModule::Create(GaiApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
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

PrimitiveModule::PrimitiveModule(GaiApi::VulkanCorePtr vVulkanCorePtr)
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
	m_PrimitiveTypes[PrimitiveTypeEnum::PRIMITIVE_TYPE_FIBONACCI_BALL] = "Fibonacci Ball";

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

bool PrimitiveModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext, const std::string& vUserDatas)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	bool change = false;
	change |= ImGui::ContrastedComboVectorDefault(0.0f, "Primitive Types", &m_PrimitiveTypeIndex, m_PrimitiveTypes, 0);
	change |= ImGui::InputFloatDefault(0.0f, "Loc X", &m_Location.x, 1.0f);
	change |= ImGui::InputFloatDefault(0.0f, "Loc Y", &m_Location.y, 1.0f);
	change |= ImGui::InputFloatDefault(0.0f, "Loc Z", &m_Location.z, 1.0f);
	change |= ImGui::InputFloatDefault(0.0f, "Rot X (deg)", &m_Rotation.x, 1.0f);
	change |= ImGui::InputFloatDefault(0.0f, "Rot Y (deg)", &m_Rotation.y, 1.0f);
	change |= ImGui::InputFloatDefault(0.0f, "Rot Z (deg)", &m_Rotation.z, 1.0f);
	change |= ImGui::SliderUIntDefaultCompact(0.0f, "Subdivision Level", &m_SubdivisionLevel, 0U, 5U, 0U);

	change |= prDrawWidgets();

	if (change)
	{
		prUpdateMesh();
	}

	return change;
}

void PrimitiveModule::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContext, const std::string& vUserDatas)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
}

void PrimitiveModule::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContext, const std::string& vUserDatas)
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
	str += vOffset + "\t<location>" + m_Location.string() + "</location>\n";
	str += vOffset + "\t<rotation>" + m_Rotation.string() + "</rotation>\n";
	str += vOffset + "\t<subdivision_level>" + ct::toStr(m_SubdivisionLevel) + "</subdivision_level>\n";

	// plane
	str += vOffset + "\t<plane_size>" + m_PlaneParams.m_Size.string() + "</plane_size>\n";

	// cube
	str += vOffset + "\t<cube_size>" + m_CubeParams.m_Size.string() + "</cube_size>\n";

	// icosaedre
	str += vOffset + "\t<icosaedre_radius>" + ct::toStr(m_IcosaedreParams.m_Radius) + "</icosaedre_radius>\n";

	// torus
	str += vOffset + "\t<torus_generate_uvs>" + (m_TorusParams.m_GenerateUVS ? "true" : "false") + "</torus_generate_uvs>\n";
	str += vOffset + "\t<torus_major_segments>" + ct::toStr(m_TorusParams.m_MajorSegments) + "</torus_major_segments>\n";
	str += vOffset + "\t<torus_minor_segments>" + ct::toStr(m_TorusParams.m_MinorSegments) + "</torus_minor_segments>\n";
	str += vOffset + "\t<torus_section_angle>" + ct::toStr(m_TorusParams.m_SectionAngle) + "</torus_section_angle>\n";
	str += vOffset + "\t<torus_section_twist>" + ct::toStr(m_TorusParams.m_SectionTwist) + "</torus_section_twist>\n";
	str += vOffset + "\t<torus_major_radius>" + ct::toStr(m_TorusParams.m_MajorRadius) + "</torus_major_radius>\n";
	str += vOffset + "\t<torus_minor_radius>" + ct::toStr(m_TorusParams.m_MinorRadius) + "</torus_minor_radius>\n";
	str += vOffset + "\t<torus_exterior_radius>" + ct::toStr(m_TorusParams.m_ExteriorRadius) + "</torus_exterior_radius>\n";
	str += vOffset + "\t<torus_interior_radius>" + ct::toStr(m_TorusParams.m_InteriorRadius) + "</torus_interior_radius>\n";
	str += vOffset + "\t<torus_mode>" + ct::toStr(m_TorusParams.m_Mode.index) + "</torus_mode>\n";

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
		if (strName == "location")
			m_Location = ct::ivariant(strValue).GetV3();
		else if (strName == "rotation")
			m_Rotation = ct::ivariant(strValue).GetV3();
		if (strName == "primitive_type")
			m_PrimitiveTypeIndex = ct::ivariant(strValue).GetI();
		else if (strName == "subdivision_level")
			m_SubdivisionLevel = ct::uvariant(strValue).GetU();
		else if (strName == "plane_size")
			m_PlaneParams.m_Size = ct::fvariant(strValue).GetV2();
		else if (strName == "cube_size")
			m_CubeParams.m_Size = ct::fvariant(strValue).GetV3();
		else if (strName == "icosaedre_radius")
			m_IcosaedreParams.m_Radius = ct::fvariant(strValue).GetF();
		else if (strName == "torus_generate_uvs")
			m_TorusParams.m_GenerateUVS = ct::ivariant(strValue).GetB();
		else if (strName == "torus_major_segments")
			m_TorusParams.m_MajorSegments = ct::fvariant(strValue).GetI();
		else if (strName == "torus_minor_segments")
			m_TorusParams.m_MinorSegments = ct::fvariant(strValue).GetI();
		else if (strName == "torus_section_angle")
			m_TorusParams.m_SectionAngle = ct::fvariant(strValue).GetF();
		else if (strName == "torus_section_twist")
			m_TorusParams.m_SectionTwist = ct::fvariant(strValue).GetI();
		else if (strName == "torus_major_radius")
			m_TorusParams.m_MajorRadius = ct::fvariant(strValue).GetF();
		else if (strName == "torus_minor_radius")
			m_TorusParams.m_MinorRadius = ct::fvariant(strValue).GetF();
		else if (strName == "torus_exterior_radius")
			m_TorusParams.m_ExteriorRadius = ct::fvariant(strValue).GetF();
		else if (strName == "torus_interior_radius")
			m_TorusParams.m_InteriorRadius = ct::fvariant(strValue).GetF();
		else if (strName == "torus_mode")
			m_TorusParams.m_Mode.index = ct::fvariant(strValue).GetI();
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

bool PrimitiveModule::prDrawWidgets()
{
	if (ImGui::CollapsingHeader("Primitives"))
	{
		bool change = false;

		switch (m_PrimitiveTypeIndex)
		{
		case PRIMITIVE_TYPE_PLANE:
		{
			change |= ImGui::CheckBoxBoolDefault("Generate UV's", &m_GenerateUVs, false);
			change |= ImGui::InputFloatDefault(0.0f, "Size X", &m_PlaneParams.m_Size.x, 1.0f);
			change |= ImGui::InputFloatDefault(0.0f, "Size Y", &m_PlaneParams.m_Size.y, 1.0f);
		}
		break;
		case PRIMITIVE_TYPE_CUBE:
		{
			change |= ImGui::CheckBoxBoolDefault("Generate UV's", &m_GenerateUVs, false);
			change |= ImGui::InputFloatDefault(0.0f, "Size X", &m_CubeParams.m_Size.x, 1.0f);
			change |= ImGui::InputFloatDefault(0.0f, "Size Y", &m_CubeParams.m_Size.y, 1.0f);
			change |= ImGui::InputFloatDefault(0.0f, "Size Z", &m_CubeParams.m_Size.z, 1.0f);
		}
		break;
		case PRIMITIVE_TYPE_ICO_SPHERE:
		{
			change |= ImGui::InputFloatDefault(0.0f, "Radius", &m_IcosaedreParams.m_Radius, 1.0f);
		}
		break;
		case PRIMITIVE_TYPE_UV_SPHERE:
		{

		}
		break;
		case PRIMITIVE_TYPE_TORUS:
		{
			// tofix : desactivé pour le moment vu que ca amrche pas
			//change |= ImGui::CheckBoxBoolDefault("Generate UV's", &m_TorusParams.m_GenerateUVS, true);
			change |= ImGui::SliderIntDefaultCompact(0.0f, "Major Segments", &m_TorusParams.m_MajorSegments, 3U, 256U, 48U);
			change |= ImGui::SliderIntDefaultCompact(0.0f, "Minor Segments", &m_TorusParams.m_MinorSegments, 2U, 256U, 12U);
			change |= ImGui::SliderFloatDefaultCompact(0.0f, "Section Angle (deg)", &m_TorusParams.m_SectionAngle, 0.0f, 360.0f, 0.0f);
			change |= ImGui::SliderIntDefaultCompact(0.0f, "Section Twist", &m_TorusParams.m_SectionTwist, 0, 256, 0);
			change |= m_TorusParams.m_Mode.DisplayCombo(0.0f, "Mode");
			if (m_TorusParams.m_Mode.index == 0)
			{
				change |= ImGui::SliderFloatDefaultCompact(0.0f, "Major Radius", &m_TorusParams.m_MajorRadius, 0.0f, 50.0f, 1.0f);
				change |= ImGui::SliderFloatDefaultCompact(0.0f, "Minor Radius", &m_TorusParams.m_MinorRadius, 0.0f, 50.0f, 0.25f);
			}
			else
			{
				change |= ImGui::SliderFloatDefaultCompact(0.0f, "Exterior Radius", &m_TorusParams.m_ExteriorRadius, 0.0f, 50.0f, 1.25f);
				change |= ImGui::SliderFloatDefaultCompact(0.0f, "Interior Radius", &m_TorusParams.m_InteriorRadius, 0.0f, 50.0f, 0.75f);
			}
		}
		break;
		case PRIMITIVE_TYPE_FIBONACCI_BALL:
		{

		}
		break;
		case PRIMITIVE_TYPE_Count:
		default:
			break;
		}

		return change;
	}

	return false;
}

void PrimitiveModule::prUpdateMesh()
{
	m_Vertices.clear();
	m_TriFaces.clear();
	m_QuadFaces.clear();
	m_HaveTextureCoords = false;

	switch (m_PrimitiveTypeIndex)
	{
	case PRIMITIVE_TYPE_PLANE:
	{
		CreatePlane();
	}
	break;
	case PRIMITIVE_TYPE_CUBE:
	{
		CreateCube();
	}
	break;
	case PRIMITIVE_TYPE_ICO_SPHERE:
	{
		CreateIcosaedre();
	}
	break;
	case PRIMITIVE_TYPE_UV_SPHERE:
	{
		CreateUVSphere();
	}
	break;
	case PRIMITIVE_TYPE_TORUS:
	{
		CreateTorus();
	}
	break;
	case PRIMITIVE_TYPE_FIBONACCI_BALL:
	{
		CreateFibonacciBall();
	}
	break;
	case PRIMITIVE_TYPE_Count:
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
	const auto& normal = ct::fvec3(0, 1, 0);
	const auto& size = ct::fvec3(m_PlaneParams.m_Size.x, 0.0f, m_PlaneParams.m_Size.y) * 0.5f;

	std::vector<ct::fvec3> plane_points = {
		ct::fvec3(-1, 0, -1),
		ct::fvec3(1, 0, -1),
		ct::fvec3(1, 0, 1),
		ct::fvec3(-1, 0, 1)
	};

	std::vector<ct::fvec2> plane_uvs = {
		ct::fvec2(0,0),
		ct::fvec2(1,0),
		ct::fvec2(1,1),
		ct::fvec2(0,1)
	};

	assert(plane_points.size() == 4);
	assert(plane_uvs.size() == 4);

	for (size_t point_idx = 0, uv_idx = 0; point_idx < plane_points.size(); ++point_idx, ++uv_idx)
	{
		auto uv = m_GenerateUVs ? plane_uvs.at(uv_idx) : ct::fvec2();
		m_Vertices.emplace_back(m_Origin + plane_points.at(point_idx) * size, normal, ct::fvec3(), ct::fvec3(), uv);
	}

	m_TriFaces.emplace_back(0, 1, 2);
	m_TriFaces.emplace_back(0, 2, 3);

	Subdivide(m_SubdivisionLevel, m_Vertices, m_TriFaces);

	BuildMesh();
}

void PrimitiveModule::CreateCube()
{
	if (m_GenerateUVs)
	{
		std::vector<ct::fvec3> cube_points = {
			ct::fvec3(-1,1,-1), ct::fvec3(1,1,-1), ct::fvec3(1,1,1), ct::fvec3(-1,1,1),		// top
			ct::fvec3(-1,-1,-1), ct::fvec3(1,-1,-1), ct::fvec3(1,-1,1), ct::fvec3(-1,-1,1),	// bottom
			ct::fvec3(-1,-1,-1), ct::fvec3(-1,1,-1), ct::fvec3(-1,1,1), ct::fvec3(-1,-1,1),	// left
			ct::fvec3(1,-1,-1), ct::fvec3(1,1,-1), ct::fvec3(1,1,1), ct::fvec3(1,-1,1),		// right
			ct::fvec3(-1,-1,-1), ct::fvec3(1,-1,-1), ct::fvec3(1,1,-1), ct::fvec3(-1,1,-1),	// back
			ct::fvec3(-1,-1,1), ct::fvec3(1,-1,1), ct::fvec3(1,1,1), ct::fvec3(-1,1,1),		// front
		};

		std::vector<ct::fvec2> cube_uvs = {
			ct::fvec2(0,0), ct::fvec2(1,0), ct::fvec2(1,1), ct::fvec2(0,1)
		};

		std::vector<ct::fvec3> cube_normals = {
			ct::fvec3(0, 1, 0),
			ct::fvec3(0, -1, 0),
			ct::fvec3(1, 0, 0),
			ct::fvec3(-1, 0, 0),
			ct::fvec3(0, 0, 1),
			ct::fvec3(0, 0, -1)
		};

		assert(cube_normals.size() == 6);
		assert(cube_points.size() == 24);
		assert(cube_uvs.size() == 4);

		m_Vertices.reserve(cube_points.size());
		m_TriFaces.reserve(cube_normals.size() * 2); // 2 triangle per faces

		const auto& mid_size = m_CubeParams.m_Size * 0.5f;
		for (size_t point_idx = 0; point_idx < cube_points.size(); ++point_idx)
		{
			const size_t& normal_idx = ct::floor(point_idx / 4);
			const size_t& uv_idx = point_idx % 4;

			m_Vertices.emplace_back(
				cube_points.at(point_idx) * mid_size,
				cube_normals.at(normal_idx),
				ct::fvec3(),
				ct::fvec3(),
				cube_uvs.at(uv_idx));
		}

		for (size_t point_idx = 0; point_idx < cube_points.size(); point_idx += 4)
		{
			m_TriFaces.emplace_back(point_idx + 0, point_idx + 1, point_idx + 2);
			m_TriFaces.emplace_back(point_idx + 0, point_idx + 2, point_idx + 3);
		}
	}
	else // no uvs, so all vertices are merged
	{
		std::vector<ct::fvec3> cube_points = {
			ct::fvec3(-1,1,-1), ct::fvec3(1,1,-1), ct::fvec3(1,1,1), ct::fvec3(-1,1,1),     // top
			ct::fvec3(-1,-1,-1), ct::fvec3(1,-1,-1), ct::fvec3(1,-1,1), ct::fvec3(-1,-1,1)	// bottom
		};

		/*
		     7---------6
		    /|        /|
		   / |       / |
		  3---------2  |
		  |  4------|--5
		  | /       | / 
          |/        |/
		  0---------1
		*/

		std::vector<uint32_t> cube_faces = {
			0,1,2,0,2,3,
			1,5,6,1,6,2,
			5,4,7,5,7,6,
			4,0,3,4,3,7,
			3,2,6,3,6,7,
			4,5,1,4,1,0
		};

		assert(cube_points.size() == 8);
		assert(cube_faces.size() == 36);

		m_Vertices.reserve(cube_points.size());
		m_TriFaces.reserve(cube_faces.size() * 3);

		const auto& mid_size = m_CubeParams.m_Size * 0.5f;
		for (size_t point_idx = 0; point_idx < cube_points.size(); ++point_idx)
		{
			const auto& point = cube_points.at(point_idx) * mid_size;
			const ct::fvec3& normal = point.GetNormalized();

			m_Vertices.emplace_back(
				point,
				normal,
				ct::fvec3(),
				ct::fvec3(),
				ct::fvec2());
		}

		for (size_t face_idx = 0; face_idx < cube_faces.size(); face_idx += 3)
		{
			m_TriFaces.emplace_back(
				cube_faces.at(face_idx + 2), 
				cube_faces.at(face_idx + 1), 
				cube_faces.at(face_idx + 0));
		}
	}

	Subdivide(m_SubdivisionLevel, m_Vertices, m_TriFaces);

	BuildMesh();
}

void PrimitiveModule::CreateIcosaedre()
{
	// create 12 vertices of a icosahedron
	float t = (1.0f + sqrtf(5.0f)) / 2.0f;

	std::vector<ct::fvec3> normals = {
		ct::fvec3(-1.0f, t, 0.0f).GetNormalized(),
		ct::fvec3(1.0f, t, 0.0f).GetNormalized(),
		ct::fvec3(-1.0f, -t, 0.0f).GetNormalized(),
		ct::fvec3(1.0f, -t, 0.0f).GetNormalized(),

		ct::fvec3(0.0f, -1.0f, t).GetNormalized(),
		ct::fvec3(0.0f, 1.0f, t).GetNormalized(),
		ct::fvec3(0.0f, -1.0f, -t).GetNormalized(),
		ct::fvec3(0.0f, 1.0f, -t).GetNormalized(),

		ct::fvec3(t, 0.0f, -1.0f).GetNormalized(),
		ct::fvec3(t, 0.0f, 1.0f).GetNormalized(),
		ct::fvec3(-t, 0.0f, -1.0f).GetNormalized(),
		ct::fvec3(-t, 0.0f, 1.0f).GetNormalized(),
	};

	for (const auto& normal : normals)
	{
		m_Vertices.emplace_back(m_Origin + normal * m_IcosaedreParams.m_Radius, normal);
	}

	// create 20 triangles of the icosahedron

	// 5 faces around point 0
	m_TriFaces.emplace_back(0, 11, 5);
	m_TriFaces.emplace_back(0, 5, 1);
	m_TriFaces.emplace_back(0, 1, 7);
	m_TriFaces.emplace_back(0, 7, 10);
	m_TriFaces.emplace_back(0, 10, 11);

	// 5 adjacent faces
	m_TriFaces.emplace_back(1, 5, 9);
	m_TriFaces.emplace_back(5, 11, 4);
	m_TriFaces.emplace_back(11, 10, 2);
	m_TriFaces.emplace_back(10, 7, 6);
	m_TriFaces.emplace_back(7, 1, 8);

	// 5 faces around point 3
	m_TriFaces.emplace_back(3, 9, 4);
	m_TriFaces.emplace_back(3, 4, 2);
	m_TriFaces.emplace_back(3, 2, 6);
	m_TriFaces.emplace_back(3, 6, 8);
	m_TriFaces.emplace_back(3, 8, 9);

	// 5 adjacent faces
	m_TriFaces.emplace_back(4, 9, 5);
	m_TriFaces.emplace_back(2, 4, 11);
	m_TriFaces.emplace_back(6, 2, 10);
	m_TriFaces.emplace_back(8, 6, 7);
	m_TriFaces.emplace_back(9, 8, 1);

	Subdivide(m_SubdivisionLevel, m_Vertices, m_TriFaces);

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

size_t PrimitiveModule::GetMiddlePoint_Plane(const size_t& p1, const size_t& p2, VerticeArray& vVertices, CacheDB& vCache)
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
	const auto& pt1 = vVertices.at(p1);
	const auto& pt2 = vVertices.at(p2);
	const auto& middle_point = (pt1.p + pt2.p) * 0.5f;
	const auto& middle_normal = (pt1.n + pt2.n).GetNormalized();
	const auto& middle_tex = (pt1.t + pt2.t) * 0.5f;

	// add vertex makes sure point is on unit sphere
	size_t i = vVertices.size();
	vVertices.emplace_back(middle_point, middle_normal, ct::fvec3(), ct::fvec3(), middle_tex);

	// store it, return index
	vCache[_block] = i;

	return i;
}

void PrimitiveModule::CreateFibonacciBall()
{
	BuildMesh();
}

void PrimitiveModule::CreateUVSphere()
{
	BuildMesh();
}

void PrimitiveModule::CreateTorus()
{
	if (m_TorusParams.m_Mode.index == TORUS_MODE_MAJOR_MINOR)
	{
		m_TorusParams.m_ExteriorRadius = m_TorusParams.m_MajorRadius + m_TorusParams.m_MinorRadius;
		m_TorusParams.m_InteriorRadius = m_TorusParams.m_MajorRadius - m_TorusParams.m_MinorRadius;
	}
	else if (m_TorusParams.m_Mode.index == TORUS_MODE_EXT_INT)
	{
		m_TorusParams.m_MinorRadius = (m_TorusParams.m_ExteriorRadius - m_TorusParams.m_InteriorRadius) * 0.5f;
		m_TorusParams.m_MajorRadius = m_TorusParams.m_InteriorRadius + m_TorusParams.m_MinorRadius;
	}

	CreateTorusMesh(m_TorusParams.m_MajorRadius, m_TorusParams.m_MinorRadius, m_TorusParams.m_MajorSegments, 
		m_TorusParams.m_MinorSegments, m_TorusParams.m_SectionAngle * DEGTORAD, m_TorusParams.m_SectionTwist);

	if (m_TorusParams.m_GenerateUVS && m_TorusParams.m_MinorSegments)
	{
		if (m_TorusParams.m_SectionTwist % m_TorusParams.m_MinorSegments == 0)
		{
			AddTorusUVs(m_TorusParams.m_MinorSegments, m_TorusParams.m_MajorSegments);
		}
		else
		{
			AddTorusUVsOneRibbon(m_TorusParams.m_MinorSegments, m_TorusParams.m_MajorSegments, m_TorusParams.m_SectionTwist);
		}
	}

	BuildMesh();
}

void PrimitiveModule::CreateTorusMesh(
	const float& vMajorRadius,
	const float& vMinorRadius,
	const int32_t& vMajorSegments,
	const int32_t& vMinorSegments,
	const float& vSectionAngle,
	const int32_t& vSectionTwist)
{
	const float pi_2 = _pi * 2.0f;
	const float majorSegFactor = 1.0f / (float)vMajorSegments;
	const float minorSegFactor = 1.0f / (float)vMinorSegments;

	const float twist_step_angle = pi_2 * minorSegFactor * majorSegFactor * vSectionTwist;
	uint32_t face_indexs[4] = {};
	uint32_t tot_verts = vMajorSegments * vMinorSegments;
	for (uint32_t major_index = 0; major_index < (uint32_t)vMajorSegments; ++major_index)
	{
		const glm::mat4 rotation_matrix = glm::rotate(major_index * majorSegFactor * pi_2, glm::vec3(0, 1, 0)); // up is Y
		const float major_twist_angle = major_index * twist_step_angle;

		for (uint32_t minor_index = 0; minor_index < (uint32_t)vMinorSegments; ++minor_index)
		{
			const float angle = pi_2 * minor_index * minorSegFactor + vSectionAngle + major_twist_angle;

			// transform a point so the last coord of the vec4 must be 1
			const auto vertex = rotation_matrix * glm::vec4(
				vMajorRadius + (cos(angle) * vMinorRadius),
				sin(angle) * vMinorRadius,
				0.0,
				1
			);

			m_Vertices.emplace_back(ct::fvec3(vertex.x, vertex.y, vertex.z));

			if (vMinorSegments > 2 && minor_index + 1 == vMinorSegments)
			{
				face_indexs[1] = major_index * vMinorSegments;
				face_indexs[2] = face_indexs[0] + vMinorSegments;
				face_indexs[3] = face_indexs[1] + vMinorSegments;
			}
			else
			{
				face_indexs[1] = face_indexs[0] + 1;
				face_indexs[2] = face_indexs[0] + vMinorSegments;
				face_indexs[3] = face_indexs[2] + 1;
			}

			if (face_indexs[1] >= tot_verts) {
				face_indexs[1] = (face_indexs[1] - tot_verts + vSectionTwist) % vMinorSegments;
			}
			if (face_indexs[2] >= tot_verts) {
				face_indexs[2] = (face_indexs[2] - tot_verts + vSectionTwist) % vMinorSegments;
			}
			if (face_indexs[3] >= tot_verts) {
				face_indexs[3] = (face_indexs[3] - tot_verts + vSectionTwist) % vMinorSegments;
			}

			m_QuadFaces.emplace_back(face_indexs[0], face_indexs[2], face_indexs[3], face_indexs[1]);

			++face_indexs[0];
		}
	}
}

void PrimitiveModule::AddTorusUVs(const int32_t& vMajorSegments, const int32_t& vMinorSegments)
{
	if (vMajorSegments && vMinorSegments)
	{
		float u_step = 1.0f / (float)vMajorSegments;
		float v_step = 1.0f / (float)vMinorSegments;

		// Round UV's, needed when segments aren't divisible by 4.
		float u_init = 0.5f + fmod(0.5f, u_step);
		float v_init = 0.5f + fmod(0.5f, v_step);

		// Calculate wrapping value under 1.0 to prevent
		// float precision errors wrapping at the wrong step.
		float u_wrap = 1.0f - (u_step * 0.5f);
		float v_wrap = 1.0f - (v_step * 0.5f);

		int32_t face_index = 0;
		float u_prev = u_init;
		float u_next = u_prev + u_step;
		float v_prev = 0.0f;
		float v_next = 0.0f;

		for (int32_t _major_index = 0; _major_index < vMajorSegments; ++_major_index)
		{
			v_prev = v_init;
			v_next = v_prev + v_step;
			for (int32_t _minor_index = 0; _minor_index < vMinorSegments; ++_minor_index)
			{
				if (face_index < m_QuadFaces.size())
				{
					auto face = m_QuadFaces.at(face_index);
					face.t0 = ct::fvec2(u_prev, v_prev);
					face.t1 = ct::fvec2(u_next, v_prev);
					face.t2 = ct::fvec2(u_next, v_next);
					face.t3 = ct::fvec2(u_prev, v_next);

					//LogVarDebug("--------------------------");
					//LogVarDebug("i0:%u, i1:%u, i2:%u, i3:%u", (uint32_t)face.v0, (uint32_t)face.v1, (uint32_t)face.v2, (uint32_t)face.v3);
					//LogVarDebug("u0:%f, u1:%f, u2:%f, u3:%f", u_prev, u_next, u_next, u_prev);
					//LogVarDebug("v0:%f, v1:%f, v2:%f, v3:%f", v_prev, v_prev, v_next, v_next);
				}

				if (v_next > v_wrap)
					v_prev = v_next - 1.0f;
				else
					v_prev = v_next;

				v_next = v_prev + v_step;

				++face_index;
			}

			if (u_next > u_wrap)
				u_prev = u_next - 1.0f;
			else
				u_prev = u_next;

			u_next = u_prev + u_step;
		}
		m_HaveTextureCoords = true;
	}
}
	
void PrimitiveModule::AddTorusUVsOneRibbon(const int32_t& vMajorSegments, const int32_t& vMinorSegments, const int32_t& vSectionTwist)
{
	if (vMajorSegments && vMinorSegments)
	{
		int32_t count = vMajorSegments * vMinorSegments;
		float u_step = 1.0f / (float)count;
		float u_next = 0.0;
		float u_prev = 0.0;
		int32_t face_index = 0;
		for (int32_t offset = 0; offset < vMinorSegments; ++offset)
		{
			const int32_t off = (offset * vSectionTwist) % vMinorSegments;
			for (int32_t idx = 0; idx < vMajorSegments; ++idx)
			{
				u_next = u_prev + u_step;
				face_index = idx * vMinorSegments + off;
				if (face_index < m_QuadFaces.size())
				{
					auto face = m_QuadFaces.at(face_index);
					face.t0 = ct::fvec2(u_prev, 0.0f);
					face.t1 = ct::fvec2(u_next, 0.0f);
					face.t2 = ct::fvec2(u_next, 1.0f);
					face.t3 = ct::fvec2(u_prev, 1.0f);

					//LogVarDebug("--------------------------");
					//LogVarDebug("i0:%u, i1:%u, i2:%u, i3:%u", (uint32_t)face.v0, (uint32_t)face.v1, (uint32_t)face.v2, (uint32_t)face.v3);
					//LogVarDebug("u0:%f, u1:%f, u2:%f, u3:%f", u_prev, u_next, u_next, u_prev);
					//LogVarDebug("v0:%f, v1:%f, v2:%f, v3:%f", 0.0f, 0.0f, 1.0f, 1.0f);
				}
				u_prev = u_next;
			}
		}
		m_HaveTextureCoords = true;
	}
}	

void PrimitiveModule::CalcNormal(Vertice& v0, Vertice& v1, Vertice& v2)
{
	const ct::fvec3 vec0 = (v2.p - v0.p).GetNormalized();
	const ct::fvec3 vec1 = (v1.p - v0.p).GetNormalized();
	const ct::fvec3 nor = cCross(vec1, vec0).GetNormalized();

	v0.n += nor; v1.n += nor; v2.n += nor;
}

void PrimitiveModule::Subdivide(const size_t& vSubdivLevel, std::vector<Vertice>& vVertices, std::vector<TriFace>& vFaces)
{
	if (vSubdivLevel > 0)
	{
		CacheDB middlePointIndexCache;

		// refine triangles
		try
		{
			for (int i = 0; i < vSubdivLevel; ++i)
			{
				size_t a = 0, b = 0, c = 0;

				std::vector<TriFace> subFaces;
				std::vector<Vertice> subVertices = m_Vertices;

				for (auto& face : m_TriFaces)
				{
					if (m_PrimitiveTypeIndex == PrimitiveTypeEnum::PRIMITIVE_TYPE_ICO_SPHERE)
					{
						a = GetMiddlePoint_Icosaedre(face.v0, face.v1, subVertices, middlePointIndexCache, m_IcosaedreParams.m_Radius);
						b = GetMiddlePoint_Icosaedre(face.v1, face.v2, subVertices, middlePointIndexCache, m_IcosaedreParams.m_Radius);
						c = GetMiddlePoint_Icosaedre(face.v2, face.v0, subVertices, middlePointIndexCache, m_IcosaedreParams.m_Radius);
					}
					else if (m_PrimitiveTypeIndex == PrimitiveTypeEnum::PRIMITIVE_TYPE_PLANE ||
						m_PrimitiveTypeIndex == PrimitiveTypeEnum::PRIMITIVE_TYPE_CUBE)
					{
						a = GetMiddlePoint_Plane(face.v0, face.v1, subVertices, middlePointIndexCache);
						b = GetMiddlePoint_Plane(face.v1, face.v2, subVertices, middlePointIndexCache);
						c = GetMiddlePoint_Plane(face.v2, face.v0, subVertices, middlePointIndexCache);
					}

					if (m_PrimitiveTypeIndex == PrimitiveTypeEnum::PRIMITIVE_TYPE_ICO_SPHERE ||
						m_PrimitiveTypeIndex == PrimitiveTypeEnum::PRIMITIVE_TYPE_PLANE ||
						m_PrimitiveTypeIndex == PrimitiveTypeEnum::PRIMITIVE_TYPE_CUBE)
					{
						subFaces.emplace_back(face.v0, a, c);
						subFaces.emplace_back(face.v1, b, a);
						subFaces.emplace_back(face.v2, c, b);
						subFaces.emplace_back(a, b, c);
					}
				}

				m_TriFaces = subFaces;
				m_Vertices = subVertices;
			}
		}
		catch (std::exception& ex)
		{
			LogVarError("error %s", std::string(ex.what()).c_str());
		}
	}
}

size_t PrimitiveModule::GenericGetMiddlePoint(const size_t& p1, const size_t& p2, VerticeArray& vVertices, CacheDB& vCache)
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
	const ct::fvec3 normal1 = vVertices.at(p1).n;
	const ct::fvec3 normal2 = vVertices.at(p2).n;
	const ct::fvec2 tex1 = vVertices.at(p1).t;
	const ct::fvec2 tex2 = vVertices.at(p2).t;

	ct::fvec3 middle_p = (point1 + point2) * 0.5f;
	ct::fvec2 middle_t = (tex1 + tex2) * 0.5f;
	ct::fvec3 middle_n = (normal1 + normal2).GetNormalized();
	
	const ct::fplane pln1(point1, normal1);
	const ct::fplane pln2(point2, normal2);
	const ct::fplane pln3(point1, point2, middle_p + middle_n);

	ct::fvec3 res_p;
	if (!ct::get_plane_intersection(pln1, pln2, pln3, res_p))
	{
		res_p = middle_p;
	}

	// add vertex makes sure point is on unit sphere
	size_t i = vVertices.size();
	vVertices.emplace_back(res_p, middle_n, ct::fvec3(), ct::fvec3(), middle_t);

	// store it, return index
	vCache[_block] = i;

	return i;
}

void PrimitiveModule::GenericSubdivide(const size_t& vSubdivLevel, std::vector<Vertice>& vVertices, std::vector<TriFace>& vFaces)
{
	if (vSubdivLevel > 0)
	{
		CacheDB middlePointIndexCache;

		// refine triangles
		try
		{
			for (int i = 0; i < vSubdivLevel; ++i)
			{
				size_t a = 0, b = 0, c = 0;

				std::vector<TriFace> subFaces;
				std::vector<Vertice> subVertices = m_Vertices;

				for (auto& face : m_TriFaces)
				{
					a = GenericGetMiddlePoint(face.v0, face.v1, subVertices, middlePointIndexCache);
					b = GenericGetMiddlePoint(face.v1, face.v2, subVertices, middlePointIndexCache);
					c = GenericGetMiddlePoint(face.v2, face.v0, subVertices, middlePointIndexCache);

					subFaces.emplace_back(face.v0, a, c);
					subFaces.emplace_back(face.v1, b, a);
					subFaces.emplace_back(face.v2, c, b);
					subFaces.emplace_back(a, b, c);
				}

				m_TriFaces = subFaces;
				m_Vertices = subVertices;
			}
		}
		catch (std::exception& ex)
		{
			LogVarError("error %s", std::string(ex.what()).c_str());
		}
	}
}

void PrimitiveModule::BuildMesh()
{
	if (!m_Vertices.empty())
	{
		VerticeArray vertices;
		IndiceArray indices;

		// split vertices per uv maps
		/*for (auto v : m_QuadFaces)
		{

		}*/

		const auto matrix = glm::yawPitchRoll(m_Rotation.x * DEGTORAD, m_Rotation.y * DEGTORAD, m_Rotation.z * DEGTORAD);
		vertices.reserve(m_Vertices.size());
		for (auto v : m_Vertices)
		{
			// apply rotation
			auto new_p = matrix * glm::vec4(v.p.x, v.p.y, v.p.z, 1.0f);
			// apply translation
			v.p = ct::fvec3(new_p.x, new_p.y, new_p.z) + m_Location;
			//save point
			vertices.push_back(v);
		}

		if (!m_TriFaces.empty())
		{
			indices.reserve(m_TriFaces.size() * 3U); // 6 => 1 triangle so 3 indices
			for (auto& f : m_TriFaces)
			{
				indices.push_back(Indice(f.v0));
				indices.push_back(Indice(f.v1));
				indices.push_back(Indice(f.v2));

				CalcNormal(
					vertices.at(f.v0),
					vertices.at(f.v1),
					vertices.at(f.v2));
			}
		}

		if (!m_QuadFaces.empty())
		{
			indices.reserve(m_QuadFaces.size() * 6U); // 6 => 2 triangles so 6 indices
			for (auto& f : m_QuadFaces)
			{
				// first triangle
				indices.push_back(Indice(f.v0));
				indices.push_back(Indice(f.v1));
				indices.push_back(Indice(f.v2));

				CalcNormal(
					vertices.at(f.v0),
					vertices.at(f.v1),
					vertices.at(f.v2));

				// second triangle
				indices.push_back(Indice(f.v0));
				indices.push_back(Indice(f.v2));
				indices.push_back(Indice(f.v3));

				CalcNormal(
					vertices.at(f.v0),
					vertices.at(f.v2),
					vertices.at(f.v3));
			}
		}

		for (auto& v : vertices)
		{
			v.n = v.n.GetNormalized();
		}

		auto mesh_ptr = SceneMesh<VertexStruct::P3_N3_TA3_BTA3_T2_C4>::Create(m_VulkanCorePtr, vertices, indices);
		if (mesh_ptr)
		{
			if (m_HaveTextureCoords)
			{
				mesh_ptr->HaveTextureCoords();
			}

			m_SceneModelPtr->clear();
			m_SceneModelPtr->Add(mesh_ptr);

			auto parentNodePtr = GetParentNode().lock();
			if (parentNodePtr)
			{
				parentNodePtr->SendFrontNotification(ModelUpdateDone);
			}
		}
	}
}
