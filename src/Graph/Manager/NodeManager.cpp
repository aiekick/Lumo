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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "NodeManager.h"

#include <Gui/MainFrame.h>
#include <ctools/FileHelper.h>
#include <ImGuizmo/ImGuizmo.h>

#include <Systems/CommonSystem.h>
#include <vkFramework/VulkanCore.h>
#include <Interfaces/CameraInterface.h>
#include <Interfaces/ShaderUpdateInterface.h>
#include <Systems/GizmoSystem.h>

#include <Plugins/PluginManager.h>

#include <Graph/Factory/NodeFactory.h>

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

using namespace vkApi;

NodeManager::NodeManager()
{
	using namespace std::placeholders;
	BaseNode::sLoadNodeFromXMLCallback = std::bind(&NodeManager::LoadNodeFromXML, this, _1, _2, _3, _4, _5, _6, _7);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool NodeManager::Init(vkApi::VulkanCore* vVulkanCore)
{
	ZoneScoped;

	m_RootNodePtr = std::make_shared<BaseNode>();
	m_RootNodePtr->m_This = m_RootNodePtr;
	if (!m_RootNodePtr->Init(vVulkanCore))
	{
		m_RootNodePtr.reset();
		return false;
	}

	return true;
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

bool NodeManager::Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer *vCmd)
{
	auto outputPtr = m_RootNodePtr->m_OutputNode.getValidShared();
	if (outputPtr)
	{
		return outputPtr->Execute(vCurrentFrame, vCmd);
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// IMGUI /////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool NodeManager::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	bool change = false;

	for (auto eff : m_RootNodePtr->m_ChildNodes)
	{
		auto nodePtr = eff.second;
		if (nodePtr)
		{
			if (ImGui::CollapsingHeader(nodePtr->name.c_str()))
			{
				change |= nodePtr->DrawWidgets(vCurrentFrame, vContext);
			}
		}
	}

	return change;
}

void NodeManager::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);

	ImGuizmo::SetRect(vRect.x, vRect.y, vRect.w, vRect.h);

	for (auto eff : m_RootNodePtr->m_ChildNodes)
	{
		auto nodePtr = eff.second;
		if (nodePtr)
		{
			nodePtr->DrawOverlays(vCurrentFrame, vRect, vContext);
		}
	}
}

void NodeManager::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	for (auto eff : m_RootNodePtr->m_ChildNodes)
	{
		auto nodePtr = eff.second;
		if (nodePtr)
		{
			nodePtr->DisplayDialogsAndPopups(vCurrentFrame, vMaxSize, vContext);
		}
	}
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
			if (!vNodeName.empty())
				nodePtr->name = vNodeName;
			nodePtr->pos = ImVec2(vPos.x, vPos.y);
			nodePtr->nodeID = vNodeId;
			if (vNodeType == "OUTPUT")
			{
				m_RootNodePtr->m_OutputNode = nodePtr;
			}
			m_RootNodePtr->AddChildNode(nodePtr);
			nodePtr->RecursParsingConfigChilds(vElem);
			nd::SetNodePosition(vNodeId, nodePtr->pos);

			// pour eviter que des slots aient le meme id qu'un nodePtr
			BaseNode::freeNodeId = ct::maxi<uint32_t>(BaseNode::freeNodeId, (uint32_t)vNodeId);

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
	assert(m_RootNodePtr);

	return m_RootNodePtr->getXml(vOffset, vUserDatas);
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

	assert(m_RootNodePtr);

	return m_RootNodePtr->setFromXml(vElem, vParent, vUserDatas);
}