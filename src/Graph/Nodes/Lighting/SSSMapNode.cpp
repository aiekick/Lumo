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

#include "SSSMapNode.h"
#include <Modules/Lighting/ShadowMapModule.h>
#include <Interfaces/ModelOutputInterface.h>

std::shared_ptr<SSSMapNode> SSSMapNode::Create(vkApi::VulkanCore* vVulkanCore)
{
	auto res = std::make_shared<SSSMapNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCore))
	{
		res.reset();
	}
	return res;
}

SSSMapNode::SSSMapNode() : BaseNode()
{
	m_NodeType = NodeTypeEnum::SHADOW_MAPPING;
}

SSSMapNode::~SSSMapNode()
{
	Unit();
}

bool SSSMapNode::Init(vkApi::VulkanCore* vVulkanCore)
{
	name = "Shadow Map";

	NodeSlot slot;

	slot.slotType = NodeSlotTypeEnum::LIGHT;
	slot.name = "Light";
	AddInput(slot, true, false);

	slot.slotType = NodeSlotTypeEnum::MESH;
	slot.name = "Mesh";
	AddInput(slot, true, false);

	slot.slotType = NodeSlotTypeEnum::LIGHT;
	slot.name = "Light";
	AddOutput(slot, true, true);

	slot.slotType = NodeSlotTypeEnum::TEXTURE_2D;
	slot.name = "Output";
	slot.descriptorBinding = 0U;
	AddOutput(slot, true, true);

	bool res = false;
	m_ShadowMapModulePtr = ShadowMapModule::Create(vVulkanCore);
	if (m_ShadowMapModulePtr)
	{
		res = true;
	}

	return res;
}

void SSSMapNode::Unit()
{
	m_ShadowMapModulePtr.reset();
}

bool SSSMapNode::Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd);

	if (m_ShadowMapModulePtr)
	{
		return m_ShadowMapModulePtr->Execute(vCurrentFrame, vCmd);
	}
	return false;
}

bool SSSMapNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_ShadowMapModulePtr)
	{
		return m_ShadowMapModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void SSSMapNode::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	if (m_ShadowMapModulePtr)
	{
		m_ShadowMapModulePtr->DrawOverlays(vCurrentFrame, vRect, vContext);
	}
}

void SSSMapNode::SetModel(SceneModelWeak vSceneModel)
{
	if (m_ShadowMapModulePtr)
	{
		m_ShadowMapModulePtr->SetModel(vSceneModel);
	}
}

void SSSMapNode::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	if (m_ShadowMapModulePtr)
	{
		m_ShadowMapModulePtr->SetLightGroup(vSceneLightGroup);
	}
}

SceneLightGroupWeak SSSMapNode::GetLightGroup()
{
	if (m_ShadowMapModulePtr)
	{
		return m_ShadowMapModulePtr->GetLightGroup();
	}

	return SceneLightGroupWeak();
}

vk::DescriptorImageInfo* SSSMapNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	if (m_ShadowMapModulePtr)
	{
		return m_ShadowMapModulePtr->GetDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void SSSMapNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_ShadowMapModulePtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == NodeSlotTypeEnum::MESH)
			{
				auto otherModelNodePtr = dynamic_pointer_cast<ModelOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherModelNodePtr)
				{
					SetModel(otherModelNodePtr->GetModel());
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

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void SSSMapNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_ShadowMapModulePtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == NodeSlotTypeEnum::MESH)
			{
				SetModel();
			}
			else if (startSlotPtr->slotType == NodeSlotTypeEnum::LIGHT)
			{
				SetLightGroup();
			}
		}
	}
}

void SSSMapNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::ModelUpdateDone:
	{
		auto emiterSlotPtr = vEmmiterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto otherNodePtr = dynamic_pointer_cast<ModelOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					SetModel(otherNodePtr->GetModel());
				}
			}
		}
		break;
	}
	case NotifyEvent::LightUpdateDone:
	{
		// maj dans ce node
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

		// propagation en output
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

void SSSMapNode::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	if (m_ShadowMapModulePtr)
	{
		m_ShadowMapModulePtr->NeedResize(vNewSize, vCountColorBuffer);
	}

	// on fait ca apres
	BaseNode::NeedResize(vNewSize, vCountColorBuffer);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string SSSMapNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_ShadowMapModulePtr)
		{
			res += m_ShadowMapModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool SSSMapNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_ShadowMapModulePtr)
	{
		m_ShadowMapModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}