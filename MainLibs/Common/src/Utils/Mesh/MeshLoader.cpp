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

#include "MeshLoader.h"

#include <imgui/imgui.h>
#include <ctools/FileHelper.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <ImWidgets/ImWidgets.h>
#include <ctools/Logger.h>

#define fileSeek fseek
#define fileTell ftell

#include <imgui/imgui.h>								// https://github.com/ocornut/imgui
#ifndef IMGUI_DEFINE_MATH_OPERATORS

#endif
#include <imgui/imgui_internal.h>

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

std::atomic<double> MeshLoader::Progress(0.0);
std::atomic<bool> MeshLoader::Working(false);
std::atomic<double> MeshLoader::GenerationTime(0.0);
std::mutex MeshLoader::MeshLoader_Mutex;

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

inline size_t GetVertexId(size_t vVertexId, const size_t* v, const size_t* vn, const size_t* vt)
{
	// vCvVertexIdhan va de 0 à 3 => 0:v0, 1:v1, 2:v2, 3:v3
	// (sachant que le obj ne support pas des face de plus de 4 vertexs)

	auto& vertexStamps = MeshLoader::Instance()->m_MeshDatas.GetVertexStampsAdr();

	size_t id = v[vVertexId], curId = id;
	size_t idt = 0; if (vt) idt = vt[vVertexId];
	size_t idn = 0; if (vn) idn = vn[vVertexId];

	if (vertexStamps.find(id) !=
		vertexStamps.end()) // trouvé
	{
		if (vertexStamps[id].find(idt) !=
			vertexStamps[id].end())  // trouvé
		{
			if (vertexStamps[id][idt].find(idn) !=
				vertexStamps[id][idt].end())  // trouvé
			{
				id = vertexStamps[curId][idt][idn];
			}
			else // non trouvé, on va devoir dupliquer le vertex (car il a deja ete utilsié lui, et ces vn et vt seront differentes)
			{
				// on duplique le vertex
				// car les valeurs de normales et/ou coords seront differente
				id = MeshLoader::Instance()->m_MeshDatas.DuplicateVertex(id);
				// on ajoute la signature avec l'index
				vertexStamps[curId][idt][idn] = id;
			}
		}
		else // non trouvé, on va devoir dupliquer le vertex (car il a deja ete utilsié lui, et ces vn et vt seront differentes)
		{
			// on duplique le vertex
			// car les valeurs de normales et/ou coords seront differente
			id = MeshLoader::Instance()->m_MeshDatas.DuplicateVertex(id);
			// on ajoute la signature avec l'index
			vertexStamps[curId][idt][idn] = id;
		}
	}
	else
	{
		// on ajoute la signature avec l'index
		vertexStamps[curId][idt][idn] = id;
	}

	if (id > 0)
		return id - 1;
	return 0;
}

//https://learnopengl.com/Advanced-Lighting/Normal-Mapping
inline void ComputeRendererTangentAndBiTangentForPoint(
	VertexStruct::P3_N3_TA3_BTA3_T2_C4* vV0,
	VertexStruct::P3_N3_TA3_BTA3_T2_C4* vV1,
	VertexStruct::P3_N3_TA3_BTA3_T2_C4* vV2)
{
	if (vV0 && vV1 && vV2)
	{
		// tangent, bi-tangent calcul
		ct::fvec3 edge1 = vV1->p - vV0->p;
		ct::fvec3 edge2 = vV2->p - vV1->p;
		ct::fvec2 deltaUV1 = vV1->t - vV0->t;
		ct::fvec2 deltaUV2 = vV2->t - vV0->t;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		ct::fvec3 tan;
		tan.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tan.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tan.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tan = tan.GetNormalized();
		vV0->tan = tan;
		vV1->tan = tan;
		vV2->tan = tan;

		ct::fvec3 btan;
		btan.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		btan.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		btan.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		btan = btan.GetNormalized();
		vV0->btan = btan;
		vV1->btan = btan;
		vV2->btan = btan;
	}
}

// re compute normals
inline void AddTriangleAndComputeRendererNormals(
	size_t vI0, size_t vI1, size_t vI2,
	const size_t* v)
{
	MeshLoader::MeshLoader_Mutex.lock();

	VertexStruct::P3_N3_TA3_BTA3_T2_C4* v0 = 0, * v1 = 0, * v2 = 0;

	size_t id0 = GetVertexId(vI0, v, 0, 0);
	size_t id1 = GetVertexId(vI1, v, 0, 0);
	size_t id2 = GetVertexId(vI2, v, 0, 0);

	size_t n = MeshLoader::Instance()->m_MeshDatas.GetCountVertexs();

	if (id0 < n) v0 = MeshLoader::Instance()->m_MeshDatas.GetVertex(id0);
	if (id1 < n) v1 = MeshLoader::Instance()->m_MeshDatas.GetVertex(id1);
	if (id2 < n) v2 = MeshLoader::Instance()->m_MeshDatas.GetVertex(id2);

	if (v0 && v1 && v2)
	{
		ct::fvec3 vec0, vec1;

		vec0 = v0->p - v1->p;
		vec0.normalize();

		vec1 = v0->p - v2->p;
		vec1.normalize();

		ct::fvec3 nor = cCross(vec0, vec1).GetNormalized();

		v0->n += nor;
		v1->n += nor;
		v2->n += nor;
		MeshLoader::Instance()->m_MeshDatas.AddIndex(id0);
		MeshLoader::Instance()->m_MeshDatas.AddIndex(id1);
		MeshLoader::Instance()->m_MeshDatas.AddIndex(id2);
	}

	MeshLoader::MeshLoader_Mutex.unlock();
}

inline void AddTriangleWithTexCoordsAndComputeRendererNormals(
	size_t vI0, size_t vI1, size_t vI2,
	const std::vector<ct::fvec2>& vTexCoords,
	const size_t* v, const size_t* vt)
{
	MeshLoader::MeshLoader_Mutex.lock();

	VertexStruct::P3_N3_TA3_BTA3_T2_C4* v0 = 0, * v1 = 0, * v2 = 0;

	size_t id0 = GetVertexId(vI0, v, 0, vt);
	size_t id1 = GetVertexId(vI1, v, 0, vt);
	size_t id2 = GetVertexId(vI2, v, 0, vt);

	size_t n = MeshLoader::Instance()->m_MeshDatas.GetCountVertexs();

	if (id0 < n) v0 = MeshLoader::Instance()->m_MeshDatas.GetVertex(id0);
	if (id1 < n) v1 = MeshLoader::Instance()->m_MeshDatas.GetVertex(id1);
	if (id2 < n) v2 = MeshLoader::Instance()->m_MeshDatas.GetVertex(id2);

	if (v0 && v1 && v2)
	{
		ct::fvec3 vec0, vec1;

		vec0 = v0->p - v1->p;
		vec0.normalize();

		vec1 = v0->p - v2->p;
		vec1.normalize();

		ct::fvec3 nor = cCross(vec0, vec1).GetNormalized();

		v0->n += nor;
		v1->n += nor;
		v2->n += nor;

		// texcoords

		size_t countTexCoords = vTexCoords.size() + 1; // pour eviter de faire if(vt[vI0]-1 < vTexCoords.size()) a chaque fois
		if (vt[vI0] > 0 && vt[vI0] < countTexCoords)
		{
			v0->t = vTexCoords[vt[vI0] - 1];
		}
		if (vt[vI1] > 0 && vt[vI1] < countTexCoords)
		{
			v1->t = vTexCoords[vt[vI1] - 1];
		}
		if (vt[vI2] > 0 && vt[vI2] < countTexCoords)
		{
			v2->t = vTexCoords[vt[vI2] - 1];
		}

		ComputeRendererTangentAndBiTangentForPoint(v0, v1, v2);

		MeshLoader::Instance()->m_MeshDatas.AddIndex(id0);
		MeshLoader::Instance()->m_MeshDatas.AddIndex(id1);
		MeshLoader::Instance()->m_MeshDatas.AddIndex(id2);
	}
	MeshLoader::MeshLoader_Mutex.unlock();
}

inline void AddTriangleWithNormals(
	size_t vI0, size_t vI1, size_t vI2,
	const std::vector<ct::fvec3>& vNormals,
	const size_t* v, const size_t* vn)
{
	MeshLoader::MeshLoader_Mutex.lock();

	VertexStruct::P3_N3_TA3_BTA3_T2_C4* v0 = 0, * v1 = 0, * v2 = 0;

	size_t id0 = GetVertexId(vI0, v, vn, 0);
	size_t id1 = GetVertexId(vI1, v, vn, 0);
	size_t id2 = GetVertexId(vI2, v, vn, 0);

	size_t n = MeshLoader::Instance()->m_MeshDatas.GetCountVertexs();

	if (id0 < n) v0 = MeshLoader::Instance()->m_MeshDatas.GetVertex(id0);
	if (id1 < n) v1 = MeshLoader::Instance()->m_MeshDatas.GetVertex(id1);
	if (id2 < n) v2 = MeshLoader::Instance()->m_MeshDatas.GetVertex(id2);

	if (v0 && v1 && v2)
	{
		size_t countNormals = vNormals.size() + 1; // pour eviter de faire if(vn[vI0]-1 < vNormals.size()) a chaque fois
		if (vn[vI0] > 0 && vn[vI0] < countNormals)
		{
			v0->n = vNormals[vn[vI0] - 1];
		}
		if (vn[vI1] > 0 && vn[vI1] < countNormals)
		{
			v1->n = vNormals[vn[vI1] - 1];
		}
		if (vn[vI2] > 0 && vn[vI2] < countNormals)
		{
			v2->n = vNormals[vn[vI2] - 1];
		}

		MeshLoader::Instance()->m_MeshDatas.AddIndex(id0);
		MeshLoader::Instance()->m_MeshDatas.AddIndex(id1);
		MeshLoader::Instance()->m_MeshDatas.AddIndex(id2);
	}

	MeshLoader::MeshLoader_Mutex.unlock();
}

inline void AddTriangleWithNormalsAndTexCoords(
	size_t vI0, size_t vI1, size_t vI2,
	const std::vector<ct::fvec3>& vNormals, const std::vector<ct::fvec2>& vTexCoords,
	const size_t* v, const size_t* vn, const size_t* vt)
{
	MeshLoader::MeshLoader_Mutex.lock();

	VertexStruct::P3_N3_TA3_BTA3_T2_C4
		* v0 = 0,
		* v1 = 0,
		* v2 = 0;

	size_t id0 = GetVertexId(vI0, v, vn, vt);
	size_t id1 = GetVertexId(vI1, v, vn, vt);
	size_t id2 = GetVertexId(vI2, v, vn, vt);

	size_t n = MeshLoader::Instance()->m_MeshDatas.GetCountVertexs();

	if (id0 < n) v0 = MeshLoader::Instance()->m_MeshDatas.GetVertex(id0);
	if (id1 < n) v1 = MeshLoader::Instance()->m_MeshDatas.GetVertex(id1);
	if (id2 < n) v2 = MeshLoader::Instance()->m_MeshDatas.GetVertex(id2);

	if (v0 && v1 && v2)
	{
		// normals
		size_t countNormals = vNormals.size() + 1; // pour eviter de faire if(vn[vI0]-1 < vNormals.size()) a chaque fois
		if (vn[vI0] > 0 && vn[vI0] < countNormals)
		{
			v0->n = vNormals[vn[vI0] - 1];
		}
		if (vn[vI1] > 0 && vn[vI1] < countNormals)
		{
			v1->n = vNormals[vn[vI1] - 1];
		}
		if (vn[vI2] > 0 && vn[vI2] < countNormals)
		{
			v2->n = vNormals[vn[vI2] - 1];
		}

		// texcoords
		size_t countTexCoords = vTexCoords.size() + 1; // pour eviter de faire if(vt[vI0]-1 < vTexCoords.size()) a chaque fois
		if (vt[vI0] > 0 && vt[vI0] < countTexCoords)
		{
			v0->t = vTexCoords[vt[vI0] - 1];
		}
		if (vt[vI1] > 0 && vt[vI1] < countTexCoords)
		{
			v1->t = vTexCoords[vt[vI1] - 1];
		}
		if (vt[vI2] > 0 && vt[vI2] < countTexCoords)
		{
			v2->t = vTexCoords[vt[vI2] - 1];
		}

		ComputeRendererTangentAndBiTangentForPoint(v0, v1, v2);

		MeshLoader::Instance()->m_MeshDatas.AddIndex(id0);
		MeshLoader::Instance()->m_MeshDatas.AddIndex(id1);
		MeshLoader::Instance()->m_MeshDatas.AddIndex(id2);
	}

	MeshLoader::MeshLoader_Mutex.unlock();
}

void MeshObjLoader(
	std::atomic< double >& vProgress,
	std::atomic< bool >& vWorking,
	std::atomic< double >& vGenerationTime)
{
	vProgress = 0.0;
	vWorking = true;
	vGenerationTime = 0.0;

	MeshLoader::MeshLoader_Mutex.lock();
	std::string filePathName = MeshLoader::Instance()->fileToLoad;
	MeshLoader::Instance()->Clear();
	MeshLoader::Instance()->m_Layouts.emplace_back("Vertex     (v3) => Empty");
	MeshLoader::Instance()->m_Layouts.emplace_back("Normal     (v3) => Empty");
	MeshLoader::Instance()->m_Layouts.emplace_back("Tangent    (v3) => Empty");
	MeshLoader::Instance()->m_Layouts.emplace_back("Bi-Tangent (v3) => Empty");
	MeshLoader::Instance()->m_Layouts.emplace_back("Tex Coord  (v2) => Empty");
	MeshLoader::Instance()->m_Layouts.emplace_back("Color      (v4) => Empty");
	MeshLoader::MeshLoader_Mutex.unlock();

	if (filePathName.size() > 0)
	{
		std::string file_string_to_parse = FileHelper::Instance()->LoadFileToString(filePathName);
		std::istringstream file_to_parse = std::istringstream(file_string_to_parse);
		if (!file_string_to_parse.empty() && file_to_parse.good())
		{
			size_t fileSize = file_string_to_parse.size();
			size_t filePos = 0U;

			ct::fvec3 p;
			ct::fvec3 n;
			ct::fvec2 t;

			size_t v[4];
			size_t vt[4];
			size_t vn[4];

			std::vector<ct::fvec3> normals;
			std::vector<ct::fvec2> texcoords;

			bool needNormalGeneration = false;

			std::string line_to_parse;
			while (vWorking && std::getline(file_to_parse, line_to_parse))
			{
				uint64_t firstTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
					(std::chrono::system_clock::now().time_since_epoch()).count();

				vProgress = ((double)filePos / (double)fileSize);

				// sscanf renvoie le nombre de variables qui ont �t� remplies

				if (line_to_parse[0] == 'v')
				{
					if (line_to_parse[1] == ' ' && sscanf(line_to_parse.c_str(), "v %f %f %f", &p.x, &p.y, &p.z) == 3)
					{
						MeshLoader::MeshLoader_Mutex.lock();

						if (MeshLoader::Instance()->m_MeshDatas.IsVertexsEmpty())
						{
							MeshLoader::Instance()->m_Layouts[0] = "Vertex     (v3)";
						}

						MeshLoader::Instance()->m_MeshDatas.AddVertex(p);

						MeshLoader::MeshLoader_Mutex.unlock();
					}
					else if (line_to_parse[1] == 'n' && sscanf(line_to_parse.c_str(), "vn %f %f %f", &n.x, &n.y, &n.z) == 3)
					{
						if (normals.empty())
						{
							MeshLoader::MeshLoader_Mutex.lock();

							MeshLoader::Instance()->m_Layouts[1] = "Normal     (v3)";

							if (MeshLoader::Instance()->m_Layouts[4] == "Tex Coord  (v2)")
							{
								MeshLoader::Instance()->m_Layouts[2] = "Tangent    (v3)";
								MeshLoader::Instance()->m_Layouts[3] = "Bi-Tangent (v3)";
							}

							MeshLoader::MeshLoader_Mutex.unlock();
						}

						normals.emplace_back(n);
					}
					else if (line_to_parse[1] == 't' && sscanf(line_to_parse.c_str(), "vt %f %f", &t.x, &t.y) == 2)
					{
						if (texcoords.empty())
						{
							MeshLoader::MeshLoader_Mutex.lock();

							if (MeshLoader::Instance()->m_Layouts[1] == "Normal     (v3)")
							{
								MeshLoader::Instance()->m_Layouts[2] = "Tangent    (v3)";
								MeshLoader::Instance()->m_Layouts[3] = "Bi-Tangent (v3)";
							}
							MeshLoader::Instance()->m_Layouts[4] = "Tex Coord  (v2)";

							MeshLoader::MeshLoader_Mutex.unlock();
						}

						texcoords.emplace_back(t);
					}
					else
					{
						LogVarDebug("Debug : maybe a bug, because this line is not recognized : %s", line_to_parse.c_str());
						LogVarDebug("Debug : Fail to recover data's from file.. Import Stopped");
						vWorking = false;
					}
				}
				else if (line_to_parse[0] == 'f')
				{
					// 12
					int res = sscanf(line_to_parse.c_str(), "f %zu/%zu/%zu %zu/%zu/%zu %zu/%zu/%zu %zu/%zu/%zu", &v[0], &vt[0], &vn[0], &v[1], &vt[1], &vn[1], &v[2], &vt[2], &vn[2], &v[3], &vt[3], &vn[3]); // v / vt / vn

					if (res != 12)
					{
						// 8
						res = sscanf(line_to_parse.c_str(), "f %zu//%zu %zu//%zu %zu//%zu %zu//%zu", &v[0], &vn[0], &v[1], &vn[1], &v[2], &vn[2], &v[3], &vn[3]); // v / vn
					}

					if (res != 8 && res != 12)
					{
						// 9
						res = sscanf(line_to_parse.c_str(), "f %zu/%zu/%zu %zu/%zu/%zu %zu/%zu/%zu", &v[0], &vt[0], &vn[0], &v[1], &vt[1], &vn[1], &v[2], &vt[2], &vn[2]); // v / vt / vn
					}

					if (res != 9 && res != 8 && res != 12)
					{
						// 6
						res = sscanf(line_to_parse.c_str(), "f %zu//%zu %zu//%zu %zu//%zu", &v[0], &vn[0], &v[1], &vn[1], &v[2], &vn[2]); // v / vn
					}

					if (res == 6)
					{
						AddTriangleWithNormals(0, 1, 2, normals, v, vn);
					}
					else if (res == 9)
					{
						AddTriangleWithNormalsAndTexCoords(0, 1, 2, normals, texcoords, v, vn, vt);
					}
					else if (res == 8)
					{
						AddTriangleWithNormals(0, 1, 2, normals, v, vn);
						AddTriangleWithNormals(0, 2, 3, normals, v, vn);
					}
					else if (res == 12)
					{
						AddTriangleWithNormalsAndTexCoords(0, 1, 2, normals, texcoords, v, vn, vt);
						AddTriangleWithNormalsAndTexCoords(0, 2, 3, normals, texcoords, v, vn, vt);
					}
					else
					{
						// 8
						res = sscanf(line_to_parse.c_str(), "f %zu/%zu %zu/%zu %zu/%zu %zu/%zu", &v[0], &vt[0], &v[1], &vt[1], &v[2], &vt[2], &v[3], &vt[3]); // v / vt

						if (res != 8)
						{
							// 4
							res = sscanf(line_to_parse.c_str(), "f %zu %zu %zu %zu", &v[0], &v[1], &v[2], &v[3]); // v
						}

						if (res != 4 && res != 8)
						{
							// 6
							res = sscanf(line_to_parse.c_str(), "f %zu/%zu %zu/%zu %zu/%zu", &v[0], &vt[0], &v[1], &vt[1], &v[2], &vt[2]); // v / vt
						}

						if (res != 6 && res != 4 && res != 8)
						{
							// 3
							res = sscanf(line_to_parse.c_str(), "f %zu %zu %zu", &v[0], &v[1], &v[2]); // v
						}

						if (res == 3)
						{
							AddTriangleAndComputeRendererNormals(0, 1, 2, v);
						}
						else if (res == 6)
						{
							AddTriangleWithTexCoordsAndComputeRendererNormals(0, 1, 2, texcoords, v, vt);
						}
						else if (res == 4)
						{
							AddTriangleAndComputeRendererNormals(0, 1, 2, v);
							AddTriangleAndComputeRendererNormals(0, 2, 3, v);
						}
						else if (res == 8)
						{
							AddTriangleWithTexCoordsAndComputeRendererNormals(0, 1, 2, texcoords, v, vt);
							AddTriangleWithTexCoordsAndComputeRendererNormals(0, 2, 3, texcoords, v, vt);
						}
						else
						{
							LogVarError("maybe a bug, but this line is not recognized : %s", line_to_parse.c_str());
							LogVarError("Fail to recover data's from file.. Import Stopped");
							vWorking = false;
						}
					}
				}

				// oddly important, might need to be glFinish()
				// glFlush();

				uint64_t secondTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
					(std::chrono::system_clock::now().time_since_epoch()).count();

				vGenerationTime = vGenerationTime + (double)(secondTimeMark - firstTimeMark) / 1000.0;

				filePos += line_to_parse.size();
			}

			if (vWorking && needNormalGeneration)
			{
				MeshLoader::MeshLoader_Mutex.lock();

				// on va finir le calcul des normales
				// on nomalize toutes les normales des point
				for (auto& vert : MeshLoader::Instance()->m_MeshDatas.GetVertexsAdr())
				{
					vert.n.normalize();

					if (!vWorking) break;
				}

				MeshLoader::MeshLoader_Mutex.unlock();
			}
		}
	}

	vWorking = false;

	/////////////////////////////////////////////////////////////////////
	/////// LOCK ////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////

	MeshLoader::MeshLoader_Mutex.lock();
	MeshLoader::MeshLoader_Mutex.unlock();

	/////////////////////////////////////////////////////////////////////
	/////// UNLOCK //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void MeshPlyLoader(
	std::atomic< double >& vProgress,
	std::atomic< bool >& vWorking,
	std::atomic< double >& vGenerationTime)
{
	vProgress = 0.0f;

	vWorking = true;

	vGenerationTime = 0.0f;

	// todo

	vWorking = false;

	/////////////////////////////////////////////////////////////////////
	/////// LOCK ////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////

	MeshLoader::MeshLoader_Mutex.lock();
	MeshLoader::MeshLoader_Mutex.unlock();

	/////////////////////////////////////////////////////////////////////
	/////// UNLOCK //////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////
///// MeshLoaderDatas IMPLEMENTATION ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void MeshLoaderDatas::Clear()
{
	ClearVertexs();
	ClearIndexs();
	ClearVertexStamps();
}

/////////////////////////////////////////////////////////////////////////
///// Vertexs ///////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

size_t MeshLoaderDatas::GetCountVertexs()
{
	return m_Vertexs.size();
}

size_t MeshLoaderDatas::AddVertex(VertexStruct::P3_N3_TA3_BTA3_T2_C4 vVertex)
{
	m_Vertexs.push_back(vVertex);
	return GetCountIndexs();
}

size_t MeshLoaderDatas::DuplicateVertex(size_t vIndex)
{
	m_Vertexs.emplace_back(m_Vertexs[vIndex - 1]);
	return m_Vertexs.size();
}

std::vector<VertexStruct::P3_N3_TA3_BTA3_T2_C4>* MeshLoaderDatas::GetVertexsPtr()
{
	return &m_Vertexs;
}

std::vector<VertexStruct::P3_N3_TA3_BTA3_T2_C4>& MeshLoaderDatas::GetVertexsAdr()
{
	return m_Vertexs;
}

std::vector<VertexStruct::P3_N3_TA3_BTA3_T2_C4> MeshLoaderDatas::GetVertexs()
{
	return m_Vertexs;
}

VertexStruct::P3_N3_TA3_BTA3_T2_C4* MeshLoaderDatas::GetVertex(size_t vIndex)
{
	return &m_Vertexs[vIndex];
}

bool MeshLoaderDatas::IsVertexsEmpty()
{
	return m_Vertexs.empty();
}

void MeshLoaderDatas::ClearVertexs()
{
	m_Vertexs.clear();
}

void MeshLoaderDatas::ClearVertexStamps()
{
	m_VertexStamps.clear();
}

MeshLoaderDatas::VertexStamps* MeshLoaderDatas::GetVertexStampsPtr()
{
	return &m_VertexStamps;
}

MeshLoaderDatas::VertexStamps& MeshLoaderDatas::GetVertexStampsAdr()
{
	return m_VertexStamps;
}

/////////////////////////////////////////////////////////////////////////
///// Indexs ////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

size_t MeshLoaderDatas::GetCountIndexs()
{
	return m_Indices.size();
}

void MeshLoaderDatas::AddIndex(size_t vIndex)
{
	m_Indices.emplace_back((uint32_t)vIndex);
}

std::vector<VertexStruct::I1>* MeshLoaderDatas::GetIndexsPtr()
{
	return &m_Indices;
}

std::vector<VertexStruct::I1>& MeshLoaderDatas::GetIndexsAdr()
{
	return m_Indices;
}

std::vector<VertexStruct::I1> MeshLoaderDatas::GetIndexs()
{
	return m_Indices;
}

VertexStruct::I1* MeshLoaderDatas::GetIndex(size_t vIndex)
{
	return &m_Indices[vIndex];
}

bool MeshLoaderDatas::IsIndexsEmpty()
{
	return m_Indices.empty();
}

void MeshLoaderDatas::ClearIndexs()
{
	m_Indices.clear();
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

MeshLoader::MeshLoader()
{
	m_FilePath = ".";
	m_GenerationTime = 0.0f;
}

MeshLoader::~MeshLoader()
{
	Clear();
}

void MeshLoader::Clear()
{
	m_Layouts.clear();
	m_MeshDatas.Clear();
}

void MeshLoader::SetFinishedFunction(std::function<void()> vFinishFunc)
{
	m_FinishedFunction = vFinishFunc;
}

void MeshLoader::OpenDialog(const std::string& vFilePathName, const std::string& vFilePath)
{
	ImGuiFileDialog::Instance()->OpenDialog(
		"OpenMeshFileDialog", "Open Mesh File", ".obj", vFilePath, vFilePathName,
		1, nullptr, ImGuiFileDialogFlags_Modal);
}

void MeshLoader::ShowDialog(ct::ivec2 vScreenSize)
{
	ImVec2 max = ImVec2((float)vScreenSize.x, (float)vScreenSize.y);
	ImVec2 min = max * 0.5f;

	if (ImGuiFileDialog::Instance()->Display("OpenMeshFileDialog",
		ImGuiWindowFlags_NoCollapse, min, max))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			m_FilePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			m_FilePath = ImGuiFileDialog::Instance()->GetCurrentPath();

			if (ImGuiFileDialog::Instance()->GetCurrentFilter() == ".obj")
			{
				LoadObjFileAsync(m_FilePathName, m_FinishedFunction);
			}
			if (ImGuiFileDialog::Instance()->GetCurrentFilter() == ".ply")
			{
				LoadPlyFileAsync(m_FilePathName, m_FinishedFunction);
			}
		}

		ImGuiFileDialog::Instance()->Close();
	}
}

void MeshLoader::LoadObjFileAsync(const std::string& vFilePathName, std::function<void()> vFinishFunc)
{
	if (!vFilePathName.empty())
	{
		fileToLoad = vFilePathName;

		CreateThread(MeshFormatEnum::MESH_FORMAT_OBJ, vFinishFunc);
	}
}

void MeshLoader::LoadPlyFileAsync(const std::string& vFilePathName, std::function<void()> vFinishFunc)
{
	if (!vFilePathName.empty())
	{
		fileToLoad = vFilePathName;

		CreateThread(MeshFormatEnum::MESH_FORMAT_PLY, vFinishFunc);
	}
}

void MeshLoader::DrawImGuiProgress(float vWidth) // return true if finished
{
	if (m_WorkerThread.joinable())
	{
		if (ImGui::ContrastedButton("Stop Loading"))
		{
			StopWorkerThread();
		}

		if (MeshLoader::Progress > 0.0)
		{
			ImGui::SameLine();

			char timeBuffer[256];
			//double t = MeshLoader::GenerationTime;
			float pr = (float)MeshLoader::Progress;
			int p = (int)(pr * 100.0f);
			snprintf(timeBuffer, 256, "%i%%", p);
			ImGui::ProgressBar(pr, ImVec2(vWidth, 0), timeBuffer);
		}
	}
}

void MeshLoader::DrawImGuiInfos(float vWidth)
{
	UNUSED(vWidth);

	ImGui::TextWrapped("Model : %s", m_FilePathName.c_str());

	ImGui::Text("Attributes :");
	int idx = 0;
	for (auto line : m_Layouts)
	{
		ImGui::TextWrapped("%i : %s", idx++, line.c_str());
	}
}

/////////////////////////////////////////////////////////////////////////
///// DATAS GET /////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

MeshLoaderDatas* MeshLoader::GetMeshDatas()
{
	return &m_MeshDatas;
}

/////////////////////////////////////////////////////////////////////////
///// THREAD ////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

//CreateThread(MeshFormatEnum::MESH_FORMAT_OBJ, std::bind(&MeshLoader::UploadDatasToGpu, this));
void MeshLoader::CreateThread(MeshFormatEnum vMeshFormat, std::function<void()> vFinishFunc)
{
	if (!StopWorkerThread())
	{
		if (vMeshFormat == MeshFormatEnum::MESH_FORMAT_OBJ)
		{
			m_FinishedFunction = vFinishFunc;
			MeshLoader::Working = true;
			m_WorkerThread =
				std::thread(
					MeshObjLoader,
					std::ref(MeshLoader::Progress),
					std::ref(MeshLoader::Working),
					std::ref(MeshLoader::GenerationTime)
				);
		}
		else if (vMeshFormat == MeshFormatEnum::MESH_FORMAT_PLY)
		{
			m_FinishedFunction = vFinishFunc;
			MeshLoader::Working = true;
			m_WorkerThread =
				std::thread(
					MeshPlyLoader,
					std::ref(MeshLoader::Progress),
					std::ref(MeshLoader::Working),
					std::ref(MeshLoader::GenerationTime)
				);
		}
	}
}

bool MeshLoader::StopWorkerThread()
{
	bool res = false;

	res = m_WorkerThread.joinable();
	if (res)
	{
		MeshLoader::Working = false;
		m_WorkerThread.join();
		CallFinishedFunction();
	}

	return res;
}

bool MeshLoader::IsJoinable()
{
	return m_WorkerThread.joinable();
}

bool MeshLoader::FinishIfRequired()
{
	if (m_WorkerThread.joinable() && !MeshLoader::Working)
	{
		m_WorkerThread.join();
		CallFinishedFunction();
		return true;
	}

	return false;
}

void MeshLoader::LoadMeshAuto(const std::string& vFilePathName)
{
	if (!vFilePathName.empty())
	{
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			if (ps.ext == "obj")
			{
				m_FilePathName = vFilePathName;
				LoadObjFileAsync(vFilePathName, m_FinishedFunction);
			}
			else if (ps.ext == "ply")
			{
				m_FilePathName = vFilePathName;
				LoadPlyFileAsync(vFilePathName, m_FinishedFunction);
			}
		}
	}
}

void MeshLoader::Join()
{
	m_WorkerThread.join();
}

void MeshLoader::CallFinishedFunction()
{
	m_IsjustFinished = true;
	if (m_FinishedFunction)
		m_FinishedFunction();
}

