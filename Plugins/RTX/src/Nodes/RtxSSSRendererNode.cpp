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

#include "RtxSSSRendererNode.h"
#include <Modules/RtxSSSRenderer.h>
#include <Interfaces/LightGroupOutputInterface.h>
#include <Interfaces/AccelStructureOutputInterface.h>

std::shared_ptr<RtxSSSRendererNode> RtxSSSRendererNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<RtxSSSRendererNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

RtxSSSRendererNode::RtxSSSRendererNode() : BaseNode()
{
	m_NodeTypeString = "RTX_SSS_RENDERER";
}

RtxSSSRendererNode::~RtxSSSRendererNode()
{
	Unit();
}

bool RtxSSSRendererNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "SSS Renderer";

	NodeSlot slot;

	slot.slotType = "RTX_ACCEL_STRUCTURE";
	slot.name = "Accel Struct";
	AddInput(slot, true, false);

	slot.slotType = "LIGHT_GROUP";
	slot.name = "Lights";
	AddInput(slot, true, false);

	slot.slotType = "TEXTURE_2D";
	slot.name = "Output";
	slot.descriptorBinding = 0U;
	AddOutput(slot, true, true);

	bool res = false;

	m_RtxSSSRendererPtr = RtxSSSRenderer::Create(vVulkanCorePtr);
	if (m_RtxSSSRendererPtr)
	{
		res = true;
	}

	return res;
}

void RtxSSSRendererNode::Unit()
{
	m_RtxSSSRendererPtr.reset();
}

bool RtxSSSRendererNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);

	if (m_RtxSSSRendererPtr)
	{
		return m_RtxSSSRendererPtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}

	return false;
}

bool RtxSSSRendererNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_RtxSSSRendererPtr)
	{
		return m_RtxSSSRendererPtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void RtxSSSRendererNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_RtxSSSRendererPtr)
	{
		m_RtxSSSRendererPtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void RtxSSSRendererNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
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

void RtxSSSRendererNode::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (m_RtxSSSRendererPtr)
	{
		m_RtxSSSRendererPtr->NeedResize(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResize(vNewSize, vCountColorBuffers);
}

vk::DescriptorImageInfo* RtxSSSRendererNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_RtxSSSRendererPtr)
	{
		return m_RtxSSSRendererPtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

void RtxSSSRendererNode::SetAccelStruct(SceneAccelStructureWeak vSceneAccelStructure)
{
	if (m_RtxSSSRendererPtr)
	{
		m_RtxSSSRendererPtr->SetAccelStruct(vSceneAccelStructure);
	}
}

void RtxSSSRendererNode::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	if (m_RtxSSSRendererPtr)
	{
		m_RtxSSSRendererPtr->SetLightGroup(vSceneLightGroup);
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connect?
void RtxSSSRendererNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_RtxSSSRendererPtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			if (startSlotPtr->slotType == "RTX_ACCEL_STRUCTURE")
			{
				auto otherModelNodePtr = dynamic_pointer_cast<AccelStructureOutputInterface>(endSlotPtr->parentNode.getValidShared());
				if (otherModelNodePtr)
				{
					SetAccelStruct(otherModelNodePtr->GetAccelStruct());
				}
			}
			else if (startSlotPtr->slotType == "LIGHT_GROUP")
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

// le start est toujours le slot de ce node, l'autre le slot du node connect?
void RtxSSSRendererNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_RtxSSSRendererPtr)
	{
		if (startSlotPtr->linkedSlots.empty()) // connected to nothing
		{
			if (startSlotPtr->slotType == "RTX_ACCEL_STRUCTURE")
			{
				SetAccelStruct();
			}
			else if (startSlotPtr->slotType == "LIGHT_GROUP")
			{
				SetLightGroup();
			}
		}
	}
}

void RtxSSSRendererNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::AccelStructureUpdateDone:
	{
		auto emiterSlotPtr = vEmitterSlot.getValidShared();
		if (emiterSlotPtr)
		{
			if (emiterSlotPtr->IsAnOutput())
			{
				auto otherNodePtr = dynamic_pointer_cast<AccelStructureOutputInterface>(emiterSlotPtr->parentNode.getValidShared());
				if (otherNodePtr)
				{
					SetAccelStruct(otherNodePtr->GetAccelStruct());
				}
			}
		}
		break;
	}
	case NotifyEvent::LightGroupUpdateDone:
	{
		auto emiterSlotPtr = vEmitterSlot.getValidShared();
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
		break;
	}
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string RtxSSSRendererNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_RtxSSSRendererPtr)
		{
			res += m_RtxSSSRendererPtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool RtxSSSRendererNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_RtxSSSRendererPtr)
	{
		m_RtxSSSRendererPtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

void RtxSSSRendererNode::UpdateShaders(const std::set<std::string>& vFiles)
{
	if (m_RtxSSSRendererPtr)
	{
		m_RtxSSSRendererPtr->UpdateShaders(vFiles);
	}
}