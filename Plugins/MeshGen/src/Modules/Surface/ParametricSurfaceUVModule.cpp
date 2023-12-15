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

#include "ParametricSurfaceUVModule.h"

#include <cinttypes>
#include <functional>
#include <LumoBackend/Systems/CommonSystem.h>
#include <LumoBackend/Graph/Base/BaseNode.h>

#ifdef CUSTOM_LUMO_BACKEND_CONFIG
#include CUSTOM_LUMO_BACKEND_CONFIG
#else
#include <LumoBackend/Headers/LumoBackendConfigHeader.h>
#endif  // CUSTOM_LUMO_BACKEND_CONFIG

using namespace GaiApi;

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<ParametricSurfaceUVModule> ParametricSurfaceUVModule::Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode) {
    ZoneScoped;

    auto res = std::make_shared<ParametricSurfaceUVModule>(vVulkanCore);
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

ParametricSurfaceUVModule::ParametricSurfaceUVModule(GaiApi::VulkanCoreWeak vVulkanCore) : m_VulkanCore(vVulkanCore) {
    ZoneScoped;
}

ParametricSurfaceUVModule::~ParametricSurfaceUVModule() {
    ZoneScoped;

    Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ParametricSurfaceUVModule::Init() {
    ZoneScoped;

    m_SceneModelPtr = SceneModel::Create();

    prAddVar("u", 0.0);
    prAddVar("v", 0.0);
    prUpdateMesh();

    return true;
}

void ParametricSurfaceUVModule::Unit() {
    ZoneScoped;

    m_SceneModelPtr.reset();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool ParametricSurfaceUVModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    prDrawWidgets();
    return false;
}

bool ParametricSurfaceUVModule::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool ParametricSurfaceUVModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// VARIABLE SLOT INPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void ParametricSurfaceUVModule::SetVariable(const uint32_t& vVarIndex, SceneVariableWeak vSceneVariable, void* vUserDatas) {
    ZoneScoped;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL OUTPUT ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneModelWeak ParametricSurfaceUVModule::GetModel() {
    ZoneScoped;

    return m_SceneModelPtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ParametricSurfaceUVModule::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;

    std::string str;

    str += vOffset + "<parametric_surface_uv_module>\n";
    str += vOffset + "\t<expr_x>" + ct::toStr(m_ExprX) + "</expr_x>\n";
    str += vOffset + "\t<expr_y>" + ct::toStr(m_ExprY) + "</expr_y>\n";
    str += vOffset + "\t<expr_z>" + ct::toStr(m_ExprZ) + "</expr_z>\n";
    str += vOffset + "\t<start_u>" + ct::toStr(m_Start_U) + "</start_u>\n";
    str += vOffset + "\t<end_u>" + ct::toStr(m_End_U) + "</end_u>\n";
    str += vOffset + "\t<step_u>" + ct::toStr(m_Step_U) + "</step_u>\n";
    str += vOffset + "\t<start_v>" + ct::toStr(m_Start_V) + "</start_v>\n";
    str += vOffset + "\t<end_v>" + ct::toStr(m_End_V) + "</end_v>\n";
    str += vOffset + "\t<step_v>" + ct::toStr(m_Step_V) + "</step_v>\n";
    str += vOffset + "\t<close_u>" + ct::toStr(m_CloseU) + "</close_u>\n";
    str += vOffset + "\t<close_v>" + ct::toStr(m_CloseV) + "</close_v>\n";
    str += vOffset + "\t<vars>\n";
    for (const auto& var : m_VarNameValues) {
        if (var.first == "u" || var.first == "v")
            continue;

        str += vOffset + "\t\t<var name=\"" + var.first + "\" value=\"" + ct::toStr(var.second) + "\"/>\n";
    }
    str += vOffset + "\t</vars>\n";
    str += vOffset + "</parametric_surface_uv_module>\n";

    return str;
}

bool ParametricSurfaceUVModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (strParentName == "parametric_surface_uv_module") {
        if (strName == "expr_x") {
            ct::ResetBuffer(m_ExprX);
            ct::AppendToBuffer(m_ExprX, ParametricSurfaceUVModule::s_EXPR_MAX_LEN, strValue);
        } else if (strName == "expr_y") {
            ct::ResetBuffer(m_ExprY);
            ct::AppendToBuffer(m_ExprY, ParametricSurfaceUVModule::s_EXPR_MAX_LEN, strValue);
        } else if (strName == "expr_z") {
            ct::ResetBuffer(m_ExprZ);
            ct::AppendToBuffer(m_ExprZ, ParametricSurfaceUVModule::s_EXPR_MAX_LEN, strValue);
        } else if (strName == "start_u") {
            m_Start_U = ct::ivariant(strValue).GetD();
        } else if (strName == "end_u") {
            m_End_U = ct::ivariant(strValue).GetD();
        } else if (strName == "step_u") {
            m_Step_U = ct::ivariant(strValue).GetD();
        } else if (strName == "start_v") {
            m_Start_V = ct::ivariant(strValue).GetD();
        } else if (strName == "end_v") {
            m_End_V = ct::ivariant(strValue).GetD();
        } else if (strName == "step_v") {
            m_Step_V = ct::ivariant(strValue).GetD();
        } else if (strName == "close_u") {
            m_CloseU = ct::ivariant(strValue).GetB();
        } else if (strName == "close_v") {
            m_CloseV = ct::ivariant(strValue).GetB();
        }
    } else if (strParentName == "vars" && strName == "var") {
        std::string var_name;
        double var_value;

        for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
            std::string attName = attr->Name();
            std::string attValue = attr->Value();

            if (attName == "name")
                var_name = attValue;
            else if (attName == "value")
                var_value = ct::fvariant(attValue).GetD();
        }

        prAddVar(var_name, var_value);
    }

    return true;
}

void ParametricSurfaceUVModule::AfterNodeXmlLoading() {
    ZoneScoped;

    prAddVar("u", 0.0);
    prAddVar("v", 0.0);
    prUpdateMesh();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ParametricSurfaceUVModule::prDrawWidgets() {
    if (ImGui::CollapsingHeader("Parametric Curve")) {
        bool change = false;

        change |= ImGui::InputDoubleDefault(0.0f, "Start U", &m_Start_U, 0.0, "%f");
        change |= ImGui::InputDoubleDefault(0.0f, "End U", &m_End_U, 3.0, "%f");
        change |= ImGui::InputDoubleDefault(0.0f, "Step U", &m_Step_U, 0.5, "%f");

        ImGui::Separator();

        change |= ImGui::InputDoubleDefault(0.0f, "Start V", &m_Start_V, 0.0, "%f");
        change |= ImGui::InputDoubleDefault(0.0f, "End V", &m_End_V, 3.0, "%f");
        change |= ImGui::InputDoubleDefault(0.0f, "Step V", &m_Step_V, 0.5, "%f");

        ImGui::Separator();

        change |= prDrawInputExpr("x(u,v)", "##expr_x", m_ExprX, ParametricSurfaceUVModule::s_EXPR_MAX_LEN, m_Err_x, "u - 1.5");
        change |= prDrawInputExpr("y(u,v)", "##expr_y", m_ExprY, ParametricSurfaceUVModule::s_EXPR_MAX_LEN, m_Err_y, "v - 1.5");
        change |= prDrawInputExpr("z(u,v)", "##expr_z", m_ExprZ, ParametricSurfaceUVModule::s_EXPR_MAX_LEN, m_Err_z, "0");

        ImGui::Separator();

        change |= ImGui::CheckBoxBoolDefault("Close U", &m_CloseU, false);
        change |= ImGui::CheckBoxBoolDefault("Close V", &m_CloseV, false);

        ImGui::Separator();

        change |= ImGui::ContrastedButton("Eval");

        ImGui::SameLine();

        if (ImGui::ContrastedButton("Center Model")) {
            CommonSystem::Instance()->SetTargetXYZ(-(ct::fvec3)m_CenterPoint, true);
        }

        change |= prDrawVars();

        if (change) {
            prUpdateMesh();
        }
    }
}

void ParametricSurfaceUVModule::prAddVar(const std::string& vName, const double& vValue) {
    if (!vName.empty()) {
        if (m_VarNameValues.find(vName) == m_VarNameValues.end()) {
            m_VarNameValues.emplace(vName, vValue);
            const auto& it = m_VarNameValues.find(vName);

            te_variable v;
            v.name = it->first.c_str();  // ref so m_VarNames name must already exist
            v.address = &it->second;
            v.context = 0;
            v.type = 0;
            m_Vars.push_back(v);
        }
    }
}

void ParametricSurfaceUVModule::prDelVar(const std::string& vName) {
    auto it = m_VarNameValues.find(vName);
    if (it != m_VarNameValues.end()) {
        for (auto var_it = m_Vars.begin(); var_it != m_Vars.end(); ++var_it) {
            if (strcmp(var_it->name, it->first.c_str()) == 0)  // is equal
            {
                m_Vars.erase(var_it);
                m_VarNameValues.erase(vName);
                break;
            }
        }
    }
}

// https://github.com/aiekick/Grid_Computational
static void prGetVertexIndexs(
    const bool& close_faces_u, const bool& close_faces_v, const int& cx, const int& cy, const int& sx, const int& sy, int* out_v) {
    /* poly p
       v03 --- v02
        |   p   |
       v00 --- v01
    */

    const int lx2 = sx - 2;
    const int lx1 = sx - 1;
    const int ly2 = sy - 2;

    if (close_faces_u && close_faces_v) {
        out_v[0] = cx + lx1 * (cy);
        out_v[1] = out_v[0] + 1;
        out_v[3] = cx + lx1 * (cy + 1);
        out_v[2] = out_v[3] + 1;

        if (cx == lx2 && cy == ly2) {
            out_v[3] = cx;
            out_v[2] = 0;
            out_v[1] = out_v[0] - lx2;
            return;  // fast quit

        } else {
            if (cy == ly2) {
                out_v[3] = cx;
                out_v[2] = out_v[3] + 1;
                out_v[1] = out_v[0] + 1;
                return;  // fast quit

            } else if (cx == lx2) {
                out_v[1] = out_v[0] - lx2;
                out_v[2] = out_v[1] + lx1;
                return;  // fast quit
            }
        }
        return;  // fast quit

    } else {
        if (close_faces_v) {
            out_v[0] = cx + sx * cy;
            out_v[1] = cx + sx * cy + 1;
            if (cy < ly2) {
                out_v[3] = cx + sx * (cy + 1);
                out_v[2] = out_v[3] + 1;
                return;  // fast quit

            } else {
                out_v[3] = cx;
                out_v[2] = out_v[3] + 1;
                return;  // fast quit
            }

        } else if (close_faces_u) {
            out_v[0] = cx + lx1 * (cy);
            out_v[3] = cx + lx1 * (cy + 1);

            if (cx < lx2) {
                out_v[1] = out_v[0] + 1;
                out_v[2] = out_v[3] + 1;
                return;  // fast quit

            } else {
                out_v[1] = out_v[0] - lx2;
                out_v[2] = out_v[1] + lx1;
                return;  // fast quit
            }

        } else {
            out_v[0] = cx + sx * (cy);
            out_v[1] = out_v[0] + 1;
            out_v[3] = cx + sx * (cy + 1);
            out_v[2] = out_v[3] + 1;
            return;  // fast quit
        }
        return;  // fast quit
    }
}

void ParametricSurfaceUVModule::prUpdateMesh() {
    te_expr* expr_x = te_compile(m_ExprX, m_Vars.data(), (int)m_Vars.size(), &m_Err_x);
    te_expr* expr_y = te_compile(m_ExprY, m_Vars.data(), (int)m_Vars.size(), &m_Err_y);
    te_expr* expr_z = te_compile(m_ExprZ, m_Vars.data(), (int)m_Vars.size(), &m_Err_z);

    if (expr_x && expr_y && expr_z) {
        VerticeArray vertices;
        IndiceArray indices;

        if (IS_DOUBLE_EQUAL(m_Step_U, 0.0))
            return;
        if (IS_DOUBLE_EQUAL(m_Step_V, 0.0))
            return;

        int verts_len_U = (int)ceil((m_End_U - m_Start_U) / m_Step_U);
        if (!verts_len_U)
            return;

        int verts_len_V = (int)ceil((m_End_V - m_Start_V) / m_Step_V);
        if (!verts_len_V)
            return;

        int verts_len = verts_len_U * verts_len_V;
        int inds_len = verts_len * 6;

        vertices.resize(verts_len);
        indices.resize(inds_len);

        m_CenterPoint = 0.0;

        // recompute of step_u and step_v for avoid discontinuity
        // (verts_len - 1) for loop connection with close_curve == false
        // or verts_len for loop connection with close_curve == true
        double step_u_d = (m_End_U - m_Start_U) / (double)(verts_len_U - 1);  // -1 for loop connection
        double step_v_d = (m_End_V - m_Start_V) / (double)(verts_len_V - 1);  // -1 for loop connection

        auto& u_value = m_VarNameValues.at("u");
        u_value = (double)m_Start_U;
        auto& v_value = m_VarNameValues.at("v");
        v_value = (double)m_Start_V;

        const int polys_len_u = verts_len_U - 1;
        const int polys_len_v = verts_len_V - 1;

        auto vert_it = vertices.begin();
        auto ind_it = indices.begin();

        double tex_u = 1.0 / (double)(verts_len_U - 1);
        double tex_v = 1.0 / (double)(verts_len_V - 1);

        int vi[4];

        double tex_u_accum = 0.0;
        double tex_v_accum = 0.0;

        for (int verts_index_v = 0; verts_index_v < verts_len_V; ++verts_index_v) {
            v_value = m_Start_U + verts_index_v * step_v_d;  // no errs propagation this way
            for (int verts_index_u = 0; verts_index_u < verts_len_U; ++verts_index_u) {
                u_value = m_Start_V + verts_index_u * step_u_d;                  // no errs propagation this way
                if (verts_index_u < polys_len_u && verts_index_v < polys_len_v)  // add a face
                {
                    /* poly p
                       v03 --- v02
                        |   p   |
                       v00 --- v01 */

                    // get vertex organization
                    prGetVertexIndexs(m_CloseU, m_CloseV, verts_index_u, verts_index_v, verts_len_U, verts_len_V, vi);

                    // first tri
                    *ind_it = vi[0];
                    ++ind_it;
                    *ind_it = vi[1];
                    ++ind_it;
                    *ind_it = vi[2];
                    ++ind_it;

                    // second tri
                    *ind_it = vi[0];
                    ++ind_it;
                    *ind_it = vi[2];
                    ++ind_it;
                    *ind_it = vi[3];
                    ++ind_it;
                }

                if (m_CloseU && (verts_index_u == polys_len_u) || m_CloseV && (verts_index_v == polys_len_v))
                    continue;

                vert_it->p.x = ct::clamp((float)te_eval(expr_x), -FLT_MAX, FLT_MAX);
                vert_it->p.y = ct::clamp((float)te_eval(expr_y), -FLT_MAX, FLT_MAX);
                vert_it->p.z = ct::clamp((float)te_eval(expr_z), -FLT_MAX, FLT_MAX);

                vert_it->n.x = 0.0;
                vert_it->n.y = 0.0;
                vert_it->n.z = 0.0;

                vert_it->t.x = (float)tex_u_accum;
                vert_it->t.y = (float)tex_v_accum;

                ++vert_it;

                tex_u_accum += tex_u;
            }

            tex_v_accum += tex_v;
        }

        auto sceneMeshPtr = SceneMesh<VertexStruct::P3_N3_TA3_BTA3_T2_C4>::Create(m_VulkanCore, vertices, indices);
        sceneMeshPtr->SetPrimitiveType(SceneModelPrimitiveType::SCENE_MODEL_PRIMITIVE_TYPE_CURVES);
        m_SceneModelPtr->clear();
        m_SceneModelPtr->Add(sceneMeshPtr);

        auto parentNodePtr = GetParentNode().lock();
        if (parentNodePtr) {
            parentNodePtr->SendFrontNotification(ModelUpdateDone);
        }
    }

    if (!expr_x)
        printf("Parse error for params x at %d\n", m_Err_x);
    if (!expr_y)
        printf("Parse error for params y at %d\n", m_Err_y);
    if (!expr_z)
        printf("Parse error for params z at %d\n", m_Err_z);

    te_free(expr_x);
    te_free(expr_y);
    te_free(expr_z);
}

bool ParametricSurfaceUVModule::prDrawInputExpr(
    const char* vLabel, const char* vBufferLabel, char* vBuffer, size_t vBufferSize, const int& vError, const char* vDdefaultValue) {
    bool change = false;

    ImGui::Text(vLabel);
    ImGui::SameLine();
    if (vError)
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::CustomStyle::BadColor);
    if (ImGui::ContrastedButton(LUMO_BACKEND_ICON_LABEL_RESET)) {
        ct::ResetBuffer(vBuffer);
        ct::AppendToBuffer(vBuffer, vBufferSize, vDdefaultValue);
    }
    ImGui::SameLine();
    change |= ImGui::InputText(vBufferLabel, vBuffer, vBufferSize);
    if (vError) {
        ImGui::PopStyleColor();
        ImGui::Text("Err at pos : %i", vError);
    }

    return change;
}
bool ParametricSurfaceUVModule::prDrawVars() {
    static std::string var_to_erase;

    bool change = false;

    ImGui::Separator();

    if (ImGui::ContrastedButton("Add Var")) {
        prAddVar(m_VarToAddBuffer, 0.0);
        change = true;
    }

    ImGui::SameLine();

    ImGui::InputText("##VarToAdd", m_VarToAddBuffer, s_EXPR_MAX_LEN);

    ImGui::Separator();

    for (auto& var : m_VarNameValues) {
        if (var.first == "u" || var.first == "v")
            continue;

        if (ImGui::ContrastedButton("R")) {
            var_to_erase = var.first;
        }
        ImGui::SameLine();
        ImGui::Text("%s", var.first.c_str());
        ImGui::SameLine();
        ImGui::PushID(&var.second);
        change |= ImGui::InputDouble("##value", &var.second);
        ImGui::PopID();
    }

    if (!var_to_erase.empty()) {
        prDelVar(var_to_erase);
        var_to_erase.clear();
        change = true;
    }

    return change;
}
