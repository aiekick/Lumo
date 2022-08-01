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

#include "MeshNode.h"
#include <Modules/Assets/MeshModule.h>

std::shared_ptr<MeshNode> MeshNode::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<MeshNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}
	return res;
}

MeshNode::MeshNode() : BaseNode()
{
	m_NodeTypeString = "MESH";
}

MeshNode::~MeshNode()
{

}

bool MeshNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	name = "Model";

	NodeSlot slot;
	slot.slotType = "MESH";
	slot.name = "Output";
	slot.showWidget = true;
	AddOutput(slot, true, true);

	m_MeshModule = MeshModule::Create(vVulkanCorePtr, m_This);
	if (m_MeshModule)
	{
		return true;
	}
	return false;
}

bool MeshNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_MeshModule)
	{
		return m_MeshModule->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void MeshNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_MeshModule)
	{
		m_MeshModule->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

SceneModelWeak MeshNode::GetModel()
{
	if (m_MeshModule)
	{
		return m_MeshModule->GetModel();
	}

	return SceneModelWeak();
}

void MeshNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::ModelUpdateDone:
	{
		auto slots = GetOutputSlotsOfType("MESH");
		for (const auto& slot : slots)
		{
			auto slotPtr = slot.getValidShared();
			if (slotPtr)
			{
				slotPtr->Notify(NotifyEvent::ModelUpdateDone, slot);
			}
		}
		break;
	}
	default:
		break;
	}
}

void MeshNode::DrawOutputWidget(BaseNodeState* vBaseNodeState, NodeSlotWeak vSlot)
{
	// one output only
	if (m_MeshModule)
	{
		ImGui::Text("%s", m_MeshModule->GetFileName().c_str());
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string MeshNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_MeshModule)
		{
			res += m_MeshModule->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool MeshNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_MeshModule)
	{
		m_MeshModule->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}