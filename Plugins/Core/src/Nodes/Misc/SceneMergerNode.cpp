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

#include "SceneMergerNode.h"
#include <Modules/Misc/SceneMergerModule.h>
#include <Graph/Slots/NodeSlotShaderPassInput.h>
#include <Graph/Slots/NodeSlotTextureOutput.h>

//////////////////////////////////////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<SceneMergerNode> SceneMergerNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	auto res = std::make_shared<SceneMergerNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}

	return res;
}

SceneMergerNode::SceneMergerNode() : BaseNode()
{
	ZoneScoped;

	m_NodeTypeString = "SCENE_MERGER";
}

SceneMergerNode::~SceneMergerNode()
{
	ZoneScoped;

	Unit();
}		

//////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMergerNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	bool res = false;

	name = "Scene Merger";

	AddInput(NodeSlotShaderPassInput::Create("Input", 0), false, false);

	AddOutput(NodeSlotTextureOutput::Create("", 0), false, true);

	m_SceneMergerModulePtr = SceneMergerModule::Create(vVulkanCorePtr, m_This);
	if (m_SceneMergerModulePtr)
	{
		res = true;
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TASK EXECUTE ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMergerNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	bool res = false;

	//BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

	if (m_SceneMergerModulePtr)
	{
		res = m_SceneMergerModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}

	return res;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMergerNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	bool res = false;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_SceneMergerModulePtr)
	{
		res = m_SceneMergerModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return res;
}

void SceneMergerNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_SceneMergerModulePtr)
	{
		m_SceneMergerModulePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW NODE ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void SceneMergerNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

void SceneMergerNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	if (m_SceneMergerModulePtr)
	{
		m_SceneMergerModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void SceneMergerNode::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{	
	ZoneScoped;

	if (m_SceneMergerModulePtr)
	{
		m_SceneMergerModulePtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	}
}

void SceneMergerNode::SetShaderPasses(const uint32_t& vBindingPoint, SceneShaderPassWeak vShaderPasses)
{
	ZoneScoped;

	if (m_SceneMergerModulePtr)
	{
		m_SceneMergerModulePtr->SetShaderPasses(vBindingPoint, vShaderPasses);
	}

	ReorganizeSlots();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* SceneMergerNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;

	if (m_SceneMergerModulePtr)
	{
		return m_SceneMergerModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string SceneMergerNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		for (auto &slot : m_Inputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		for (auto &slot : m_Outputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		if (m_SceneMergerModulePtr)
		{
			res += m_SceneMergerModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool SceneMergerNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_SceneMergerModulePtr)
	{
		m_SceneMergerModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	// continue recurse child exploring
	return true;
}

void SceneMergerNode::AfterNodeXmlLoading()
{
	ZoneScoped;

	if (m_SceneMergerModulePtr)
	{
		m_SceneMergerModulePtr->AfterNodeXmlLoading();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SHADER UPDATE ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void SceneMergerNode::UpdateShaders(const std::set<std::string>& vFiles)
{	
	ZoneScoped;

	if (m_SceneMergerModulePtr)
	{
		m_SceneMergerModulePtr->UpdateShaders(vFiles);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// INPUTS UPDATE ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void SceneMergerNode::ReorganizeSlots()
{
	if (m_SceneMergerModulePtr)
	{
		// il ne faudra pas re numerotter l'idx des slot connecté, 
		// sinon ca va peter la verif des lien dans le module, 
		// et je pourrasi plus virer les passes qui sont deconnectée
		
		// 1) on va checker les slot connecté a rien et retenir leur binding point dans arr
		uint32_t max_idx = 0U;
		std::vector<uint32_t> binding_points_arr;
		for (auto input : m_Inputs)
		{
			if (input.second)
			{
				if (!input.second->connected)
				{
					binding_points_arr.push_back(input.first);
				}

				// on stocke le plus grand id
				max_idx = ct::maxi(max_idx, input.second->descriptorBinding);
			}
		}

		// 2) on supprime les binding connecté a rien trouvé en 1)
		for (auto idx : binding_points_arr)
		{
			m_Inputs.erase(idx);
		}

		// 3) ajotuer un slot vide avec l'id max_idx + 1
		AddInput(NodeSlotShaderPassInput::Create("Input", max_idx + 1U), false, false);
	}
}