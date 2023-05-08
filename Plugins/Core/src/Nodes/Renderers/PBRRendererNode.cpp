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

#include "PBRRendererNode.h"
#include <Modules/Renderers/PBRRenderer.h>
#include <Interfaces/LightGroupOutputInterface.h>
#include <Interfaces/TextureGroupOutputInterface.h>
#include <Graph/Slots/NodeSlotLightGroupInput.h>
#include <Graph/Slots/NodeSlotTextureInput.h>
#include <Graph/Slots/NodeSlotTextureGroupInput.h>
#include <Graph/Slots/NodeSlotTextureOutput.h>
#include <Graph/Slots/NodeSlotShaderPassOutput.h>

std::shared_ptr<PBRRendererNode> PBRRendererNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<PBRRendererNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

PBRRendererNode::PBRRendererNode() : BaseNode()
{
	m_NodeTypeString = "PBR_RENDERER";
}

PBRRendererNode::~PBRRendererNode()
{
	Unit();
}

bool PBRRendererNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "PBR Renderer";

	AddInput(NodeSlotLightGroupInput::Create("Lights"), true, false);
	AddInput(NodeSlotTextureInput::Create("Position", 0U), true, false);
	AddInput(NodeSlotTextureInput::Create("Normal", 0U), true, false);
	AddInput(NodeSlotTextureInput::Create("Albedo", 0U), true, false);
	AddInput(NodeSlotTextureInput::Create("Mask", 0U), true, false);
	AddInput(NodeSlotTextureInput::Create("AO", 0U), true, false);
	AddInput(NodeSlotTextureGroupInput::Create("Shadow Maps"), true, false);
	AddOutput(NodeSlotTextureOutput::Create("Output", 0U), true, true);
	AddOutput(NodeSlotShaderPassOutput::Create("Output", 1U), true, true);

	bool res = false;

	m_PBRRendererPtr = PBRRenderer::Create(vVulkanCorePtr);
	if (m_PBRRendererPtr)
	{
		res = true;
	}

	return res;
}

void PBRRendererNode::Unit()
{
	m_PBRRendererPtr.reset();
}

bool PBRRendererNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureInputDescriptorImageInfos(m_Inputs);

	if (m_PBRRendererPtr)
	{
		return m_PBRRendererPtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}

	return false;
}

bool PBRRendererNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (m_PBRRendererPtr)
	{
		return m_PBRRendererPtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void PBRRendererNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (m_PBRRendererPtr)
	{
		m_PBRRendererPtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void PBRRendererNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
{
	if (vBaseNodeState && vBaseNodeState->debug_mode)
	{
		auto drawList = nd::GetNodeBackgroundDrawList(nodeID);
		if (drawList)
		{
			char debugBuffer[255] = "\0";
			snprintf(debugBuffer, 254,
				"Used(%s)\nCell(%i, %i)"/*\nPos(%.1f, %.1f)\nSize(%.1f, %.1f)*/,
				(used ? "true" : "false"), cell.x, cell.y/*, pos.x, pos.y, size.x, size.y*/);
			ImVec2 txtSize = ImGui::CalcTextSize(debugBuffer);
			drawList->AddText(pos - ImVec2(0, txtSize.y), ImGui::GetColorU32(ImGuiCol_Text), debugBuffer);
		}
	}
}

void PBRRendererNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (m_PBRRendererPtr)
	{
		m_PBRRendererPtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void PBRRendererNode::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	if (m_PBRRendererPtr)
	{
		m_PBRRendererPtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	}
}

vk::DescriptorImageInfo* PBRRendererNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_PBRRendererPtr)
	{
		return m_PBRRendererPtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SHADER PASS SLOT OUTPUT /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneShaderPassWeak PBRRendererNode::GetShaderPasses(const uint32_t& vSlotID)
{
	if (m_PBRRendererPtr)
	{
		return m_PBRRendererPtr->GetShaderPasses(vSlotID);
	}

	return SceneShaderPassWeak();
}

void PBRRendererNode::SetTextures(const uint32_t& vBindingPoint, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes)
{
	if (m_PBRRendererPtr)
	{
		m_PBRRendererPtr->SetTextures(vBindingPoint, vImageInfos, vOutSizes);
	}
}

void PBRRendererNode::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	if (m_PBRRendererPtr)
	{
		m_PBRRendererPtr->SetLightGroup(vSceneLightGroup);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string PBRRendererNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
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

		if (m_PBRRendererPtr)
		{
			res += m_PBRRendererPtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool PBRRendererNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	BaseNode::setFromXml(vElem, vParent, vUserDatas);

	if (m_PBRRendererPtr)
	{
		m_PBRRendererPtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

void PBRRendererNode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_PBRRendererPtr)
	{
		m_PBRRendererPtr->UpdateShaders(vFiles);
	}
}