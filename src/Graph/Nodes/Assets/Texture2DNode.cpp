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

#include "Texture2DNode.h"
#include <Modules/Assets/Texture2DModule.h>

std::shared_ptr<Texture2DNode> Texture2DNode::Create(vkApi::VulkanCore* vVulkanCore)
{
	auto res = std::make_shared<Texture2DNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCore))
	{
		res.reset();
	}
	return res;
}

Texture2DNode::Texture2DNode() : BaseNode()
{
	m_NodeType = NodeTypeEnum::TEXTURE_2D;
}

Texture2DNode::~Texture2DNode()
{

}

bool Texture2DNode::Init(vkApi::VulkanCore* vVulkanCore)
{
	name = "Texture 2D";

	NodeSlot slot;
	slot.slotType = NodeSlotTypeEnum::TEXTURE_2D;
	slot.name = "Output";
	slot.showWidget = true;
	AddOutput(slot, true, true);

	m_Texture2DModule = Texture2DModule::Create(vVulkanCore, m_This);
	if (m_Texture2DModule)
	{
		return true;
	}

	return false;
}

bool Texture2DNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_Texture2DModule)
	{
		return m_Texture2DModule->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void Texture2DNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	if (m_Texture2DModule)
	{
		m_Texture2DModule->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void Texture2DNode::DrawOutputWidget(BaseNodeStateStruct* vCanvasState, NodeSlotWeak vSlot)
{
	// one output only
	if (m_Texture2DModule)
	{
		m_Texture2DModule->DrawTexture2D(100);
	}
}

void Texture2DNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::TextureUpdateDone:
	{
		auto slots = GetOutputSlotsOfType(NodeSlotTypeEnum::TEXTURE_2D);
		for (const auto& slot : slots)
		{
			auto slotPtr = slot.getValidShared();
			if (slotPtr)
			{
				slotPtr->Notify(NotifyEvent::TextureUpdateDone, slot);
			}
		}
		break;
	}
	default:
		break;
	}
}

vk::DescriptorImageInfo* Texture2DNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	if (m_Texture2DModule)
	{
		return m_Texture2DModule->GetDescriptorImageInfo(vBindingPoint);
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