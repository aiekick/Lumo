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

#include "ShadowMapNode.h"
#include <Modules/Lighting/ShadowMapModule.h>
#include <Interfaces/ModelOutputInterface.h>

std::shared_ptr<ShadowMapNode> ShadowMapNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<ShadowMapNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

ShadowMapNode::ShadowMapNode() : BaseNode()
{
	m_NodeTypeString = "SHADOW_MAPPING";
}

ShadowMapNode::~ShadowMapNode()
{
	Unit();
}

bool ShadowMapNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Shadow Map";

	NodeSlot slot;

	slot.slotType = NodeSlotTypeEnum::LIGHT_GROUP;
	slot.name = "Lights";
	AddInput(slot, true, false);

	slot.slotType = NodeSlotTypeEnum::MESH;
	slot.name = "Mesh";
	AddInput(slot, true, false);

	slot.slotType = NodeSlotTypeEnum::LIGHT_GROUP;
	slot.name = "Lights";
	AddOutput(slot, true, true);

	slot.slotType = NodeSlotTypeEnum::TEXTURE_2D_GROUP;
	slot.name = "Outputs";
	slot.descriptorBinding = 0U;
	AddOutput(slot, true, true);

	bool res = false;
	m_ShadowMapModulePtr = ShadowMapModule::Create(vVulkanCorePtr);
	if (m_ShadowMapModulePtr)
	{
		res = true;
	}

	return res;
}

void ShadowMapNode::Unit()
{
	m_ShadowMapModulePtr.reset();
}

bool ShadowMapNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd);

	if (m_ShadowMapModulePtr)
	{
		return m_ShadowMapModulePtr->Execute(vCurrentFrame, vCmd);
	}
	return false;
}

bool ShadowMapNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_ShadowMapModulePtr)
	{
		return m_ShadowMapModulePtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void ShadowMapNode::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	if (m_ShadowMapModulePtr)
	{
		m_ShadowMapModulePtr->DrawOverlays(vCurrentFrame, vRect, vContext);
	}
}

void ShadowMapNode::SetModel(SceneModelWeak vSceneModel)
{
	if (m_ShadowMapModulePtr)
	{
		m_ShadowMapModulePtr->SetModel(vSceneModel);
	}
}

void ShadowMapNode::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	if (m_ShadowMapModulePtr)
	{
		m_ShadowMapModulePtr->SetLightGroup(vSceneLightGroup);
	}
}

SceneLightGroupWeak ShadowMapNode::GetLightGroup()
{
	if (m_ShadowMapModulePtr)
	{
		return m_ShadowMapModulePtr->GetLightGroup();
	}

	return SceneLightGroupWeak();
}

std::vector<vk::DescriptorImageInfo>* ShadowMapNode::GetDescriptorImageInfos(const uint32_t& vBindingPoint)
{
	if (m_ShadowMapModulePtr)
	{
		return m_ShadowMapModulePtr->GetDescriptorImageInfos(vBindingPoint);
	}

	return nullptr;
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void ShadowMapNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
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
			else if (startSlotPtr->slotType == NodeSlotTypeEnum::LIGHT_GROUP)
			{
				auto otherLightGroupNodePtr = dynamic_pointer_cast<LightGroupOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherLightGroupNodePtr)
				{
					SetLightGroup(otherLightGroupNodePtr->GetLightGroup());
				}
			}
		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void ShadowMapNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
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
			else if (startSlotPtr->slotType == NodeSlotTypeEnum::LIGHT_GROUP)
			{
				SetLightGroup();
			}
		}
	}
}

void ShadowMapNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& vReceiverSlot)
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
	case NotifyEvent::LightGroupUpdateDone:
	{
		// maj dans ce node
		auto emiterSlotPtr = vEmmiterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto otherNodePtr = dynamic_pointer_cast<LightGroupOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					SetLightGroup(otherNodePtr->GetLightGroup());
				}
			}
		}

		// propagation en output
		auto slots = GetOutputSlotsOfType(NodeSlotTypeEnum::LIGHT_GROUP);
		for (const auto& slot : slots)
		{
			auto slotPtr = slot.getValidShared();
			if (slotPtr)
			{
				slotPtr->Notify(NotifyEvent::LightGroupUpdateDone, slot);
			}
		}

		break;
	}
	default:
		break;
	}
}

void ShadowMapNode::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
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

std::string ShadowMapNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_ShadowMapModulePtr)
		{
			res += m_ShadowMapModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool ShadowMapNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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