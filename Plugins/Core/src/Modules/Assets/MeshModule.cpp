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

#include "MeshModule.h"
#include <assimp/scene.h>
#include <ctools/Logger.h>
#include <assimp/cimport.h>
#include <assimp/version.h>
#include <ctools/FileHelper.h>
#include <assimp/postprocess.h>
#include <ImWidgets.h>
#include <ImGuiFileDialog.h>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<MeshModule> MeshModule::Create(GaiApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<MeshModule>(vVulkanCorePtr);
	res->m_This = res;
	res->SetParentNode(vParentNode);
	if (!res->Init())
	{
		res.reset();
	}
	return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

MeshModule::MeshModule(GaiApi::VulkanCorePtr vVulkanCorePtr)
{
	unique_OpenMeshFileDialog_id = ct::toStr("OpenMeshFileDialog%u", (uintptr_t)this);
	m_VulkanCorePtr = vVulkanCorePtr;
}

MeshModule::~MeshModule()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// PUBLIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool MeshModule::Init()
{
	ZoneScoped;

	m_SceneModelPtr = SceneModel::Create();

	return true;
}

void MeshModule::Unit()
{
	ZoneScoped;

	m_SceneModelPtr.reset();
}

bool MeshModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	ZoneScoped;

	if (ImGui::CollapsingHeader("Model"))
	{
		ImGui::BeginHorizontal("Buttons");

		if (ImGui::ContrastedButton("Load"))
		{
			ImGuiFileDialog::Instance()->OpenDialog(
				unique_OpenMeshFileDialog_id, "Open 3D File", "3D files{.obj,.gltf,.ply,.fbx}", m_FilePath, m_FilePathName,
				1, nullptr, ImGuiFileDialogFlags_Modal);
		}

		if (ImGui::ContrastedButton("ReLoad"))
		{
			LoadMesh(m_FilePathName);
		}

		if (ImGui::ContrastedButton("Center"))
		{
			m_SceneModelPtr->CenterCameraToModel();
		}

		if (ImGui::ContrastedButton("Clear"))
		{
			m_SceneModelPtr->clear();
		}

		ImGui::EndHorizontal();

		ImGui::Header("Infos");

		if (!m_SceneModelPtr->empty())
		{
			ImGui::Text("File name : %s", m_FileName.c_str());
			ImGui::TextWrapped("File path name: %s", m_FilePathName.c_str());
			ImGui::Text("Mesh Count : %u", (uint32_t)m_SceneModelPtr->size());
			const auto& pos = m_SceneModelPtr->GetCenter();
			ImGui::Text("Model Center : %.2f,%.2f, %.2f", pos.x, pos.y, pos.z);
			uint32_t idx = 0;
			for (const auto& meshPtr : *m_SceneModelPtr)
			{
				if (meshPtr)
				{
					ImGui::Text("Mesh %u : %s %s %s %s %s %s", idx++,
						meshPtr->HasNormals() ? "N" : " ",
						meshPtr->HasTangeants() ? "Tan" : " ",
						meshPtr->HasBiTangeants() ? "BTan" : " ",
						meshPtr->HasTextureCoords() ? "UV" : " ",
						meshPtr->HasVertexColors() ? "Col" : " ",
						meshPtr->HasIndices() ? "Ind" : " ");
				}
			}
		}
	}	

	return false;
}

bool MeshModule::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	ZoneScoped;

    return false;
}

bool MeshModule::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	ImVec2 max = ImVec2((float)vMaxSize.x, (float)vMaxSize.y);
	ImVec2 min = max * 0.5f;

	if (ImGuiFileDialog::Instance()->Display(unique_OpenMeshFileDialog_id,
		ImGuiWindowFlags_NoCollapse, min, max))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			m_FilePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			m_FilePath = ImGuiFileDialog::Instance()->GetCurrentPath();

			LoadMesh(m_FilePathName);
		}

		ImGuiFileDialog::Instance()->Close();
    }

    return false;
}

SceneModelWeak MeshModule::GetModel()
{
	return m_SceneModelPtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MeshModule::LoadMesh(const std::string& vFilePathName)
{
	if (!vFilePathName.empty())
	{
		m_FilePathName = vFilePathName;

		try
		{
			const aiScene* scene = aiImportFile(m_FilePathName.c_str(),
				aiProcess_CalcTangentSpace |
				aiProcess_GenSmoothNormals |
				//aiProcess_MakeLeftHanded |
				aiProcess_JoinIdenticalVertices |
				aiProcess_Triangulate
			);

			if (scene &&
				scene->HasMeshes())
			{
				m_SceneModelPtr->clear();

				size_t _last_index_offset = 0U;

				for (size_t k = 0; k != scene->mNumMeshes; ++k)
				{
					const aiMesh* mesh = scene->mMeshes[k];

					if (mesh)
					{
						auto sceneMeshPtr = SceneMesh<VertexStruct::P3_N3_TA3_BTA3_T2_C4>::Create(m_VulkanCorePtr);
						if (sceneMeshPtr)
						{
							sceneMeshPtr->GetVertices()->reserve(mesh->mNumVertices);
							if (mesh->mNormals)
							{
								sceneMeshPtr->HaveNormals();
							}
							if (mesh->mTangents)
							{
								sceneMeshPtr->HaveTangeants();
							}
							if (mesh->mBitangents)
							{
								sceneMeshPtr->HaveBiTangeants();
							}
							if (mesh->mTextureCoords)
							{
								if (mesh->mNumUVComponents[0] == 2U)
								{
									if (mesh->mTextureCoords[0])
									{
										sceneMeshPtr->HaveTextureCoords();
									}
								}
							}
							if (mesh->mColors)
							{
								if (mesh->mColors[0])
								{
									sceneMeshPtr->HaveVertexColors();
								}
							}

							VertexStruct::P3_N3_TA3_BTA3_T2_C4 v;

							ct::fAABBCC aabbcc;
							for (size_t i = 0; i != mesh->mNumVertices; ++i)
							{
								v = VertexStruct::P3_N3_TA3_BTA3_T2_C4();

								const auto& vert = mesh->mVertices[i];
								v.p = ct::fvec3(vert.x, vert.y, vert.z);

								if (mesh->mNormals)
								{
									const auto& norm = mesh->mNormals[i];
									v.n = ct::fvec3(norm.x, norm.y, norm.z);
								}

								if (mesh->mTangents)
								{
									const auto& tang = mesh->mTangents[i];
									v.tan = ct::fvec3(tang.x, tang.y, tang.z);
								}

								if (mesh->mBitangents)
								{
									const auto& btan = mesh->mBitangents[i];
									v.btan = ct::fvec3(btan.x, btan.y, btan.z);
								}

								if (mesh->mTextureCoords[0] && 
									mesh->mNumUVComponents[0] == 2U)
								{
									const auto& coor = mesh->mTextureCoords[0][i];
									v.t = ct::fvec2(coor.x, coor.y);
								}

								if (mesh->mColors)
								{
									if (mesh->mColors[0])
									{
										const auto& colo = mesh->mColors[0][i];
										v.c = ct::fvec4(colo.r, colo.g, colo.b, colo.a);
									}
								}

								sceneMeshPtr->GetVertices()->push_back(v);
								sceneMeshPtr->SetPrimitiveType((SceneModelPrimitiveType)((int)mesh->mPrimitiveTypes-1U));
								m_SceneModelPtr->CombinePointInBoundingBox(v.p);
							}

							sceneMeshPtr->GetIndices()->reserve(mesh->mNumFaces * 3U);
							if (mesh->mNumFaces) sceneMeshPtr->HaveIndices();
							for (size_t i = 0; i != mesh->mNumFaces; ++i)
							{
								const aiFace& face = mesh->mFaces[i];
								for (size_t j = 0; j != face.mNumIndices; ++j)
								{
									sceneMeshPtr->GetIndices()->push_back(face.mIndices[j]);
								}
							}

							if (sceneMeshPtr->Init())
							{
								m_SceneModelPtr->Add(sceneMeshPtr);
							}
							else
							{
								sceneMeshPtr.reset();
								LogVarError("Failed to build the mesh %u VBO/IBO for %s", (uint32_t)k, m_FilePathName.c_str());
							}
						}
					}
				}

				aiReleaseImport(scene);

				MeshLoadingFinished();
			}
		}
		catch (const std::exception& ex)
		{
			LogVarError("unknown error : %s", ex.what());
		}
	}
}

void MeshModule::MeshLoadingFinished()
{
	ZoneScoped;

	auto ps = FileHelper::Instance()->ParsePathFileName(m_FilePathName);
	if (ps.isOk)
	{
		m_FileName = ps.name;
	}

	auto parentNodePtr = GetParentNode().lock();
	if (parentNodePtr)
	{
		parentNodePtr->SendFrontNotification(ModelUpdateDone);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MeshModule::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<mesh_module>\n";

	str += vOffset + "\t<file_path_name>" + m_FilePathName + "</file_path_name>\n";
	str += vOffset + "\t<file_path>" + m_FilePath + "</file_path>\n";

	str += vOffset + "</mesh_module>\n";

	return str;
}

bool MeshModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
{
	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strParentName == "mesh_module")
	{
		if (strName == "file_path_name")
		{
			m_FilePathName = strValue;
			LoadMesh(m_FilePathName);
		}
		if (strName == "file_path")
			m_FilePath = strValue;
	}

	return true;
}
