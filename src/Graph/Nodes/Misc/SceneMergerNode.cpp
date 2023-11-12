/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "SceneMergerNode.h"
#include <Graph/Modules/Misc/SceneMergerModule.h>
#include <LumoBackend/Graph/Slots/NodeSlotShaderPassInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureOutput.h>

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<SceneMergerNode> SceneMergerNode::Create(GaiApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	auto res = std::make_shared<SceneMergerNode>();
	res->m_This = res;
	if (!res->Init(vVulkanCorePtr))
	{
		res.reset();
	}

	return res;
}

SceneMergerNode::SceneMergerNode() : BaseNode()
{
	ZoneScoped;

	m_NodeTypeString = "SCENE_MERGER";

	// variable slot count only for inputs
	// important for xml loading
	m_InputSlotsInternalMode = NODE_INTERNAL_MODE_Enum::NODE_INTERNAL_MODE_DYNAMIC;
}

SceneMergerNode::~SceneMergerNode()
{
	ZoneScoped;

	Unit();
}		

//////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMergerNode::Init(GaiApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	bool res = false;

	name = "Scene Merger";

	AddInput(NodeSlotShaderPassInput::Create(""), false, true);

	AddOutput(NodeSlotTextureOutput::Create("", 0), false, true);

	m_SceneMergerModulePtr = SceneMergerModule::Create(vVulkanCorePtr, m_This);
	if (m_SceneMergerModulePtr)
	{
		res = true;
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TASK EXECUTE ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMergerNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	bool res = false;

	BaseNode::ExecuteInputTasks(vCurrentFrame, vCmd, vBaseNodeState);

	if (m_SceneMergerModulePtr)
	{
		res = m_SceneMergerModulePtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMergerNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	ZoneScoped;

	bool res = false;

	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);

	if (m_SceneMergerModulePtr)
	{
		res = m_SceneMergerModulePtr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
	}

	return res;
}

bool SceneMergerNode::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool SceneMergerNode::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;

	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);

	if (m_SceneMergerModulePtr)
	{
        return m_SceneMergerModulePtr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW NODE ///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void SceneMergerNode::DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	if (vBaseNodeState && vBaseNodeState->debug_mode)
	{
		auto drawList = nd::GetNodeBackgroundDrawList(nodeID);
		if (drawList)
		{
			char debugBuffer[255] = "\0";
			snprintf(debugBuffer, 254,
				"Used[%s]\nCell[%i, %i]",
				(used ? "true" : "false"), cell.x, cell.y);
			ImVec2 txtSize = ImGui::CalcTextSize(debugBuffer);
			drawList->AddText(pos - ImVec2(0, txtSize.y), ImGui::GetColorU32(ImGuiCol_Text), debugBuffer);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// RESIZE //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void SceneMergerNode::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	if (m_SceneMergerModulePtr)
	{
		m_SceneMergerModulePtr->NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
	}

	// on fait ca apres
	BaseNode::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void SceneMergerNode::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{	
	ZoneScoped;

	if (m_SceneMergerModulePtr)
	{
		m_SceneMergerModulePtr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	}
}

void SceneMergerNode::SetShaderPasses(const uint32_t& vSlotID, SceneShaderPassWeak vShaderPasses)
{
	ZoneScoped;

	if (m_SceneMergerModulePtr)
	{
		m_SceneMergerModulePtr->SetShaderPasses(vSlotID, vShaderPasses);
		ReorganizeSlots(m_SceneMergerModulePtr->GetJustDeletedSceneShaderPassSlots());
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* SceneMergerNode::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	ZoneScoped;

	if (m_SceneMergerModulePtr)
	{
		return m_SceneMergerModulePtr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string SceneMergerNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

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

		for (auto& slot : m_Inputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		for (auto& slot : m_Outputs)
		{
			res += slot.second->getXml(vOffset + "\t", vUserDatas);
		}

		if (m_SceneMergerModulePtr)
		{
			res += m_SceneMergerModulePtr->getXml(vOffset + "\t", vUserDatas);
		}

		res += vOffset + "</node>\n";
	}

	return res;
}

bool SceneMergerNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	ZoneScoped;

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

	if (m_SceneMergerModulePtr)
	{
		m_SceneMergerModulePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	// continue recurse child exploring
	return true;
}

NodeSlotWeak SceneMergerNode::AddPreDefinedInput(const NodeSlot& vNodeSlot)
{
	if (vNodeSlot.slotType == "SHADER_PASS")
	{
		auto slot_ptr = NodeSlotShaderPassInput::Create("");
		if (slot_ptr)
		{
			slot_ptr->parentNode = m_This;
			slot_ptr->slotPlace = NodeSlot::PlaceEnum::INPUT;
			slot_ptr->hideName = true;
			slot_ptr->type = Lumo::uType::uTypeEnum::U_FLOW;
			slot_ptr->index = vNodeSlot.index;
			slot_ptr->pinID = vNodeSlot.pinID;
			const auto& slotID = vNodeSlot.GetSlotID();


			// pour eviter que des slots aient le meme id qu'un nodePtr
			BaseNode::freeNodeId = ct::maxi<uint32_t>(BaseNode::freeNodeId, slotID) + 1U;

			m_Inputs[slotID] = slot_ptr;
			return m_Inputs.at(slotID);
		}
	}
	else
	{
		CTOOL_DEBUG_BREAK;
		LogVarError("node slot is of type %s.. must be of type SHADER_PASS", vNodeSlot.slotType.c_str());
	}

	return NodeSlotWeak();
}

void SceneMergerNode::BeforeNodeXmlLoading()
{
	// only when load the xml
	m_Inputs.clear();
}

void SceneMergerNode::AfterNodeXmlLoading()
{
	ZoneScoped;

	if (m_SceneMergerModulePtr)
	{
		m_SceneMergerModulePtr->AfterNodeXmlLoading();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SHADER UPDATE ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void SceneMergerNode::UpdateShaders(const std::set<std::string>& vFiles)
{	
	ZoneScoped;

	if (m_SceneMergerModulePtr)
	{
		m_SceneMergerModulePtr->UpdateShaders(vFiles);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// INPUTS UPDATE ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void SceneMergerNode::ReorganizeSlots(const std::vector<uint32_t>& vSlotsToErase)
{
	if (m_SceneMergerModulePtr)
	{
		// here the goal is :
		// - delete orphan slots (connected to nothing)
		// - add slots if all slots are full 
		
		auto graph_ptr = this->GetParentNode().lock();
		if (graph_ptr)
		{
			// array of slot to prevent destroying during this connect
			const auto& slots_to_keep = graph_ptr->m_SlotsToNotDestroyDuringThisConnect;
			
			// 1) erase slots who was removed by the module
			for (const auto& slotID : vSlotsToErase)
			{
				if (m_Inputs.find(slotID) != m_Inputs.end())
				{
					if (slots_to_keep.find(slotID) == slots_to_keep.end())
					{
						m_Inputs.erase(slotID);
						LogVarDebugWarning("erase input slot(%u)", slotID);
					}
					else
					{
                        LogVarDebugWarning(
                            "input slot(%u) not destroyed because of 'no destroy' rule during this connect", slotID);
					}
				}
				else
				{
					CTOOL_DEBUG_BREAK;
					LogVarError("a slot was erased but dont exist in the node.. there is a bug somewhere");
				}
			}

			// 2) check if all slots are full
			bool need_to_add_a_orphan_slot = true;
			for (const auto& slot : m_Inputs)
			{
				if (slot.second &&
					slot.second->linkedSlots.empty()) // if we found at least one empty nide, we will not add a slot
				{
					need_to_add_a_orphan_slot = false;
					break;
				}
			}

			// 3) we add an orphan slot if needed in 2)
			if (need_to_add_a_orphan_slot)
			{
				AddInput(NodeSlotShaderPassInput::Create(""), false, true);
			}

			LogVarLightInfo("--------------------------------");
		}
		
	}
}