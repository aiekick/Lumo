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

#include "ParametricCurveModule.h"

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

std::shared_ptr<ParametricCurveModule> ParametricCurveModule::Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode)
{
	ZoneScoped;

	
	auto res = std::make_shared<ParametricCurveModule>(vVulkanCore);
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

ParametricCurveModule::ParametricCurveModule(GaiApi::VulkanCoreWeak vVulkanCore)
	: m_VulkanCore(vVulkanCore)
{
	ZoneScoped;
}

ParametricCurveModule::~ParametricCurveModule()
{
	ZoneScoped;

	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ParametricCurveModule::Init()
{
	ZoneScoped;

	m_SceneModelPtr = SceneModel::Create();

	prAddVar("t", 0.0);
	prUpdateMesh();

	return true;
}

void ParametricCurveModule::Unit()
{
	ZoneScoped;

	m_SceneModelPtr.reset();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool ParametricCurveModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	ZoneScoped;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	prDrawWidgets();
	return false;
}

bool ParametricCurveModule::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool ParametricCurveModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// VARIABLE SLOT INPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void ParametricCurveModule::SetVariable(const uint32_t& vVarIndex, SceneVariableWeak vSceneVariable)
{	
	ZoneScoped;

}

//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL OUTPUT ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneModelWeak ParametricCurveModule::GetModel()
{	
	ZoneScoped;

	return m_SceneModelPtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ParametricCurveModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += vOffset + "<parametric_curve_module>\n";
	str += vOffset + "\t<expr_x>" + ct::toStr(m_ExprX) + "</expr_x>\n";
	str += vOffset + "\t<expr_y>" + ct::toStr(m_ExprY) + "</expr_y>\n";
	str += vOffset + "\t<expr_z>" + ct::toStr(m_ExprZ) + "</expr_z>\n";
	str += vOffset + "\t<start_t>" + ct::toStr(m_Start_T) + "</start_t>\n";
	str += vOffset + "\t<end_t>" + ct::toStr(m_End_T) + "</end_t>\n";
	str += vOffset + "\t<step_t>" + ct::toStr(m_Step_T) + "</step_t>\n";
	str += vOffset + "\t<close_curve>" + ct::toStr(m_CloseCurve) + "</close_curve>\n";
	str += vOffset + "\t<vars>\n";
	for (const auto& var : m_VarNameValues)
	{
		if (var.first == "t")
			continue;

		str += vOffset + "\t\t<var name=\"" + var.first + "\" value=\"" + ct::toStr(var.second) + "\"/>\n";
	}
	str += vOffset + "\t</vars>\n";
	str += vOffset + "</parametric_curve_module>\n";

	return str;
}

bool ParametricCurveModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "parametric_curve_module")
	{
		if (strName == "expr_x") {
			ct::ResetBuffer(m_ExprX);
			ct::AppendToBuffer(m_ExprX, ParametricCurveModule::s_EXPR_MAX_LEN, strValue);
		}
		else if (strName == "expr_y") {
			ct::ResetBuffer(m_ExprY);
			ct::AppendToBuffer(m_ExprY, ParametricCurveModule::s_EXPR_MAX_LEN, strValue);
		}
		else if (strName == "expr_z") {
			ct::ResetBuffer(m_ExprZ);
			ct::AppendToBuffer(m_ExprZ, ParametricCurveModule::s_EXPR_MAX_LEN, strValue);
		}
		else if (strName == "start_t") {
			m_Start_T = ct::ivariant(strValue).GetD();
		}
		else if (strName == "end_t") {
			m_End_T = ct::ivariant(strValue).GetD();
		}
		else if (strName == "step_t") {
			m_Step_T = ct::ivariant(strValue).GetD();
		}
		else if (strName == "close_curve") {
			m_CloseCurve = ct::ivariant(strValue).GetB();
		}
	}
	else if (strParentName == "vars" && strName == "var")
	{
		std::string var_name;
		double var_value;

		for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
		{
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

void ParametricCurveModule::AfterNodeXmlLoading()
{
	ZoneScoped;

	prAddVar("t", 0.0);
	prUpdateMesh();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ParametricCurveModule::prDrawWidgets()
{
	if (ImGui::CollapsingHeader("Parametric Curve"))
	{
		bool change = false;

		change |= ImGui::InputDoubleDefault(0.0f, "Start t", &m_Start_T, 0.0, "%f");
		change |= ImGui::InputDoubleDefault(0.0f, "End t", &m_End_T, 1.0, "%f");
		change |= ImGui::InputDoubleDefault(0.0f, "Step t", &m_Step_T, 0.01, "%f");

		ImGui::Separator();

		change |= prDrawInputExpr("x(t)", "##expr_x", m_ExprX, ParametricCurveModule::s_EXPR_MAX_LEN, m_Err_x, "t - pi * 0.5");
		change |= prDrawInputExpr("y(t)", "##expr_y", m_ExprY, ParametricCurveModule::s_EXPR_MAX_LEN, m_Err_y, "0");
		change |= prDrawInputExpr("z(t)", "##expr_z", m_ExprZ, ParametricCurveModule::s_EXPR_MAX_LEN, m_Err_z, "sin(t * 5.0) * 0.25");

		ImGui::Separator();

		change |= ImGui::CheckBoxBoolDefault("Close Curve", &m_CloseCurve, false);

		ImGui::Separator();

		change |= ImGui::ContrastedButton("Eval");

		ImGui::SameLine();

		if (ImGui::ContrastedButton("Center Model"))
		{
			CommonSystem::Instance()->SetTargetXYZ(-(ct::fvec3)m_CenterPoint, true); 
		}

		change |= prDrawVars();

		if (change)
		{
			prUpdateMesh();
		}
	}
}

void ParametricCurveModule::prAddVar(const std::string& vName, const double& vValue)
{
	if (!vName.empty())
	{
		if (m_VarNameValues.find(vName) == m_VarNameValues.end())
		{
			m_VarNameValues.emplace(vName, vValue);
			const auto& it = m_VarNameValues.find(vName);

			te_variable v;
			v.name = it->first.c_str(); // ref so m_VarNames name must already exist
			v.address = &it->second;
			v.context = 0;
			v.type = 0;
			m_Vars.push_back(v);
		}
	}
}

void ParametricCurveModule::prDelVar(const std::string& vName)
{
	auto it = m_VarNameValues.find(vName);
	if (it != m_VarNameValues.end())
	{
		for (auto var_it = m_Vars.begin(); var_it != m_Vars.end(); ++var_it)
		{
			if (strcmp(var_it->name, it->first.c_str()) == 0) // is equal
			{
				m_Vars.erase(var_it);
				m_VarNameValues.erase(vName);
				break;
			}
		}
	}
}

void ParametricCurveModule::prUpdateMesh()
{
	VerticeArray vertices;
	IndiceArray indices;

	if (IS_DOUBLE_EQUAL(m_Step_T, 0.0))
		return;

	int verts_len = (int)ceil((m_End_T - m_Start_T) / m_Step_T);
	if (!verts_len)
		return;

	vertices.resize(verts_len);
	indices.resize(m_CloseCurve ? verts_len + 1 : verts_len);

	te_expr* expr_x = te_compile(m_ExprX, m_Vars.data(), (int)m_Vars.size(), &m_Err_x);
	te_expr* expr_y = te_compile(m_ExprY, m_Vars.data(), (int)m_Vars.size(), &m_Err_y);
	te_expr* expr_z = te_compile(m_ExprZ, m_Vars.data(), (int)m_Vars.size(), &m_Err_z);

	if (expr_x && expr_y && expr_z)
	{
		m_CenterPoint = 0.0;

		double step_u_f = (m_End_T - m_Start_T) / (double)(verts_len - 1);  // -1 for loop connection

		auto& time_value = m_VarNameValues.at("t");
		time_value = (double)m_Start_T;

		ct::fvec3 pos;
		size_t id = 0U;

		double ratio = 0.0;

		int verts_index;
		for (verts_index = 0; verts_index < verts_len; ++verts_index)
		{
			ratio = (double)verts_index / (double)verts_len;

			auto& v = vertices.at(verts_index);
			v.p.x = (float)te_eval(expr_x);
			v.p.y = (float)te_eval(expr_y);
			v.p.z = (float)te_eval(expr_z);
			v.c = (float)ratio;
			m_CenterPoint += v.p;
			indices[verts_index] = verts_index;

			time_value += step_u_f;
		}

		m_CenterPoint /= (double)verts_len;

		if (m_CloseCurve)
		{
			indices[verts_len] = 0;
		}

		auto sceneMeshPtr = SceneMesh<VertexStruct::P3_N3_TA3_BTA3_T2_C4>::Create(m_VulkanCore, vertices, indices);
		sceneMeshPtr->SetPrimitiveType(SceneModelPrimitiveType::SCENE_MODEL_PRIMITIVE_TYPE_CURVES);
		m_SceneModelPtr->clear();
		m_SceneModelPtr->Add(sceneMeshPtr);

		auto parentNodePtr = GetParentNode().lock();
		if (parentNodePtr)
		{
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

bool ParametricCurveModule::prDrawInputExpr(
	const char* vLabel,
	const char* vBufferLabel,
	char* vBuffer,
	size_t vBufferSize,
	const int& vError,
	const char* vDdefaultValue)
{
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
bool ParametricCurveModule::prDrawVars()
{
	static std::string var_to_erase;

	bool change = false;

	ImGui::Separator();

	if (ImGui::ContrastedButton("Add Var"))
	{
		prAddVar(m_VarToAddBuffer, 0.0);
		change = true;
	}

	ImGui::SameLine();

	ImGui::InputText("##VarToAdd", m_VarToAddBuffer, s_EXPR_MAX_LEN);

	ImGui::Separator();

	for (auto& var : m_VarNameValues)
	{
		if (var.first == "t")
			continue;

		if (ImGui::ContrastedButton("R"))
		{
			var_to_erase = var.first;
		}
		ImGui::SameLine();
		ImGui::Text("%s", var.first.c_str());
		ImGui::SameLine();
		ImGui::PushID(&var.second);
		change |= ImGui::InputDouble("##value", &var.second);
		ImGui::PopID();
	}

	if (!var_to_erase.empty())
	{
		prDelVar(var_to_erase);
		var_to_erase.clear();
		change = true;
	}

	return change;
}
