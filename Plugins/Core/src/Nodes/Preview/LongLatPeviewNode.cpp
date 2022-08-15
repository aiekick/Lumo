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

#include "LongLatPeviewNode.h"
#include <Modules/Preview/LongLatPeviewModule.h>
#include <Graph/Slots/NodeSlotTextureInput.h>
#include <Graph/Slots/NodeSlotTextureOutput.h>

//////////////////////////////////////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<LongLatPeviewNode> LongLatPeviewNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	auto res = std::make_shared<LongLatPeviewNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

LongLatPeviewNode::LongLatPeviewNode() : BaseNode()
{
	ZoneScoped;

	m_NodeTypeString = "LONG_LAT_PREVIEW";
}

LongLatPeviewNode::~LongLatPeviewNode()
{
	ZoneScoped;

	Unit();
}		

//////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool LongLatPeviewNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	bool res = false;

	name = "LongLat Peview";

	AddInput(NodeSlotTextureInput::Create("LongLat", 0), false, false);
	AddOutput(NodeSlotTextureOutput::Create("New Slot", 0), false, true);

	m_LongLatPeviewModulePtr = LongLatPeviewModule::Create(vVulkanCorePtr, m_This);
	if (m_LongLatPeviewModulePtr)
	{
		res = true;
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TASK EXECUTE ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool LongLatPeviewNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	bool res = false;

	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureInputDescriptorImageInfos(m_Inputs);

	if (m_LongLatPeviewModulePtr)
	{
		res = m_LongLatPeviewModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}

	return res;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool LongLatPeviewNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	bool res = false;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LongLatPeviewModulePtr)
	{
		res = m_LongLatPeviewModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return res;
}

void LongLatPeviewNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LongLatPeviewModulePtr)
	{
		m_LongLatPeviewModulePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW NODE ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void LongLatPeviewNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

void LongLatPeviewNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	if (m_LongLatPeviewModulePtr)
	{
		m_LongLatPeviewModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void LongLatPeviewNode::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{	
	ZoneScoped;

	if (m_LongLatPeviewModulePtr)
	{
		m_LongLatPeviewModulePtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* LongLatPeviewNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;

	if (m_LongLatPeviewModulePtr)
	{
		return m_LongLatPeviewModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string LongLatPeviewNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_LongLatPeviewModulePtr)
		{
			res += m_LongLatPeviewModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool LongLatPeviewNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_LongLatPeviewModulePtr)
	{
		m_LongLatPeviewModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	// continue recurse child exploring
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SHADER UPDATE ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void LongLatPeviewNode::UpdateShaders(const std::set<std::string>& vFiles)
{	
	ZoneScoped;

	if (m_LongLatPeviewModulePtr)
	{
		m_LongLatPeviewModulePtr->UpdateShaders(vFiles);
	}
}
