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

#include "DeferredRendererNode.h"
#include <Modules/Renderers/DeferredRenderer.h>
#include <Graph/Slots/NodeSlotTextureInput.h>
#include <Graph/Slots/NodeSlotTextureOutput.h>
#include <Graph/Slots/NodeSlotShaderPassOutput.h>

std::shared_ptr<DeferredRendererNode> DeferredRendererNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<DeferredRendererNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

DeferredRendererNode::DeferredRendererNode() : BaseNode()
{
	m_NodeTypeString = "DEFERRED_RENDERER";
}

DeferredRendererNode::~DeferredRendererNode()
{
	Unit();
}

bool DeferredRendererNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Deferred Renderer";

	AddInput(NodeSlotTextureInput::Create("Position", 0U), true, false);
	AddInput(NodeSlotTextureInput::Create("Normal", 1U), true, false);
	AddInput(NodeSlotTextureInput::Create("Albedo", 2U), true, false);
	AddInput(NodeSlotTextureInput::Create("Diffuse", 3U), true, false);
	AddInput(NodeSlotTextureInput::Create("Specular", 4U), true, false); 
	AddInput(NodeSlotTextureInput::Create("Attenuation", 5U), true, false);
	AddInput(NodeSlotTextureInput::Create("Mask", 6U), true, false);
	AddInput(NodeSlotTextureInput::Create("AO", 7U), true, false);
	AddInput(NodeSlotTextureInput::Create("Shadow", 8U), true, false); 
	AddOutput(NodeSlotTextureOutput::Create("Output", 0U), true, true);
	AddOutput(NodeSlotShaderPassOutput::Create("Output", 1U), true, true);

	bool res = false;

	m_DeferredRendererPtr = DeferredRenderer::Create(vVulkanCorePtr);
	if (m_DeferredRendererPtr)
	{
		res = true;
	}

	return res;
}

void DeferredRendererNode::Unit()
{
	m_DeferredRendererPtr.reset();
}

bool DeferredRendererNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureInputDescriptorImageInfos(m_Inputs);

	if (m_DeferredRendererPtr)
	{
		return m_DeferredRendererPtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}

	return false;
}

bool DeferredRendererNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (m_DeferredRendererPtr)
	{
		return m_DeferredRendererPtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void DeferredRendererNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (m_DeferredRendererPtr)
	{
		m_DeferredRendererPtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void DeferredRendererNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

void DeferredRendererNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (m_DeferredRendererPtr)
	{
		m_DeferredRendererPtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void DeferredRendererNode::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	if (m_DeferredRendererPtr)
	{
		m_DeferredRendererPtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	}
}

vk::DescriptorImageInfo* DeferredRendererNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_DeferredRendererPtr)
	{
		return m_DeferredRendererPtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SHADER PASS SLOT OUTPUT /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneShaderPassWeak DeferredRendererNode::GetShaderPasses(const uint32_t& vSlotID)
{
	if (m_DeferredRendererPtr)
	{
		return m_DeferredRendererPtr->GetShaderPasses(vSlotID);
	}

	return SceneShaderPassWeak();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string DeferredRendererNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_DeferredRendererPtr)
		{
			res += m_DeferredRendererPtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool DeferredRendererNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_DeferredRendererPtr)
	{
		m_DeferredRendererPtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

void DeferredRendererNode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_DeferredRendererPtr)
	{
		m_DeferredRendererPtr->UpdateShaders(vFiles);
	}
}