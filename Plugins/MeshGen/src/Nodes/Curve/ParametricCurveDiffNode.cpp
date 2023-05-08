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

#include "ParametricCurveDiffNode.h"
#include <Modules/Curve/ParametricCurveDiffModule.h>
#include <Graph/Slots/NodeSlotVariableInput.h>
#include <Graph/Slots/NodeSlotModelOutput.h>

//////////////////////////////////////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<ParametricCurveDiffNode> ParametricCurveDiffNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	auto res = std::make_shared<ParametricCurveDiffNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}

	return res;
}

ParametricCurveDiffNode::ParametricCurveDiffNode() : BaseNode()
{
	ZoneScoped;

	m_NodeTypeString = "PARAMETRIC_CURVE_DIFF";
}

ParametricCurveDiffNode::~ParametricCurveDiffNode()
{
	ZoneScoped;

	Unit();
}		

//////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool ParametricCurveDiffNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	bool res = false;

	name = "Parametric Curve Diff";
	AddInput(NodeSlotVariableInput::Create("Close", "WIDGET_BOOLEAN", 0), false, false);

	AddOutput(NodeSlotModelOutput::Create("Curve"), false, false);

	m_ParametricCurveDiffModulePtr = ParametricCurveDiffModule::Create(vVulkanCorePtr, m_This);
	if (m_ParametricCurveDiffModulePtr)
	{
		res = true;
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool ParametricCurveDiffNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	bool res = false;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_ParametricCurveDiffModulePtr)
	{
		res = m_ParametricCurveDiffModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return res;
}

void ParametricCurveDiffNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_ParametricCurveDiffModulePtr)
	{
		m_ParametricCurveDiffModulePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW SLOTS WIDGET ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void ParametricCurveDiffNode::DrawInputWidget(BaseNodeState* vBaseNodeState, NodeSlotWeak vSlot)
{
	ZoneScoped;

	auto slotPtr = vSlot.getValidShared();
	if (slotPtr && slotPtr->showWidget)
	{
		if (m_ParametricCurveDiffModulePtr)
		{
			//m_ParametricCurveDiffModulePtr->DrawTexture(50);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW NODE ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void ParametricCurveDiffNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	if (vBaseNodeState && vBaseNodeState->debug_mode)
	{
		auto drawList = nd::GetNodeBackgroundDrawList(nodeID);
		if (drawList)
		{
			char debugBuffer[255] = "\0";
			snprintf(debugBuffer, 254,
				"Used[%s]\nCell[%i, %i]",
				(used ? "true" : "false"), cell.x, cell.y);
			ImVec2 txtSize = ImGui::CalcTextSize(debugBuffer);
			drawList->AddText(pos - ImVec2(0, txtSize.y), ImGui::GetColorU32(ImGuiCol_Text), debugBuffer);
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////
//// VARIABLE SLOT INPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void ParametricCurveDiffNode::SetVariable(const uint32_t& vVarIndex, SceneVariableWeak vSceneVariable)
{	
	ZoneScoped;

	if (m_ParametricCurveDiffModulePtr)
	{
		m_ParametricCurveDiffModulePtr->SetVariable(vVarIndex, vSceneVariable);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL OUTPUT ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneModelWeak ParametricCurveDiffNode::GetModel()
{	
	ZoneScoped;

	if (m_ParametricCurveDiffModulePtr)
	{
		return m_ParametricCurveDiffModulePtr->GetModel();
	}

	return SceneModelWeak();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string ParametricCurveDiffNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
{	
	ZoneScoped;

	std::string res;

	if (!m_ChildNodes.empty())
	{
		res += BaseNode::getXml(vOffset, vUserDatas);
	}
	else
	{
		res += vOffset + ct::toStr("<node name=\"%s\" type=\"%s\" pos=\"%s\" id=\"%u\">\n",
			name.c_str(),
			m_NodeTypeString.c_str(),
			ct::fvec2(pos.x, pos.y).string().c_str(),
			(uint32_t)GetNodeID());

		for (auto slot : m_Inputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		for (auto slot : m_Outputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		if (m_ParametricCurveDiffModulePtr)
		{
			res += m_ParametricCurveDiffModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool ParametricCurveDiffNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	BaseNode::setFromXml(vElem, vParent, vUserDatas);

	if (m_ParametricCurveDiffModulePtr)
	{
		m_ParametricCurveDiffModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	// continue recurse child exploring
	return true;
}

void ParametricCurveDiffNode::AfterNodeXmlLoading()
{
	ZoneScoped;

	if (m_ParametricCurveDiffModulePtr)
	{
		m_ParametricCurveDiffModulePtr->AfterNodeXmlLoading();
	}
}
