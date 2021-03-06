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

#include "BaseNode.h"
#include <imgui/imgui.h>
#include <ImWidgets/ImWidgets.h>
#include <imgui/imgui_internal.h>
#include <vkFramework/VulkanCore.h>
#include <imgui_node_editor/NodeEditor/Source/imgui_node_editor_internal.h>

#include <utility>

static bool showNodeStyleEditor = false;
uint32_t BaseNode::freeNodeId = MAGIC_NUMBER;

//////////////////////////////////////////////////////////////////////////////
////// STATIC ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

uint32_t BaseNode::GetNextNodeId()
{
	return ++BaseNode::freeNodeId;
}

BaseNodePtr BaseNode::GetSharedFromWeak(const BaseNodeWeak& vNode)
{
	BaseNodePtr res = nullptr;

	if (!vNode.expired())
	{
		res = vNode.lock();
	}

	return res;
}

std::function<void(BaseNodeWeak)> BaseNode::sOpenGraphCallback;
void BaseNode::OpenGraph_Callback(const BaseNodeWeak& vNode)
{
	if (BaseNode::sOpenGraphCallback)
	{
		BaseNode::sOpenGraphCallback(vNode);
	}
}

std::function<void(std::string)> BaseNode::sOpenCodeCallback;
void BaseNode::OpenCode_Callback(std::string vCode)
{
	if (BaseNode::sOpenCodeCallback)
	{
		BaseNode::sOpenCodeCallback(std::move(vCode));
	}
}

std::function<void(std::string)> BaseNode::sLogErrorsCallback;
void BaseNode::LogErrors_Callback(std::string vErrors)
{
	if (BaseNode::sLogErrorsCallback)
	{
		BaseNode::sLogErrorsCallback(std::move(vErrors));
	}
}

std::function<void(std::string)> BaseNode::sLogInfosCallback;
void BaseNode::LogInfos_Callback(std::string vInfos)
{
	if (BaseNode::sLogInfosCallback)
	{
		BaseNode::sLogInfosCallback(std::move(vInfos));
	}
}

std::function<void(BaseNodeWeak)> BaseNode::sSelectCallback; // select node
void BaseNode::Select_Callback(const BaseNodeWeak& vNode)
{
	if (BaseNode::sSelectCallback)
	{
		BaseNode::sSelectCallback(vNode);
	}
}

std::function<BaseNodeWeak(BaseNodeWeak vNodeGraph, BaseNodeState* vBaseNodeState)> BaseNode::sShowNewNodeMenuCallback; // new node menu
void BaseNode::ShowNewNodeMenu_Callback(const BaseNodeWeak& vNodeGraph, BaseNodeState* vBaseNodeState)
{
	if (BaseNode::sShowNewNodeMenuCallback)
	{
		BaseNode::sShowNewNodeMenuCallback(vNodeGraph, vBaseNodeState);
	}
}

std::function<bool(BaseNodeWeak, tinyxml2::XMLElement*, tinyxml2::XMLElement*,
	const std::string&, const std::string&, const ct::fvec2&, const size_t&)> BaseNode::sLoadNodeFromXMLCallback; // log infos
bool BaseNode::LoadNodeFromXML_Callback(const BaseNodeWeak& vBaseNodeWeak, tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent,
	const std::string& vNodeName, const std::string& vNodeType, const ct::fvec2& vPos, const size_t& vNodeId)
{
	if (BaseNode::sLoadNodeFromXMLCallback)
	{
		return BaseNode::sLoadNodeFromXMLCallback(vBaseNodeWeak, vElem,  vParent,
			 vNodeName,  vNodeType,  vPos,  vNodeId);
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////
////// CONSTRUCTOR ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BaseNode::BaseNode()
{
	m_NodeTypeString = "NONE";
	nodeID = GetNextNodeId();
}

BaseNode::~BaseNode()
{
	Unit();
}

//////////////////////////////////////////////////////////////////////////////
////// INIT //////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

bool BaseNode::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	m_VulkanCorePtr = vVulkanCorePtr;

	InitGraph(m_Style);

	return true;
}

bool BaseNode::Init(const BaseNodeWeak& vThis)
{
	m_This = vThis;

	return true;
}

bool BaseNode::Init(const std::string& vCode, const BaseNodeWeak& vThis)
{
	UNUSED(vCode);

	m_This = vThis;

	InitGraph(m_Style);

	return true;
}

void BaseNode::InitGraph(const ax::NodeEditor::Style& vStyle)
{
	if (!m_BaseNodeState.m_NodeGraphContext)
	{
		ax::NodeEditor::Config config;
		if (!m_ParentNode.expired() && !name.empty())
			m_NodeGraphConfigFile = "json\\" + name + ".json";
		else
			m_NodeGraphConfigFile = "json\\root.json";

		config.SettingsFile = m_NodeGraphConfigFile.c_str();
		m_BaseNodeState.m_NodeGraphContext = nd::CreateEditor(&config);

		if (m_BaseNodeState.m_NodeGraphContext)
		{
			nd::SetCurrentEditor(m_BaseNodeState.m_NodeGraphContext);
			m_Style = vStyle;
			nd::SetStyle(m_Style);
			nd::EnableShortcuts(true);
		}
	}
} 

//////////////////////////////////////////////////////////////////////////////
////// DESTROY NODE / GRAPH //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void BaseNode::Unit()
{
	ClearNode();
	UnitGraph();
}

void BaseNode::UnitGraph()
{
	if (m_BaseNodeState.m_NodeGraphContext)
	{
		ClearGraph();
		nd::DestroyEditor(m_BaseNodeState.m_NodeGraphContext);
		m_BaseNodeState.m_NodeGraphContext = nullptr;
	}

	m_LinksToBuildAfterLoading.clear();
}

// on va cree les links apres le chargement du xml
void BaseNode::FinalizeGraphLoading()
{
	for (const auto& entry : m_LinksToBuildAfterLoading)
	{
		const SlotEntry& entIn = entry.first;
		const SlotEntry& entOut = entry.second;

		auto inSlot = FindNodeSlotById(entIn.first, entIn.second);
		auto outSlot = FindNodeSlotById(entOut.first, entOut.second);

		ConnectSlots(inSlot, outSlot);
	}

	for (const auto& child : m_ChildNodes)
	{
		auto nodePtr = child.second;
		if (nodePtr)
		{
			nodePtr->Notify(NotifyEvent::GraphIsLoaded);
		}
	}
}

void BaseNode::DoGraphActions(BaseNodeState *vBaseNodeState)
{
	auto editor = nd::GetCurrentEditor();
	if (editor)
	{
		if (nd::IsBackgroundClicked())
		{
			vBaseNodeState->node_to_select.reset();
			vBaseNodeState->current_selected_node.reset();
		}
	}

	OpenNodeInNewPane(vBaseNodeState);
	SelectNodeforPreview(vBaseNodeState);
}

void BaseNode::OpenNodeInNewPane(BaseNodeState* vBaseNodeState)
{
	// open node in new pane
	if (!vBaseNodeState->node_to_open.expired())
	{
		auto nodePtr = vBaseNodeState->node_to_open.lock();
		if (nodePtr)
		{
			// dans notre cas, si le graph est vide c'est pas grave, car on doit pouvoir y ajouter du code
			//if (!vBaseNodeState->node_to_open->m_ChildNodes.empty())
			if (!nodePtr->graphDisabled)
			{
				OpenGraph_Callback(vBaseNodeState->node_to_open);
			}

			vBaseNodeState->node_to_open.reset();
		}
	}
}

void BaseNode::SelectNodeforPreview(BaseNodeState* vBaseNodeState)
{
	// select node for preview
	if (!vBaseNodeState->node_to_select.expired())
	{
		auto current_selected_nodePtr = vBaseNodeState->current_selected_node.lock();
		auto node_to_selectPtr = vBaseNodeState->node_to_select.lock();

		//if (current_selected_nodePtr != node_to_selectPtr)
		{
			vBaseNodeState->current_selected_node = vBaseNodeState->node_to_select;

			if (!vBaseNodeState->current_selected_node.expired())
			{
				auto nodePtr = vBaseNodeState->current_selected_node.lock();
				if (nodePtr)
				{
					Select_Callback(nodePtr);
				}
			}

			vBaseNodeState->node_to_select.reset();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
////// CLEAR /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void BaseNode::ClearNode()
{
	
}

void BaseNode::ClearGraph()
{
	m_ChildNodes.clear();
	m_Links.clear();
	m_NodeStamps.clear();
	m_LinksToBuildAfterLoading.clear();
}

void BaseNode::ClearSlots()
{
	m_Inputs.clear();
	m_Outputs.clear();
}

void BaseNode::UpdateSlots()
{

}

void BaseNode::ClearDescriptors()
{

}

//////////////////////////////////////////////////////////////////////////////
////// CLEAR /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

bool BaseNode::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer *vCmd, BaseNodeState* vBaseNodeState)
{
	if (vBaseNodeState)
		vBaseNodeState->m_CurrentFrame = vCurrentFrame;
	
	return BaseNode::ExecuteChilds(vCurrentFrame, vCmd, vBaseNodeState);
}

bool BaseNode::ExecuteChilds(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	bool res = false;

	if (vBaseNodeState)
		vBaseNodeState->m_CurrentFrame = vCurrentFrame;
	
	for (auto input : m_Inputs)
	{
		if (input.second)
		{
			for (auto slot : input.second->linkedSlots)
			{
				auto otherSLotPtr = slot.getValidShared();
				if (otherSLotPtr)
				{
					auto otherParentPtr = otherSLotPtr->parentNode.getValidShared();
					if (otherParentPtr)
					{
						res &= otherParentPtr->Execute(vCurrentFrame, vCmd, vBaseNodeState);
					}
				}
			}
		}
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////////
////// GAPH NAVIGATION ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void BaseNode::ZoomToContent() const
{
	if (m_BaseNodeState.m_NodeGraphContext)
	{
		auto context = reinterpret_cast<ax::NodeEditor::Detail::EditorContext*>(m_BaseNodeState.m_NodeGraphContext);
		context->NavigateTo(context->GetContentBounds(), true);
	}
}

void BaseNode::NavigateToContent() const
{
	if (m_BaseNodeState.m_NodeGraphContext)
	{
		auto context = reinterpret_cast<ax::NodeEditor::Detail::EditorContext*>(m_BaseNodeState.m_NodeGraphContext);
		context->NavigateTo(context->GetContentBounds(), false);
	}
}

void BaseNode::ZoomToSelection() const
{
	if (m_BaseNodeState.m_NodeGraphContext)
	{
		auto context = reinterpret_cast<ax::NodeEditor::Detail::EditorContext*>(m_BaseNodeState.m_NodeGraphContext);
		context->NavigateTo(context->GetSelectionBounds(), true);
	}
}

void BaseNode::NavigateToSelection() const
{
	if (m_BaseNodeState.m_NodeGraphContext)
	{
		auto context = reinterpret_cast<ax::NodeEditor::Detail::EditorContext*>(m_BaseNodeState.m_NodeGraphContext);
		context->NavigateTo(context->GetSelectionBounds(), false);
	}
}

//////////////////////////////////////////////////////////////////////////////
////// CANVAS QUERY //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

ImVec2 BaseNode::GetCanvasOffset() const
{
	if (m_BaseNodeState.m_NodeGraphContext)
	{
		auto context = reinterpret_cast<ax::NodeEditor::Detail::EditorContext*>(m_BaseNodeState.m_NodeGraphContext);
		return context->GetView().Origin;
	}

	return ImVec2(0, 0);
}

float BaseNode::GetCanvasScale() const
{
	if (m_BaseNodeState.m_NodeGraphContext)
	{
		auto context = reinterpret_cast<ax::NodeEditor::Detail::EditorContext*>(m_BaseNodeState.m_NodeGraphContext);
		return context->GetView().Scale;
	}

	return 1.0f;
}

void BaseNode::SetCanvasOffset(const ImVec2& vOffset)
{
	if (m_BaseNodeState.m_NodeGraphContext)
	{
		auto context = reinterpret_cast<ax::NodeEditor::Detail::EditorContext*>(m_BaseNodeState.m_NodeGraphContext);
		context->NavigateTo(vOffset);
	}
}

void BaseNode::SetCanvasScale(const float& vScale)
{
	if (m_BaseNodeState.m_NodeGraphContext)
	{
		auto context = reinterpret_cast<ax::NodeEditor::Detail::EditorContext*>(m_BaseNodeState.m_NodeGraphContext);
		context->NavigateTo(vScale);
	}
}

//////////////////////////////////////////////////////////////////////////////
////// COPY / PASTE / DUPLICATE //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void BaseNode::CopySelectedNodes()
{
	auto countSelectecdNodes = nd::GetSelectedObjectCount();
	m_NodesToCopy.resize(countSelectecdNodes);
	nd::GetActionContextNodes(m_NodesToCopy.data(), (int)m_NodesToCopy.size());

	// calcul du point de centre de tout ces nodes
	// sa servira d'offset avec le point de destinatiion
	m_BaseCopyOffset = ImVec2(0,0);
	for (auto id : m_NodesToCopy)
	{
		m_BaseCopyOffset += nd::GetNodePosition(id) * 0.5f;
	}
}

void BaseNode::PasteNodesAtMousePos()
{
	nd::Suspend(); // necessaire pour avoir le bon MousePos
	auto newOffset = nd::ScreenToCanvas(ImGui::GetMousePos()) - m_BaseCopyOffset;
	nd::Resume();
	DuplicateSelectedNodes(newOffset);
}

void BaseNode::DuplicateSelectedNodes(ImVec2 vOffset)
{
	for (auto &it : m_NodesToCopy)
	{
		DuplicateNode((uint32_t)it.Get(), vOffset);
	}
	m_NodesToCopy.clear();
}

//////////////////////////////////////////////////////////////////////////////
////// NODE DRAWING //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void BaseNode::DrawNode(BaseNodeState *vBaseNodeState)
{
	if (vBaseNodeState)
	{
		ImGui::SetCurrentContext(vBaseNodeState->m_Context);
		nd::SetCurrentEditor(vBaseNodeState->m_NodeGraphContext);

		if (DrawBegin(vBaseNodeState))
		{
			DrawHeader(vBaseNodeState);
			DrawNodeContent(vBaseNodeState);
			DrawFooter(vBaseNodeState);
			DrawEnd(vBaseNodeState);
		}
		size = ImGui::GetItemRectSize();
		pos = ImGui::GetItemRectMin();

		if (ImGui::IsItemHovered())
		{
			if (ImGui::IsMouseClicked(0)) // bouton gauche click
			{
				assert(!m_This.expired());
				vBaseNodeState->node_to_select = m_This;
			}
		}
	}
}

void BaseNode::FillState(BaseNodeState *vBaseNodeState)
{
	vBaseNodeState->hoveredItem = ImGui::IsItemHovered();
	vBaseNodeState->activeItem = ImGui::IsItemActive();
	vBaseNodeState->is_any_hovered_items |= vBaseNodeState->hoveredItem;
	vBaseNodeState->is_any_active_items |= vBaseNodeState->activeItem;

	if (vBaseNodeState->hoveredItem)
	{
		assert(!m_This.expired());
		vBaseNodeState->current_hovered_node = m_This;
	}
	else
	{
		vBaseNodeState->current_hovered_node.reset();
		if (ImGui::IsMouseClicked(0)) // bouton gauche click
		{
			vBaseNodeState->node_to_select.reset();
			vBaseNodeState->current_selected_node.reset();
		}
	}

	if (vBaseNodeState->hoveredItem &&
		vBaseNodeState->activeItem)
	{
		assert(!m_This.expired());
		vBaseNodeState->current_selected_node = m_This;
	}
	else
	{

	}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

bool BaseNode::DrawNodeContent(BaseNodeState *vBaseNodeState)
{
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0));

	ImGui::BeginHorizontal("content");
	ImGui::Spring(0, 0);

	ImGui::BeginVertical("inputs", ImVec2(0, 0), 0.0f);
	for (auto & param : m_Inputs) // input flows
	{
		param.second->DrawContent(vBaseNodeState);
	}
	ImGui::EndVertical();

	ImGui::Spring(1, 5.0f); // pour que BeginVertical soi pouss?? au bout

	ImGui::BeginVertical("outputs", ImVec2(0, 0), 1.0f); // 1.0f pour que l'interieur soit align?? sur la fin
	for (auto & param : m_Outputs) // output flows
	{
		param.second->DrawContent(vBaseNodeState);
	}
	ImGui::EndVertical();

	ImGui::EndHorizontal();

	ImGui::PopStyleVar();

	return true;
}

void BaseNode::DrawInputWidget(BaseNodeState *vBaseNodeState, NodeSlotWeak vSlot)
{
	UNUSED(vBaseNodeState);
	UNUSED(vSlot);
}

void BaseNode::DrawOutputWidget(BaseNodeState *vBaseNodeState, NodeSlotWeak vSlot)
{
	UNUSED(vBaseNodeState);
	UNUSED(vSlot);
}

void BaseNode::DrawContextMenuForSlot(BaseNodeState *vBaseNodeState, NodeSlotWeak vSlot)
{
	UNUSED(vBaseNodeState);
	UNUSED(vSlot);
}

void BaseNode::DrawContextMenuForNode(BaseNodeState *vBaseNodeState)
{
	UNUSED(vBaseNodeState);
}

void BaseNode::DrawCustomContextMenuForNode(BaseNodeState *vBaseNodeState)
{
	UNUSED(vBaseNodeState);
}

std::string BaseNode::GetNodeCode(bool vRecursChilds)
{
	UNUSED(vRecursChilds);
	return "";
}

bool BaseNode::IsCodeDirty()
{
	return m_IsCodeDirty;
}

void BaseNode::SetCodeDirty(bool vFlag)
{
	m_IsCodeDirty = vFlag;
}

void BaseNode::JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
#ifdef _DEBUG
	//LogVarInfo("BaseNode::JustConnectedBySlots catched, some class not implment it. maybe its wanted");
#endif
}

void BaseNode::JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot)
{
#ifdef _DEBUG
	//LogVarInfo("BaseNode::JustDisConnectedBySlots catched, some class not implment it. maybe its wanted");
#endif
}

void BaseNode::CompilGeneratedCode()
{
	/*std::string code = GetNodeCode();
	if (!code.empty())
	{
		
	}*/
}

bool BaseNode::DrawBegin(BaseNodeState *vBaseNodeState)
{
	UNUSED(vBaseNodeState);

	nd::BeginNode(nodeID);
	ImGui::PushID(nodeID.AsPointer());
	ImGui::BeginVertical("node");

	return true;
}

bool BaseNode::DrawHeader(BaseNodeState *vBaseNodeState)
{
	UNUSED(vBaseNodeState);

	ImGui::BeginHorizontal("header");
	ImGui::Spring(1, 5.0f);
	ImGui::TextUnformatted(name.c_str());
	ImGui::Spring(1, 5.0f);
	ImGui::Dummy(ImVec2(0, 24));
	ImGui::EndHorizontal();

    m_HeaderRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

	return true;
}

bool BaseNode::DrawFooter(BaseNodeState *vBaseNodeState)
{
	UNUSED(vBaseNodeState);

	return true;
}

bool BaseNode::DrawEnd(BaseNodeState *vBaseNodeState)
{
	ImGui::EndVertical();

	nd::EndNode();

	if (ImGui::IsItemVisible())
	{
		auto drawList = nd::GetNodeBackgroundDrawList(nodeID);
		if (drawList)
		{
			if (m_HeaderRect.GetSize().y > 0.0f)
			{
				const auto halfBorderWidth = nd::GetStyle().NodeBorderWidth * 0.5f;
				/*drawList->AddRectFilled(
					m_HeaderRect.Min - ImVec2(nd::GetStyle().NodePadding.x - halfBorderWidth, 4 - halfBorderWidth),
					m_HeaderRect.Max + ImVec2(nd::GetStyle().NodePadding.z - halfBorderWidth, 0),
					ImGui::GetColorU32(m_HeaderColor), nd::GetStyle().NodeRounding, 1 | 2);*/
				auto alpha = static_cast<int>(255 * ImGui::GetStyle().Alpha);
				drawList->AddLine(
					ImVec2(m_HeaderRect.Min.x - (nd::GetStyle().NodePadding.x - halfBorderWidth), m_HeaderRect.Max.y - 0.5f),
					ImVec2(m_HeaderRect.Max.x + (nd::GetStyle().NodePadding.z - halfBorderWidth), m_HeaderRect.Max.y - 0.5f),
					ImColor(255, 255, 255, 96 * alpha / (3 * 255)), 1.0f);
			}

			DisplayInfosOnTopOfTheNode(vBaseNodeState);
		}
		else
		{
			LogVarDebug("why drawList is null ?? in BaseNode::DrawEnd");
		}
	}

	ImGui::PopID();

	return true;
}

void BaseNode::DrawLinks(BaseNodeState *vBaseNodeState)
{
	UNUSED(vBaseNodeState);
	if (!m_BaseNodeState.showLinks) return;

	if (!m_ChildNodes.empty())
	{
		for (const auto &link : m_Links)
		{
			if (!link.second->in.expired() && 
				!link.second->out.expired())
			{
				auto inPtr = link.second->in.lock();
				auto outPtr = link.second->out.lock();
				if (inPtr && outPtr)
				{
					nd::Link(
						link.first, inPtr->pinID, outPtr->pinID,
						inPtr->color, link.second->thick);
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

bool BaseNode::DrawDebugInfos(BaseNodeState *vBaseNodeState)
{
	UNUSED(vBaseNodeState);
	ImGui::Separator();


	ImGui::Text("Name : %s", name.c_str());
	ImGui::Text("Type : %s", m_NodeTypeString.c_str());
	
	ImGui::Separator();

	if (!m_Inputs.empty())
	{
		if (ImGui::TreeNode("Inputs :"))
		{
			ImGui::Indent();
			for (auto & uni : m_Inputs) // input calls
			{
				ImGui::Text("Name : %s", uni.second->name.c_str());
				ImGui::Text("Type : %s", uni.second->slotType.c_str());
				ImGui::Text("Count connections : %u", (uint32_t)uni.second->linkedSlots.size());
				ImGui::Indent();
				for (auto lslot : uni.second->linkedSlots)
				{
					auto lslotPtr = lslot.getValidShared();
					if (lslotPtr)
					{
						auto pNodePtr = lslotPtr->parentNode.getValidShared();
						if (pNodePtr)
						{
							ImGui::Text("Node %s", pNodePtr->name.c_str());
						}
					}
				}
				ImGui::Unindent();
				ImGui::Text("--------------------");
			}
			ImGui::Unindent();

			ImGui::TreePop();
		}
	}

	if (!m_Outputs.empty())
	{
		if (ImGui::TreeNode("Outputs :"))
		{
			ImGui::Indent();
			for (auto & uni : m_Outputs) // input calls
			{
				ImGui::Text("Name : %s", uni.second->name.c_str());
				ImGui::Text("Type : %s", uni.second->slotType.c_str());
				ImGui::Text("Count connections : %u", (uint32_t)uni.second->linkedSlots.size());
				ImGui::Indent();
				for (auto lslot : uni.second->linkedSlots)
				{
					auto lslotPtr = lslot.getValidShared();
					if (lslotPtr)
					{
						auto pNodePtr = lslotPtr->parentNode.getValidShared();
						if (pNodePtr)
						{
							ImGui::Text("Node %s", pNodePtr->name.c_str());
						}
					}
				}
				ImGui::Unindent();
				ImGui::Text("--------------------");
				ImGui::Separator();
			}
			ImGui::Unindent();

			ImGui::TreePop();
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void BaseNode::DisplayInfosOnTopOfTheNode(BaseNodeState *vBaseNodeState)
{
	if (vBaseNodeState && vBaseNodeState->debug_mode)
	{
		auto drawList = nd::GetNodeBackgroundDrawList(nodeID);
		if (drawList)
		{
			char debugBuffer[255] = "\0";
			snprintf(debugBuffer, 254, 
				"Used(%s)\nCell(%i, %i)"/*\nPos(%.1f, %.1f)\nSize(%.1f, %.1f)*/, 
				(used?"true":"false"), cell.x, cell.y/*, pos.x, pos.y, size.x, size.y*/);
			ImVec2 txtSize = ImGui::CalcTextSize(debugBuffer);
			drawList->AddText(pos - ImVec2(0, txtSize.y), ImGui::GetColorU32(ImGuiCol_Text), debugBuffer);
		}
	}
}

void BaseNode::DrawProperties(BaseNodeState* vBaseNodeState)
{
	UNUSED(vBaseNodeState);
	ImGui::Text("Properties of %s", name.c_str());
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

NodeSlotWeak BaseNode::AddInput(NodeSlot vFlow, bool vIncSlotId, bool vHideName)
{
	assert(!m_This.expired());
	vFlow.parentNode = m_This;
	vFlow.slotPlace = NodeSlotPlaceEnum::INPUT;
	vFlow.hideName = vHideName;
	vFlow.type = uType::uTypeEnum::U_FLOW;
	if (vIncSlotId)
	{
		vFlow.pinID = NodeSlot::GetNewSlotId();
	}
	vFlow.index = (uint32_t)m_Inputs.size();
	m_Inputs[(int)vFlow.pinID.Get()] = NodeSlot::Create(vFlow);
	return m_Inputs[(int)vFlow.pinID.Get()];
}

NodeSlotWeak BaseNode::AddOutput(NodeSlot vFlow, bool vIncSlotId, bool vHideName)
{
	assert(!m_This.expired());
	vFlow.parentNode = m_This;
	vFlow.slotPlace = NodeSlotPlaceEnum::OUTPUT;
	vFlow.hideName = vHideName;
	vFlow.type = uType::uTypeEnum::U_FLOW;
	if (vIncSlotId)
	{
		vFlow.pinID = NodeSlot::GetNewSlotId();
	}
	vFlow.index = (uint32_t)m_Inputs.size();
	m_Outputs[(int)vFlow.pinID.Get()] = NodeSlot::Create(vFlow);
	return m_Outputs[(int)vFlow.pinID.Get()];
}

//////////////////////////////////////////////////////////////////////////////
////// NODE STYLE ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void BaseNode::DrawStyleMenu()
{
	static GraphStyleStruct graphStyleDefault = GraphStyleStruct();

	if (ImGui::BeginMenu("Node"))
	{
		ImGui::SliderFloatDefault(200, "Width", &m_BaseNodeState.graphStyle.DEFAULT_WIDTH, 1.0f,
			graphStyleDefault.DEFAULT_WIDTH*3.0f, graphStyleDefault.DEFAULT_WIDTH);
		ImGui::SliderFloatDefault(200, "Padding", &m_BaseNodeState.graphStyle.WINDOW_PADDING, 1.0f,
			graphStyleDefault.WINDOW_PADDING*3.0f, graphStyleDefault.DEFAULT_WIDTH);
		ImGui::SliderFloatDefault(200, "Radius", &m_BaseNodeState.graphStyle.BACKGROUND_RADIUS, 1.0f,
			20.0, graphStyleDefault.BACKGROUND_RADIUS);
		ImGui::ColorEdit4Default(200, "Header", &m_BaseNodeState.graphStyle.HEADER_COLOR.x, 
			&graphStyleDefault.HEADER_COLOR.x);
		ImGui::ColorEdit4Default(200, "Header Hovered", &m_BaseNodeState.graphStyle.HOVERED_HEADER_COLOR.x, 
			&graphStyleDefault.HOVERED_HEADER_COLOR.x);
		ImGui::ColorEdit4Default(200, "Background", &m_BaseNodeState.graphStyle.BACKGROUND_COLOR.x, 
			&graphStyleDefault.BACKGROUND_COLOR.x);
		ImGui::ColorEdit4Default(200, "Background Hovered", &m_BaseNodeState.graphStyle.HOVERED_BACKGROUND_COLOR.x, 
			&graphStyleDefault.HOVERED_BACKGROUND_COLOR.x);
		
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Slot"))
	{
		ImGui::SliderFloatDefault(200, "Radius", &m_BaseNodeState.graphStyle.SLOT_RADIUS, 1.0f,
			graphStyleDefault.SLOT_RADIUS*3.0f, graphStyleDefault.SLOT_RADIUS);
		ImGui::ColorEdit4Default(200, "Color", &m_BaseNodeState.graphStyle.SLOT_COLOR.x,
			&graphStyleDefault.SLOT_COLOR.x);
		ImGui::ColorEdit4Default(200, "Flow Color", &m_BaseNodeState.graphStyle.FLOW_SLOT_COLOR.x,
			&graphStyleDefault.FLOW_SLOT_COLOR.x);
		ImGui::ColorEdit4Default(200, "Function Color", &m_BaseNodeState.graphStyle.FLOW_SLOT_COLOR.x,
			&graphStyleDefault.FLOW_SLOT_COLOR.x);

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Grid"))
	{
		ImGui::ColorEdit4Default(200, "Grid Lines", &m_BaseNodeState.graphStyle.GRID_COLOR.x, 
			&graphStyleDefault.GRID_COLOR.x);
		ImGui::ColorEdit4Default(200, "Grid Zero x", &m_BaseNodeState.graphStyle.GRID_COLOR_ZERO_X.x, 
			&graphStyleDefault.GRID_COLOR_ZERO_X.x);
		ImGui::ColorEdit4Default(200, "Grid Zero y", &m_BaseNodeState.graphStyle.GRID_COLOR_ZERO_Y.x, 
			&graphStyleDefault.GRID_COLOR_ZERO_Y.x);
		ImGui::SliderFloatDefault(200, "Grid Spacing", &m_BaseNodeState.graphStyle.GRID_SPACING, 1.0f,
			256, graphStyleDefault.GRID_SPACING);

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Link"))
	{
		ImGui::ColorEdit4Default(200, "link", &m_BaseNodeState.graphStyle.linkDefaultColor.x,
			&graphStyleDefault.linkDefaultColor.x);
		ImGui::ColorEdit4Default(200, "selected Link", &m_BaseNodeState.graphStyle.selectedLinkColor.x,
			&graphStyleDefault.selectedLinkColor.x);
		ImGui::ColorEdit4Default(200, "extract Link", &m_BaseNodeState.graphStyle.extractionLinkColor.x,
			&graphStyleDefault.extractionLinkColor.x);
		ImGui::ColorEdit4Default(200, "selected Extract Link", &m_BaseNodeState.graphStyle.selectedExtractionLinkColor.x,
			&graphStyleDefault.selectedExtractionLinkColor.x);

		ImGui::EndMenu();
	}

	ImGui::MenuItem("Editor", "", &showNodeStyleEditor);
}

void BaseNode::DrawNodeGraphStyleMenu() const
{
	if (m_BaseNodeState.m_NodeGraphContext && showNodeStyleEditor)
	{
		if (ImGui::Begin("Editor", &showNodeStyleEditor))
		{
			nd::SetCurrentEditor(m_BaseNodeState.m_NodeGraphContext);

			static nd::Style def = nd::Style();
			auto& editorStyle = nd::GetStyle();

			ImGui::SliderFloatDefault(200, "Node Padding x", &editorStyle.NodePadding.x, 0.0f, 40.0f, def.NodePadding.x);
			ImGui::SliderFloatDefault(200, "Node Padding y", &editorStyle.NodePadding.y, 0.0f, 40.0f, def.NodePadding.y);
			ImGui::SliderFloatDefault(200, "Node Padding z", &editorStyle.NodePadding.z, 0.0f, 40.0f, def.NodePadding.z);
			ImGui::SliderFloatDefault(200, "Node Padding w", &editorStyle.NodePadding.w, 0.0f, 40.0f, def.NodePadding.w);
			ImGui::SliderFloatDefault(200, "Node Rounding", &editorStyle.NodeRounding, 0.0f, 40.0f, def.NodeRounding);
			ImGui::SliderFloatDefault(200, "Node Border Width", &editorStyle.NodeBorderWidth, 0.0f, 15.0f, def.NodeBorderWidth);
			ImGui::SliderFloatDefault(200, "Hovered Node Border Width", &editorStyle.HoveredNodeBorderWidth, 0.0f, 15.0f, def.HoveredNodeBorderWidth);
			ImGui::SliderFloatDefault(200, "Selected Node Border Width", &editorStyle.SelectedNodeBorderWidth, 0.0f, 15.0f, def.SelectedNodeBorderWidth);
			ImGui::SliderFloatDefault(200, "Slot Rounding", &editorStyle.PinRounding, 0.0f, 40.0f, def.PinRounding);
			ImGui::SliderFloatDefault(200, "Slot Border Width", &editorStyle.PinBorderWidth, 0.0f, 15.0f, def.PinBorderWidth);
			ImGui::SliderFloatDefault(200, "Link Strength", &editorStyle.LinkStrength, 0.0f, 500.0f, def.LinkStrength);

			//ImVec2 SourceDirection;
			//ImVec2 TargetDirection;
			ImGui::SliderFloatDefault(200, "Scroll Duration", &editorStyle.ScrollDuration, 0.0f, 2.0f, def.ScrollDuration);
			ImGui::SliderFloatDefault(200, "Flow Marker Distance", &editorStyle.FlowMarkerDistance, 1.0f, 200.0f, def.FlowMarkerDistance);
			ImGui::SliderFloatDefault(200, "Flow Speed", &editorStyle.FlowSpeed, 1.0f, 2000.0f, def.FlowSpeed);
			ImGui::SliderFloatDefault(200, "Flow Duration", &editorStyle.FlowDuration, 0.0f, 5.0f, def.FlowDuration);

			//ImVec2 PivotAlignment;
			//ImVec2 PivotSize;
			//ImVec2 PivotScale;
			//float SlotCorners;
			//float SlotRadius;
			//float SlotArrowSize;
			//float SlotArrowWidth;
			ImGui::SliderFloatDefault(200, "Group Rounding", &editorStyle.GroupRounding, 0.0f, 40.0f, def.GroupRounding);
			ImGui::SliderFloatDefault(200, "Group Border Width", &editorStyle.GroupBorderWidth, 0.0f, 15.0f, def.GroupBorderWidth);

			//ImGui::EndMenu();
		}
		ImGui::End();
	}
}

void BaseNode::DrawToolMenu()
{
	if (ImGui::BeginMenu("Tools"))
	{
		ImGui::CheckBoxBoolDefault("Debug mode", &m_BaseNodeState.debug_mode, false);

		ImGui::EndMenu();
	}
}

bool BaseNode::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	return false;
}

void BaseNode::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& /*vRect*/, ImGuiContext* vContext)
{
	assert(vContext);

}

void BaseNode::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& /*vMaxSize*/, ImGuiContext* vContext)
{
	assert(vContext);

}

//////////////////////////////////////////////////////////////////////////////
////// NODE GRAPH ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

bool BaseNode::DrawGraph()
{
	bool change = false;

	m_BaseNodeState.m_Context = ImGui::GetCurrentContext();
	if (m_BaseNodeState.m_NodeGraphContext)
	{
		nd::SetCurrentEditor(m_BaseNodeState.m_NodeGraphContext);
		
		DrawNodeGraphStyleMenu();

		nd::Begin("GraphNode");
		
		m_BaseNodeState.itemPushId = 1;
		if (!m_ChildNodes.empty())
		{
			for (auto& node : m_ChildNodes)
			{
				auto nodePtr = node.second;
				if (nodePtr)
				{
					nodePtr->DrawNode(&m_BaseNodeState);
				}
			}

			DrawLinks(&m_BaseNodeState);

			DoCreateLinkOrNode(&m_BaseNodeState);
			DoDeleteLinkOrNode(&m_BaseNodeState);
			DoShorcutsOnNode(&m_BaseNodeState);
		}

		DoPopups(&m_BaseNodeState);

		/*nd::Suspend();
		ImVec2 smp = ImGui::GetMousePos();
		ImVec2 cmp = nd::ScreenToCanvas(smp);
		ImGui::SetTooltip("Screen Mouse Pos : %.1f %.1f\nCanvas Mouse Pos : %.1f %.1f\nCanvas offset : %.1f %.1f / %.1f %.1f", smp.x, smp.y, cmp.x, cmp.y, co1.x, co1.y, co2.x, co2.y);
		nd::Resume();*/
		
		nd::End();
		nd::SetCurrentEditor(nullptr);

		DoGraphActions(&m_BaseNodeState);
	}
	
	return change;
}

bool BaseNode::GenerateGraphFromCode(const std::string& /*vCode*/)
{
	assert(!m_This.expired());
	//GraphGenerator::Instance()->GenerateGraphFromCodeForHostNode(vCode, m_This);

	return true;
}

BaseNodeWeak BaseNode::FindNode(nd::NodeId vId)
{
	BaseNodeWeak res;

	uint32_t queryId = (uint32_t)vId.Get();
	if (m_ChildNodes.find(queryId) != m_ChildNodes.end()) // trouv??
	{
		auto nodePtr = m_ChildNodes[queryId];
		if (nodePtr)
		{
			if (nodePtr->nodeID == vId)
			{
				res = m_ChildNodes[queryId];
			}
			else
			{
				uint32_t nodeId = (uint32_t)nodePtr->nodeID.Get();
				LogVarDebug("Comment c'est possible que le m_ChildNodes avec id %u dans la map ait un autre indice %u", queryId, nodeId);
			}
		}
	}
		
	return res;
}

BaseNodeWeak BaseNode::FindNodeByName(std::string vName)
{
	BaseNodeWeak res;

	for (auto node : m_ChildNodes)
	{
		auto nodePtr = node.second;
		if (nodePtr)
		{
			if (nodePtr->name == vName)
			{
				res = node.second;
				break;
			}
		}
	}

	return res;
}

std::vector<BaseNodeWeak> BaseNode::GetPublicNodes()
{
	std::vector<BaseNodeWeak> res;

	for (auto node : m_ChildNodes)
	{
		auto nodePtr = node.second;
		if (nodePtr)
		{
			if (nodePtr->name.find("PUBLIC_") == 0)
			{
				res.push_back(node.second);
			}
		}
	}

	return res;
}

ct::cWeak<NodeLink> BaseNode::FindLink(nd::LinkId vId)
{
	UNUSED(vId);

	for (auto & link : m_Links)
	{
		if (link.second->linkId == vId.Get())
		{
			return link.second;
		}
	}

	return ct::cWeak<NodeLink>();
}

NodeSlotWeak BaseNode::FindSlot(nd::PinId vId)
{
	if (vId)
	{
		for (auto & node : m_ChildNodes)
		{
			auto nodePtr = node.second;
			if (nodePtr)
			{
				for (auto& pin : nodePtr->m_Inputs)
				{
					if (pin.second->pinID == vId)
					{
						return pin.second;
					}
				}
				for (auto& pin : nodePtr->m_Outputs)
				{
					if (pin.second->pinID == vId)
					{
						return pin.second;
					}
				}
			}
		}
	}
	
	return NodeSlotWeak();
}

NodeSlotWeak BaseNode::FindNodeSlotByName(BaseNodeWeak vNode, std::string vName)
{
	if (!vNode.expired())
	{
		auto nodePtr = vNode.lock();
		if (nodePtr)
		{
			for (auto& pin : nodePtr->m_Inputs)
			{
				if (pin.second->name == vName)
				{
					return pin.second;
				}
			}
			for (auto& pin : nodePtr->m_Outputs)
			{
				if (pin.second->name == vName)
				{
					return pin.second;
				}
			}
		}
	}

	return NodeSlotWeak();
}

NodeSlotWeak BaseNode::FindNodeSlotById(nd::NodeId vNodeId, nd::PinId vSlotId)
{
	auto nodePtr = FindNode(vNodeId).getValidShared();
	if (nodePtr)
	{
		for (auto& pin : nodePtr->m_Inputs)
		{
			if (pin.second->pinID == vSlotId)
			{
				return pin.second;
			}
		}
			
		for (auto& pin : nodePtr->m_Outputs)
		{
			if (pin.second->pinID == vSlotId)
			{
				return pin.second;
			}
		}
	}

	return NodeSlotWeak();
}

std::vector<NodeSlotWeak> BaseNode::GetSlotsOfType(NodeSlotPlaceEnum vPlace, std::string vType)
{
	std::vector<NodeSlotWeak> slots;

	if (vPlace == NodeSlotPlaceEnum::INPUT)
	{
		for (const auto& pin : m_Inputs)
		{
			if (pin.second && pin.second->slotType == vType)
			{
				slots.push_back(pin.second);
			}
		}
	}
	else if (vPlace == NodeSlotPlaceEnum::OUTPUT)
	{
		for (const auto& pin : m_Outputs)
		{
			if (pin.second && pin.second->slotType == vType)
			{
				slots.push_back(pin.second);
			}
		}
	}

	return slots;
}

std::vector<NodeSlotWeak> BaseNode::GetInputSlotsOfType(std::string vType)
{
	return GetSlotsOfType(NodeSlotPlaceEnum::INPUT, vType);
}

std::vector<NodeSlotWeak> BaseNode::GetOutputSlotsOfType(std::string vType)
{
	return GetSlotsOfType(NodeSlotPlaceEnum::OUTPUT , vType);
}

void BaseNode::DoCreateLinkOrNode(BaseNodeState *vBaseNodeState)
{
	if (nd::BeginCreate(ImColor(255, 255, 255), 2.0f))
	{
		auto showLabel = [](const char* label, ImColor color)
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
			auto _size_ = ImGui::CalcTextSize(label);

			auto padding = ImGui::GetStyle().FramePadding;
			auto spacing = ImGui::GetStyle().ItemSpacing;

			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

			auto rectMin = ImGui::GetCursorScreenPos() - padding;
			auto rectMax = ImGui::GetCursorScreenPos() + _size_ + padding;

			auto drawList = ImGui::GetWindowDrawList();
			drawList->AddRectFilled(rectMin, rectMax, color, _size_.y * 0.15f);
			ImGui::TextUnformatted(label);
		};

		// new link
		nd::PinId startSlotId = 0, endSlotId = 0;
		if (nd::QueryNewLink(&startSlotId, &endSlotId))
		{
			auto startSlot = FindSlot(startSlotId);
			auto endSlot = FindSlot(endSlotId);

			if (!startSlot.expired())
			{
				auto startSlotPtr = startSlot.lock();
				if (startSlotPtr)
				{
					if (!endSlot.expired())
					{
						auto endSlotPtr = endSlot.lock();
						if (endSlotPtr)
						{
							if (startSlotPtr == endSlotPtr)
							{
								nd::RejectNewItem(ImColor(255, 0, 0), 2.0f);
							}
							else if (startSlotPtr->slotPlace == endSlotPtr->slotPlace)
							{
								showLabel("x Incompatible Slot Kind", ImColor(45, 32, 32, 180));
								nd::RejectNewItem(ImColor(255, 0, 0), 2.0f);
							}
							/*else if (endSlot->parentNode == startSlot->parentNode) // desactiv?? pour les self connect en compute et fragment effect
							{
								showLabel("x Cannot connect to self", ImColor(45, 32, 32, 180));
								nd::RejectNewItem(ImColor(255, 0, 0), 1.0f);
							}*/
							else if (!startSlotPtr->CanWeConnectToSlot(endSlot) || 
								!endSlotPtr->CanWeConnectToSlot(startSlot)) // si un des deux est pas d'accord pour ken on ce barre
							{
								showLabel("x Incompatible Slot Type", ImColor(45, 32, 32, 180));
								nd::RejectNewItem(ImColor(255, 128, 128), 1.0f);
							}
							else
							{
								showLabel("+ Create Link", ImColor(32, 45, 32, 180));
								if (nd::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
								{
									ConnectSlots(startSlotPtr, endSlotPtr);
								}
							}
						}
					}
				}
			}
		}

		// new node
		nd::PinId slotId = 0;
		if (nd::QueryNewNode(&slotId))
		{
			auto newNodeLinkSlot = FindSlot(slotId);
			if (!newNodeLinkSlot.expired())
			{
				auto newLinkSlotPtr = newNodeLinkSlot.lock();
				if (newLinkSlotPtr)
				{
					showLabel("Add node", ImColor(32, 45, 32, 180));
				}
			}

			if (nd::AcceptNewItem())
			{
				vBaseNodeState->linkFromSlot = newNodeLinkSlot;
				m_CreateNewNode = true;
				nd::Suspend();
				ImGui::OpenPopup("CreateNewNode");
				nd::Resume();
			}
		}
	}
	nd::EndCreate();
}

void BaseNode::DoDeleteLinkOrNode(BaseNodeState *vBaseNodeState)
{
	UNUSED(vBaseNodeState);

	if (nd::BeginDelete())
	{
		bool canDeleteLinks = true;
		nd::NodeId nodeId = 0;
		while (nd::QueryDeletedNode(&nodeId))
		{
			if (nd::AcceptDeletedItem())
			{
				if (!DestroyChildNodeByIdIfAllowed((int)nodeId.Get(), false))
				{
					canDeleteLinks = false;
				}
			}
		}

		if (canDeleteLinks)
		{
			nd::LinkId linkId = 0;
			while (nd::QueryDeletedLink(&linkId))
			{
				if (nd::AcceptDeletedItem())
				{
					auto id = (int)linkId.Get();
					if (m_Links.find(id) != m_Links.end())
					{
						auto in = m_Links[id]->in;
						auto out = m_Links[id]->out;
						Del_VisualLink(id);
						DisConnectSlots(in, out);
					}
				}
			}
		}
	}

	nd::EndDelete();
}

void BaseNode::DoShorcutsOnNode(BaseNodeState *vBaseNodeState)
{
	UNUSED(vBaseNodeState);

	if (nd::BeginShortcut())
	{
		if (nd::AcceptCopy())
		{
			CopySelectedNodes();
		}

		if (nd::AcceptPaste())
		{
			PasteNodesAtMousePos();
		}

		if (nd::AcceptDuplicate())
		{
			//DuplicateSelectedNodes();
		}
	}

	nd::EndShortcut();
}

void BaseNode::DoPopups(BaseNodeState *vBaseNodeState)
{
	m_OpenPopupPosition = ImGui::GetMousePos();

	nd::Suspend();

	if (vBaseNodeState->m_CustomContextMenuRequested &&
		!vBaseNodeState->m_CustomContextMenuNode.expired())
	{
		ImGui::OpenPopup("CustomNodePopup");
		vBaseNodeState->m_CustomContextMenuRequested = false;
	}
	else if (nd::ShowNodeContextMenu(&m_ContextMenuNodeId))
	{
		ImGui::OpenPopup("NodeContextMenu");
	}
	else if (nd::ShowPinContextMenu(&m_ContextMenuSlotId))
	{
		ImGui::OpenPopup("SlotContextMenu");
	}
	else if (nd::ShowLinkContextMenu(&m_ContextMenuLinkId))
	{
		ImGui::OpenPopup("LinkContextMenu");
	}
	else if (nd::ShowBackgroundContextMenu())
	{
		vBaseNodeState->linkFromSlot.reset();
		ImGui::OpenPopup("CreateNewNode");
	}

	DoCheckNodePopup(vBaseNodeState);
	DoCheckSlotPopup(vBaseNodeState);
	DoCheckLinkPopup(vBaseNodeState);
	DoNewNodePopup(vBaseNodeState);

	nd::Resume();
}

void BaseNode::DoCheckNodePopup(BaseNodeState *vBaseNodeState)
{
	if (ImGui::BeginPopup("CustomNodePopup"))
	{
		if (!vBaseNodeState->m_CustomContextMenuNode.expired())
		{
			auto nodePtr = vBaseNodeState->m_CustomContextMenuNode.lock();
			if (nodePtr)
			{
				nodePtr->DrawCustomContextMenuForNode(vBaseNodeState);
			}
		}
		
		ImGui::EndPopup();
	}
	else
	{
		vBaseNodeState->m_CustomContextMenuNode.reset();

		if (ImGui::BeginPopup("NodeContextMenu"))
		{
			auto node = FindNode(m_ContextMenuNodeId);
			if (!node.expired())
			{
				auto nodePtr = node.lock();
				if (nodePtr)
				{
					nodePtr->DrawContextMenuForNode(vBaseNodeState);
				}
				else
				{
					ImGui::Text("cant lock node: %p", m_ContextMenuNodeId.AsPointer());
				}
				ImGui::Separator();
			}
			else
			{
				ImGui::Text("Unknown node: %p", m_ContextMenuNodeId.AsPointer());
				ImGui::Separator();
			}
			
			if (ImGui::MenuItem("Delete"))
			{
				nd::DeleteNode(m_ContextMenuNodeId);
			}

			ImGui::EndPopup();
		}
	}
}

void BaseNode::DoCheckSlotPopup(BaseNodeState *vBaseNodeState)
{
	UNUSED(vBaseNodeState);

	if (ImGui::BeginPopup("SlotContextMenu"))
	{
		auto slot = FindSlot(m_ContextMenuSlotId);
		if (!slot.expired())
		{
			auto slotPtr = slot.lock();
			if (slotPtr)
			{
				if (!slotPtr->parentNode.expired())
				{
					auto nodePtr = slotPtr->parentNode.lock();
					if (nodePtr)
					{
						nodePtr->DrawContextMenuForSlot(vBaseNodeState, slot);
					}
				}
			}
		}
		else
		{
			ImGui::Text("Unknown Slot: %p", m_ContextMenuSlotId.AsPointer());
		}
		ImGui::EndPopup();
	}
}

void BaseNode::DoCheckLinkPopup(BaseNodeState *vBaseNodeState)
{
	UNUSED(vBaseNodeState);

	if (ImGui::BeginPopup("LinkContextMenu"))
	{
		auto link = FindLink(m_ContextMenuLinkId);
		if (link.expired())
		{
			ImGui::Text("Unknown link: %p", m_ContextMenuLinkId.AsPointer());
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Delete"))
			nd::DeleteLink(m_ContextMenuLinkId);
		ImGui::EndPopup();
	}
}

void BaseNode::DoNewNodePopup(BaseNodeState *vBaseNodeState)
{
	UNUSED(vBaseNodeState);

	assert(!m_This.expired());

	if (m_Depth == 0)
	{
		ShowNewNodeMenu_Callback(m_This, vBaseNodeState);
	}
	else
	{
		//NodeLibrary::Instance()->ShowNewNodeMenu(NodeLibrary::NodeLibraryTypeEnum::LIBRARY_TYPE_BLUEPRINT, m_This);
	}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void BaseNode::DuplicateNode(uint32_t vNodeId, ImVec2 vOffsetPos)
{
	nd::NodeId nodeId = vNodeId;
	auto foundNode = FindNode(nodeId);
	if (!foundNode.expired())
	{
		auto foundNodePtr = foundNode.lock();
		if (foundNodePtr)
		{
			BaseNodeWeak createdNode;

			if (!createdNode.expired())
			{
				auto createdNodePtr = createdNode.lock();
				if (createdNodePtr)
				{
					nd::SetNodePosition(
						createdNodePtr->nodeID,
						createdNodePtr->pos + vOffsetPos);
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

std::string BaseNode::GetAvailableNodeStamp(const std::string& vNodeStamp)
{
	std::string res;

	if (m_NodeStamps.find(vNodeStamp) != m_NodeStamps.end()) // trouv???
	{
		m_NodeStamps[vNodeStamp]++;
		res = vNodeStamp + "_" + ct::toStr(m_NodeStamps[vNodeStamp]);
	}
	else
	{
		m_NodeStamps[vNodeStamp] = 0;
		res = vNodeStamp;
	}

	return res;
}

BaseNodeWeak BaseNode::AddChildNode(BaseNodePtr vNodePtr, bool vIncNodeId)
{
	if (vNodePtr)
	{
		assert(!m_This.expired());
		vNodePtr->m_ParentNode = m_This;

		if (vIncNodeId)
			vNodePtr->nodeID = GetNextNodeId();

		m_ChildNodes[(int)vNodePtr->nodeID.Get()] = vNodePtr;

		return vNodePtr;
	}

	return BaseNodeWeak();
}

BaseNodePtr BaseNode::GetRootNodeByCalls(BaseNodeWeak vNode) // get the root node by exploring tree with out call slots
{
	BaseNodeWeak res = vNode;

	if (!vNode)
	{
		assert(!m_This.expired());
		if (!m_This.expired())
		{
			auto nodePtr = m_This.lock();
			if (nodePtr)
			{
				res = nodePtr;
			}
		}
	}

	auto resPtr = res.getValidShared();
	if (resPtr)
	{
		if (resPtr->m_Outputs.size() == 1U)
		{
			auto call = resPtr->m_Outputs[0];
			if (call)
			{
				if (!call->parentNode.expired())
				{
					auto nodePtr = call->parentNode.lock();
					if (nodePtr)
					{
						resPtr = GetRootNodeByCalls(nodePtr);
					}
				}
			}
		}
	}

	return resPtr;
}

void BaseNode::DestroyChildNode(BaseNodeWeak vNode)
{
	if (!vNode.expired())
	{
		auto nodePtr = vNode.lock();
		if (nodePtr)
		{
			int nid = (int)nodePtr->nodeID.Get();
			if (m_ChildNodes.find(nid) != m_ChildNodes.end()) // trouv??
			{
				m_ChildNodes.erase(nid);
			}
		}
	}
}

bool BaseNode::DestroyChildNodeByIdIfAllowed(int vNodeID, bool vDestroy)
{
	bool res = false;

	if (m_ChildNodes.find(vNodeID) != m_ChildNodes.end()) // trouv??
	{
		auto nodePtr = m_ChildNodes[vNodeID];
		if (nodePtr && !nodePtr->deletionDisabled)
		{
			if (vDestroy)
			{
				m_ChildNodes.erase(vNodeID);
			}
			else
			{
				m_NodeIdToDelete.emplace(vNodeID);
			}

			res = true;
		}
	}

	return res;
}

void BaseNode::DestroySlotOfAnyMap(NodeSlotWeak vSlot)
{
	auto slotPtr = vSlot.getValidShared();
	if (slotPtr)
	{
		int sid = (int)slotPtr->pinID.Get();

		DisConnectSlot(vSlot);

		if (m_Inputs.find(sid) != m_Inputs.end())
			m_Inputs.erase(sid);
		if (m_Outputs.find(sid) != m_Outputs.end())
			m_Outputs.erase(sid);
	}
}

// a executer apres le rendu de imgui
void BaseNode::DestroyNodesIfAnys()
{
	for (auto & nodeId : m_NodeIdToDelete)
	{
		DestroyChildNodeByIdIfAllowed(nodeId, true);
	}
	m_NodeIdToDelete.clear();

	// check for sub graphs
	for (auto & node : m_ChildNodes)
	{
		auto nodePtr = node.second;
		if (nodePtr)
		{
			nodePtr->DestroyNodesIfAnys();
		}
	}
}

void BaseNode::SetChanged(bool vFlag)
{
	changed = vFlag;
	auto ptr = m_ParentNode.getValidShared();
	if (ptr)
	{
		ptr->SetChanged(vFlag);
	}
}

void BaseNode::Notify(const NotifyEvent& /*vEvent*/, const NodeSlotWeak& /*vEmitterSlot*/, const NodeSlotWeak& /*vReceiverSlot*/)
{
#ifdef _DEBUG
	LogVarInfo("BaseNode::Notify catched by the node \"%s\", some class not implement it. maybe its wanted", name.c_str());
#endif
}

void BaseNode::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	for (auto input : m_Inputs)
	{
		if (input.second)
		{
			for (auto slot : input.second->linkedSlots)
			{
				auto slotPtr = slot.getValidShared();
				if (slotPtr)
				{
					auto parentPtr = slotPtr->parentNode.getValidShared();
					if (parentPtr)
					{
						parentPtr->NeedResize(vNewSize, vCountColorBuffers);
					}
				}
			}
		}
	}
}

ct::fvec2 BaseNode::GetOutputSize()
{
	for (auto input : m_Inputs)
	{
		if (input.second)
		{
			for (auto slot : input.second->linkedSlots)
			{
				auto slotPtr = slot.getValidShared();
				if (slotPtr)
				{
					auto parentPtr = slotPtr->parentNode.getValidShared();
					if (parentPtr)
					{
						return parentPtr->GetOutputSize();
					}
				}
			}
		}
	}

	return 0.0f;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// LAYOUT //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void BaseNode::DoLayout()
{
	
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// GET LINKS / SLOTS ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::vector<ct::cWeak<NodeLink>> BaseNode::GetLinksAssociatedToSlot(NodeSlotWeak vSlot)
{
	std::vector<ct::cWeak<NodeLink>> res;

	if (!vSlot.expired())
	{
		auto slotPtr = vSlot.lock();
		if (slotPtr)
		{
			uint32_t pinId = (uint32_t)slotPtr->pinID.Get();

			if (m_LinksDico.find(pinId) != m_LinksDico.end()) // trouve
			{
				auto linkIds = m_LinksDico[pinId]; // link Ptr pointe sur une entr??e de m_Links
				for (auto lid : linkIds)
				{
					if (m_Links.find(lid) != m_Links.end())
					{
						res.push_back(m_Links[lid]);
					}
				}
			}
		}
	}

	return res;
}

std::vector<NodeSlotWeak> BaseNode::GetSlotsAssociatedToSlot(NodeSlotWeak vSlot)
{
	std::vector<NodeSlotWeak> res;

	if (!vSlot.expired())
	{
		auto slotPtr = vSlot.lock();
		if (slotPtr)
		{
			res = slotPtr->linkedSlots;
		}
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// ADD/DELETE VISUAL LINKS (NO CHANGE BEHIND) //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void BaseNode::Add_VisualLink(NodeSlotWeak vStart, NodeSlotWeak vEnd)
{
	if (!vStart.expired() && 
		!vEnd.expired())
	{
		auto startPtr = vStart.lock();
		auto endPtr = vEnd.lock();

		if (startPtr && endPtr)
		{
			startPtr->connected = true;
			endPtr->connected = true;

			startPtr->linkedSlots.push_back(vEnd);
			endPtr->linkedSlots.push_back(vStart);

			NodeLink link;
			link.in = vStart;
			link.out = vEnd;
			link.linkId = GetNextNodeId();

			m_Links[link.linkId] = std::make_shared<NodeLink>(link);
			m_LinksDico[(uint32_t)startPtr->pinID.Get()].emplace(m_Links[link.linkId]->linkId);
			m_LinksDico[(uint32_t)endPtr->pinID.Get()].emplace(m_Links[link.linkId]->linkId);
		}
	}
}

void BaseNode::Del_VisualLink(uint32_t vLinkId)
{
	if (m_Links.find(vLinkId) != m_Links.end())
	{
		if (!m_Links[vLinkId]->in.expired() &&
			!m_Links[vLinkId]->out.expired())
		{
			auto inPtr = m_Links[vLinkId]->in.lock();
			auto outPtr = m_Links[vLinkId]->out.lock();

			if (inPtr && outPtr)
			{
				inPtr->linkedSlots.clear();
				inPtr->connected = false;
				m_LinksDico.erase((uint32_t)inPtr->pinID.Get());
				
				outPtr->linkedSlots.clear();
				outPtr->connected = false;
				m_LinksDico.erase((uint32_t)outPtr->pinID.Get());
				
				m_Links.erase(vLinkId);
			}
		}
	}
	else
	{
		CTOOL_DEBUG_BREAK;
		LogVarDebug("Link id %i not found", vLinkId);
	}
}

void BaseNode::Break_VisualLinks_ConnectedToSlot(NodeSlotWeak vSlot)
{
	if (!vSlot.expired())
	{
		auto slotPtr = vSlot.lock();
		if (slotPtr)
		{
			uint32_t pinId = (uint32_t)slotPtr->pinID.Get();

			if (m_LinksDico.find(pinId) != m_LinksDico.end()) // trouve
			{
				auto linkIds = m_LinksDico[pinId]; // link Ptr pointe sur une entr??e de m_Links
				for (auto lid : linkIds)
				{
					Del_VisualLink(lid);
				}
			}
			else
			{
				CTOOL_DEBUG_BREAK;
			}
		}
	}
}

void BaseNode::Break_VisualLink_ConnectedToSlots(NodeSlotWeak vFrom, NodeSlotWeak vTo)
{
	if (!vFrom.expired() &&
		!vTo.expired())
	{
		auto fromPtr = vFrom.lock();
		auto toPtr = vTo.lock();

		if (fromPtr && toPtr)
		{
			uint32_t fromPinId = (uint32_t)fromPtr->pinID.Get();
			uint32_t endPinId = (uint32_t)toPtr->pinID.Get();
			
			if (m_LinksDico.find(fromPinId) != m_LinksDico.end() &&
				m_LinksDico.find(endPinId) != m_LinksDico.end()) // trouve
			{
				// find link between vFrom and vTo
				uint32_t foundId = 0;
				auto fromLinks = m_LinksDico[fromPinId];
				auto toLinks = m_LinksDico[endPinId];
				for (auto fid : fromLinks)
				{
					for (auto tid : toLinks)
					{
						if (tid == fid)
						{
							foundId = tid;
							break;
						}
					}
					if (foundId)
						break;
				}

				// delete link visually
				if (foundId)
				{
					Del_VisualLink(foundId);
				}
			}
			else
			{
				//CTOOL_DEBUG_BREAK;
			}

			// averti le parent que les slot on chang?? leur statut de connection
			fromPtr->NotifyConnectionChangeToParent(true);
			toPtr->NotifyConnectionChangeToParent(true);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONNECT / DISCONNECT SLOTS BEHIND ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool BaseNode::ConnectSlots(NodeSlotWeak vFrom, NodeSlotWeak vTo)
{
	bool res = false;

	auto fromPtr = vFrom.getValidShared();
	auto toPtr = vTo.getValidShared();
	if (fromPtr && toPtr)
	{
		// ensure that start is an output and end an input
		if (fromPtr->slotPlace == NodeSlotPlaceEnum::INPUT &&
			toPtr->slotPlace == NodeSlotPlaceEnum::OUTPUT)
		{
			//peu etre que std::swap merde quand c'est des shared_ptr ou weak_ptr
			vFrom.swap(vTo); 
			fromPtr = vTo.lock();
			toPtr = vFrom.lock();
		}

		auto fromParentNodePtr = fromPtr->parentNode.getValidShared();
		auto toParentNodePtr = toPtr->parentNode.getValidShared();
		if (fromParentNodePtr && toParentNodePtr)
		{
			if (toPtr->CanWeConnectToSlot(fromPtr))
			{
				//LogVarDebug("Connection Success from %s to %s", fromParentNodePtr->name.c_str(), toParentNodePtr->name.c_str());
								
				// un output peut etre connect?? a plusieurs inputs
				// un input ne peut etre connect?? qu'a un seul output
				if (toPtr->IsAnInput())
				{
					DestroySlotOfAnyMap(toPtr);
				}
				else if (fromPtr->IsAnInput())
				{
					DestroySlotOfAnyMap(fromPtr);
				}

				Add_VisualLink(fromPtr, toPtr);

				ConnectNodeSlots(fromPtr, toPtr);

				// averti le parent que les slot on chang?? leur statut de connection
				fromPtr->NotifyConnectionChangeToParent(true);
				toPtr->NotifyConnectionChangeToParent(true);

				auto output3DNodePtr = m_Output3DNode.getValidShared();
				if (output3DNodePtr)
				{
					output3DNodePtr->Notify(NotifyEvent::SomeTasksWasUpdated);
				}

				auto output2DNodePtr = m_Output2DNode.getValidShared();
				if (output2DNodePtr)
				{
					output2DNodePtr->Notify(NotifyEvent::SomeTasksWasUpdated);
				}

				res = true;
			}
					
			if (!res)
			{
				std::string fromTypeStr = uType::ConvertUniformsTypeEnumToString(fromPtr->type);
				std::string toTypeStr = uType::ConvertUniformsTypeEnumToString(toPtr->type);

				if (!fromParentNodePtr->m_ParentNode.expired())
				{
					auto parentParentNodePtr = fromParentNodePtr->m_ParentNode.lock();
					if (parentParentNodePtr)
					{
						LogVarDebug("graph(%s) => Cant connect slots :\n\t- slot(stamp:%s,type:%s) from node(%s)\n\t- to\n\t- slot(stamp:%s,type:%s) from node(%s)\n",
							parentParentNodePtr->name.c_str(),
							fromPtr->stamp.typeStamp.c_str(), fromTypeStr.c_str(), fromParentNodePtr->name.c_str(),
							toPtr->stamp.typeStamp.c_str(), toTypeStr.c_str(), toParentNodePtr->name.c_str());
					}
				}
				else
				{
					//LogVarDebug("Was Swapped : %s", swapped ? "true" : "false");
					LogVarDebug("graph(NULL) => Cant connect slots :\n\t- slot(stamp:%s,type:%s) from node(%s)\n\t- to\n\t- slot(stamp:%s,type:%s) from node(%s)\n",
						fromPtr->stamp.typeStamp.c_str(), fromTypeStr.c_str(), fromParentNodePtr->name.c_str(),
						toPtr->stamp.typeStamp.c_str(), toTypeStr.c_str(), toParentNodePtr->name.c_str());
				}
			}
		}
		else
		{
			LogVarDebug("Le node parent du slot 'From' et 'to' est vide");
		}
	}
	else
	{
		LogVarDebug("l'un des from et to ne peut pas etre locked");
	}

	if (res)
	{
		/*if (NodeGraphSystem::Instance()->m_AutoCompilOnChanges)
		{
			NodeGraphSystem::Instance()->m_NeedReCompilation = true;
		}*/
	}

	return res;
}

bool BaseNode::DisConnectSlots(NodeSlotWeak vFrom, NodeSlotWeak vTo)
{
	Break_VisualLink_ConnectedToSlots(vFrom, vTo);
	DisConnectNodeSlots(vFrom, vTo);

	return true;
}

bool BaseNode::DisConnectSlot(NodeSlotWeak vSlot)
{
	auto slots = GetSlotsAssociatedToSlot(vSlot);
	for (auto slotPtr : slots)
	{
		Break_VisualLink_ConnectedToSlots(vSlot, slotPtr);
		DisConnectNodeSlots(vSlot, slotPtr);
	}

	return true;
}

void BaseNode::NotifyConnectionChangeOfThisSlot(NodeSlotWeak vSlot, bool vConnected) // ce solt a ??t?? connect??
{
	UNUSED(vConnected);
	if (!vSlot.expired())
	{
		auto ptr = vSlot.lock();
		if (ptr)
		{
			if (ptr->slotPlace == NodeSlotPlaceEnum::INPUT)
			{
				//if (!m_Code.m_Sections.empty())
				{
					if (ptr->connected)
					{
						// on va remplacer le code venant du node connect?? au node de ce slot
						// pour ce faire, on a besoin :
						// - du nom de la fonction originale, pour localiser la section dont le code doit etre remplac?? dans le node du slot (de destination)
						// - du code de la fonction connect?? au slot

						// 1, on recupere le code de la fonction connect?? au slot
						std::string newCode;
						if (!ptr->linkedSlots.empty())
						{
							auto linkedSlot = ptr->linkedSlots[0];
							if (!linkedSlot.expired())
							{
								auto linkedSlotPtr = linkedSlot.lock();
								if (linkedSlotPtr)
								{
									// on a le slot
									// maintenant on va recuperer le node parent de ce slot
									if (!linkedSlotPtr->parentNode.expired())
									{
										auto parentNodePtr = linkedSlotPtr->parentNode.lock();
										if (parentNodePtr)
										{
											// on a le node
											newCode = parentNodePtr->GetNodeCode();
										}
									}
								}
							}
						}
						else
						{
							LogVarDebug("pas normal, un slot connect?? devrait avoir des liens");
						}

						// 2, on va remplacer le code de la section correspondante de m_Code par le nouveau code
						if (!newCode.empty())
						{
							//m_Code.ReplaceSection(ptr->originalFuncName, newCode);
							CompilGeneratedCode();
						}
					}
					else
					{
						//m_Code.ResetSection(ptr->originalFuncName);
						CompilGeneratedCode();
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE /////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool BaseNode::ConnectNodeSlots(NodeSlotWeak vStart, NodeSlotWeak vEnd)
{
	bool res = false;

	auto startPtr = vStart.getValidShared();
	auto endPtr = vEnd.getValidShared();
	if (startPtr && endPtr)
	{
		auto nodeStartPtr = startPtr->parentNode.getValidShared();
		auto nodeEndPtr = endPtr->parentNode.getValidShared();
		if (nodeStartPtr && nodeEndPtr)
		{
			nodeEndPtr->SetCodeDirty(true);

			nodeStartPtr->JustConnectedBySlots(vStart, vEnd);
			nodeEndPtr->JustConnectedBySlots(vEnd, vStart);

			res = true;
		}
		else
		{
			LogVarDebug("Error, le node parent de vStart ou vEnd est null");
		}
	}

	return res;
}

bool BaseNode::DisConnectNodeSlots(NodeSlotWeak vStart, NodeSlotWeak vEnd)
{
	auto startPtr = vStart.getValidShared();
	auto endPtr = vEnd.getValidShared();
	if (startPtr && endPtr)
	{
		auto nodeStartPtr = startPtr->parentNode.getValidShared();
		auto nodeEndPtr = endPtr->parentNode.getValidShared();
		if (nodeStartPtr && nodeEndPtr)
		{
			nodeEndPtr->SetCodeDirty(true);

			nodeStartPtr->JustDisConnectedBySlots(vStart, vEnd);
			nodeEndPtr->JustDisConnectedBySlots(vEnd, vStart);

			nodeStartPtr->SetChanged();
			nodeEndPtr->SetChanged();
		}
		else
		{
			LogVarDebug("Error, le node parent de vStart ou vEnd est null");
		}
	}

	// en fait si ca merde on doit pouvoir supprimer le lien quand meme
	// le but de cette fonc est plus de reinit le pilotage
	// donc on renvoie true pour dire que ca a reussit meme si ca peu avoir foir??
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SLOT CONNECTION TEST ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool BaseNode::CanWeConnectSlots(NodeSlotWeak vFrom, NodeSlotWeak vTo)
{
	if (!vFrom.expired() &&
		!vTo.expired())
	{
		auto fromPtr = vFrom.getValidShared();
		auto toPtr = vTo.getValidShared();

		if (fromPtr && toPtr)
		{
			auto fromParentPtr = fromPtr->parentNode.getValidShared();
			auto toParentPtr = toPtr->parentNode.getValidShared();

			if (fromParentPtr && toParentPtr && 
				fromParentPtr != toParentPtr) // not same node
			{
				if (fromPtr->slotType == toPtr->slotType) // same slot type
				{
					return true;
				}
			}
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SLOT SPLITTER ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::vector<NodeSlotWeak> BaseNode::InjectTypeInSlot(NodeSlotWeak vSlotToSplit, uType::uTypeEnum vType)
{
	std::vector<NodeSlotWeak> res;

	if (vType != uType::uTypeEnum::U_VOID)
	{
		if (!vSlotToSplit.expired())
		{
			auto slotPtr = vSlotToSplit.lock();
			if (slotPtr)
			{
				auto slotType = slotPtr->type;
				auto countChannels = uType::GetCountChannelForType(slotType);
				auto newCountChannels = uType::GetCountChannelForType(vType);
				if ((countChannels > 1U && newCountChannels < countChannels) || 
					slotPtr->IsGenericType())
				{

				}
			}
		}
	}
	else
	{
		LogVarDebug("Cant inject void type in slot");
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

std::string BaseNode::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string res;

	if (!m_ChildNodes.empty())
	{
		res += vOffset + "<graph>\n";

		res += vOffset + "\t<canvas>\n";
		res += vOffset + "\t\t<offset>" + ct::fvec2(GetCanvasOffset()).string() + "</offset>\n";
		res += vOffset + "\t\t<scale>" + ct::toStr(GetCanvasScale()) + "</scale>\n";
		res += vOffset + "\t</canvas>\n";

		// childs
		res += vOffset + "\t<nodes>\n";
		for (auto& node : m_ChildNodes)
		{
			auto nodePtr = node.second;
			if (nodePtr)
			{
				res += nodePtr->getXml(vOffset + "\t\t", vUserDatas);
			}
		}
		res += vOffset + "\t</nodes>\n";

		// links
		res += vOffset + "\t<links>\n";
		for (auto link : m_Links)
		{
			if (!link.second->in.expired() &&
				!link.second->out.expired())
			{
				auto inPtr = link.second->in.lock();
				auto outPtr = link.second->out.lock();
				if (inPtr && outPtr)
				{
					if (!inPtr->parentNode.expired() &&
						!outPtr->parentNode.expired())
					{
						auto inParentPtr = inPtr->parentNode.lock();
						auto outParentPtr = outPtr->parentNode.lock();

						if (inParentPtr && outParentPtr)
						{
							std::string inNodeIdSlotId = ct::toStr("%u:%u", (uint32_t)inParentPtr->nodeID.Get(), (uint32_t)inPtr->pinID.Get());
							std::string outNodeIdSlotId = ct::toStr("%u:%u", (uint32_t)outParentPtr->nodeID.Get(), (uint32_t)outPtr->pinID.Get());
							res += vOffset + "\t\t<link in=\"" + inNodeIdSlotId + "\" out=\"" + outNodeIdSlotId + "\"/>\n";
						}
					}
				}
			}
		}
		res += vOffset + "\t</links>\n";

		res += vOffset + "</graph>\n";
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

		res += vOffset + "</node>\n";
	}

	return res;
}

bool BaseNode::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "canvas")
	{
		if (strName == "offset")
			SetCanvasOffset(ct::toImVec2(ct::fvariant(strValue).GetV2()));
		else if (strName == "scale")
			SetCanvasScale(ct::fvariant(strValue).GetF());

		return false;
	}
	else if (strParentName == "nodes")
	{
		if (strName == "node")
		{
			std::string _name;
			std::string _type;
			ct::fvec2 _pos;
			uint32_t _nodeId = 0;

			for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
			{
				std::string attName = attr->Name();
				std::string attValue = attr->Value();

				if (attName == "name")
					_name = attValue;
				else if (attName == "type")
					_type = attValue;
				else if (attName == "pos")
					_pos = ct::fvariant(attValue).GetV2();
				else if (attName == "id")
					_nodeId = ct::ivariant(attValue).GetU();
			}

			if (LoadNodeFromXML_Callback(m_This, vElem, vParent, _name, _type, _pos, _nodeId))
			{
				RecursParsingConfigChilds(vElem, vUserDatas);
			}

			return false;
		}
	}
	else if (strParentName == "links")
	{
		if (strName == "link")
		{
			std::string inStr;
			std::string outStr;

			for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
			{
				std::string attName = attr->Name();
				std::string attValue = attr->Value();

				if (attName == "in")
					inStr = attValue;
				else if (attName == "out")
					outStr = attValue;
			}

			auto vecIn = ct::splitStringToVector(inStr, ':');
			auto vecOut = ct::splitStringToVector(outStr, ':');

			if (vecIn.size() == 2 && vecOut.size() == 2)
			{
				SlotEntry entIn;
				entIn.first = ct::ivariant(vecIn[0]).GetU();
				entIn.second = ct::ivariant(vecIn[1]).GetU();

				SlotEntry entOut;
				entOut.first = ct::ivariant(vecOut[0]).GetU();
				entOut.second = ct::ivariant(vecOut[1]).GetU();

				LinkEntry link;
				link.first = entIn;
				link.second = entOut;

				m_LinksToBuildAfterLoading.push_back(link);
			}
		}

		return false;
	}
	else if (strParentName == "node")
	{
		NodeSlot slot;
		if (strName == "slot")
		{
			for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
			{
				std::string attName = attr->Name();
				std::string attValue = attr->Value();

				if (attName == "index")
					slot.index = ct::ivariant(attValue).GetU();
				else if (attName == "name")
					slot.name = attValue;
				else if (attName == "type")
					slot.slotType = attValue;
				else if (attName == "place")
					slot.slotPlace = GetNodeSlotPlaceEnumFromString(attValue);
				else if (attName == "id")
					slot.pinID = ct::ivariant(attValue).GetU();
			}	

			if (slot.slotPlace == NodeSlotPlaceEnum::INPUT)
			{
				if (!m_Inputs.empty())
				{
					bool wasSet = false;
					for (auto input : m_Inputs)
					{
						if (input.second->index == slot.index)
						{
							wasSet = !input.second->setFromXml(vElem, vParent);
							if (wasSet)
							{
								m_Inputs.erase(input.first);
								m_Inputs[(uint32_t)input.second->pinID.Get()] = input.second;
								break;
							}
						}
					}
					if (!wasSet)
					{
						auto slotPtr = AddInput(slot).getValidShared();
						if (slotPtr)
						{
							slotPtr->idAlreadySetbyXml = true;
						}
					}
				}
			}
			else if (slot.slotPlace == NodeSlotPlaceEnum::OUTPUT)
			{
				if (!m_Outputs.empty())
				{
					bool wasSet = false;
					for (auto output : m_Outputs)
					{
						if (output.second->index == slot.index)
						{
							wasSet = !output.second->setFromXml(vElem, vParent);
							if (wasSet)
							{
								m_Outputs.erase(output.first);
								m_Outputs[(uint32_t)output.second->pinID.Get()] = output.second;
								break;
							}
						}
					}
					if (!wasSet)
					{
						auto slotPtr = AddOutput(slot).getValidShared();
						if (slotPtr)
						{
							slotPtr->idAlreadySetbyXml = true;
						}
					}
				}
			}
		}

		return false;
	}

	return true;
}
