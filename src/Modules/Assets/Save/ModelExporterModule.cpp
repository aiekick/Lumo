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
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Gaia/Buffer/FrameBuffer.h>

#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/version.h>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>
#include <ImGuiFileDialog.h>
#include <LumoBackend/SceneGraph/SceneModel.h>
#include <LumoBackend/SceneGraph/SceneMesh.hpp>

using namespace GaiApi;

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

std::shared_ptr<ModelExporterModule> ModelExporterModule::Create(GaiApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode) {
    ZoneScoped;

    if (!vVulkanCorePtr)
        return nullptr;
    auto res = std::make_shared<ModelExporterModule>(vVulkanCorePtr);
    res->SetParentNode(vParentNode);
    res->m_This = res;
    if (!res->Init()) {
        res.reset();
    }

    return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

ModelExporterModule::ModelExporterModule(GaiApi::VulkanCorePtr vVulkanCorePtr) : m_VulkanCorePtr(vVulkanCorePtr) {
    ZoneScoped;
    unique_SaveMeshFileDialog_id = ct::toStr("OpenMeshFileDialog%u", (uintptr_t)this);
}

ModelExporterModule::~ModelExporterModule() {
    ZoneScoped;

    Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ModelExporterModule::Init() {
    ZoneScoped;

    return true;
}

void ModelExporterModule::Unit() {
    ZoneScoped;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool ModelExporterModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (ImGui::ContrastedButton("Save Model")) {
        // tofix : gltf export crash ...
        ImGuiFileDialog::Instance()->OpenDialog(
            unique_SaveMeshFileDialog_id, "Save 3D File", ".ply", m_FilePath, m_FilePathName, 1, nullptr, ImGuiFileDialogFlags_Modal);
    }

    return false;
}

bool ModelExporterModule::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool ModelExporterModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    ImVec2 max = ImVec2((float)vMaxSize.x, (float)vMaxSize.y);
    ImVec2 min = max * 0.5f;
    if (ImGuiFileDialog::Instance()->Display(unique_SaveMeshFileDialog_id, ImGuiWindowFlags_NoCollapse, min, max)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            SaveModel(ImGuiFileDialog::Instance()->GetFilePathName());
        }
        ImGuiFileDialog::Instance()->Close();
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL INPUT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void ModelExporterModule::SetModel(SceneModelWeak vSceneModel) {
    ZoneScoped;

    m_InputModel = vSceneModel;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ModelExporterModule::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;

    std::string str;

    str += vOffset + "<model_exporter_module>\n";

    str += vOffset + "</model_exporter_module>\n";

    return str;
}

bool ModelExporterModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (strParentName == "model_exporter_module") {
    }

    return true;
}

void ModelExporterModule::AfterNodeXmlLoading() {
    ZoneScoped;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ModelExporterModule::SaveModel(const std::string& vFilePathName) {
    if (!vFilePathName.empty()) {
        m_FilePathName = vFilePathName;

        try {
            auto modelPtr = m_InputModel.lock();
            if (modelPtr) {
                aiScene* scene_ptr = new aiScene();

                scene_ptr->mRootNode = new aiNode();

                scene_ptr->mMaterials = new aiMaterial*[1];
                scene_ptr->mMaterials[0] = nullptr;
                scene_ptr->mNumMaterials = 1;

                scene_ptr->mMaterials[0] = new aiMaterial();

                size_t count_meshes = modelPtr->size();

                scene_ptr->mMeshes = new aiMesh*[count_meshes];
                scene_ptr->mNumMeshes = (uint32_t)count_meshes;

                scene_ptr->mRootNode->mMeshes = new uint32_t[count_meshes];
                scene_ptr->mRootNode->mNumMeshes = (uint32_t)count_meshes;

                for (size_t mesh_idx = 0U; mesh_idx < count_meshes; ++mesh_idx) {
                    auto meshPtr = modelPtr->at(mesh_idx).lock();
                    if (meshPtr) {
                        // quand le mesh a été modifié dans le gpu, il faut l'extraire pour
                        // pouvoir le sauver vers un fichier
                        // normalement on a pas besoin de transform feedback pour faire ca
                        // avec vulkan.. a voir !?
                        auto gpu_vertices = meshPtr->GetVerticesFromGPU();
                        auto gpu_indices = meshPtr->GetIndicesFromGPU();

                        scene_ptr->mMeshes[mesh_idx] = new aiMesh();
                        scene_ptr->mMeshes[mesh_idx]->mMaterialIndex = 0;
                        scene_ptr->mRootNode->mMeshes[mesh_idx] = (uint32_t)mesh_idx;
                        auto pMesh = scene_ptr->mMeshes[mesh_idx];

                        size_t vertice_count = gpu_vertices.size();
                        if (vertice_count) {
                            pMesh->mVertices = new aiVector3D[vertice_count];
                            pMesh->mNumVertices = (uint32_t)vertice_count;

                            size_t indice_count = gpu_indices.size();

                            // tout mes mesh's n'ont que des faces de type triangle, donc 3 points
                            // donc si le nombre d'indice n'est modulo 3 alors c'est un
                            // model autre que face, type curve par ex
                            auto primitive_type = meshPtr->GetPrimitiveType();
                            if (primitive_type == SceneModelPrimitiveType::SCENE_MODEL_PRIMITIVE_TYPE_POINTS) {
                                pMesh->mNumFaces = (uint32_t)indice_count;  // independants
                                pMesh->mPrimitiveTypes = aiPrimitiveType_POINT;
                            } else if (primitive_type == SceneModelPrimitiveType::SCENE_MODEL_PRIMITIVE_TYPE_CURVES) {
                                pMesh->mNumFaces = (uint32_t)(indice_count - 1U);  // strip line
                                pMesh->mPrimitiveTypes = aiPrimitiveType_LINE;
                            } else if (primitive_type == SceneModelPrimitiveType::SCENE_MODEL_PRIMITIVE_TYPE_FACES) {
                                pMesh->mNumFaces = (uint32_t)(indice_count / 3U);  // independants
                                pMesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
                            }

                            bool _has_texture_coords = meshPtr->HasTextureCoords();
                            const uint32_t uv_map_index = ct::mini((uint32_t)mesh_idx, (uint32_t)AI_MAX_NUMBER_OF_TEXTURECOORDS);
                            if (mesh_idx < AI_MAX_NUMBER_OF_TEXTURECOORDS) {
                                if (_has_texture_coords) {
                                    pMesh->mNumUVComponents[uv_map_index] = 2U;
                                    pMesh->mTextureCoords[uv_map_index] = new aiVector3D[vertice_count];
                                }
                            } else {
                                // assimp not support more than AI_MAX_NUMBER_OF_TEXTURECOORDS uv maps
                                // so we disable the writing of anothers uv maps
                                _has_texture_coords = false;
                            }

                            bool _has_vertex_colors = meshPtr->HasVertexColors();
                            const uint32_t color_map_index = ct::mini((uint32_t)mesh_idx, (uint32_t)AI_MAX_NUMBER_OF_COLOR_SETS);
                            if (mesh_idx < AI_MAX_NUMBER_OF_COLOR_SETS) {
                                if (_has_vertex_colors) {
                                    pMesh->mColors[color_map_index] = new aiColor4D[vertice_count];
                                }
                            } else {
                                // assimp not support more than AI_MAX_NUMBER_OF_TEXTURECOORDS uv maps
                                // so we disable the writing of anothers uv maps
                                _has_texture_coords = false;
                            }

                            bool _has_normals = meshPtr->HasTextureCoords();
                            
                            pMesh->mNormals = new aiVector3D[vertice_count];
                            pMesh->mFaces = new aiFace[pMesh->mNumFaces];

                            for (size_t vert_idx = 0U; vert_idx < vertice_count; ++vert_idx) {
                                auto& v = gpu_vertices.at(vert_idx);
                                pMesh->mVertices[vert_idx] = aiVector3D(v.p.x, v.p.y, v.p.z);
                                if (pMesh->mNumFaces) {
                                    pMesh->mNormals[vert_idx] = aiVector3D(v.n.x, v.n.y, v.n.z);
                                }
                                if (_has_vertex_colors) {
                                    pMesh->mColors[color_map_index][vert_idx] = aiColor4D(v.c.x, v.c.y, v.c.z, v.c.w);
                                }
                                if (_has_texture_coords) {
                                    pMesh->mTextureCoords[uv_map_index][vert_idx] = aiVector3D(v.t.x, v.t.y, 0.0f);
                                }
                            }

                            if (primitive_type == SceneModelPrimitiveType::SCENE_MODEL_PRIMITIVE_TYPE_POINTS) {
                                for (size_t face_idx = 0U; face_idx < pMesh->mNumFaces; ++face_idx) {
                                    auto& i0 = gpu_indices.at(face_idx);

                                    aiFace& face = pMesh->mFaces[face_idx];
                                    face.mIndices = new uint32_t[1];
                                    face.mNumIndices = 1;

                                    face.mIndices[0] = i0;
                                }
                            } else if (primitive_type == SceneModelPrimitiveType::SCENE_MODEL_PRIMITIVE_TYPE_CURVES) {
                                for (size_t face_idx = 0U; face_idx < pMesh->mNumFaces; ++face_idx) {
                                    auto& i0 = gpu_indices.at(face_idx + 0);
                                    auto& i1 = gpu_indices.at(face_idx + 1);

                                    aiFace& face = pMesh->mFaces[face_idx];
                                    face.mIndices = new uint32_t[2];
                                    face.mNumIndices = 2;

                                    face.mIndices[0] = i0;
                                    face.mIndices[1] = i1;
                                }
                            } else if (primitive_type == SceneModelPrimitiveType::SCENE_MODEL_PRIMITIVE_TYPE_FACES) {
                                for (size_t face_idx = 0U, ind_idx = 0U; face_idx < pMesh->mNumFaces; ++face_idx, ind_idx += 3U) {
                                    auto& i0 = gpu_indices.at(ind_idx + 0);
                                    auto& i1 = gpu_indices.at(ind_idx + 1);
                                    auto& i2 = gpu_indices.at(ind_idx + 2);

                                    aiFace& face = pMesh->mFaces[face_idx];
                                    face.mIndices = new uint32_t[3];
                                    face.mNumIndices = 3;

                                    face.mIndices[0] = i0;
                                    face.mIndices[1] = i1;
                                    face.mIndices[2] = i2;
                                }
                            }
                        }
                    }
                }

                Assimp::Exporter aiExporter;
                auto ps = FileHelper::Instance()->ParsePathFileName(m_FilePathName);
                if (ps.isOk) {
                    if (ps.ext == "ply") {
                        aiExporter.Export(scene_ptr, "ply", m_FilePathName);
                    }
                }

                // scene et tout les object créé dans scene
                // seront detruit en quittant le scope
                delete scene_ptr;
            }
        } catch (const std::exception& ex) {
            LogVarError("unknown error : %s", ex.what());
        }
    }
}
