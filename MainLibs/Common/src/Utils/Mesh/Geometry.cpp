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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "Geometry.h"
#include <ImWidgets/ImWidgets.h>
#include <Utils/Mesh/MeshLoader.h>

namespace Geometry
{
	size_t Geometry::GetMiddlePoint_Icosaedre(size_t p1, size_t p2, std::vector<Vertex>* vVertices, std::map<std::tuple<size_t, size_t>, size_t>* vCache, float vRadius)
	{
		std::tuple<size_t, size_t> key(ct::mini(p1, p2), ct::maxi(p1, p2));

		if (vCache->find(key) != vCache->end())
		{
			return (*vCache)[key];
		}

		// not in cache, calculate it
		ct::fvec3 point1 = (*vVertices)[p1].p;
		ct::fvec3 point2 = (*vVertices)[p2].p;
		ct::fvec3 middle = (point1 + point2) * 0.5f;

		// add vertex makes sure point is on unit sphere
		size_t i = vVertices->size();
		vVertices->emplace_back(middle.GetNormalized() * vRadius, middle.GetNormalized());

		// store it, return index
		(*vCache)[key] = i;

		return i;
	}

	size_t Geometry::GetMiddlePoint_Plane(size_t p1, size_t p2, std::vector<Vertex>* vVertices, std::map<std::tuple<size_t, size_t>, size_t>* vCache, ct::fvec3 vNormal)
	{
		std::tuple<size_t, size_t> key(ct::mini(p1, p2), ct::maxi(p1, p2));

		if (vCache->find(key) != vCache->end())
		{
			return (*vCache)[key];
		}

		// not in cache, calculate it
		ct::fvec3 point1 = (*vVertices)[p1].p;
		ct::fvec3 point2 = (*vVertices)[p2].p;
		ct::fvec3 middle = (point1 + point2) * 0.5f;

		// add vertex makes sure point is on unit sphere
		size_t i = vVertices->size();
		vVertices->emplace_back(middle, vNormal);

		// store it, return index
		(*vCache)[key] = i;

		return i;
	}

	////////////////////////////////////////////////////////////

	Geometry::Geometry()
	{
	}

	Geometry::~Geometry()
	{
	}

	void Geometry::AddVertex(ct::fvec3 vP, ct::fvec3 vTarget)
	{
		vertices.emplace_back(vP, (vP - vTarget).GetNormalized());
	}

	void Geometry::CreateGeometry(bool vGenerateCellNetWork)
	{
		if (m_GeometryType == GeometryTypeEnum::GEOMETRY_TYPE_ICOSAEDRE)
		{
			CreateIcosaedre(m_GeometryIcosaedre.radius, vGenerateCellNetWork);
		}
		else if (m_GeometryType == GeometryTypeEnum::GEOMETRY_TYPE_PLANE)
		{
			CreatePlane(m_GeometryPlane.origin, m_GeometryPlane.target, m_GeometryPlane.size, vGenerateCellNetWork);
		}
		else if (m_GeometryType == GeometryTypeEnum::GEOMETRY_TYPE_CYLINDER)
		{
		}
		else if (m_GeometryType == GeometryTypeEnum::GEOMETRY_TYPE_CUBE)
		{
		}
		else if (m_GeometryType == GeometryTypeEnum::GEOMETRY_TYPE_TORUS)
		{
		}
		else if (m_GeometryType == GeometryTypeEnum::GEOMETRY_FROM_FILE)
		{
		}

		Subdivide(m_GeometryGeneral.subdivisionLvl, vGenerateCellNetWork);
	}

	void Geometry::CreateGeometryAsync(bool vGenerateCellNetWork) // version threadé
	{
		UNUSED(vGenerateCellNetWork);
	}

	void Geometry::CreateIcosaedre(float vRadius, int vSubdivLevel, bool vGenerateCellNetWork)
	{
		vertices.clear();
		faces.clear();
		fixedEdges.clear();

		m_GeometryType = GeometryTypeEnum::GEOMETRY_TYPE_ICOSAEDRE;
		m_GeometryIcosaedre.radius = vRadius;

		// create 12 vertices of a icosahedron
		float t = (1.0f + sqrtf(5.0f)) / 2.0f;

		AddVertex(ct::fvec3(-1.0f, t, 0.0f).GetNormalized() * vRadius, 0.0f);
		AddVertex(ct::fvec3(1.0f, t, 0.0f).GetNormalized() * vRadius, 0.0f);
		AddVertex(ct::fvec3(-1.0f, -t, 0.0f).GetNormalized() * vRadius, 0.0f);
		AddVertex(ct::fvec3(1.0f, -t, 0.0f).GetNormalized() * vRadius, 0.0f);

		AddVertex(ct::fvec3(0.0f, -1.0f, t).GetNormalized() * vRadius, 0.0f);
		AddVertex(ct::fvec3(0.0f, 1.0f, t).GetNormalized() * vRadius, 0.0f);
		AddVertex(ct::fvec3(0.0f, -1.0f, -t).GetNormalized() * vRadius, 0.0f);
		AddVertex(ct::fvec3(0.0f, 1.0f, -t).GetNormalized() * vRadius, 0.0f);

		AddVertex(ct::fvec3(t, 0.0f, -1.0f).GetNormalized() * vRadius, 0.0f);
		AddVertex(ct::fvec3(t, 0.0f, 1.0f).GetNormalized() * vRadius, 0.0f);
		AddVertex(ct::fvec3(-t, 0.0f, -1.0f).GetNormalized() * vRadius, 0.0f);
		AddVertex(ct::fvec3(-t, 0.0f, 1.0f).GetNormalized() * vRadius, 0.0f);

		// create 20 triangles of the icosahedron

		// 5 faces around point 0
		AddFace(0, 11, 5, &faces, vGenerateCellNetWork);
		AddFace(0, 5, 1, &faces, vGenerateCellNetWork);
		AddFace(0, 1, 7, &faces, vGenerateCellNetWork);
		AddFace(0, 7, 10, &faces, vGenerateCellNetWork);
		AddFace(0, 10, 11, &faces, vGenerateCellNetWork);

		// 5 adjacent faces
		AddFace(1, 5, 9, &faces, vGenerateCellNetWork);
		AddFace(5, 11, 4, &faces, vGenerateCellNetWork);
		AddFace(11, 10, 2, &faces, vGenerateCellNetWork);
		AddFace(10, 7, 6, &faces, vGenerateCellNetWork);
		AddFace(7, 1, 8, &faces, vGenerateCellNetWork);

		// 5 faces around point 3
		AddFace(3, 9, 4, &faces, vGenerateCellNetWork);
		AddFace(3, 4, 2, &faces, vGenerateCellNetWork);
		AddFace(3, 2, 6, &faces, vGenerateCellNetWork);
		AddFace(3, 6, 8, &faces, vGenerateCellNetWork);
		AddFace(3, 8, 9, &faces, vGenerateCellNetWork);

		// 5 adjacent faces
		AddFace(4, 9, 5, &faces, vGenerateCellNetWork);
		AddFace(2, 4, 11, &faces, vGenerateCellNetWork);
		AddFace(6, 2, 10, &faces, vGenerateCellNetWork);
		AddFace(8, 6, 7, &faces, vGenerateCellNetWork);
		AddFace(9, 8, 1, &faces, vGenerateCellNetWork);

		// subdivide if needed (needed == (vSubdivLevel > 0))
		Subdivide(vSubdivLevel, vGenerateCellNetWork);
	}

	void Geometry::CreatePlane(ct::fvec3 vOrigin, ct::fvec3 vTarget, ct::fvec2 vSize, int vSubdivLevel, bool vGenerateCellNetWork)
	{
		vertices.clear();
		faces.clear();
		fixedEdges.clear();

		m_GeometryType = GeometryTypeEnum::GEOMETRY_TYPE_PLANE;
		m_GeometryPlane.origin = vOrigin;
		m_GeometryPlane.target = vTarget;
		m_GeometryPlane.size = vSize;

		ct::fvec3 size = ct::fvec3(m_GeometryPlane.size.x, 0.0f, m_GeometryPlane.size.y);
		ct::fvec3 org = m_GeometryPlane.origin;
		ct::fvec3 tgt = m_GeometryPlane.target;

		AddVertex(ct::fvec3(-0.5f, 0.0f, -0.5f) * size + org, ct::fvec3(-0.5f, 0.0f, -0.5f) * size + tgt);
		AddVertex(ct::fvec3(0.5f, 0.0f, -0.5f) * size + org, ct::fvec3(0.5f, 0.0f, -0.5f) * size + tgt);
		AddVertex(ct::fvec3(0.5f, 0.0f, 0.5f) * size + org, ct::fvec3(0.5f, 0.0f, 0.5f) * size + tgt);
		AddVertex(ct::fvec3(-0.5f, 0.0f, 0.5f) * size + org, ct::fvec3(-0.5f, 0.0f, 0.5f) * size + tgt);

		AddFace(0, 1, 2, &faces, vGenerateCellNetWork);
		AddFace(0, 2, 3, &faces, vGenerateCellNetWork);

		// subdivide if needed (needed == (vSubdivLevel > 0))
		Subdivide(vSubdivLevel, vGenerateCellNetWork);
	}

	void Geometry::Subdivide(int vSubdivLevel, bool vGenerateCellNetWork)
	{
		if (vSubdivLevel > 0)
		{
			std::map<std::tuple<size_t, size_t>, size_t> middlePointIndexCache;

			ct::fvec3 planeNormal;

			if (m_GeometryType == GeometryTypeEnum::GEOMETRY_TYPE_PLANE)
			{
				planeNormal = (m_GeometryPlane.origin - m_GeometryPlane.target).GetNormalized();
			}
			// refine triangles
			for (int i = 0; i < vSubdivLevel; ++i)
			{
				fixedEdges.clear();

				size_t a = 0, b = 0, c = 0;

				std::vector<Face> subFaces;
				for (auto& tri : faces)
				{
					// replace triangle by 4 triangles
					if (m_GeometryType == GeometryTypeEnum::GEOMETRY_TYPE_ICOSAEDRE)
					{
						a = GetMiddlePoint_Icosaedre(tri.v1, tri.v2, &vertices, &middlePointIndexCache, m_GeometryIcosaedre.radius);
						b = GetMiddlePoint_Icosaedre(tri.v2, tri.v3, &vertices, &middlePointIndexCache, m_GeometryIcosaedre.radius);
						c = GetMiddlePoint_Icosaedre(tri.v3, tri.v1, &vertices, &middlePointIndexCache, m_GeometryIcosaedre.radius);
					}
					else if (m_GeometryType == GeometryTypeEnum::GEOMETRY_TYPE_PLANE)
					{
						a = GetMiddlePoint_Plane(tri.v1, tri.v2, &vertices, &middlePointIndexCache, planeNormal);
						b = GetMiddlePoint_Plane(tri.v2, tri.v3, &vertices, &middlePointIndexCache, planeNormal);
						c = GetMiddlePoint_Plane(tri.v3, tri.v1, &vertices, &middlePointIndexCache, planeNormal);
					}

					if (m_GeometryType == GeometryTypeEnum::GEOMETRY_TYPE_ICOSAEDRE ||
						m_GeometryType == GeometryTypeEnum::GEOMETRY_TYPE_PLANE)
					{
						AddFace(tri.v1, a, c, &subFaces, vGenerateCellNetWork);
						AddFace(tri.v2, b, a, &subFaces, vGenerateCellNetWork);
						AddFace(tri.v3, c, b, &subFaces, vGenerateCellNetWork);
						AddFace(a, b, c, &subFaces, vGenerateCellNetWork);
					}
				}
				faces = subFaces;
			}
		}
	}

	void Geometry::CalcNormal(
		VertexStruct::P3_N3_TA3_BTA3_T2_C4& v0,
		VertexStruct::P3_N3_TA3_BTA3_T2_C4& v1,
		VertexStruct::P3_N3_TA3_BTA3_T2_C4& v2)
	{
		ct::fvec3 vec0, vec1, nor;

		vec0.x = v2.p.x - v0.p.x;
		vec0.y = v2.p.y - v0.p.y;
		vec0.z = v2.p.z - v0.p.z;
		vec0.normalize();

		vec1.x = v1.p.x - v0.p.x;
		vec1.y = v1.p.y - v0.p.y;
		vec1.z = v1.p.z - v0.p.z;
		vec1.normalize();

		// un produit vectoriel de deux vecteur normaux est normalisé !?
		// je sais pas donc dans le doute je le re normalize
		nor = cCross(vec1, vec0).GetNormalized();

		float d0 = 1.0f; //ct::sign(v0.d);
		float d1 = 1.0f; //ct::sign(v1.d);
		float d2 = 1.0f; //ct::sign(v2.d);

		v0.n.x += (float)(nor.x * d0);
		v0.n.y += (float)(nor.y * d0);
		v0.n.z += (float)(nor.z * d0);

		v1.n.x += (float)(nor.x * d1);
		v1.n.y += (float)(nor.y * d1);
		v1.n.z += (float)(nor.z * d1);

		v2.n.x += (float)(nor.x * d2);
		v2.n.y += (float)(nor.y * d2);
		v2.n.z += (float)(nor.z * d2);
	}

	void Geometry::AddFace(size_t vV0, size_t vV1, size_t vV2, std::vector<Face>* vFaces, bool vGenerateCellNetWork)
	{
		//size_t faceId = vFaces->size();

		vFaces->emplace_back(vV0, vV1, vV2);

		if (vGenerateCellNetWork)
		{
			AddEdge(vV0, vV1);
			AddEdge(vV0, vV2);
			AddEdge(vV1, vV0);
			AddEdge(vV1, vV2);
			AddEdge(vV2, vV0);
			AddEdge(vV2, vV1);
		}
	}

	void Geometry::AddEdge(size_t vV0, size_t vV1)
	{
		fixedEdges[vV0][vV1] = false;
	}

	GeometryDataStruct Geometry::GetDatas()
	{
		GeometryDataStruct res;

		if (m_GeometryType == GeometryTypeEnum::GEOMETRY_FROM_FILE)
		{
			res.vertexs = MeshLoader::Instance()->GetMeshDatas()->GetVertexs();
			res.indexs = MeshLoader::Instance()->GetMeshDatas()->GetIndexs();
		}
		else
		{
			ct::fvec3 e3; // empty fvec3
			ct::fvec2 e2; // empty fvec2
			for (auto& v : vertices)
			{
				res.vertexs.push_back(VertexStruct::P3_N3_TA3_BTA3_T2_C4(v.p, v.n, e3, e3, e2, v.c));
			}

			for (auto& f : faces)
			{
				res.indexs.push_back(VertexStruct::I1(f.v1));
				res.indexs.push_back(VertexStruct::I1(f.v2));
				res.indexs.push_back(VertexStruct::I1(f.v3));

				VertexStruct::P3_N3_TA3_BTA3_T2_C4& v0 = res.vertexs.at(f.v1);
				VertexStruct::P3_N3_TA3_BTA3_T2_C4& v1 = res.vertexs.at(f.v2);
				VertexStruct::P3_N3_TA3_BTA3_T2_C4& v2 = res.vertexs.at(f.v3);

				CalcNormal(v0, v1, v2);
			}

			for (auto& v : res.vertexs)
			{
				v.n = v.n.GetNormalized();
			}
		}

		return res;
	}

	void Geometry::ResetTimeFrameFile()
	{
		FILE* fp = nullptr;
		errno_t lastError = fopen_s(&fp, "sketchfab.timeframe", "wt"); // write new file mode test
		if (lastError == 0)
		{
			if (fp)
			{
				fclose(fp);
			}
		}
	}

	void Geometry::AddTimeFrameToFile(float vTimeFrame, std::string vPrefix, int vFrame)
	{
		FILE* fp = nullptr;
		errno_t lastError = fopen_s(&fp, "sketchfab.timeframe", "at"); // append to end of file mode text
		if (lastError == 0)
		{
			char buffer[1024];
			int n = snprintf(buffer, 1024, "%.5f %s%05d.obj\n", vTimeFrame, vPrefix.c_str(), vFrame);

			if (fp)
			{
				fwrite(&buffer, sizeof(char), n, fp);

				fclose(fp);
			}
		}
	}

	void Geometry::SaveToObjFile(std::string vPrefix, int vFrame)
	{
		FILE* fp = nullptr;
		char fileNameBuffer[1024];
		snprintf(fileNameBuffer, 1024, "%s%05d.obj", vPrefix.c_str(), vFrame);
		errno_t lastError = fopen_s(&fp, fileNameBuffer, "wb");
		if (lastError == 0 && fp)
		{
			char vertBuffer[1024];
			char norBuffer[1024];
			char faceBuffer[1024];

			int n = 0;

			//vertexs
			for (auto& v : vtxs)
			{
				n = snprintf(vertBuffer, 1024, "v %.5f %.5f %.5f\n", v.p.x, v.p.y, v.p.z);

				fwrite(&vertBuffer, sizeof(char), n, fp);
			}

			//normals
			for (auto& v : vtxs)
			{
				n = snprintf(norBuffer, 1024, "vn %.5f %.5f %.5f\n", v.n.x, v.n.y, v.n.z);

				fwrite(&norBuffer, sizeof(char), n, fp);
			}

			//faces
			for (auto f : faces)
			{
				// le format est v/vt/vn pour chaque point de la face
				// donc si on avait des normales de faces, on aurait le meme id vn pour les 3 points
				// sinon un id par points
				n = snprintf(faceBuffer, 1024, "f %I64u//%I64u %I64u//%I64u %I64u//%I64u\n",
					f.v1 + 1, f.v1 + 1,
					f.v2 + 1, f.v2 + 1,
					f.v3 + 1, f.v3 + 1
				);

				fwrite(&faceBuffer, sizeof(char), n, fp);
			}

			fclose(fp);
		}
	}

	MeshFrameStruct	Geometry::GetMeshFrame(int vFrame)
	{
		MeshFrameStruct frame;

		frame.datas.vertexs = vtxs;
		frame.datas.indexs = inds;
		frame.frame = vFrame;
		frame.isAbsolute = true;

		return frame;
	}

	bool Geometry::drawImGui()
	{
		bool change = false;

		if (ImGui::Combo("Geometry Type##GEOMETRY_TYPE",
			&m_VaccumGeometryTypeArrayIndex, m_VaccumGeometryTypeComboString, -1))
		{
			m_GeometryType = (Geometry::Geometry::GeometryTypeEnum)m_VaccumGeometryTypeArrayIndex;
			change = true;
		}

		if (m_GeometryType == Geometry::Geometry::GeometryTypeEnum::GEOMETRY_TYPE_ICOSAEDRE)
		{
			change |= ImGui::SliderFloatDefault(200, "Radius", &m_GeometryIcosaedre.radius, 0.0f, 10.0f, 3.0f);
		}
		else if (m_GeometryType == Geometry::Geometry::GeometryTypeEnum::GEOMETRY_TYPE_PLANE)
		{
			change |= ImGui::SliderFloatDefault(200, "Width", &m_GeometryPlane.size.x, 0.0f, 10.0f, 3.0f);
			change |= ImGui::SliderFloatDefault(200, "Height", &m_GeometryPlane.size.y, 0.0f, 10.0f, 3.0f);
			change |= ImGui::SliderFloatDefault(200, "Origin X", &m_GeometryPlane.origin.x, 0.0f, 10.0f, 0.0f);
			change |= ImGui::SliderFloatDefault(200, "Origin Y", &m_GeometryPlane.origin.y, 0.0f, 10.0f, 3.0f);
			change |= ImGui::SliderFloatDefault(200, "Origin Z", &m_GeometryPlane.origin.z, 0.0f, 10.0f, 0.0f);
			change |= ImGui::SliderFloatDefault(200, "Target X", &m_GeometryPlane.target.x, 0.0f, 10.0f, 0.0f);
			change |= ImGui::SliderFloatDefault(200, "Target Y", &m_GeometryPlane.target.y, 0.0f, 10.0f, 0.0f);
			change |= ImGui::SliderFloatDefault(200, "Target Z", &m_GeometryPlane.target.z, 0.0f, 10.0f, 0.0f);
		}
		else if (m_GeometryType == Geometry::Geometry::GeometryTypeEnum::GEOMETRY_TYPE_CYLINDER)
		{
		}
		else if (m_GeometryType == Geometry::Geometry::GeometryTypeEnum::GEOMETRY_TYPE_CUBE)
		{
		}
		else if (m_GeometryType == Geometry::Geometry::GeometryTypeEnum::GEOMETRY_TYPE_TORUS)
		{
		}
		else if (m_GeometryType == GeometryTypeEnum::GEOMETRY_FROM_FILE)
		{
			if (ImGui::ContrastedButton("Open Model 3D"))
			{
				//MeshLoader::Instance()->OpenDialog();
			}
			float aw = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x;
			MeshLoader::Instance()->DrawImGuiProgress(aw);
			MeshLoader::Instance()->DrawImGuiInfos(aw);
		}

		ImGui::Separator();

		if (m_GeometryType != GeometryTypeEnum::GEOMETRY_FROM_FILE)
		{
			change |= ImGui::SliderIntDefault(200, "Subdivision", &m_GeometryGeneral.subdivisionLvl, 0, 7, 2);
		}

		ImGui::Separator();

		ImGui::Text("Stats : Vertexs(%zu) Indexs(%zu)", vertices.size(), faces.size() * 3);

		return change;
	}

	bool Geometry::DrawDialogsAndPopups(ct::ivec2 vScreenSize)
	{
		bool change = false;

		MeshLoader::Instance()->ShowDialog(vScreenSize);

		return change;
	}
}