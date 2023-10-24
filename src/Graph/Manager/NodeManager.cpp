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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "NodeManager.h"

#include <Frontend/MainFrontend.h>
#include <ctools/FileHelper.h>
#include <ImGuiPack.h>
#include <ImGuizmo.h>

#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <LumoBackend/Interfaces/CameraInterface.h>
#include <LumoBackend/Interfaces/ShaderUpdateInterface.h>
#include <LumoBackend/Systems/GizmoSystem.h>
#include <Plugins/PluginManager.h>

#include <Panes/DebugPane.h>
#include <Panes/TuningPane.h>
#include <Panes/View2DPane.h>
#include <Panes/View3DPane.h>
 
#include <Graph/Factory/NodeFactory.h>
#include <Graph/Library/UserNodeLibrary.h>

using namespace GaiApi;
using namespace std::placeholders;

NodeManager::NodeManager()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool NodeManager::Init(GaiApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	m_RootNodePtr = BaseNode::Create(vVulkanCorePtr);
	if (m_RootNodePtr) {
        m_RootNodePtr->SetLoadNodeFromXMLCallback(std::bind(&NodeManager::LoadNodeFromXML, this, _1, _2, _3, _4, _5, _6, _7));
        m_RootNodePtr->SetSelectNodeCallback(std::bind(&NodeManager::SelectNode, this, _1));
        m_RootNodePtr->SetSelectForGraphOutputCallback(std::bind(&NodeManager::SelectNodeForGraphOutput, this, _1, _2));
        m_RootNodePtr->SetNewNodeMenuCallback(std::bind(&UserNodeLibrary::ShowNewNodeMenu, UserNodeLibrary::Instance(), _1, _2));
        NodeSlot::sSlotGraphOutputMouseLeftColor = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);
        NodeSlot::sSlotGraphOutputMouseMiddleColor = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);
		return true;
	}

	return false;
}

void NodeManager::Unit()
{
	ZoneScoped; 

	m_RootNodePtr.reset();
}

void NodeManager::Clear()
{
	m_RootNodePtr->ClearGraph();
}

void NodeManager::PrepareToLoadGraph()
{
	nd::SetCurrentEditor(
		m_RootNodePtr->m_BaseNodeState.m_NodeGraphContext);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// TASK //////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool NodeManager::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	bool res = false;

	m_RootNodePtr->m_BaseNodeState.m_CurrentFrame = vCurrentFrame;
	m_RootNodePtr->m_BaseNodeState.m_Context = ImGui::GetCurrentContext();

	// double click on a node
	auto rootNode3DPtr = m_RootNodePtr->m_GraphRoot3DNode.lock();
	if (rootNode3DPtr)
	{
		res |= rootNode3DPtr->Execute(vCurrentFrame, vCmd, &m_RootNodePtr->m_BaseNodeState);
	}

	auto rootNode2DPtr = m_RootNodePtr->m_GraphRoot2DNode.lock();
	if (rootNode2DPtr)
	{
		res |= rootNode2DPtr->Execute(vCurrentFrame, vCmd, &m_RootNodePtr->m_BaseNodeState);
	}

	return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// IMGUI /////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool NodeManager::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	ZoneScoped;

	bool change = false;

	CommonSystem::Instance()->DrawImGui();

	for (auto eff : m_RootNodePtr->m_ChildNodes)
	{
		auto nodePtr = eff.second;
		if (nodePtr)
		{
			if (ImGui::CollapsingHeader(nodePtr->name.c_str()))
			{
				change |= nodePtr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
			}
		}
	}

	return change;
}

bool NodeManager::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	bool change = false;

	ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);
	ImGuizmo::SetRect(vRect.Min.x, vRect.Min.y, vRect.GetWidth(), vRect.GetHeight());

	for (auto eff : m_RootNodePtr->m_ChildNodes)
	{
		auto nodePtr = eff.second;
		if (nodePtr)
		{
			change |= nodePtr->DrawOverlays(vCurrentFrame, vRect, vContextPtr, vUserDatas);
		}
	}

	return change;
}

bool NodeManager::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	bool change = false;

	for (auto eff : m_RootNodePtr->m_ChildNodes)
	{
		auto nodePtr = eff.second;
		if (nodePtr)
		{
			change |= nodePtr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
		}
	}

	return change;
}

void NodeManager::FinalizeGraphLoading()
{
	m_RootNodePtr->FinalizeGraphLoading();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// LOAD //////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool NodeManager::LoadNodeFromXML(
	BaseNodeWeak vBaseNodeWeak, 
	tinyxml2::XMLElement* vElem,
	tinyxml2::XMLElement* vParent,
	const std::string& vNodeName,
	const std::string& vNodeType,
	const ct::fvec2& vPos,
	const size_t& vNodeId)
{
	ZoneScoped;

	bool continueXMLParsing = true;

	if (m_RootNodePtr)
	{
		BaseNodePtr nodePtr = NodeFactory::CreateNode(vBaseNodeWeak, vNodeType);

		if (!nodePtr) // maybe a plugin
		{
			nodePtr = PluginManager::Instance()->CreatePluginNode(vNodeType);
		}

		if (nodePtr)
		{
            nodePtr->m_RootNode = m_RootNodePtr;

			if (!vNodeName.empty())
				nodePtr->name = vNodeName;
			nodePtr->pos = ImVec2(vPos.x, vPos.y);
			nodePtr->nodeID = vNodeId;
			
			/*if (vNodeType == "OUTPUT_3D")
			{
				m_RootNodePtr->m_Output3DNode = nodePtr;
			}
			
			if (vNodeType == "OUTPUT_2D")
			{
				m_RootNodePtr->m_Output2DNode = nodePtr;
			}*/

			m_RootNodePtr->AddChildNode(nodePtr);

			nodePtr->BeforeNodeXmlLoading();

			nodePtr->RecursParsingConfigChilds(vElem);
			nd::SetNodePosition(vNodeId, nodePtr->pos);

			// pour eviter que des slots aient le meme id qu'un nodePtr
			BaseNode::freeNodeId = ct::maxi<uint32_t>(BaseNode::freeNodeId, (uint32_t)vNodeId);

			nodePtr->AfterNodeXmlLoading();

			continueXMLParsing = true;
		}
	}

	return continueXMLParsing;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// Update Shaders //////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NodeManager::UpdateShaders(const std::set<std::string>& vFiles) const
{
	for (auto node : m_RootNodePtr->m_ChildNodes)
	{
		if (node.second)
		{
			auto otherShaderUpdateNodePtr = dynamic_pointer_cast<ShaderUpdateInterface>(node.second);
			if (otherShaderUpdateNodePtr)
			{
				otherShaderUpdateNodePtr->UpdateShaders(vFiles);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// LOAD / SAVE GRAPH ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string NodeManager::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	if (m_RootNodePtr)
	{
		return m_RootNodePtr->getXml(vOffset, vUserDatas);
	}

	return "";
}

bool NodeManager::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_RootNodePtr)
	{
		return m_RootNodePtr->setFromXml(vElem, vParent, vUserDatas);
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SELECT NODES ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void NodeManager::SelectNode(const BaseNodeWeak& vNode) {
    TuningPane::Instance()->Select(vNode);
    DebugPane::Instance()->Select(vNode);
}

void NodeManager::SelectNodeForGraphOutput(const NodeSlotWeak& vSlot, const ImGuiMouseButton& vButton) {
    if (NodeManager::Instance()->m_RootNodePtr) {
        if (vButton == ImGuiMouseButton_Left) {
            View3DPane::Instance()->SetOrUpdateOutput(vSlot);
        } else if (vButton == ImGuiMouseButton_Middle) {
            View2DPane::Instance()->SetOrUpdateOutput(vSlot);
        } else if (vButton == ImGuiMouseButton_Right) {
        }
    }
}
