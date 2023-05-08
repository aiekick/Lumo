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

#include "LightGroupNode.h"
#include <Modules/Lighting/LightGroupModule.h>
#include <Graph/Slots/NodeSlotLightGroupOutput.h>

std::shared_ptr<LightGroupNode> LightGroupNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<LightGroupNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

LightGroupNode::LightGroupNode() : BaseNode()
{
	m_NodeTypeString = "LIGHT_GROUP";
}

LightGroupNode::~LightGroupNode()
{

}

bool LightGroupNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Lights";

	AddOutput(NodeSlotLightGroupOutput::Create("Output"), true, true);

	bool res = false;
	m_LightGroupModulePtr = LightGroupModule::Create(vVulkanCorePtr, m_This);
	if (m_LightGroupModulePtr)
	{
		res = true;
	}

	return res;
}

bool LightGroupNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	if (m_LightGroupModulePtr)
	{
		return m_LightGroupModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}

	return false;
}

bool LightGroupNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (m_LightGroupModulePtr)
	{
		return m_LightGroupModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void LightGroupNode::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (m_LightGroupModulePtr)
	{
		m_LightGroupModulePtr->DrawOverlays(vCurrentFrame, vRect, vContext);
	}
}

SceneLightGroupWeak LightGroupNode::GetLightGroup()
{
	if (m_LightGroupModulePtr)
	{
		return m_LightGroupModulePtr->GetLightGroup();
	}

	return SceneLightGroupWeak();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string LightGroupNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_LightGroupModulePtr)
		{
			res += m_LightGroupModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool LightGroupNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_LightGroupModulePtr)
	{
		m_LightGroupModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}