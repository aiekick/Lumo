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

#include "BillBoardRendererNode.h"
#include <Modules/Renderers/BillBoardRendererModule.h>
#include <Graph/Slots/NodeSlotModelInput.h>
#include <Graph/Slots/NodeSlotTextureInput.h>
#include <Graph/Slots/NodeSlotTextureOutput.h>
#include <Graph/Slots/NodeSlotShaderPassOutput.h>

//////////////////////////////////////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<BillBoardRendererNode> BillBoardRendererNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	auto res = std::make_shared<BillBoardRendererNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}

	return res;
}

BillBoardRendererNode::BillBoardRendererNode() : BaseNode()
{
	ZoneScoped;

	m_NodeTypeString = "BILLBOARD_RENDERER";
}

BillBoardRendererNode::~BillBoardRendererNode()
{
	ZoneScoped;

	Unit();
}		

//////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool BillBoardRendererNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	bool res = false;

	name = "BillBoard Renderer";
	AddInput(NodeSlotModelInput::Create("Mesh"), false, false);
	AddInput(NodeSlotTextureInput::Create("Texture", 0), false, false);

	AddOutput(NodeSlotTextureOutput::Create("", 0), false, true);
	AddOutput(NodeSlotShaderPassOutput::Create("Output", 1U), true, true);

	m_BillBoardRendererModulePtr = BillBoardRendererModule::Create(vVulkanCorePtr, m_This);
	if (m_BillBoardRendererModulePtr)
	{
		res = true;
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TASK EXECUTE ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool BillBoardRendererNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	bool res = false;

	BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureInputDescriptorImageInfos(m_Inputs);
	if (m_BillBoardRendererModulePtr)
	{
		res = m_BillBoardRendererModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}

	return res;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool BillBoardRendererNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	bool res = false;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_BillBoardRendererModulePtr)
	{
		res = m_BillBoardRendererModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return res;
}

void BillBoardRendererNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_BillBoardRendererModulePtr)
	{
		m_BillBoardRendererModulePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW NODE ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void BillBoardRendererNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

void BillBoardRendererNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	if (m_BillBoardRendererModulePtr)
	{
		m_BillBoardRendererModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL INPUT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void BillBoardRendererNode::SetModel(SceneModelWeak vSceneModel)
{	
	ZoneScoped;

	if (m_BillBoardRendererModulePtr)
	{
		m_BillBoardRendererModulePtr->SetModel(vSceneModel);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void BillBoardRendererNode::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{	
	ZoneScoped;

	if (m_BillBoardRendererModulePtr)
	{
		m_BillBoardRendererModulePtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* BillBoardRendererNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;

	if (m_BillBoardRendererModulePtr)
	{
		return m_BillBoardRendererModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SHADER PASS SLOT OUTPUT /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneShaderPassWeak BillBoardRendererNode::GetShaderPasses(const uint32_t& vBindingPoint)
{
	if (m_BillBoardRendererModulePtr)
	{
		return m_BillBoardRendererModulePtr->GetShaderPasses(vBindingPoint);
	}

	return SceneShaderPassWeak();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string BillBoardRendererNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_BillBoardRendererModulePtr)
		{
			res += m_BillBoardRendererModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool BillBoardRendererNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_BillBoardRendererModulePtr)
	{
		m_BillBoardRendererModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	// continue recurse child exploring
	return true;
}

void BillBoardRendererNode::AfterNodeXmlLoading()
{
	ZoneScoped;

	if (m_BillBoardRendererModulePtr)
	{
		m_BillBoardRendererModulePtr->AfterNodeXmlLoading();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SHADER UPDATE ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void BillBoardRendererNode::UpdateShaders(const std::set<std::string>& vFiles)
{	
	ZoneScoped;

	if (m_BillBoardRendererModulePtr)
	{
		m_BillBoardRendererModulePtr->UpdateShaders(vFiles);
	}
}
