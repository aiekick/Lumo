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

#include "ModelExporterModule.h"

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

#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/version.h>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <SceneGraph/SceneModel.h>
#include <SceneGraph/SceneMesh.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<ModelExporterModule> ModelExporterModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<ModelExporterModule>(vVulkanCorePtr);
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

ModelExporterModule::ModelExporterModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: m_VulkanCorePtr(vVulkanCorePtr)
{
	ZoneScoped;
	unique_SaveMeshFileDialog_id = ct::toStr("OpenMeshFileDialog%u", (uintptr_t)this);
}

ModelExporterModule::~ModelExporterModule()
{
	ZoneScoped;

	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ModelExporterModule::Init()
{
	ZoneScoped;

	return true;
}

void ModelExporterModule::Unit()
{
	ZoneScoped;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool ModelExporterModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext); 
	
	if (ImGui::ContrastedButton("Save Model"))
	{
		//tofix : gltf export crash ...
		ImGuiFileDialog::Instance()->OpenDialog(
			unique_SaveMeshFileDialog_id, "Save 3D File", ".obj,.ply,.fbx", 
			m_FilePath, m_FilePathName,
			1, nullptr, ImGuiFileDialogFlags_Modal);
	}

	return false;
}

void ModelExporterModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
}

void ModelExporterModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	ImVec2 max = ImVec2((float)vMaxSize.x, (float)vMaxSize.y); 
	ImVec2 min = max * 0.5f;
	if (ImGuiFileDialog::Instance()->Display(unique_SaveMeshFileDialog_id,
		ImGuiWindowFlags_NoCollapse, min, max))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			SaveModel(ImGuiFileDialog::Instance()->GetFilePathName());
		}
		ImGuiFileDialog::Instance()->Close();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL INPUT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void ModelExporterModule::SetModel(SceneModelWeak vSceneModel)
{	
	ZoneScoped;

	m_InputModel = vSceneModel;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ModelExporterModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += vOffset + "<model_exporter_module>\n";

	str += vOffset + "</model_exporter_module>\n";

	return str;
}

bool ModelExporterModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "model_exporter_module")
	{

	}

	return true;
}

void ModelExporterModule::AfterNodeXmlLoading()
{
	ZoneScoped;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ModelExporterModule::SaveModel(const std::string& vFilePathName)
{
	if (!vFilePathName.empty())
	{
		m_FilePathName = vFilePathName;

		try
		{
			auto modelPtr = m_InputModel.lock();
			if (modelPtr)
			{
				aiScene* scene_ptr = new aiScene();

				scene_ptr->mRootNode = new aiNode();

				scene_ptr->mMaterials = new aiMaterial * [1];
				scene_ptr->mMaterials[0] = nullptr;
				scene_ptr->mNumMaterials = 1;

				scene_ptr->mMaterials[0] = new aiMaterial();

				size_t count_meshes = modelPtr->size();

				scene_ptr->mMeshes = new aiMesh*[count_meshes];
				scene_ptr->mNumMeshes = (uint32_t)count_meshes;

				scene_ptr->mRootNode->mMeshes = new uint32_t[count_meshes];
				scene_ptr->mRootNode->mNumMeshes = (uint32_t)count_meshes;
				
				for (size_t mesh_idx = 0U; mesh_idx < count_meshes; ++mesh_idx)
				{
					auto meshPtr = modelPtr->at(mesh_idx).lock();
					if (meshPtr)
					{
						scene_ptr->mMeshes[mesh_idx] = new aiMesh();
						scene_ptr->mMeshes[mesh_idx]->mMaterialIndex = 0;
						scene_ptr->mRootNode->mMeshes[mesh_idx] = (uint32_t)mesh_idx;
						auto pMesh = scene_ptr->mMeshes[mesh_idx];

						size_t vertice_count = meshPtr->GetVerticesCount();
						pMesh->mVertices = new aiVector3D[vertice_count];
						pMesh->mNumVertices = (uint32_t)vertice_count;
						
						size_t indice_count = meshPtr->GetIndicesCount();

						// tout mes mesh's n'ont que des faces de type triangle, donc 3 points
						// donc si le nombre d'indice n'est modulo 3 alors c'est un 
						// model autre que face, type curve par ex
						auto primitive_type = meshPtr->GetPrimitiveType();
						if (primitive_type == SceneModelPrimitiveType::SCENE_MODEL_PRIMITIVE_TYPE_POINTS)
						{
							pMesh->mNumFaces = (uint32_t)indice_count; // independants
							pMesh->mPrimitiveTypes = aiPrimitiveType_POINT;
						}
						else if (primitive_type == SceneModelPrimitiveType::SCENE_MODEL_PRIMITIVE_TYPE_CURVES)
						{
							pMesh->mNumFaces = (uint32_t)(indice_count - 1U); // strip line
							pMesh->mPrimitiveTypes = aiPrimitiveType_LINE;
						}
						else if (primitive_type == SceneModelPrimitiveType::SCENE_MODEL_PRIMITIVE_TYPE_FACES)
						{
							pMesh->mNumFaces = (uint32_t)(indice_count / 3U); // independants
							pMesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
						}
												
						pMesh->mNormals = new aiVector3D[vertice_count];
						pMesh->mFaces = new aiFace[pMesh->mNumFaces];

						for (size_t vert_idx = 0U; vert_idx < vertice_count; ++vert_idx)
						{
							auto& v = meshPtr->GetVertices()->at(vert_idx);
							pMesh->mVertices[vert_idx] = aiVector3D(v.p.x, v.p.y, v.p.z);
							if (pMesh->mNumFaces)
							{
								pMesh->mNormals[vert_idx] = aiVector3D(v.n.x, v.n.y, v.n.z);
							}
						}

						if (primitive_type == SceneModelPrimitiveType::SCENE_MODEL_PRIMITIVE_TYPE_POINTS)
						{
							for (size_t face_idx = 0U; face_idx < pMesh->mNumFaces; ++face_idx)
							{
								auto& i0 = meshPtr->GetIndices()->at(face_idx);

								aiFace& face = pMesh->mFaces[face_idx];
								face.mIndices = new unsigned int[1];
								face.mNumIndices = 1;

								face.mIndices[0] = i0;
							}
						}
						else if (primitive_type == SceneModelPrimitiveType::SCENE_MODEL_PRIMITIVE_TYPE_CURVES)
						{
							for (size_t face_idx = 0U; face_idx < pMesh->mNumFaces; ++face_idx)
							{
								auto& i0 = meshPtr->GetIndices()->at(face_idx + 0);
								auto& i1 = meshPtr->GetIndices()->at(face_idx + 1);

								aiFace& face = pMesh->mFaces[face_idx];
								face.mIndices = new unsigned int[2];
								face.mNumIndices = 2;

								face.mIndices[0] = i0;
								face.mIndices[1] = i1;
							}
						}
						else if (primitive_type == SceneModelPrimitiveType::SCENE_MODEL_PRIMITIVE_TYPE_FACES)
						{
							for (size_t face_idx = 0U, ind_idx = 0U; face_idx < pMesh->mNumFaces; ++face_idx, ind_idx += 3U)
							{
								auto& i0 = meshPtr->GetIndices()->at(ind_idx + 0);
								auto& i1 = meshPtr->GetIndices()->at(ind_idx + 1);
								auto& i2 = meshPtr->GetIndices()->at(ind_idx + 2);

								aiFace& face = pMesh->mFaces[face_idx];
								face.mIndices = new unsigned int[3];
								face.mNumIndices = 3;

								face.mIndices[0] = i0;
								face.mIndices[1] = i1;
								face.mIndices[2] = i2;
							}
						}
					}
				}


				Assimp::Exporter aiExporter;
				auto ps = FileHelper::Instance()->ParsePathFileName(m_FilePathName);
				if (ps.isOk)
				{
					if (ps.ext == "ply")
					{
						aiExporter.Export(scene_ptr, "ply", m_FilePathName);
					}
					else if (ps.ext == "obj")
					{
						aiExporter.Export(scene_ptr, "obj", m_FilePathName);
					}
					else if (ps.ext == "fbx")
					{
						aiExporter.Export(scene_ptr, "fbx", m_FilePathName);
					}
					else if (ps.ext == "gltf")
					{
						aiExporter.Export(scene_ptr, "gltf", m_FilePathName);
					}
				}

				// scene et tout les object créé dans scene 
				// seront detruit en quittant le scope
				delete scene_ptr;
			}
		}
		catch (const std::exception& ex)
		{
			LogVarError("unknown error : %s", ex.what());
		}
	}
}