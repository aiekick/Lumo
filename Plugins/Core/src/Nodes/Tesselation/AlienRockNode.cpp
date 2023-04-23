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

#include "AlienRockNode.h"
#include <Modules/Tesselation/AlienRockModule.h>
#include <Graph/Slots/NodeSlotTextureOutput.h>

//////////////////////////////////////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<AlienRockNode> AlienRockNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	auto res = std::make_shared<AlienRockNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}

	return res;
}

AlienRockNode::AlienRockNode() : BaseNode()
{
	ZoneScoped;

	m_NodeTypeString = "ALIEN_ROCK";
}

AlienRockNode::~AlienRockNode()
{
	ZoneScoped;

	Unit();
}		

//////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool AlienRockNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	bool res = false;

	name = "Alien Rock";


	AddOutput(NodeSlotTextureOutput::Create("", 0), false, true);

	m_AlienRockModulePtr = AlienRockModule::Create(vVulkanCorePtr, m_This);
	if (m_AlienRockModulePtr)
	{
		res = true;
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TASK EXECUTE ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool AlienRockNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	bool res = false;

	BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

	if (m_AlienRockModulePtr)
	{
		res = m_AlienRockModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}

	return res;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool AlienRockNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	bool res = false;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_AlienRockModulePtr)
	{
		res = m_AlienRockModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return res;
}

void AlienRockNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_AlienRockModulePtr)
	{
		m_AlienRockModulePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW NODE ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void AlienRockNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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
//// RESIZE //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void AlienRockNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	if (m_AlienRockModulePtr)
	{
		m_AlienRockModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* AlienRockNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;

	if (m_AlienRockModulePtr)
	{
		return m_AlienRockModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string AlienRockNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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
			(uint32_t)nodeID.Get());

		for (auto slot : m_Inputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		for (auto slot : m_Outputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		if (m_AlienRockModulePtr)
		{
			res += m_AlienRockModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool AlienRockNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_AlienRockModulePtr)
	{
		m_AlienRockModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	// continue recurse child exploring
	return true;
}

void AlienRockNode::AfterNodeXmlLoading()
{
	ZoneScoped;

	if (m_AlienRockModulePtr)
	{
		m_AlienRockModulePtr->AfterNodeXmlLoading();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SHADER UPDATE ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void AlienRockNode::UpdateShaders(const std::set<std::string>& vFiles)
{	
	ZoneScoped;

	if (m_AlienRockModulePtr)
	{
		m_AlienRockModulePtr->UpdateShaders(vFiles);
	}
}
