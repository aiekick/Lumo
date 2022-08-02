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

#include "Layering2DNode.h"
#include <Modules/Simulation/Layering2DModule.h>
#include <Graph/Slots/Texture/NodeSlotTextureInput.h>
#include <Graph/Slots/Texture/NodeSlotTextureOutput.h>

std::shared_ptr<Layering2DNode> Layering2DNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<Layering2DNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

Layering2DNode::Layering2DNode() : BaseNode()
{
	m_NodeTypeString = "2D_LAYERING";
}

Layering2DNode::~Layering2DNode()
{
	Unit();
}

bool Layering2DNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "2D Layering";

	AddInput(NodeSlotTextureInput::Create("Input", 0U), true, false);
	AddInput(NodeSlotTextureInput::Create("Color", 1U), true, false);
	AddOutput(NodeSlotTextureOutput::Create("Output", 0U), true, true);

	bool res = false;

	m_Layering2DModulePtr = Layering2DModule::Create(vVulkanCorePtr);
	if (m_Layering2DModulePtr)
	{
		res = true;
	}

	return res;
}

bool Layering2DNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	bool res = false;

	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureInputDescriptorImageInfos(m_Inputs);

	if (m_Layering2DModulePtr)
	{
		res = m_Layering2DModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);

		//SendFrontNotification(NotifyEvent::TextureUpdateDone);
	}

	return res;
}

bool Layering2DNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_Layering2DModulePtr)
	{
		return m_Layering2DModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void Layering2DNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_Layering2DModulePtr)
	{
		m_Layering2DModulePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void Layering2DNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

void Layering2DNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (m_Layering2DModulePtr)
	{
		m_Layering2DModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void Layering2DNode::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	if (m_Layering2DModulePtr)
	{
		m_Layering2DModulePtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	}
}

vk::DescriptorImageInfo* Layering2DNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_Layering2DModulePtr)
	{
		return m_Layering2DModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string Layering2DNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_Layering2DModulePtr)
		{
			res += m_Layering2DModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool Layering2DNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_Layering2DModulePtr)
	{
		m_Layering2DModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

void Layering2DNode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_Layering2DModulePtr)
	{
		m_Layering2DModulePtr->UpdateShaders(vFiles);
	}
}