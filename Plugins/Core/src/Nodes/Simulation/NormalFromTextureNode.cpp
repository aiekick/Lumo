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

#include "NormalFromTextureNode.h"
#include <Modules/Simulation/NormalFromTextureModule.h>
#include <Graph/Slots/NodeSlotTextureInput.h>
#include <Graph/Slots/NodeSlotTextureOutput.h>

std::shared_ptr<NormalFromTextureNode> NormalFromTextureNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<NormalFromTextureNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

NormalFromTextureNode::NormalFromTextureNode() : BaseNode()
{
	m_NodeTypeString = "2D_NORMAL_FROM_TEXTURE";
}

NormalFromTextureNode::~NormalFromTextureNode()
{
	Unit();
}

bool NormalFromTextureNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "NormalFromTexture";

	AddInput(NodeSlotTextureInput::Create("Input", 0U), true, false);
	AddOutput(NodeSlotTextureOutput::Create("Output", 0U), true, true);

	bool res = false;

	m_NormalFromTextureModulePtr = NormalFromTextureModule::Create(vVulkanCorePtr);
	if (m_NormalFromTextureModulePtr)
	{
		res = true;
	}

	return res;
}

bool NormalFromTextureNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	bool res = false;

	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureInputDescriptorImageInfos(m_Inputs);

	if (m_NormalFromTextureModulePtr)
	{
		res = m_NormalFromTextureModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);

		//SendFrontNotification(NotifyEvent::TextureUpdateDone);
	}

	return res;
}

bool NormalFromTextureNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_NormalFromTextureModulePtr)
	{
		return m_NormalFromTextureModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void NormalFromTextureNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_NormalFromTextureModulePtr)
	{
		m_NormalFromTextureModulePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void NormalFromTextureNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

void NormalFromTextureNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (m_NormalFromTextureModulePtr)
	{
		m_NormalFromTextureModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void NormalFromTextureNode::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	if (m_NormalFromTextureModulePtr)
	{
		m_NormalFromTextureModulePtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	}
}

vk::DescriptorImageInfo* NormalFromTextureNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_NormalFromTextureModulePtr)
	{
		return m_NormalFromTextureModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string NormalFromTextureNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_NormalFromTextureModulePtr)
		{
			res += m_NormalFromTextureModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool NormalFromTextureNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_NormalFromTextureModulePtr)
	{
		m_NormalFromTextureModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

void NormalFromTextureNode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_NormalFromTextureModulePtr)
	{
		m_NormalFromTextureModulePtr->UpdateShaders(vFiles);
	}
}