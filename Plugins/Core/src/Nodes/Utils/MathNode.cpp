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

#include "MathNode.h"
#include <Modules/Utils/MathModule.h>
#include <Graph/Slots/NodeSlotTextureInput.h>
#include <Graph/Slots/NodeSlotTextureOutput.h>

std::shared_ptr<MathNode> MathNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<MathNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

MathNode::MathNode() : BaseNode()
{
	m_NodeTypeString = "MATH";
}

MathNode::~MathNode()
{
	Unit();
}

bool MathNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Math";

	for (uint32_t i = 0U; i < 3U; ++i)
	{
		auto slotPtr = NodeSlotTextureInput::Create("", i);
		if (slotPtr)
		{
			slotPtr->hidden = true;
			AddInput(slotPtr, true, false);
		}
	}

	auto slotPtr = NodeSlotTextureOutput::Create("Output", 0U);
	if (slotPtr)
	{
		slotPtr->showWidget = true;
		AddOutput(slotPtr, true, false);
	}

	bool res = false;

	m_MathModulePtr = MathModule::Create(vVulkanCorePtr);
	if (m_MathModulePtr)
	{
		ReorganizeSlots();

		res = true;
	}

	return res;
}

bool MathNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	// UpdateTextureInputDescriptorImageInfos(m_Inputs);

	bool res = false;

	if (m_MathModulePtr)
	{
		res = m_MathModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);

		SendFrontNotification(TextureUpdateDone);
	}
	
	return res;
}

bool MathNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	bool change = false;

	if (m_MathModulePtr)
	{
		change = m_MathModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	if (change)
	{
		ReorganizeSlots();
	}

	return change;
}

void MathNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (m_MathModulePtr)
	{
		m_MathModulePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void MathNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

void MathNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (m_MathModulePtr)
	{
		m_MathModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void MathNode::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	if (m_MathModulePtr)
	{
		m_MathModulePtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	}
}

vk::DescriptorImageInfo* MathNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_MathModulePtr)
	{
		return m_MathModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

void MathNode::TreatNotification(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot)
{
	if (vEvent == GraphIsLoaded)
	{
		ReorganizeSlots();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string MathNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_MathModulePtr)
		{
			res += m_MathModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool MathNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_MathModulePtr)
	{
		m_MathModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

void MathNode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_MathModulePtr)
	{
		m_MathModulePtr->UpdateShaders(vFiles);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE /////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void MathNode::DrawOutputWidget(BaseNodeState* vBaseNodeState, NodeSlotWeak vSlot)
{
	if (vBaseNodeState)
	{
		auto slotPtr = vSlot.getValidShared();
		if (slotPtr)
		{
			if (m_MathModulePtr)
			{
				m_MathModulePtr->DrawNodeWidget(vBaseNodeState->m_CurrentFrame, ImGui::GetCurrentContext());
			}
		}
	}
}

void MathNode::ReorganizeSlots()
{
	if (m_MathModulePtr)
	{
		auto count = m_MathModulePtr->GetComponentCount();

		// on va montrer que les slots utiles

		uint32_t idx = 0;
		for (auto& input : m_Inputs)
		{
			if (input.second)
			{ 
				if (idx >= count)
				{
					input.second->hidden = true;
					DisConnectSlot(input.second);
				}
				else
				{
					input.second->hidden = false;
					input.second->name = m_MathModulePtr->GetInputName(idx);
				}
			}
			++idx;
		}
	}
}