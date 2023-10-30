/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

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

#include "ChannelRendererNode.h"
#include <Modules/Renderers/ChannelRenderer.h>
#include <LumoBackend/Interfaces/ModelOutputInterface.h>
#include <LumoBackend/Graph/Slots/NodeSlotModelInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureOutput.h>
#include <LumoBackend/Graph/Slots/NodeSlotShaderPassOutput.h>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

std::shared_ptr<ChannelRendererNode> ChannelRendererNode::Create(GaiApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<ChannelRendererNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

ChannelRendererNode::ChannelRendererNode() : BaseNode()
{
	m_NodeTypeString = "CHANNEL_RENDERER";
}

ChannelRendererNode::~ChannelRendererNode()
{
	Unit();
}

bool ChannelRendererNode::Init(GaiApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Channels";

	AddInput(NodeSlotModelInput::Create("Model"), true, true);
	AddOutput(NodeSlotTextureOutput::Create("Output", 0U), true, true);
	AddOutput(NodeSlotShaderPassOutput::Create("Output", 1U), true, true);

	bool res = false;

	m_ChannelRenderer = ChannelRenderer::Create(vVulkanCorePtr);
	if (m_ChannelRenderer)
	{
		res = true;
	}

	return res;
}

void ChannelRendererNode::Unit()
{
	m_ChannelRenderer.reset();
}

bool ChannelRendererNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

	if (m_ChannelRenderer)
	{
		return m_ChannelRenderer->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}
	return false;
}

bool ChannelRendererNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	if (m_ChannelRenderer)
	{
		return m_ChannelRenderer->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
	}

	return false;
}

bool ChannelRendererNode::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool ChannelRendererNode::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	if (m_ChannelRenderer)
	{
        return m_ChannelRenderer->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
    }
    return false;
}

void ChannelRendererNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

void ChannelRendererNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (m_ChannelRenderer)
	{
		m_ChannelRenderer->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void ChannelRendererNode::SetModel(SceneModelWeak vSceneModel)
{
	if (m_ChannelRenderer)
	{
		m_ChannelRenderer->SetModel(vSceneModel);
	}
}

vk::DescriptorImageInfo* ChannelRendererNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_ChannelRenderer)
	{
		return m_ChannelRenderer->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SHADER PASS SLOT OUTPUT /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneShaderPassWeak ChannelRendererNode::GetShaderPasses(const uint32_t& vSlotID)
{
	if (m_ChannelRenderer)
	{
		return m_ChannelRenderer->GetShaderPasses(vSlotID);
	}

	return SceneShaderPassWeak();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string ChannelRendererNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_ChannelRenderer)
		{
			res += m_ChannelRenderer->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool ChannelRendererNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_ChannelRenderer)
	{
		m_ChannelRenderer->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}