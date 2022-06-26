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

#include "ComputeSmoothMeshNormalNode.h"
#include <Modules/Modifiers/ComputeSmoothMeshNormal.h>

std::shared_ptr<ComputeSmoothMeshNormalNode> ComputeSmoothMeshNormalNode::Create(vkApi::VulkanCore* vVulkanCore)
{
	auto res = std::make_shared<ComputeSmoothMeshNormalNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCore))
	{
		res.reset();
	}
	return res;
}

ComputeSmoothMeshNormalNode::ComputeSmoothMeshNormalNode() : BaseNode()
{
	m_NodeType = NodeTypeEnum::COMPUTE_SMOOTH_MESH_NORMAL;
}

ComputeSmoothMeshNormalNode::~ComputeSmoothMeshNormalNode()
{

}

bool ComputeSmoothMeshNormalNode::Init(vkApi::VulkanCore* vVulkanCore)
{
	name = "Smooth Normal";

	NodeSlot slot;
	slot.slotType = NodeSlotTypeEnum::MESH;
	slot.showWidget = true;
	AddInput(slot, true, true);

	slot.slotType = NodeSlotTypeEnum::MESH;
	slot.name = "Output";
	slot.showWidget = true;
	AddOutput(slot, true, true);

	m_ComputeSmoothMeshNormalPtr = ComputeSmoothMeshNormal::Create(vVulkanCore, m_This, ct::uvec3(1,0,0));
	if (m_ComputeSmoothMeshNormalPtr)
	{
		return true;
	}

	return false;
}

bool ComputeSmoothMeshNormalNode::Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer *vCmd)
{
	BaseNode::ExecuteChilds(vCurrentFrame, vCmd);

	if (m_ComputeSmoothMeshNormalPtr)
	{
		return m_ComputeSmoothMeshNormalPtr->Execute(vCurrentFrame, vCmd);
	}
	return false;
}

bool ComputeSmoothMeshNormalNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_ComputeSmoothMeshNormalPtr)
	{
		return m_ComputeSmoothMeshNormalPtr->DrawWidgets(vCurrentFrame, vContext);
	}

	return false;
}

void ComputeSmoothMeshNormalNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	if (m_ComputeSmoothMeshNormalPtr)
	{
		m_ComputeSmoothMeshNormalPtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
	}
}

void ComputeSmoothMeshNormalNode::DisplayInfosOnTopOfTheNode(BaseNodeStateStruct* vCanvasState)
{
	if (vCanvasState && vCanvasState->debug_mode)
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

void ComputeSmoothMeshNormalNode::SetModel(SceneModelWeak vSceneModel)
{
	if (m_ComputeSmoothMeshNormalPtr)
	{
		m_ComputeSmoothMeshNormalPtr->SetModel(vSceneModel);
	}
}

SceneModelWeak ComputeSmoothMeshNormalNode::GetModel()
{
	if (m_ComputeSmoothMeshNormalPtr)
	{
		return m_ComputeSmoothMeshNormalPtr->GetModel();
	}

	return SceneModelWeak();
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void ComputeSmoothMeshNormalNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr && m_ComputeSmoothMeshNormalPtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			auto otherNodePtr = dynamic_pointer_cast<ModelOutputInterface>(endSlotPtr->parentNode.getValidShared());
			if (otherNodePtr)
			{
				SetModel(otherNodePtr->GetModel());
			}
			else
			{
#ifdef _DEBUG
				LogVarInfo("This node not inherit of ModelInterface");
#endif
			}
		}
	}
}

// le start est toujours le slot de ce node, l'autre le slot du node connecté
void ComputeSmoothMeshNormalNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
	auto startSlotPtr = vStartSlot.getValidShared();
	auto endSlotPtr = vEndSlot.getValidShared();
	if (startSlotPtr && endSlotPtr)
	{
		if (startSlotPtr->IsAnInput())
		{
			SetModel();
		}
	}
}

void ComputeSmoothMeshNormalNode::Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot, const NodeSlotWeak& vReceiverSlot)
{
	switch (vEvent)
	{
	case NotifyEvent::ModelUpdateDone:
	{
		auto emiterSlotPtr = vEmmiterSlot.getValidShared();
		if (emiterSlotPtr && m_ComputeSmoothMeshNormalPtr)
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
	default:
		break;
	}
}

void ComputeSmoothMeshNormalNode::DrawOutputWidget(BaseNodeStateStruct* vCanvasState, NodeSlotWeak vSlot)
{
	// one output only
	//if (m_ComputeSmoothMeshNormalPtr)
	{
		//ImGui::Text("%s", m_ComputeSmoothMeshNormal->GetFileName().c_str());
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string ComputeSmoothMeshNormalNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
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

		if (m_ComputeSmoothMeshNormalPtr)
		{
			res += m_ComputeSmoothMeshNormalPtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool ComputeSmoothMeshNormalNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_ComputeSmoothMeshNormalPtr)
	{
		m_ComputeSmoothMeshNormalPtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}