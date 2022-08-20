/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "ParamDiffCurveModule.h"

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

#include <Modules/Procedural/Pass/ParamDiffCurveModule_Comp_1D_Pass.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<ParamDiffCurveModule> ParamDiffCurveModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<ParamDiffCurveModule>(vVulkanCorePtr);
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

ParamDiffCurveModule::ParamDiffCurveModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: BaseRenderer(vVulkanCorePtr)
{
	ZoneScoped;
}

ParamDiffCurveModule::~ParamDiffCurveModule()
{
	ZoneScoped;

	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ParamDiffCurveModule::Init()
{
	ZoneScoped;

	m_Loaded = false;


	uint32_t map_size = 512;

	if (BaseRenderer::InitCompute1D(map_size))
	{
		//SetExecutionWhenNeededOnly(true);

		m_ParamDiffCurveModule_Comp_1D_Pass_Ptr = std::make_shared<ParamDiffCurveModule_Comp_1D_Pass>(m_VulkanCorePtr);
		if (m_ParamDiffCurveModule_Comp_1D_Pass_Ptr)
		{
			// by default but can be changed via widget
			//m_ParamDiffCurveModule_Comp_1D_Pass_Ptr->AllowResizeOnResizeEvents(false);
			//m_ParamDiffCurveModule_Comp_1D_Pass_Ptr->AllowResizeByHand(true);

			if (m_ParamDiffCurveModule_Comp_1D_Pass_Ptr->InitCompute1D(map_size))
			{
				AddGenericPass(m_ParamDiffCurveModule_Comp_1D_Pass_Ptr);
				m_Loaded = true;
			}
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ParamDiffCurveModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("Param Diff Curve", vCmd);

	return true;
}

bool ParamDiffCurveModule::ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("Param Diff Curve", vCmd);

	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool ParamDiffCurveModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Param Diff Curve##ParamDiffCurveModule", -1.0f, true, true, &m_CanWeRender))
		{
			bool change = false;

			if (m_ParamDiffCurveModule_Comp_1D_Pass_Ptr)
			{
				change |= m_ParamDiffCurveModule_Comp_1D_Pass_Ptr->DrawWidgets(vCurrentFrame, vContext);
			}

			return change;
		}

	}

	return false;
}

void ParamDiffCurveModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}

}

void ParamDiffCurveModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}

}

void ParamDiffCurveModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	// do some code
	
	BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL OUTPUT ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneModelWeak ParamDiffCurveModule::GetModel()
{	
	ZoneScoped;
	if (m_ParamDiffCurveModule_Comp_1D_Pass_Ptr)
	{
		return m_ParamDiffCurveModule_Comp_1D_Pass_Ptr->GetModel();
	}

	return SceneModelWeak();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ParamDiffCurveModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += vOffset + "<param_diff_curve_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	if (m_ParamDiffCurveModule_Comp_1D_Pass_Ptr)
	{
		str += m_ParamDiffCurveModule_Comp_1D_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
	}

	str += vOffset + "</param_diff_curve_module>\n";

	return str;
}

bool ParamDiffCurveModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "param_diff_curve_module")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();

	if (m_ParamDiffCurveModule_Comp_1D_Pass_Ptr)
	{
		m_ParamDiffCurveModule_Comp_1D_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
	}

	}

	return true;
}

void ParamDiffCurveModule::AfterNodeXmlLoading()
{
	if (m_ParamDiffCurveModule_Comp_1D_Pass_Ptr)
	{
		m_ParamDiffCurveModule_Comp_1D_Pass_Ptr->AfterNodeXmlLoading();
	}
}
