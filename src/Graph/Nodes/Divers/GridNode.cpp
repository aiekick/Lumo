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

#include "GridNode.h"

#include <Modules/Divers/GridModule.h>

std::shared_ptr<GridNode> GridNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<GridNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

GridNode::GridNode() : BaseNode()
{
	m_NodeTypeString = "GRID_AXIS";
}

GridNode::~GridNode()
{

}

bool GridNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Grid / Axis";

	NodeSlot slot;
	slot.slotType = "TEXTURE_2D";
	slot.name = "Output";
	AddOutput(slot, true, true);

	m_GridModulePtr = GridModule::Create(vVulkanCorePtr);
	if (m_GridModulePtr)
	{
		return true;
	}
	
	return false;
}

void GridNode::Unit()
{
	m_GridModulePtr.reset();
}

bool GridNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	if (m_GridModulePtr)
	{
		return m_GridModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}
	return false;
}

bool GridNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_GridModulePtr)
	{
		return m_GridModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void GridNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

}

void GridNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

void GridNode::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (m_GridModulePtr)
	{
		m_GridModulePtr->NeedResize(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResize(vNewSize, vCountColorBuffers);
}

vk::DescriptorImageInfo* GridNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_GridModulePtr)
	{
		return m_GridModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string GridNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_GridModulePtr)
		{
			res += m_GridModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool GridNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_GridModulePtr)
	{
		m_GridModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}