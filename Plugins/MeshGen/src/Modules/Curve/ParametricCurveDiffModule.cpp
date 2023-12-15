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

#include "ParametricCurveDiffModule.h"

#include <cinttypes>
#include <functional>
#include <LumoBackend/Systems/CommonSystem.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Interfaces/NotifyInterface.h>

#ifdef CUSTOM_LUMO_BACKEND_CONFIG
#include CUSTOM_LUMO_BACKEND_CONFIG
#else
#include <LumoBackend/Headers/LumoBackendConfigHeader.h>
#endif  // CUSTOM_LUMO_BACKEND_CONFIG

using namespace GaiApi;

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<ParametricCurveDiffModule> ParametricCurveDiffModule::Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode) {
    ZoneScoped;

    auto res = std::make_shared<ParametricCurveDiffModule>(vVulkanCore);
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

ParametricCurveDiffModule::ParametricCurveDiffModule(GaiApi::VulkanCoreWeak vVulkanCore) : m_VulkanCore(vVulkanCore) {
    ZoneScoped;
}

ParametricCurveDiffModule::~ParametricCurveDiffModule() {
    ZoneScoped;

    Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ParametricCurveDiffModule::Init() {
    ZoneScoped;

    m_SceneModelPtr = SceneModel::Create();

    prAddVar("x", 0.0);
    prAddVar("y", 0.0);
    prAddVar("z", 0.0);
    prUpdateMesh();

    return true;
}

void ParametricCurveDiffModule::Unit() {
    ZoneScoped;

    m_SceneModelPtr.reset();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool ParametricCurveDiffModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    prDrawWidgets();
    return false;
}

bool ParametricCurveDiffModule::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool ParametricCurveDiffModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// VARIABLE SLOT INPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void ParametricCurveDiffModule::SetVariable(const uint32_t& vVarIndex, SceneVariableWeak vSceneVariable, void* vUserDatas) {
    ZoneScoped;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL OUTPUT ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneModelWeak ParametricCurveDiffModule::GetModel() {
    ZoneScoped;

    return m_SceneModelPtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ParametricCurveDiffModule::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;

    std::string str;

    str += vOffset + "<parametric_curve_diff_module>\n";
    str += vOffset + "\t<expr_x>" + ct::toStr(m_ExprX) + "</expr_x>\n";
    str += vOffset + "\t<expr_y>" + ct::toStr(m_ExprY) + "</expr_y>\n";
    str += vOffset + "\t<expr_z>" + ct::toStr(m_ExprZ) + "</expr_z>\n";
    str += vOffset + "\t<start_location>" + m_StartLocation.string() + "</start_location>\n";
    str += vOffset + "\t<step_count>" + ct::toStr(m_StepCount) + "</step_count>\n";
    str += vOffset + "\t<step_size>" + ct::toStr(m_StepSize) + "</step_size>\n";
    str += vOffset + "\t<vars>\n";
    for (const auto& var : m_VarNameValues) {
        if (var.first == "x" || var.first == "y" || var.first == "z")
            continue;

        str += vOffset + "\t\t<var name=\"" + var.first + "\" value=\"" + ct::toStr(var.second) + "\"/>\n";
    }
    str += vOffset + "\t</vars>\n";
    str += vOffset + "</parametric_curve_diff_module>\n";

    return str;
}

bool ParametricCurveDiffModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (strParentName == "parametric_curve_diff_module") {
        if (strName == "expr_x") {
            ct::ResetBuffer(m_ExprX);
            ct::AppendToBuffer(m_ExprX, ParametricCurveDiffModule::s_EXPR_MAX_LEN, strValue);
        } else if (strName == "expr_y") {
            ct::ResetBuffer(m_ExprY);
            ct::AppendToBuffer(m_ExprY, ParametricCurveDiffModule::s_EXPR_MAX_LEN, strValue);
        } else if (strName == "expr_z") {
            ct::ResetBuffer(m_ExprZ);
            ct::AppendToBuffer(m_ExprZ, ParametricCurveDiffModule::s_EXPR_MAX_LEN, strValue);
        } else if (strName == "start_location") {
            m_StartLocation = ct::dvariant(strValue).GetV3();
        } else if (strName == "step_count") {
            m_StepCount = (size_t)ct::uvariant(strValue).GetU();
        } else if (strName == "step_size") {
            m_StepSize = ct::ivariant(strValue).GetD();
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

void ParametricCurveDiffModule::AfterNodeXmlLoading() {
    ZoneScoped;

    prAddVar("x", 0.0);
    prAddVar("y", 0.0);
    prAddVar("z", 0.0);
    prUpdateMesh();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ParametricCurveDiffModule::prAddVar(const std::string& vName, const double& vValue) {
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

void ParametricCurveDiffModule::prDelVar(const std::string& vName) {
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

bool ParametricCurveDiffModule::prDrawInputExpr(
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

bool ParametricCurveDiffModule::prDrawVars() {
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
        if (var.first == "x" || var.first == "y" || var.first == "z")
            continue;

        if (ImGui::ContrastedButton(LUMO_BACKEND_ICON_LABEL_RESET)) {
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

void ParametricCurveDiffModule::prDrawWidgets() {
    if (ImGui::CollapsingHeader("Parametric Curve Diff")) {
        bool change = false;

        change |= ImGui::InputDoubleDefault(0.0f, "Start Location x", &m_StartLocation.x, 0.001, "%f");
        change |= ImGui::InputDoubleDefault(0.0f, "Start Location Y", &m_StartLocation.y, 0.001, "%f");
        change |= ImGui::InputDoubleDefault(0.0f, "Start Location z", &m_StartLocation.z, 0.001, "%f");
        change |= ImGui::InputUIntDefault(0.0f, "Step Count", &m_StepCount, 10000U, 1, 10);
        change |= ImGui::InputDoubleDefault(0.0f, "Step Size", &m_StepSize, 0.01, "%f");

        ImGui::Separator();

        change |= prDrawInputExpr("dx(x,y,z)", "##expr_x", m_ExprX, ParametricCurveDiffModule::s_EXPR_MAX_LEN, m_Err_x, "10.0*(y-x)");
        change |= prDrawInputExpr("dy(x,y,z)", "##expr_y", m_ExprY, ParametricCurveDiffModule::s_EXPR_MAX_LEN, m_Err_y, "28.0*x-y-x*z");
        change |= prDrawInputExpr("dz(x,y,z)", "##expr_z", m_ExprZ, ParametricCurveDiffModule::s_EXPR_MAX_LEN, m_Err_z, "x*y-2.66667*z");

        ImGui::Separator();

        change |= ImGui::CheckBoxBoolDefault("Close Curve", &m_CloseCurve, false);

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

void ParametricCurveDiffModule::prUpdateMesh() {
    VerticeArray vertices;
    IndiceArray indices;

    const int verts_len = m_StepCount;
    if (verts_len < 2) {
        LogVarError("Vertex count < 2, no mesh will be generated");
        return;
    }

    vertices.resize(verts_len);
    indices.resize(m_CloseCurve ? verts_len + 1 : verts_len);

    te_expr* expr_x_ptr = te_compile(m_ExprX, m_Vars.data(), (int)m_Vars.size(), &m_Err_x);
    te_expr* expr_y_ptr = te_compile(m_ExprY, m_Vars.data(), (int)m_Vars.size(), &m_Err_y);
    te_expr* expr_z_ptr = te_compile(m_ExprZ, m_Vars.data(), (int)m_Vars.size(), &m_Err_z);

    if (expr_x_ptr && expr_y_ptr && expr_z_ptr) {
        const double step_d = m_StepSize;  // double conv

        auto& x_value = m_VarNameValues.at("x");
        x_value = m_StartLocation.x;

        auto& y_value = m_VarNameValues.at("y");
        y_value = m_StartLocation.y;

        auto& z_value = m_VarNameValues.at("z");
        z_value = m_StartLocation.z;

        m_CenterPoint = 0.0;

        double ratio = 0.0;
        double px, py, pz;
        int verts_index;
        for (verts_index = 0; verts_index < verts_len; ++verts_index) {
            ratio = (double)verts_index / (double)verts_len;

            px = te_eval(expr_x_ptr);
            py = te_eval(expr_y_ptr);
            pz = te_eval(expr_z_ptr);

            x_value += px * step_d;
            y_value += py * step_d;
            z_value += pz * step_d;

            if (!isfinite(abs(x_value)))
                x_value = 0.0;
            if (!isfinite(abs(y_value)))
                y_value = 0.0;
            if (!isfinite(abs(z_value)))
                z_value = 0.0;

            auto& v = vertices.at(verts_index);
            v.p.x = (float)x_value;
            v.p.y = (float)y_value;
            v.p.z = (float)z_value;
            v.c = (float)ratio;
            m_CenterPoint += v.p;
            indices[verts_index] = verts_index;
        }

        m_CenterPoint /= (double)verts_len;

        if (m_CloseCurve) {
            indices[verts_len] = 0;
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

    if (!expr_x_ptr)
        LogVarError("Parse error for params x at %d", m_Err_x);
    if (!expr_y_ptr)
        LogVarError("Parse error for params y at %d", m_Err_y);
    if (!expr_z_ptr)
        LogVarError("Parse error for params z at %d", m_Err_z);

    te_free(expr_x_ptr);
    te_free(expr_y_ptr);
    te_free(expr_z_ptr);
}
