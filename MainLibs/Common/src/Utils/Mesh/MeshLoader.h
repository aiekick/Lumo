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

#include <list>
#include <mutex>
#include <string>
#include <vector>
#include <future>
#include <thread>
#include <atomic>
#include <cstring>
#include <functional>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <ctools/cTools.h>
#include <Utils/Mesh/VertexStruct.h>

enum class MeshFormatEnum : uint8_t
{
	MESH_FORMAT_OBJ = 0,
	MESH_FORMAT_PLY,
	MESH_FORMAT_GLTF,
	MESh_FORMAT_Count
};

class MeshLoaderDatas
{
public:
	typedef std::unordered_map<size_t, std::unordered_map<size_t, std::unordered_map<size_t, size_t>>> VertexStamps;

private:
	std::vector<VertexStruct::P3_N3_TA3_BTA3_T2_C4> m_Vertexs;
	std::vector<VertexStruct::I1> m_Indices;

	// -------------------------------------------------------------------------------
	// il faut faire ca, car on utilise un buffer intercalé dans le gpu
	// si on utilsiais 4 buffer le setup serait plus simple mais la gestion
	// serait moins facile, en gros il aura des vertex en doublons que sur la couture
	// d'un mesh clos
	// -------------------------------------------------------------------------------
	// map qui contient une signature unique de vertex
	// car deux vertex peuvent avoir des coordonnées de texture et normales differentes
	// en ce cas il faudra cree un nouvear vertex, et ce map va donner
	// la nouvelle pos du vertex pour la signature donnée
	// v, vt, vn, id
	VertexStamps m_VertexStamps;

public:
	size_t GetCountVertexs();
	size_t AddVertex(VertexStruct::P3_N3_TA3_BTA3_T2_C4 vVertex);
	VertexStruct::P3_N3_TA3_BTA3_T2_C4* GetVertex(size_t vIndex);
	size_t DuplicateVertex(size_t vIndex);
	std::vector<VertexStruct::P3_N3_TA3_BTA3_T2_C4>* GetVertexsPtr();
	std::vector<VertexStruct::P3_N3_TA3_BTA3_T2_C4>& GetVertexsAdr();
	std::vector<VertexStruct::P3_N3_TA3_BTA3_T2_C4> GetVertexs();

	bool IsVertexsEmpty();
	void ClearVertexs();

	void ClearVertexStamps();
	VertexStamps* GetVertexStampsPtr();
	VertexStamps& GetVertexStampsAdr();

	size_t GetCountIndexs();
	void AddIndex(size_t vIndex);
	VertexStruct::I1* GetIndex(size_t vIndex);
	std::vector<VertexStruct::I1>* GetIndexsPtr();
	std::vector<VertexStruct::I1>& GetIndexsAdr();
	std::vector<VertexStruct::I1> GetIndexs();
	bool IsIndexsEmpty();
	void ClearIndexs();
	void Clear();
};

class MeshLoader
{
public:
	static std::mutex MeshLoader_Mutex;
	static std::atomic<double> Progress;
	static std::atomic<bool> Working;
	static std::atomic<double> GenerationTime;

private:
	std::thread m_WorkerThread;
	float m_GenerationTime = 0.0f;
	std::function<void()> m_FinishedFunction;
	bool m_IsjustFinished = false;

public:
	std::string fileToLoad;
	std::vector<std::string> m_Layouts;
	MeshLoaderDatas m_MeshDatas;

public:
	std::string m_FilePathName;
	std::string m_FilePath;

public:
	static MeshLoader* Instance()
	{
		static MeshLoader _instance;
		return &_instance;
	}

protected:
	MeshLoader(); // Prevent construction
	MeshLoader(const MeshLoader&) = default; // Prevent construction by copying
	MeshLoader& operator =(const MeshLoader&) { return *this; }; // Prevent assignment
	~MeshLoader(); // Prevent unwanted destruction

public:
	void Clear();

	void SetFinishedFunction(std::function<void()> vFinishFunc);

	void OpenDialog(const std::string& vFilePathName, const std::string& vFilePath);
	void ShowDialog(ct::ivec2 vScreenSize);

	void LoadObjFileAsync(const std::string& vFilePathName, std::function<void()> vFinishFunc);
	void LoadPlyFileAsync(const std::string& vFilePathName, std::function<void()> vFinishFunc);

	void DrawImGuiProgress(float vWidth = 150.0f); // return true if finished
	void DrawImGuiInfos(float vWidth = 150.0f);

public:
	MeshLoaderDatas* GetMeshDatas();

public:
	void CreateThread(MeshFormatEnum vMeshFormat, std::function<void()> vFinishFunc);
	bool StopWorkerThread();
	bool IsJoinable();
	void Join();
	bool FinishIfRequired();

	/// <summary>
	/// will determine the loader to use based on the extention of the m_FilePathName
	/// the m_FilePathName at least, and the finish function callback must be defined before
	/// </summary>
	void LoadMeshAuto(const std::string& vFilePathName = "");

private:
	void CallFinishedFunction();
};
