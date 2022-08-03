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

#include "Texture2DNode.h"
#include <Modules/Assets/Texture2DModule.h>
#include <Graph/Slots/NodeSlotTextureOutput.h>

std::shared_ptr<Texture2DNode> Texture2DNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<Texture2DNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

Texture2DNode::Texture2DNode() : BaseNode()
{
	m_NodeTypeString = "TEXTURE_2D";
}

Texture2DNode::~Texture2DNode()
{

}

bool Texture2DNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Texture 2D";

	AddOutput(NodeSlotTextureOutput::Create("Output", 0U), true, true);

	m_Texture2DModule = Texture2DModule::Create(vVulkanCorePtr, m_This);
	if (m_Texture2DModule)
	{
		return true;
	}

	return false;
}

bool Texture2DNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (m_Texture2DModule)
	{
		return m_Texture2DModule->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void Texture2DNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (m_Texture2DModule)
	{
		m_Texture2DModule->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void Texture2DNode::DrawOutputWidget(BaseNodeState* vBaseNodeState, NodeSlotWeak vSlot)
{
	// one output only
	if (m_Texture2DModule)
	{
		m_Texture2DModule->DrawTexture(50);
	}
}

vk::DescriptorImageInfo* Texture2DNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_Texture2DModule)
	{
		return m_Texture2DModule->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string Texture2DNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_Texture2DModule)
		{
			res += m_Texture2DModule->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool Texture2DNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_Texture2DModule)
	{
		m_Texture2DModule->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}