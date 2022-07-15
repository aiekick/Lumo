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
#include <Gui/MainFrame.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanSubmitter.h>
#include <utils/Mesh/VertexStruct.h>
#include <Graph/Base/BaseNode.h>
using namespace vkApi;

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

	m_VariablePtr = std::make_shared<SceneVariable>(vNodeType);

	return true;
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

	if (m_LastExecutedFrame != vCurrentFrame)
	{
		m_LastExecutedFrame = vCurrentFrame;
	}

	return true;
}

bool VariableModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Boolean"))
		{
			return DrawNodeWidget(vCurrentFrame, vContext);
		}
	}

	return false;
}

void VariableModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void VariableModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

bool VariableModule::DrawNodeWidget(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	//if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (m_VariablePtr)
		{
			if (m_VariablePtr->GetType() == "WIDGET_BOOLEAN")
			{
				if (ImGui::CheckBoxBoolDefault("Variable", &m_VariablePtr->GetDatas().m_Boolean, false))
				{
					auto parentNodePtr = GetParentNode().getValidShared();
					if (parentNodePtr)
					{
						parentNodePtr->Notify(NotifyEvent::VariableUpdateDone);
					}
				}
			}
			else if (m_VariablePtr->GetType() == "WIDGET_FLOAT")
			{
				/*if (ImGui::CheckBoxBoolDefault("Variable", &m_VariablePtr->GetDatas().m_Float, false))
				{
					auto parentNodePtr = GetParentNode().getValidShared();
					if (parentNodePtr)
					{
						parentNodePtr->Notify(NotifyEvent::VariableUpdateDone);
					}
				}*/
			}
			else if (m_VariablePtr->GetType() == "WIDGET_INT")
			{
				/*if (ImGui::CheckBoxBoolDefault("Variable", &m_VariablePtr->GetDatas().m_Boolean, false))
				{
					auto parentNodePtr = GetParentNode().getValidShared();
					if (parentNodePtr)
					{
						parentNodePtr->Notify(NotifyEvent::VariableUpdateDone);
					}
				}*/
			}
			else if (m_VariablePtr->GetType() == "WIDGET_UINT")
			{
				/*if (ImGui::CheckBoxBoolDefault("Variable", &m_VariablePtr->GetDatas().m_Boolean, false))
				{
					auto parentNodePtr = GetParentNode().getValidShared();
					if (parentNodePtr)
					{
						parentNodePtr->Notify(NotifyEvent::VariableUpdateDone);
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
		if (m_VariablePtr->GetType() == "TYPE_BOOLEAN")
		{
			str += vOffset + ct::toStr("\t<bool>%s</bool>\n", m_VariablePtr->GetDatas().m_Boolean ? "true" :"false");
		}
		else if (m_VariablePtr->GetType() == "TYPE_FLOAT")
		{
			str += vOffset + ct::toStr("\t<float>%.5f</float>\n", m_VariablePtr->GetDatas().m_Float);
		}
		else if (m_VariablePtr->GetType() == "TYPE_INT")
		{
			str += vOffset + ct::toStr("\t<int>%i</int>\n", m_VariablePtr->GetDatas().m_Int32);
		}
		else if (m_VariablePtr->GetType() == "TYPE_UINT")
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
			m_VariablePtr->SetType("TYPE_BOOLEAN");
			m_VariablePtr->GetDatas().m_Boolean = ct::ivariant(strValue).GetB();
		}
		else if (strName == "float")
		{
			m_VariablePtr->SetType("TYPE_FLOAT");
			m_VariablePtr->GetDatas().m_Float = ct::fvariant(strValue).GetF();
		}
		else if (strName == "int")
		{
			m_VariablePtr->SetType("TYPE_INT");
			m_VariablePtr->GetDatas().m_Int32 = ct::ivariant(strValue).GetI();
		}
		else if (strName == "uint")
		{
			m_VariablePtr->SetType("TYPE_UINT");
			m_VariablePtr->GetDatas().m_Uint32 = ct::uvariant(strValue).GetU();
		}
	}

	return true;
}
