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

#include <ctools/cTools.h>
#include <Utils/Mesh/VertexStruct.h>

#include <future>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>

namespace Geometry
{
	struct Face
	{
		size_t v1;
		size_t v2;
		size_t v3;

		Face(size_t vV1, size_t vV2, size_t vV3)
		{
			v1 = vV1;
			v2 = vV2;
			v3 = vV3;
		}
	};

	struct Vertex
	{
		ct::fvec3 p;
		ct::fvec3 n;
		ct::fvec4 c;
		float d;
		bool fixed;
		size_t lvl;
		ct::fvec2 texel;

		Vertex()
		{
			fixed = false;
			lvl = 0;
			texel = 0.0f;
			d = 0.0f;
		}

		Vertex(ct::fvec3 vP, ct::fvec3 vN = 0.0f)
		{
			p = vP;
			n = vN;
			fixed = false;
			lvl = 0;
			texel = 0.0f;
			d = 0.0f;
		}
	};

	struct GeometryDataStruct
	{
		std::vector<VertexStruct::P3_N3_TA3_BTA3_T2_C4> vertexs;
		std::vector<VertexStruct::I1> indexs;
	};

	struct MeshFrameStruct
	{
		int frame = 0;
		bool isAbsolute = true;
		GeometryDataStruct datas;
	};

	class Geometry
	{
	public:
		enum class GeometryTypeEnum : uint8_t
		{
			GEOMETRY_TYPE_ICOSAEDRE = 0,
			GEOMETRY_TYPE_PLANE,
			GEOMETRY_TYPE_CYLINDER,
			GEOMETRY_TYPE_CUBE,
			GEOMETRY_TYPE_TORUS,
			GEOMETRY_FROM_FILE,
			GEOMETRY_TYPE_Count
		};

		struct GeometryGeneralStruct
		{
			int subdivisionLvl = 2;
		};

		struct GeometryIcosaedreStruct
		{
			float radius = 3.0f;
		};

		struct GeometryPlaneStruct
		{
			ct::fvec2 size = 3.0f;
			ct::fvec3 origin = ct::fvec3(0, 3, 0);
			ct::fvec3 target = 0.0f;
		};

	private:
		GeometryTypeEnum m_GeometryType = GeometryTypeEnum::GEOMETRY_TYPE_ICOSAEDRE;
		GeometryGeneralStruct m_GeometryGeneral;
		GeometryIcosaedreStruct m_GeometryIcosaedre;
		GeometryPlaneStruct m_GeometryPlane;

	private:
		int m_VaccumGeometryTypeArrayIndex = 0;
		const char* m_VaccumGeometryTypeComboString = "ICOSAEDRE\0PLANE\0CYLINDER\0CUBE\0TORUS\0FROM FILE\0\0";

	private:
		std::vector<VertexStruct::P3_N3_TA3_BTA3_T2_C4> vtxs;
		std::vector<VertexStruct::I1> inds;

	public:
		std::vector<Vertex> vertices;
		std::vector<Face> faces;
		std::map<size_t, std::map<size_t, bool>> fixedEdges;

	public:
		static size_t GetMiddlePoint_Icosaedre(size_t p1, size_t p2, std::vector<Vertex>* vVertices, std::map<std::tuple<size_t, size_t>, size_t>* vCache, float vRadius);
		static size_t GetMiddlePoint_Plane(size_t p1, size_t p2, std::vector<Vertex>* vVertices, std::map<std::tuple<size_t, size_t>, size_t>* vCache, ct::fvec3 vNormal);

	public:
		Geometry();
		~Geometry();

	public:
		void CreateGeometry(bool vGenerateCellNetWork = false);
		void CreateGeometryAsync(bool vGenerateCellNetWork = false);
		void CreateIcosaedre(float vRadius, int vSubdivLevel = 0, bool vGenerateCellNetWork = false);
		void CreatePlane(ct::fvec3 vOrigin, ct::fvec3 vTarget, ct::fvec2 vSize, int vSubdivLevel = 0, bool vGenerateCellNetWork = false);
		void Subdivide(int vSubdivLevel, bool vGenerateCellNetWork);
		GeometryDataStruct GetDatas();
		bool DrawDialogsAndPopups(ct::ivec2 vScreenSize);

	public: // animated sketchfab model
		void ResetTimeFrameFile();
		void AddTimeFrameToFile(float vTimeFrame, std::string vPrefix, int vFrame);
		void SaveToObjFile(std::string vPrefix, int vFrame);
		MeshFrameStruct GetMeshFrame(int vFrame);

	public: // imgui
		bool drawImGui();

	private:
		void AddVertex(ct::fvec3 vP, ct::fvec3 vTarget);
		void AddFace(size_t vV0, size_t vV1, size_t vV2, std::vector<Face>* vFaces, bool vGenerateCellNetWork);
		void AddEdge(size_t vV0, size_t vV1);
		void CalcNormal(
			VertexStruct::P3_N3_TA3_BTA3_T2_C4& v0,
			VertexStruct::P3_N3_TA3_BTA3_T2_C4& v1,
			VertexStruct::P3_N3_TA3_BTA3_T2_C4& v2);
	};
}