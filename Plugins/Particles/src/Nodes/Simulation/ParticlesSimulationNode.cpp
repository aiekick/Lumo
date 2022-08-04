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

#include "ParticlesSimulationNode.h"
#include <Modules/Simulation/ParticlesSimulationModule.h>
#include <Graph/Slots/NodeSlotTexelBufferInput.h>
#include <Graph/Slots/NodeSlotTexelBufferOutput.h>

std::shared_ptr<ParticlesSimulationNode> ParticlesSimulationNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<ParticlesSimulationNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

ParticlesSimulationNode::ParticlesSimulationNode() : BaseNode()
{
	ZoneScoped;

	m_NodeTypeString = "PARTICLES_SIMULATION";
}

ParticlesSimulationNode::~ParticlesSimulationNode()
{
	ZoneScoped;

	Unit();
}

bool ParticlesSimulationNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	name = "Simulation";

	NodeSlot slot;

	AddInput(NodeSlotTexelBufferInput::Create("Particles", "PARTICLES"), true, false);
	AddOutput(NodeSlotTexelBufferOutput::Create("Particles", "PARTICLES"), true, false);

	m_ParticlesSimulationModulePtr = ParticlesSimulationModule::Create(vVulkanCorePtr, m_This);
	if (m_ParticlesSimulationModulePtr)
	{
		return true;
	}

	return false;
}

bool ParticlesSimulationNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	if (m_ParticlesSimulationModulePtr)
	{
		return m_ParticlesSimulationModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}
	return false;
}

bool ParticlesSimulationNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext);

	if (m_ParticlesSimulationModulePtr)
	{
		return m_ParticlesSimulationModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void ParticlesSimulationNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext);

	if (m_ParticlesSimulationModulePtr)
	{
		m_ParticlesSimulationModulePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void ParticlesSimulationNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

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

void ParticlesSimulationNode::SetTexelBuffer(const uint32_t& vBinding, vk::Buffer* vTexelBuffer, ct::uvec2* vTexelBufferSize)
{
	ZoneScoped;

	if (m_ParticlesSimulationModulePtr)
	{
		m_ParticlesSimulationModulePtr->SetTexelBuffer(vBinding, vTexelBuffer, vTexelBufferSize);
	}
}

void ParticlesSimulationNode::SetTexelBufferView(const uint32_t& vBinding, vk::BufferView* vTexelBufferView, ct::uvec2* vTexelBufferSize)
{
	ZoneScoped;

	if (m_ParticlesSimulationModulePtr)
	{
		m_ParticlesSimulationModulePtr->SetTexelBufferView(vBinding, vTexelBufferView, vTexelBufferSize);
	}
}

vk::Buffer* ParticlesSimulationNode::GetTexelBuffer(const uint32_t& vBindingPoint, ct::uvec2* vOutSize)
{
	ZoneScoped;

	if (m_ParticlesSimulationModulePtr)
	{
		return m_ParticlesSimulationModulePtr->GetTexelBuffer(vBindingPoint, vOutSize);
	}

	return nullptr;
}

vk::BufferView* ParticlesSimulationNode::GetTexelBufferView(const uint32_t& vBindingPoint, ct::uvec2* vOutSize)
{
	ZoneScoped;

	if (m_ParticlesSimulationModulePtr)
	{
		return m_ParticlesSimulationModulePtr->GetTexelBufferView(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string ParticlesSimulationNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_ParticlesSimulationModulePtr)
		{
			res += m_ParticlesSimulationModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool ParticlesSimulationNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_ParticlesSimulationModulePtr)
	{
		m_ParticlesSimulationModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

void ParticlesSimulationNode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_ParticlesSimulationModulePtr)
	{
		m_ParticlesSimulationModulePtr->UpdateShaders(vFiles);
	}
}