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

#include "SSAONode.h"
#include <Modules/PostPro/SSAOModule.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureOutput.h>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

std::shared_ptr<SSAONode> SSAONode::Create(GaiApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<SSAONode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

SSAONode::SSAONode() : BaseNode()
{
	m_NodeTypeString = "SSAO";
}

SSAONode::~SSAONode()
{
	Unit();
}

bool SSAONode::Init(GaiApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "SSAO";

	AddInput(NodeSlotTextureInput::Create("Position", 0U), true, false);
	AddInput(NodeSlotTextureInput::Create("Normal", 1U), true, false);
	AddInput(NodeSlotTextureInput::Create("Noise", 2U), true, false);
	AddOutput(NodeSlotTextureOutput::Create("Output", 0U), true, true);

	bool res = false;

	m_SSAOModulePtr = SSAOModule::Create(vVulkanCorePtr);
	if (m_SSAOModulePtr)
	{
		res = true;
	}

	return res;
}

bool SSAONode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

	// for update input texture buffer infos => avoid vk crash
	UpdateTextureInputDescriptorImageInfos(m_Inputs);

	if (m_SSAOModulePtr)
	{
		return m_SSAOModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}
	return false;
}

bool SSAONode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	if (m_SSAOModulePtr)
	{
		return m_SSAOModulePtr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
	}

	return false;
}

bool SSAONode::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool SSAONode::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	if (m_SSAOModulePtr)
	{
        return m_SSAOModulePtr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
    }
    return false;
}

void SSAONode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

void SSAONode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (m_SSAOModulePtr)
	{
		m_SSAOModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void SSAONode::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	if (m_SSAOModulePtr)
	{
		m_SSAOModulePtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	}
}

vk::DescriptorImageInfo* SSAONode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_SSAOModulePtr)
	{
		return m_SSAOModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string SSAONode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_SSAOModulePtr)
		{
			res += m_SSAOModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool SSAONode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_SSAOModulePtr)
	{
		m_SSAOModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

void SSAONode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_SSAOModulePtr)
	{
		m_SSAOModulePtr->UpdateShaders(vFiles);
	}
}