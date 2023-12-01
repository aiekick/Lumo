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

#include <ctools/FileHelper.h>

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

std::shared_ptr<ModelExporterModule> ModelExporterModule::Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode) {
    ZoneScoped;

    auto res = std::make_shared<ModelExporterModule>(vVulkanCore);
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

ModelExporterModule::ModelExporterModule(GaiApi::VulkanCoreWeak vVulkanCore) : m_VulkanCore(vVulkanCore) {
    ZoneScoped;
    unique_SaveMeshFileDialog_id = ct::toStr("OpenMeshFileDialog%u", (uintptr_t)this);
    unique_SavePathFileDialog_id = ct::toStr("SavePathFileDialog%u", (uintptr_t)this);
    m_InputTextPrefix.SetText("frame");
    m_InputTextSaveFilePathName.SetText("export.dae");
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
    SetExecutionWhenNeededOnly(true);
    return true;
}

void ModelExporterModule::Unit() {
    ZoneScoped;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TASK ////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool ModelExporterModule::ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState) {
    UNUSED(vCurrentFrame);
    UNUSED(vCmd);
    UNUSED(vBaseNodeState);

    m_AutoSaveModelIfNeeded(vCurrentFrame);

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool ModelExporterModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    bool change = false;

    ImGui::Header("Auto exporter");
    if (m_ExportFrames) {
        if (ImGui::ContrastedButton("Stop Auto Save")) {
            m_StopAutoSave();
            change = true;
        }
        if (m_FramesCountToExport) {
            float ratio = (float)m_CurrentFrameToExport / (float)m_FramesCountToExport;
            const auto& str = ct::toStr("%u/%u frames", m_CurrentFrameToExport, m_FramesCountToExport);
            ImGui::ProgressBar(ratio, ImVec2(-1.0f, 0.0f), str.c_str());
        }
    } else {
        change |= ImGui::Checkbox("auto save preview enabled", &m_AutoSaverPreviewEnabled);
        if (!m_AutoSaverPreviewEnabled) {
            change |= ImGui::Checkbox("auto save enabled", &m_AutoSaverEnabled);
        }
        if (m_AutoSaverEnabled || m_AutoSaverPreviewEnabled) {
            change |= ImGui::InputUIntDefault(200.0f, "frames to export", &m_FramesCountToExport, 1, 10, 1);
            change |= ImGui::InputUIntDefault(200.0f, "frames to jump", &m_FramesCountToJump, 1, 10, 1);
            if (ImGui::InputUIntDefault(200.0f, "frames per sec", &m_FramesCountPerSec, 1, 10, 1)) {
                m_FramesCountPerSec = ct::maxi(m_FramesCountPerSec, 1U);
                change = true;
            }
            if (m_AutoSaverEnabled) {
                change |= m_InputTextPrefix.DisplayInputText(0.0f, "Postfix", "_frame_");
                change |= ImGui::Checkbox("Generate SketchFab TimeFrame", &m_GenerateSketchFabTimeFrame);
                change |= m_InputTextSaveFilePathName.DisplayInputText(0.0f, "File path name", "test.ply");
                change |= ImGui::CheckBoxBoolDefault("Export Normals", &m_ExportNormals, true);
                change |= ImGui::CheckBoxBoolDefault("Export Vertexs Color", &m_ExportVertexColors, true);
                if (ImGui::ContrastedButton("Set 3D file path name")) {
                    ImGuiFileDialog::Instance()->OpenDialog(unique_SavePathFileDialog_id, "Save 3D File", ".ply,.dae", m_FilePath, m_FilePathName, 1,
                        nullptr, ImGuiFileDialogFlags_Modal);
                }
            }
            if (ImGui::ContrastedButton("Start Auto Saver")) {
                m_StartAutoSave();
                change = true;
            }
        }
    }

    ImGui::Header("Manual export");
    if (ImGui::ContrastedButton("Save Model")) {
        // tofix : gltf export crash ...
        ImGuiFileDialog::Instance()->OpenDialog(
            unique_SaveMeshFileDialog_id, "Save 3D File", ".ply,.dae", m_FilePath, m_FilePathName, 1, nullptr, ImGuiFileDialogFlags_Modal);
    }

    if (change) {
        m_PreviewStarted = false;
    }

    return change;
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
    // manual save file
    if (ImGuiFileDialog::Instance()->Display(unique_SaveMeshFileDialog_id, ImGuiWindowFlags_NoCollapse, min, max)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            m_SaveModel(ImGuiFileDialog::Instance()->GetFilePathName());
        }
        ImGuiFileDialog::Instance()->Close();
    }
    // path where to auto save
    if (ImGuiFileDialog::Instance()->Display(unique_SavePathFileDialog_id, ImGuiWindowFlags_NoCollapse, min, max)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            m_InputTextSaveFilePathName.SetText(ImGuiFileDialog::Instance()->GetFilePathName());
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

//////////////////////////////////////////////////////////////////////////////////////////////
//// VARIABLE INPUT //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void ModelExporterModule::SetVariable(const uint32_t& vVarIndex, SceneVariableWeak vSceneVariable) {
    ZoneScoped;
    if (vVarIndex < m_SceneVariables.size()) {
        m_SceneVariables[vVarIndex] = vSceneVariable;
        auto varPtr = m_SceneVariables[vVarIndex].lock();
        if (varPtr) {
            if (varPtr->GetType() == "WIDGET_BOOLEAN") {
                if (vVarIndex == 0U) {
                    if (varPtr->GetDatas().m_Boolean) {
                        m_StartAutoSave(); // start if true at least
                        // the stop must be done by the user or if count frames are reached
                    }
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL OUTPUT ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneModelWeak ModelExporterModule::GetModel() {
    ZoneScoped;
    return m_InputModel;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ModelExporterModule::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;
    std::string str;
    str += vOffset + "<model_exporter_module>\n";
    str += vOffset + "<frames_to_export>" + ct::toStr(m_FramesCountToExport) + "</frames_to_export>\n";
    str += vOffset + "<frames_to_jump>" + ct::toStr(m_FramesCountToJump) + "</frames_to_jump>\n";
    str += vOffset + "<frames_per_secs>" + ct::toStr(m_FramesCountPerSec) + "</frames_per_secs>\n";
    str += vOffset + "<generate_sketchfab_timeframe>" + (m_GenerateSketchFabTimeFrame ? "true" : "false") + "</generate_sketchfab_timeframe>\n";
    str += vOffset + "<export_vertexs_color>" + (m_ExportVertexColors ? "true" : "false") + "</export_vertexs_color>\n";
    str += vOffset + "<export_normals>" + (m_ExportNormals ? "true" : "false") + "</export_normals>\n";
    str += vOffset + "<auto_saver_enabled>" + (m_AutoSaverEnabled ? "true" : "false") + "</auto_saver_enabled>\n";
    str += vOffset + "<auto_saver_preview_enabled>" + (m_AutoSaverPreviewEnabled ? "true" : "false") + "</auto_saver_preview_enabled>\n";
    str += vOffset + "<postfix>" + m_InputTextPrefix.GetText() + "</postfix>\n";
    str += vOffset + "<file_save>" + m_InputTextSaveFilePathName.GetText() + "</file_save>\n";
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
        if (strName == "frames_to_export") {
            m_FramesCountToExport = ct::uvariant(strValue).GetU();
        } else if (strName == "frames_to_jump") {
            m_FramesCountToJump = ct::uvariant(strValue).GetU();
        } else if (strName == "frames_per_secs") {
            m_FramesCountPerSec = ct::uvariant(strValue).GetU();
        } else if (strName == "generate_sketchfab_timeframe") {
            m_GenerateSketchFabTimeFrame = ct::ivariant(strValue).GetB();
        } else if (strName == "export_vertexs_color") {
            m_ExportVertexColors = ct::ivariant(strValue).GetB();
        } else if (strName == "export_normals") {
            m_ExportNormals = ct::ivariant(strValue).GetB();
        } else if (strName == "auto_saver_enabled") {
            m_AutoSaverEnabled = ct::ivariant(strValue).GetB();
        } else if (strName == "auto_saver_preview_enabled") {
            m_AutoSaverPreviewEnabled = ct::ivariant(strValue).GetB();
        } else if (strName == "postfix") {
            m_InputTextPrefix.SetText(strValue);
        } else if (strName == "file_save") {
            m_InputTextSaveFilePathName.SetText(strValue);
        }
    }

    return true;
}

void ModelExporterModule::AfterNodeXmlLoading() {
    ZoneScoped;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ModelExporterModule::m_SaveModel(const std::string& vFilePathName) {
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

                            bool _has_vertex_colors = m_ExportVertexColors && meshPtr->HasVertexColors();
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

                            // bool _has_normals = meshPtr->HasTextureCoords();

                            if (m_ExportNormals) {
                                pMesh->mNormals = new aiVector3D[vertice_count];
                            }
                            pMesh->mFaces = new aiFace[pMesh->mNumFaces];

                            for (size_t vert_idx = 0U; vert_idx < vertice_count; ++vert_idx) {
                                auto& v = gpu_vertices.at(vert_idx);
                                pMesh->mVertices[vert_idx] = aiVector3D(v.p.x, v.p.y, v.p.z);
                                if (m_ExportNormals && pMesh->mNumFaces) {
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
                    } else if (ps.ext == "dae") {
                        aiExporter.Export(scene_ptr, "collada", m_FilePathName);
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

void ModelExporterModule::m_StartAutoSave() {
    if ((m_AutoSaverEnabled || m_AutoSaverPreviewEnabled) && !m_ExportFrames && m_FramesCountToExport) {
        m_ExportFrames = true;
        m_PreviewStarted = true;
        m_TimeStep = (float)m_FramesCountToJump / (float)m_FramesCountPerSec;
        m_CurrentFrameToExport = 0U;
        m_SketchFabTimeFrameFileContent.clear();
        NeedNewExecution();
    }
}

void ModelExporterModule::m_StopAutoSave() {
    m_ExportFrames = false;
    if (!m_AutoSaverPreviewEnabled) {
        m_PreviewStarted = false;
    }
    if (m_GenerateSketchFabTimeFrame) {
        auto ps = FileHelper::Instance()->ParsePathFileName(m_InputTextSaveFilePathName.GetText());
        if (ps.isOk) {
            const auto& file_path_name = ps.GetFPNE_WithNameExt("sketchfab", "timeframe");
            FileHelper::Instance()->SaveStringToFile(m_SketchFabTimeFrameFileContent, file_path_name);
        }
    }
}

void ModelExporterModule::m_AutoSaveModelIfNeeded(const uint32_t& vCurrentFrame) {
    if (m_ExportFrames) {
        if (m_CurrentFrameToExport < m_FramesCountToExport) {
            if ((m_LastSavedFrame + m_FramesCountToJump) <= vCurrentFrame) {
                if (!m_AutoSaverPreviewEnabled) {
                    auto ps = FileHelper::Instance()->ParsePathFileName(m_InputTextSaveFilePathName.GetText());
                    if (ps.isOk) {
                        const auto& file_name = ps.name + "_" + m_InputTextPrefix.GetText() + "_" + ct::toStr("%u", m_CurrentFrameToExport);
                        const auto& file_path_name = ps.GetFPNE_WithName(file_name);
                        m_SaveModel(file_path_name);
                        if (m_GenerateSketchFabTimeFrame) {
                            m_SketchFabTimeFrameFileContent += ct::toStr("%.5f %s.%s\n", m_TimeStep, file_name.c_str(), ps.ext.c_str());
                        }
                    }
                }
                m_LastSavedFrame = vCurrentFrame;
                ++m_CurrentFrameToExport;
            }
            NeedNewExecution();
        } else {
            m_StopAutoSave();
        }
    }
}
