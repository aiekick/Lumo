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
#include <cmath>
#include <glm/gtx/euler_angles.hpp>

using namespace vkApi;

#define TORUS_MODE_MAJOR_MINOR 0
#define TORUS_MODE_EXT_INT 1

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

bool PrimitiveModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
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

	change |= prDrawWidgets();

	if (change)
	{
		prUpdateMesh();
	}

	return change;
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
	str += vOffset + "\t<location>" + m_Location.string() + "</location>\n";
	str += vOffset + "\t<rotation>" + m_Rotation.string() + "</rotation>\n";

	// plane
	str += vOffset + "\t<plane_centered>" + (m_PlaneCentered ? "true" : "false") + "</plane_centered>\n";
	str += vOffset + "\t<plane_subdivision_level>" + ct::toStr(m_PlaneSubdivisionLevel) + "</plane_subdivision_level>\n";
	str += vOffset + "\t<plane_size>" + m_PlaneSize.string() + "</plane_size>\n";

	// cube
	str += vOffset + "\t<cube_centered>" + (m_CubeCentered ? "true" : "false") + "</cube_centered>\n";
	str += vOffset + "\t<cube_subdivision_level>" + ct::toStr(m_CubeSubdivisionLevel) + "</cube_subdivision_level>\n";
	str += vOffset + "\t<cube_size>" + m_CubeSize.string() + "</cube_size>\n";

	// icosaedre
	str += vOffset + "\t<icosaedre_radius>" + ct::toStr(m_IcosaedreRadius) + "</icosaedre_radius>\n";
	str += vOffset + "\t<icosaedre_subdivision_level>" + ct::toStr(m_IcosaedreSubdivisionLevel) + "</icosaedre_subdivision_level>\n";

	// torus
	str += vOffset + "\t<torus_generate_uvs>" + (m_GenerateUVS ? "true" : "false") + "</torus_generate_uvs>\n";
	str += vOffset + "\t<torus_major_segments>" + ct::toStr(m_MajorSegments) + "</torus_major_segments>\n";
	str += vOffset + "\t<torus_minor_segments>" + ct::toStr(m_MinorSegments) + "</torus_minor_segments>\n";
	str += vOffset + "\t<torus_section_angle>" + ct::toStr(m_SectionAngle) + "</torus_section_angle>\n";
	str += vOffset + "\t<torus_section_twist>" + ct::toStr(m_SectionTwist) + "</torus_section_twist>\n";
	str += vOffset + "\t<torus_major_radius>" + ct::toStr(m_MajorRadius) + "</torus_major_radius>\n";
	str += vOffset + "\t<torus_minor_radius>" + ct::toStr(m_MinorRadius) + "</torus_minor_radius>\n";
	str += vOffset + "\t<torus_exterior_radius>" + ct::toStr(m_ExteriorRadius) + "</torus_exterior_radius>\n";
	str += vOffset + "\t<torus_interior_radius>" + ct::toStr(m_InteriorRadius) + "</torus_interior_radius>\n";
	str += vOffset + "\t<torus_mode>" + ct::toStr(m_Mode.index) + "</torus_mode>\n";

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
		else if (strName == "torus_generate_uvs")
			m_GenerateUVS = ct::ivariant(strValue).GetB();
		else if (strName == "torus_major_segments")
			m_MajorSegments = ct::fvariant(strValue).GetI();
		else if (strName == "torus_minor_segments")
			m_MinorSegments = ct::fvariant(strValue).GetI();
		else if (strName == "torus_section_angle")
			m_SectionAngle = ct::fvariant(strValue).GetF();
		else if (strName == "torus_section_twist")
			m_SectionTwist = ct::fvariant(strValue).GetI();
		else if (strName == "torus_major_radius")
			m_MajorRadius = ct::fvariant(strValue).GetF();
		else if (strName == "torus_minor_radius")
			m_MinorRadius = ct::fvariant(strValue).GetF();
		else if (strName == "torus_exterior_radius")
			m_ExteriorRadius = ct::fvariant(strValue).GetF();
		else if (strName == "torus_interior_radius")
			m_InteriorRadius = ct::fvariant(strValue).GetF();
		else if (strName == "torus_mode")
			m_Mode.index = ct::fvariant(strValue).GetI();
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
			change |= ImGui::CheckBoxBoolDefault("Centered ?", &m_PlaneCentered, true);
			change |= ImGui::SliderUIntDefaultCompact(0.0f, "Subdivision Level", &m_PlaneSubdivisionLevel, 0U, 5U, 0U);
			change |= ImGui::InputFloatDefault(0.0f, "Size X", &m_PlaneSize.x, 1.0f);
			change |= ImGui::InputFloatDefault(0.0f, "Size Y", &m_PlaneSize.y, 1.0f);
		}
		break;
		case PRIMITIVE_TYPE_CUBE:
		{
			change |= ImGui::SliderUIntDefaultCompact(0.0f, "Subdivision Level", &m_CubeSubdivisionLevel, 0U, 5U, 0U);
			change |= ImGui::InputFloatDefault(0.0f, "Size X", &m_CubeSize.x, 1.0f);
			change |= ImGui::InputFloatDefault(0.0f, "Size Y", &m_CubeSize.y, 1.0f);
			change |= ImGui::InputFloatDefault(0.0f, "Size Z", &m_CubeSize.z, 1.0f);
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
			change |= ImGui::CheckBoxBoolDefault("Generate UV's", &m_GenerateUVS, true);
			change |= ImGui::SliderIntDefaultCompact(0.0f, "Major Segments", &m_MajorSegments, 3U, 256U, 48U);
			change |= ImGui::SliderIntDefaultCompact(0.0f, "Minor Segments", &m_MinorSegments, 2U, 256U, 12U);
			change |= ImGui::SliderFloatDefaultCompact(0.0f, "Section Angle (deg)", &m_SectionAngle, 0.0f, 360.0f, 0.0f);
			change |= ImGui::SliderIntDefaultCompact(0.0f, "Section Twist", &m_SectionTwist, 0, 256, 0);
			change |= m_Mode.DisplayCombo(0.0f, "Mode");
			if (m_Mode.index == 0)
			{
				change |= ImGui::SliderFloatDefaultCompact(0.0f, "Major Radius", &m_MajorRadius, 0.0f, 50.0f, 1.0f);
				change |= ImGui::SliderFloatDefaultCompact(0.0f, "Minor Radius", &m_MinorRadius, 0.0f, 50.0f, 0.25f);
			}
			else
			{
				change |= ImGui::SliderFloatDefaultCompact(0.0f, "Exterior Radius", &m_ExteriorRadius, 0.0f, 50.0f, 1.25f);
				change |= ImGui::SliderFloatDefaultCompact(0.0f, "Interior Radius", &m_InteriorRadius, 0.0f, 50.0f, 0.75f);
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
	ct::fvec3 size = ct::fvec3(m_PlaneSize.x, 0.0f, m_PlaneSize.y);

	AddVertex(m_Origin + ct::fvec3(-0.5f, 0.0f, -0.5f) * size, m_Target, ct::fvec2(0.0f, 0.0f), m_Vertices);
	AddVertex(m_Origin + ct::fvec3(0.5f, 0.0f, -0.5f) * size, m_Target, ct::fvec2(1.0f, 0.0f), m_Vertices);
	AddVertex(m_Origin + ct::fvec3(0.5f, 0.0f, 0.5f) * size, m_Target, ct::fvec2(1.0f, 1.0f), m_Vertices);
	AddVertex(m_Origin + ct::fvec3(-0.5f, 0.0f, 0.5f) * size, m_Target, ct::fvec2(0.0f, 1.0f), m_Vertices);

	AddFace(0, 1, 2, m_TriFaces);
	AddFace(0, 2, 3, m_TriFaces);

	Subdivide(m_PlaneSubdivisionLevel, m_Vertices, m_TriFaces);

	BuildMesh();
}

void PrimitiveModule::CreateCube()
{
	BuildMesh();
}

void PrimitiveModule::CreateIcosaedre()
{
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
	AddFace(0, 11, 5, m_TriFaces);
	AddFace(0, 5, 1, m_TriFaces);
	AddFace(0, 1, 7, m_TriFaces);
	AddFace(0, 7, 10, m_TriFaces);
	AddFace(0, 10, 11, m_TriFaces);

	// 5 adjacent faces
	AddFace(1, 5, 9, m_TriFaces);
	AddFace(5, 11, 4, m_TriFaces);
	AddFace(11, 10, 2, m_TriFaces);
	AddFace(10, 7, 6, m_TriFaces);
	AddFace(7, 1, 8, m_TriFaces);

	// 5 faces around point 3
	AddFace(3, 9, 4, m_TriFaces);
	AddFace(3, 4, 2, m_TriFaces);
	AddFace(3, 2, 6, m_TriFaces);
	AddFace(3, 6, 8, m_TriFaces);
	AddFace(3, 8, 9, m_TriFaces);

	// 5 adjacent faces
	AddFace(4, 9, 5, m_TriFaces);
	AddFace(2, 4, 11, m_TriFaces);
	AddFace(6, 2, 10, m_TriFaces);
	AddFace(8, 6, 7, m_TriFaces);
	AddFace(9, 8, 1, m_TriFaces);

	// subdivide if needed (vSubdivLevel > 0))
	Subdivide(m_IcosaedreSubdivisionLevel, m_Vertices, m_TriFaces);

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
	if (m_Mode.index == TORUS_MODE_MAJOR_MINOR)
	{
		m_ExteriorRadius = m_MajorRadius + m_MinorRadius;
		m_InteriorRadius = m_MajorRadius - m_MinorRadius;
	}
	else if (m_Mode.index == TORUS_MODE_EXT_INT)
	{
		m_MinorRadius = (m_ExteriorRadius - m_InteriorRadius) * 0.5f;
		m_MajorRadius = m_InteriorRadius + m_MinorRadius;
	}

	CreateTorusMesh(m_MajorRadius, m_MinorRadius, m_MajorSegments, m_MinorSegments, m_SectionAngle * DEGTORAD, m_SectionTwist);

	if (m_GenerateUVS && m_MinorSegments)
	{
		if (m_SectionTwist % m_MinorSegments == 0)
		{
			AddTorusUVs(m_MinorSegments, m_MajorSegments);
		}
		else
		{
			AddTorusUVsOneRibbon(m_MinorSegments, m_MajorSegments, m_SectionTwist);
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

			AddVertex(ct::fvec3(vertex.x, vertex.y, vertex.z), ct::fvec3(), ct::fvec2(), m_Vertices);

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

			AddFace(face_indexs[0], face_indexs[2], face_indexs[3], face_indexs[1], m_QuadFaces);

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
					m_Vertices.at(face.v0).t = ct::fvec2(u_prev, v_prev);
					m_Vertices.at(face.v1).t = ct::fvec2(u_next, v_prev);
					m_Vertices.at(face.v2).t = ct::fvec2(u_next, v_next);
					m_Vertices.at(face.v3).t = ct::fvec2(u_prev, v_next);
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
					m_Vertices.at(face.v0).t = ct::fvec2(u_prev, 0.0f);
					m_Vertices.at(face.v1).t = ct::fvec2(u_next, 0.0f);
					m_Vertices.at(face.v2).t = ct::fvec2(u_next, 1.0f);
					m_Vertices.at(face.v3).t = ct::fvec2(u_prev, 1.0f);
				}
				u_prev = u_next;
			}
		}
		m_HaveTextureCoords = true;
	}
}	

void PrimitiveModule::AddVertex(const ct::fvec3& vP, const ct::fvec3& vN, const ct::fvec2& vUV, std::vector<Vertice>& vVertices)
{
	vVertices.emplace_back(vP, vN, 0.0f, 0.0f, vUV);
}

void PrimitiveModule::AddIcosaedreVertex(const ct::fvec3& vNormal, const ct::fvec3& vOrigin, const float& vRadius, std::vector<Vertice>& vVertices)
{
	vVertices.emplace_back(vOrigin + vNormal * vRadius, vNormal);
}

void PrimitiveModule::AddFace(const size_t& vV0, const size_t& vV1, const size_t& vV2, std::vector<TriFace>& vFaces)
{
	vFaces.emplace_back(vV0, vV1, vV2);
}

void PrimitiveModule::AddFace(const size_t& vV0, const size_t& vV1, const size_t& vV2, const size_t& vV3, std::vector<QuadFace>& vFaces)
{
	vFaces.emplace_back(vV0, vV1, vV2, vV3);
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

				std::vector<TriFace> subFaces;
				std::vector<Vertice> subVertices = m_Vertices;

				for (auto& face : m_TriFaces)
				{
					if (m_PrimitiveTypeIndex == PrimitiveTypeEnum::PRIMITIVE_TYPE_ICO_SPHERE)
					{
						a = GetMiddlePoint_Icosaedre(face.v0, face.v1, subVertices, middlePointIndexCache, m_IcosaedreRadius);
						b = GetMiddlePoint_Icosaedre(face.v1, face.v2, subVertices, middlePointIndexCache, m_IcosaedreRadius);
						c = GetMiddlePoint_Icosaedre(face.v2, face.v0, subVertices, middlePointIndexCache, m_IcosaedreRadius);
					}
					else if (m_PrimitiveTypeIndex == PrimitiveTypeEnum::PRIMITIVE_TYPE_PLANE)
					{
						a = GetMiddlePoint_Plane(face.v0, face.v1, subVertices, middlePointIndexCache, planeNormal);
						b = GetMiddlePoint_Plane(face.v1, face.v2, subVertices, middlePointIndexCache, planeNormal);
						c = GetMiddlePoint_Plane(face.v2, face.v0, subVertices, middlePointIndexCache, planeNormal);
					}

					if (m_PrimitiveTypeIndex == PrimitiveTypeEnum::PRIMITIVE_TYPE_ICO_SPHERE ||
						m_PrimitiveTypeIndex == PrimitiveTypeEnum::PRIMITIVE_TYPE_PLANE)
					{
						AddFace(face.v0, a, c, subFaces);
						AddFace(face.v1, b, a, subFaces);
						AddFace(face.v2, c, b, subFaces);
						AddFace(a, b, c, subFaces);
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

void PrimitiveModule::BuildMesh()
{
	if (!m_Vertices.empty())
	{
		VerticeArray vertices;
		IndiceArray indices;

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

		auto mesh_ptr = SceneMesh::Create(m_VulkanCorePtr, vertices, indices);
		if (mesh_ptr)
		{
			if (m_HaveTextureCoords)
			{
				mesh_ptr->HaveTextureCoords();
			}

			m_SceneModelPtr->clear();
			m_SceneModelPtr->Add(mesh_ptr);

			auto parentNodePtr = GetParentNode().getValidShared();
			if (parentNodePtr)
			{
				parentNodePtr->SendFrontNotification(ModelUpdateDone);
			}
		}
	}
}
