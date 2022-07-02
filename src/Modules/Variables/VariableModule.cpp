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

std::shared_ptr<VariableModule> VariableModule::Create(vkApi::VulkanCore* vVulkanCore, BaseNodeWeak vParentNode)
{
	auto res = std::make_shared<VariableModule>(vVulkanCore);
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

VariableModule::VariableModule(vkApi::VulkanCore* vVulkanCore)
{

}

VariableModule::~VariableModule()
{
	
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool VariableModule::Init()
{
	ZoneScoped;

	m_VariablePtr = std::make_shared<SceneVariable>();

	return true;
}


void VariableModule::Unit()
{
	ZoneScoped;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool VariableModule::Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd)
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
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void VariableModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

bool VariableModule::DrawNodeWidget(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	//if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (m_VariablePtr)
		{
			if (m_VariablePtr->GetType() == NodeTypeEnum::TYPE_BOOLEAN)
			{
				if (ImGui::CheckBoxBoolDefault("Variable##bool", &m_VariablePtr->GetDatas().m_Boolean, false))
				{
					auto parentNodePtr = GetParentNode().getValidShared();
					if (parentNodePtr)
					{
						parentNodePtr->Notify(NotifyEvent::VariableUpdateDone);
					}
				}
			}
		}
	}

	return false;
}

SceneVariableWeak VariableModule::GetVariable()
{
	return m_VariablePtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string VariableModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

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

	return true;
}
