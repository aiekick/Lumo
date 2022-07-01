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

#include "ModelShadowNode.h"
#include <Modules/Lighting/ModelShadowModule.h>
#include <Interfaces/LightOutputInterface.h>

std::shared_ptr<ModelShadowNode> ModelShadowNode::Create(vkApi::VulkanCore* vVulkanCore)
{
	auto res = std::make_shared<ModelShadowNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCore))
	{
		res.reset();
	}
	return res;
}

ModelShadowNode::ModelShadowNode() : BaseNode()
{
	m_NodeType = NodeTypeEnum::MODEL_SHADOW;
}

ModelShadowNode::~ModelShadowNode()
{
	Unit();
}

bool ModelShadowNode::Init(vkApi::VulkanCore* vVulkanCore)
{
	name = "Model Shadow";

	NodeSlot slot;

	slot.slotType = NodeSlotTypeEnum::LIGHT;
	slot.name = "Light";
	AddInput(slot, true, false);

	slot.slotType = NodeSlotTypeEnum::TEXTURE_2D;
	slot.name = "Position";
	slot.descriptorBinding = 0U;
	AddInput(slot, true, false);

	slot.slotType = NodeSlotTypeEnum::DEPTH;
	slot.name = "Shadow Map";
	slot.descriptorBinding = 1U;
	AddInput(slot, true, false);

	slot.slotType = NodeSlotTypeEnum::TEXTURE_2D;
	slot.name = "Output";
	slot.descriptorBinding = 0U;
	AddOutput(slot, true, true);

	bool res = false;
	m_ModelShadowModulePtr = ModelShadowModule::Create(vVulkanCore);
	if (m_ModelShadowModulePtr)
	{
		res = true;
	}

	return res;
}

void ModelShadowNode::Unit()
{
	m_ModelShadowModulePtr.reset();
}

bool ModelShadowNode::Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd);

	// for update input texture buffer infos => avoid vk crash
	UpdateInputDescriptorImageInfos(m_Inputs);

	if (m_ModelShadowModulePtr)
	{
		return m_ModelShadowModulePtr->Execute(vCurrentFrame, vCmd);
	}
	return false;
}

bool ModelShadowNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_ModelShadowModulePtr)
	{
		return m_ModelShadowModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void ModelShadowNode::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	if (m_ModelShadowModulePtr)
	{
		m_ModelShadowModulePtr->DrawOverlays(vCurrentFrame, vRect, vContext);
	}
}

void ModelShadowNode::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	if (m_ModelShadowModulePtr)
	{
		m_ModelShadowModulePtr->SetLightGroup(vSceneLightGroup);
	}
}

void ModelShadowNode::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	if (m_ModelShadowModulePtr)
	{
		m_ModelShadowModulePtr->SetTexture(vBinding, vImageInfo);
	}
}

vk::DescriptorImageInfo* ModelShadowNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	if (m_ModelShadowModulePtr)
	{
		return m_ModelShadowModulePtr->GetDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

// le start est toujours le slot de ce node, l'autre le slot du node connect�
void ModelShadowNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_ModelShadowModulePtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == NodeSlotTypeEnum::TEXTURE_2D)
			{
				auto otherTextureNodePtr = dynamic_pointer_cast<TextureOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherTextureNodePtr)
				{
					SetTexture(startSlotPtr->descriptorBinding, otherTextureNodePtr->GetDescriptorImageInfo(endSlotPtr->descriptorBinding));
				}
			}
			else if (startSlotPtr->slotType == NodeSlotTypeEnum::LIGHT)
			{
				auto otherLightGroupNodePtr = dynamic_pointer_cast<LightOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherLightGroupNodePtr)
				{
					SetLightGroup(otherLightGroupNodePtr->GetLightGroup());
				}
			}
		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connect�
void ModelShadowNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_ModelShadowModulePtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == NodeSlotTypeEnum::TEXTURE_2D)
			{
				SetTexture(startSlotPtr->descriptorBinding, nullptr);
			}
			else if (startSlotPtr->slotType == NodeSlotTypeEnum::LIGHT)
			{
				SetLightGroup();
			}
		}
	}
}

void ModelShadowNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::TextureUpdateDone:
	{
		auto emiterSlotPtr = vEmmiterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto otherNodePtr = dynamic_pointer_cast<TextureOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					auto receiverSlotPtr = vReceiverSlot.getValidShared();
					if (receiverSlotPtr)
					{
						SetTexture(receiverSlotPtr->descriptorBinding, otherNodePtr->GetDescriptorImageInfo(emiterSlotPtr->descriptorBinding));
					}
				}
			}
		}

		//todo emit notification
		break;
	}
	case NotifyEvent::LightUpdateDone:
	{
		auto emiterSlotPtr = vEmmiterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto otherNodePtr = dynamic_pointer_cast<LightOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					SetLightGroup(otherNodePtr->GetLightGroup());
				}
			}
		}
		break;
	}
	default:
		break;
	}
}

void ModelShadowNode::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	if (m_ModelShadowModulePtr)
	{
		m_ModelShadowModulePtr->NeedResize(vNewSize, vCountColorBuffer);
	}

	// on fait ca apres
	BaseNode::NeedResize(vNewSize, vCountColorBuffer);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string ModelShadowNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_ModelShadowModulePtr)
		{
			res += m_ModelShadowModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool ModelShadowNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_ModelShadowModulePtr)
	{
		m_ModelShadowModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}