/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "LightNode.h"
#include <Modules/Lighting/LightGroupModule.h>

std::shared_ptr<LightNode> LightNode::Create(vkApi::VulkanCore* vVulkanCore)
{
	auto res = std::make_shared<LightNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCore))
	{
		res.reset();
	}
	return res;
}

LightNode::LightNode() : BaseNode()
{
	m_NodeType = NodeTypeEnum::LIGHT;
}

LightNode::~LightNode()
{

}

bool LightNode::Init(vkApi::VulkanCore* vVulkanCore)
{
	name = "Light";

	NodeSlot slot;
	slot.slotType = NodeSlotTypeEnum::LIGHT;
	slot.name = "Output";
	AddOutput(slot, true, true);

	bool res = false;
	m_LightGroupModulePtr = LightGroupModule::Create(vVulkanCore, m_This);
	if (m_LightGroupModulePtr)
	{
		res = true;
	}

	return res;
}

bool LightNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_LightGroupModulePtr)
	{
		return m_LightGroupModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void LightNode::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	if (m_LightGroupModulePtr)
	{
		m_LightGroupModulePtr->DrawOverlays(vCurrentFrame, vRect, vContext);
	}
}

SceneLightGroupWeak LightNode::GetLightGroup()
{
	if (m_LightGroupModulePtr)
	{
		return m_LightGroupModulePtr->GetLightGroup();
	}

	return SceneLightGroupWeak();
}

void LightNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::LightUpdateDone:
	{
		auto slots = GetOutputSlotsOfType(NodeSlotTypeEnum::LIGHT);
		for (const auto& slot : slots)
		{
			auto slotPtr = slot.getValidShared();
			if (slotPtr)
			{
				slotPtr->Notify(NotifyEvent::LightUpdateDone, slot);
			}
		}
		break;
	}
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string LightNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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
			Graph::GetStringFromNodeTypeEnum(m_NodeType).c_str(),
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

		if (m_LightGroupModulePtr)
		{
			res += m_LightGroupModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool LightNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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