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

#include "VariableModule.h"

#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
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

std::shared_ptr<VariableModule> VariableModule::Create(const std::string& vNodeType, BaseNodeWeak vParentNode)
{
	auto res = std::make_shared<VariableModule>();
	res->SetParentNode(vParentNode);
	if (!res->Init(vNodeType))
	{
		res.reset();
	}
	return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

VariableModule::VariableModule()
{

}

VariableModule::~VariableModule()
{
	
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool VariableModule::Init(const std::string& vNodeType)
{
	ZoneScoped;

	m_VariablePtr = SceneVariable::Create(vNodeType);
	if (m_VariablePtr)
	{
		return true;
	}
	return false;
}


void VariableModule::Unit()
{
	ZoneScoped;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool VariableModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	m_LastExecutedFrame = vCurrentFrame;

	return true;
}

bool VariableModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Boolean"))
		{
			return DrawNodeWidget(vCurrentFrame, vContextPtr);
		}
	}

	return false;
}

bool VariableModule::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
    return false;
}

bool VariableModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
    return false;
}

bool VariableModule::DrawNodeWidget(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	//if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (m_VariablePtr)
		{
			if (m_VariablePtr->GetType() == "WIDGET_BOOLEAN")
			{
				if (ImGui::CheckBoxBoolDefault("Variable", &m_VariablePtr->GetDatas().m_Boolean, false))
				{
					auto parentNodePtr = GetParentNode().lock();
					if (parentNodePtr)
					{
						parentNodePtr->SendFrontNotification(VariableUpdateDone);
					}
				}
			}
			else if (m_VariablePtr->GetType() == "WIDGET_FLOAT")
			{
				/*if (ImGui::CheckBoxBoolDefault("Variable", &m_VariablePtr->GetDatas().m_Float, false))
				{
					auto parentNodePtr = GetParentNode().lock();
					if (parentNodePtr)
					{
						parentNodePtr->SendFrontNotification(VariableUpdateDone);
					}
				}*/
			}
			else if (m_VariablePtr->GetType() == "WIDGET_INT")
			{
				/*if (ImGui::CheckBoxBoolDefault("Variable", &m_VariablePtr->GetDatas().m_Boolean, false))
				{
					auto parentNodePtr = GetParentNode().lock();
					if (parentNodePtr)
					{
						parentNodePtr->SendFrontNotification(VariableUpdateDone);
					}
				}*/
			}
			else if (m_VariablePtr->GetType() == "WIDGET_UINT")
			{
				/*if (ImGui::CheckBoxBoolDefault("Variable", &m_VariablePtr->GetDatas().m_Boolean, false))
				{
					auto parentNodePtr = GetParentNode().lock();
					if (parentNodePtr)
					{
						parentNodePtr->SendFrontNotification(VariableUpdateDone);
					}
				}*/
			}
		}
	}

	return false;
}

SceneVariableWeak VariableModule::GetVariable(const uint32_t& vVariableIndex)
{
	return m_VariablePtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string VariableModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += vOffset + "<variable_module>\n";

	if (m_VariablePtr)
	{
		if (m_VariablePtr->GetType() == "WIDGET_BOOLEAN")
		{
			str += vOffset + ct::toStr("\t<bool>%s</bool>\n", m_VariablePtr->GetDatas().m_Boolean ? "true" :"false");
		}
		else if (m_VariablePtr->GetType() == "WIDGET_FLOAT")
		{
			str += vOffset + ct::toStr("\t<float>%.5f</float>\n", m_VariablePtr->GetDatas().m_Float);
		}
		else if (m_VariablePtr->GetType() == "WIDGET_INT")
		{
			str += vOffset + ct::toStr("\t<int>%i</int>\n", m_VariablePtr->GetDatas().m_Int32);
		}
		else if (m_VariablePtr->GetType() == "WIDGET_UINT")
		{
			str += vOffset + ct::toStr("\t<uint>%u</uint>\n", m_VariablePtr->GetDatas().m_Uint32);
		}
	}

	str += vOffset + "</variable_module>\n";

	return str;
}

bool VariableModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "variable_module")
	{
		if (strName == "bool")
		{
			m_VariablePtr->SetType("WIDGET_BOOLEAN");
			m_VariablePtr->GetDatas().m_Boolean = ct::ivariant(strValue).GetB();
		}
		else if (strName == "float")
		{
			m_VariablePtr->SetType("WIDGET_FLOAT");
			m_VariablePtr->GetDatas().m_Float = ct::fvariant(strValue).GetF();
		}
		else if (strName == "int")
		{
			m_VariablePtr->SetType("WIDGET_INT");
			m_VariablePtr->GetDatas().m_Int32 = ct::ivariant(strValue).GetI();
		}
		else if (strName == "uint")
		{
			m_VariablePtr->SetType("WIDGET_UINT");
			m_VariablePtr->GetDatas().m_Uint32 = ct::uvariant(strValue).GetU();
		}
	}

	return true;
}
