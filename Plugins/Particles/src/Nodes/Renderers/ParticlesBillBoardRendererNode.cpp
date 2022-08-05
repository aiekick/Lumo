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

#include "ParticlesBillBoardRendererNode.h"
#include <Modules/Renderers/ParticlesBillBoardRenderer.h>
#include <Graph/Slots/NodeSlotTexelBufferInput.h>
#include <Graph/Slots/NodeSlotTextureOutput.h>

std::shared_ptr<ParticlesBillBoardRendererNode> ParticlesBillBoardRendererNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<ParticlesBillBoardRendererNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

ParticlesBillBoardRendererNode::ParticlesBillBoardRendererNode() : BaseNode()
{
	m_NodeTypeString = "PARTICLES_BILLBOARDS_RENDERER";
}

ParticlesBillBoardRendererNode::~ParticlesBillBoardRendererNode()
{
	Unit();
}

bool ParticlesBillBoardRendererNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Sprite Renderer";

	AddInput(NodeSlotTexelBufferInput::Create("Particles", "PARTICLES"), true, false);
	AddOutput(NodeSlotTextureOutput::Create("Output", 0U), true);

	bool res = false;

	m_ParticlesBillBoardRenderer = ParticlesBillBoardRenderer::Create(vVulkanCorePtr);
	if (m_ParticlesBillBoardRenderer)
	{
		res = true;
	}

	return res;
}

void ParticlesBillBoardRendererNode::Unit()
{
	m_ParticlesBillBoardRenderer.reset();
}

bool ParticlesBillBoardRendererNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	if (m_ParticlesBillBoardRenderer)
	{
		return m_ParticlesBillBoardRenderer->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}

	return false;
}

bool ParticlesBillBoardRendererNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_ParticlesBillBoardRenderer)
	{
		return m_ParticlesBillBoardRenderer->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void ParticlesBillBoardRendererNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_ParticlesBillBoardRenderer)
	{
		m_ParticlesBillBoardRenderer->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void ParticlesBillBoardRendererNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

void ParticlesBillBoardRendererNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (m_ParticlesBillBoardRenderer)
	{
		m_ParticlesBillBoardRenderer->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void ParticlesBillBoardRendererNode::SetTexelBuffer(const uint32_t& vBinding, vk::Buffer* vTexelBuffer, ct::uvec2* vTexelBufferSize)
{
	ZoneScoped;

	if (m_ParticlesBillBoardRenderer)
	{
		return m_ParticlesBillBoardRenderer->SetTexelBuffer(vBinding, vTexelBuffer, vTexelBufferSize);
	}
}

void ParticlesBillBoardRendererNode::SetTexelBufferView(const uint32_t& vBinding, vk::BufferView* vTexelBufferView, ct::uvec2* vTexelBufferSize)
{
	ZoneScoped;

	if (m_ParticlesBillBoardRenderer)
	{
		return m_ParticlesBillBoardRenderer->SetTexelBufferView(vBinding, vTexelBufferView, vTexelBufferSize);
	}
}

vk::DescriptorImageInfo* ParticlesBillBoardRendererNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	ZoneScoped;

	if (m_ParticlesBillBoardRenderer)
	{
		return m_ParticlesBillBoardRenderer->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return VK_NULL_HANDLE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string ParticlesBillBoardRendererNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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
			(uint32_t)nodeID.Get());

		for (auto slot : m_Inputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}
			
		for (auto slot : m_Outputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		if (m_ParticlesBillBoardRenderer)
		{
			res += m_ParticlesBillBoardRenderer->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool ParticlesBillBoardRendererNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_ParticlesBillBoardRenderer)
	{
		m_ParticlesBillBoardRenderer->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

void ParticlesBillBoardRendererNode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_ParticlesBillBoardRenderer)
	{
		m_ParticlesBillBoardRenderer->UpdateShaders(vFiles);
	}
}