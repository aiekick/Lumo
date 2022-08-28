// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include "LayoutManager.h"

#include <ctools/FileHelper.h>
#include <ctools/Logger.h>

#include <imgui/imgui_internal.h>

LayoutManager::LayoutManager() = default;
LayoutManager::~LayoutManager() = default;

void LayoutManager::AddPane(
	AbstractPanePtr vPanePtr,
	const char* vName,
	PaneFlags vFlag,
	PaneDisposal vPaneDisposal,
	bool vIsOpenedDefault, 
	bool vIsFocusedDefault)
{
	assert(vFlag); // flag != 0
	assert(vPanePtr); // vPane != nullptr
	assert(vName && strlen(vName)); // vPane->m_PaneName not nullptr
	assert(m_PanesByName.find(vName) == m_PanesByName.end()); // pane name not already exist
	assert(m_PanesByFlag.find(vFlag) == m_PanesByFlag.end()); // pane flag not already exist
	
	vPanePtr->m_PaneName = vName;
	vPanePtr->m_PaneFlag = vFlag;
	vPanePtr->m_PaneDisposal = vPaneDisposal;
	vPanePtr->m_OpenedDefault = vIsOpenedDefault;
	vPanePtr->m_FocusedDefault = vIsFocusedDefault;
	if (vIsOpenedDefault)
		m_Pane_Opened_Default |= vPanePtr->m_PaneFlag;
	if (vIsFocusedDefault)
		m_Pane_Focused_Default |= vPanePtr->m_PaneFlag;
	m_PanesByDisposal[vPanePtr->m_PaneDisposal] = vPanePtr;
	m_PanesByName[vPanePtr->m_PaneName] = vPanePtr;
	m_PanesByFlag[vPanePtr->m_PaneFlag] = vPanePtr;
	m_PanesInDisplayOrder.push_back(vPanePtr);
}

void LayoutManager::SetPaneDisposalSize(const PaneDisposal& vPaneDisposal, float vSize)
{
	if (vPaneDisposal == PaneDisposal::CENTRAL ||
		vPaneDisposal == PaneDisposal::Count)
		return;

	m_PaneDisposalSizes[(int)vPaneDisposal] = vSize;
}

void LayoutManager::Init(const char* vMenuLabel, const char* vDefautlMenuLabel)
{
	assert(vMenuLabel);
	assert(vDefautlMenuLabel);

#ifdef MSVC
	strncpy_s(m_MenuLabel, vMenuLabel, ct::mini((size_t)PANE_NAME_BUFFER_SIZE, strlen(vMenuLabel)));
	strncpy_s(m_DefaultMenuLabel, vDefautlMenuLabel, ct::mini((size_t)PANE_NAME_BUFFER_SIZE, strlen(vDefautlMenuLabel)));
#else
	strncpy(m_MenuLabel, vMenuLabel, ct::mini((size_t)PANE_NAME_BUFFER_SIZE, strlen(vMenuLabel)));
	strncpy(m_DefaultMenuLabel, vDefautlMenuLabel, ct::mini((size_t)PANE_NAME_BUFFER_SIZE, strlen(vDefautlMenuLabel)));
#endif

	if (!FileHelper::Instance()->IsFileExist("imgui.ini"))
	{
		m_FirstLayout = true; // need default layout
		LogVarDebug("We will apply default layout :)");
	}
}

void LayoutManager::Unit()
{
	for (const auto& pane : m_PanesByFlag)
	{
		pane.second->Unit();
	}
}

void LayoutManager::InitAfterFirstDisplay(ImVec2 vSize)
{
	if (m_FirstLayout)
	{
		ApplyInitialDockingLayout(vSize);
		m_FirstLayout = false;
	}

	if (m_FirstStart)
	{
		// focus after start of panes
		Internal_SetFocusedPanes(m_Pane_Focused);
		m_FirstStart = false;
	}
}

bool LayoutManager::BeginDockSpace(ImGuiDockNodeFlags vFlags)
{
	const auto viewport = ImGui::GetMainViewport();

	m_LastSize = viewport->Size;

	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	auto host_window_flags = 0;
	host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
	host_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	if (vFlags & ImGuiDockNodeFlags_PassthruCentralNode)
		host_window_flags |= ImGuiWindowFlags_NoBackground;

	char label[100 + 1];
	ImFormatString(label, 100, "DockSpaceViewport_%08X", viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	const auto res = ImGui::Begin(label, nullptr, host_window_flags);
	ImGui::PopStyleVar(3);

	m_DockSpaceID = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(m_DockSpaceID, ImVec2(0.0f, 0.0f), vFlags);

	return res;
}

void LayoutManager::EndDockSpace()
{
	ImGui::End();
}

bool LayoutManager::IsDockSpaceHoleHovered()
{
	auto& g = *GImGui;
	return g.HoveredDockNode == nullptr && g.HoveredWindow == nullptr;
}

void LayoutManager::ApplyInitialDockingLayout(ImVec2 vSize)
{
	if (IS_FLOAT_EQUAL(vSize.x, 0.0f) || IS_FLOAT_EQUAL(vSize.y, 0.0f))
	{
		vSize = m_LastSize;

		if (IS_FLOAT_EQUAL(m_LastSize.x, 0.0f) || IS_FLOAT_EQUAL(m_LastSize.y, 0.0f))
		{
			return;
		}
	}

	ImGui::DockBuilderRemoveNode(m_DockSpaceID); // Clear out existing layout
	ImGui::DockBuilderAddNode(m_DockSpaceID, ImGuiDockNodeFlags_DockSpace); // Add empty node
	ImGui::DockBuilderSetNodeSize(m_DockSpaceID, vSize);

	// just for readability
	const auto& left_size = m_PaneDisposalSizes[1];
	const auto& right_size = m_PaneDisposalSizes[2];
	const auto& bottom_size = m_PaneDisposalSizes[3];
	const auto& top_size = m_PaneDisposalSizes[4];

	auto dockMainID = m_DockSpaceID; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
	const auto dockLeftID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Left, left_size / vSize.x, nullptr, &dockMainID);
	const auto dockRightID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Right, right_size / (vSize.x - left_size), nullptr, &dockMainID);
	const auto dockBottomID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Down, bottom_size / vSize.y, nullptr, &dockMainID);
	const auto dockTopID = ImGui::DockBuilderSplitNode(dockMainID, ImGuiDir_Up, top_size / (vSize.y - bottom_size), nullptr, &dockMainID);

	for (const auto& pane : m_PanesByName)
	{
		switch (pane.second->m_PaneDisposal)
		{
			case PaneDisposal::CENTRAL:
			{
				ImGui::DockBuilderDockWindow(pane.first, dockMainID);
				break;
			}
			case PaneDisposal::LEFT:
			{
				ImGui::DockBuilderDockWindow(pane.first, dockLeftID);
				break;
			}
			case PaneDisposal::RIGHT:
			{
				ImGui::DockBuilderDockWindow(pane.first, dockRightID);
				break;
			}
			case PaneDisposal::BOTTOM:
			{
				ImGui::DockBuilderDockWindow(pane.first, dockBottomID);
				break;
			}
			case PaneDisposal::TOP:
			{
				ImGui::DockBuilderDockWindow(pane.first, dockTopID);
				break;
			}
		};
	}
	
	ImGui::DockBuilderFinish(m_DockSpaceID);

	m_Pane_Shown = m_Pane_Opened_Default; // will show when pane will be passed
	m_Pane_Focused = m_Pane_Focused_Default;

	Internal_SetFocusedPanes(m_Pane_Focused);
}

template<typename T>
static bool LayoutManager_MenuItem(const char* label, const char* shortcut, T* vContainer, T vFlag, bool vOnlyOneSameTime = false)
{
	bool selected = *vContainer & vFlag;
	const bool res = ImGui::MenuItem(label, shortcut, &selected, true);
	if (res) 
	{
		if (selected) 
		{
			if (vOnlyOneSameTime) 
				*vContainer = vFlag; // set
			else 
				*vContainer = (T)(*vContainer | vFlag);// add
		}
		else if (!vOnlyOneSameTime) 
				*vContainer = (T)(*vContainer & ~vFlag); // remove
	}
	return res;
}

void LayoutManager::DisplayMenu(ImVec2 vSize)
{
	if (ImGui::BeginMenu(m_MenuLabel))
	{
		if (ImGui::MenuItem(m_DefaultMenuLabel))
		{
			ApplyInitialDockingLayout(vSize);
		}

		ImGui::Separator();

		static char buffer[100 + 1] = "\0";
		for (auto pane : m_PanesInDisplayOrder)
		{
			if (pane->CanWeDisplay())
			{
				snprintf(buffer, 100, "%s Pane", pane->m_PaneName);
				LayoutManager_MenuItem<PaneFlags>(buffer, "", &m_Pane_Shown, pane->m_PaneFlag);
			}
		}
		
		ImGui::EndMenu();
	}
}

int LayoutManager::DisplayPanes(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas)
{
	for (const auto& pane : m_PanesByFlag)
	{
		if (pane.second->CanWeDisplay())
		{
			vWidgetId = pane.second->DrawPanes(vCurrentFrame, vWidgetId, vUserDatas);
		}
	}

	return vWidgetId;
}

void LayoutManager::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, std::string vUserDatas)
{
	for (const auto& pane : m_PanesByFlag)
	{
		if (pane.second->CanWeDisplay())
		{
			pane.second->DrawDialogsAndPopups(vCurrentFrame, vUserDatas);
		}
	}
}

int LayoutManager::DrawWidgets(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas)
{
	for (const auto& pane : m_PanesByFlag)
	{
		if (pane.second->CanWeDisplay())
		{
			vWidgetId = pane.second->DrawWidgets(vCurrentFrame, vWidgetId, vUserDatas);
		}
	}

	return vWidgetId;
}

void LayoutManager::ShowSpecificPane(PaneFlags vPane)
{
	m_Pane_Shown = (PaneFlags)((int32_t)m_Pane_Shown | (int32_t)vPane);
}

void LayoutManager::HideSpecificPane(PaneFlags vPane)
{
	m_Pane_Shown = (PaneFlags)((int32_t)m_Pane_Shown & ~(int32_t)vPane);
}

void LayoutManager::FocusSpecificPane(PaneFlags vPane)
{
	ShowSpecificPane(vPane);

	if (m_PanesByFlag.find(vPane) != m_PanesByFlag.end())
	{
		FocusSpecificPane(m_PanesByFlag[vPane]->m_PaneName);
	}
}

void LayoutManager::ShowAndFocusSpecificPane(PaneFlags vPane)
{
	ShowSpecificPane(vPane);
	FocusSpecificPane(vPane);
}

bool LayoutManager::IsSpecificPaneFocused(PaneFlags vPane)
{
	if (m_PanesByFlag.find(vPane) != m_PanesByFlag.end())
	{
		return IsSpecificPaneFocused(m_PanesByFlag[vPane]->m_PaneName);
	}

	return false;
}

void LayoutManager::AddSpecificPaneToExisting(const char* vNewPane, const char* vExistingPane)
{
	ImGuiWindow* window = ImGui::FindWindowByName(vExistingPane);
	if (window)
	{
		auto dockid = window->DockId;
		ImGui::DockBuilderDockWindow(vNewPane, dockid);
	}
}

///////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////
///////////////////////////////////////////////////////

bool LayoutManager::IsSpecificPaneFocused(const char *vlabel)
{
	ImGuiWindow* window = ImGui::FindWindowByName(vlabel);
	if (window)
	{
		return 
			window->DockTabIsVisible || 
			window->ViewportOwned;
	}
	return false;
}

void LayoutManager::FocusSpecificPane(const char *vlabel)
{
	ImGuiWindow* window = ImGui::FindWindowByName(vlabel);
	if (window)
	{
		if(!window->DockTabIsVisible)
			ImGui::FocusWindow(window);
	}
}

///////////////////////////////////////////////////////
//// CONFIGURATION PRIVATE ////////////////////////////
///////////////////////////////////////////////////////

PaneFlags LayoutManager::Internal_GetFocusedPanes()
{
	PaneFlags flag = 0;

	for (const auto& pane : m_PanesByFlag)
	{
		if (IsSpecificPaneFocused(pane.second->m_PaneName))
			flag = (PaneFlags)((int32_t)flag | (int32_t)pane.first);
	}

	return flag;
}

void LayoutManager::Internal_SetFocusedPanes(PaneFlags vActivePanes)
{
	for (const auto& pane : m_PanesByFlag)
	{
		if (vActivePanes & pane.first)
			FocusSpecificPane(pane.second->m_PaneName);
	}
}

///////////////////////////////////////////////////////
//// CONFIGURATION PUBLIC /////////////////////////////
///////////////////////////////////////////////////////

std::string LayoutManager::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	if (vUserDatas == "app")
	{
		str += vOffset + "<layout>\n";
		m_Pane_Focused = Internal_GetFocusedPanes();
		str += vOffset + "\t<panes opened=\"" + ct::ivariant((int32_t)m_Pane_Shown).GetS() + "\" active=\"" + ct::ivariant((int32_t)m_Pane_Focused).GetS() + "\"/>\n";
		str += vOffset + "</layout>\n";
	}
	else if (vUserDatas == "project")
	{
		// per pane settings
	}

	return str;
}

bool LayoutManager::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	// The value of this child identifies the name of this element
	std::string strName = "";
	std::string strValue = "";
	std::string strParentName = "";

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != 0)
		strParentName = vParent->Value();

	if (vUserDatas == "app")
	{
		if (strParentName == "layout")
		{
			for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
			{
				std::string attName = attr->Name();
				std::string attValue = attr->Value();

				if (attName == "opened")
					m_Pane_Shown = (PaneFlags)ct::ivariant(attValue).GetI();
				if (attName == "active")
					m_Pane_Focused = (PaneFlags)ct::ivariant(attValue).GetI();
			}
		}
	}
	else if (vUserDatas == "project")
	{

	}

	return true;
}