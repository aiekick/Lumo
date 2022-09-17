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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "View2DPane.h"

#include <Panes/Manager/LayoutManager.h>
#include <ImWidgets/ImWidgets.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <vkFramework/VulkanImGuiRenderer.h>
#include <Project/ProjectFile.h>
#include <imgui/imgui_internal.h>
#include <Interfaces/TextureOutputInterface.h>
#include <Graph/Base/BaseNode.h>
#include <Graph/Base/NodeSlot.h>
#include <Graph/Manager/NodeManager.h>
#include <Systems/CommonSystem.h>
#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <cinttypes> // printf zu

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

static int SourcePane_WidgetId = 0;

///////////////////////////////////////////////////////////////////////////////////
//// CONSTRUCTORS /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

View2DPane::View2DPane() = default;
View2DPane::~View2DPane()
{
	Unit();
}

///////////////////////////////////////////////////////////////////////////////////
//// INIT/UNIT ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool View2DPane::Init()
{
	ZoneScoped;

	return true;
}

void View2DPane::Unit()
{
	ZoneScoped;

	m_TextureOutputSlot.reset();
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

int View2DPane::DrawPanes(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas, PaneFlags& vInOutPaneShown)
{
	ZoneScoped;

	SourcePane_WidgetId = vWidgetId;

	if (vInOutPaneShown & m_PaneFlag)
	{
		static ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_MenuBar;
		if (ImGui::Begin<PaneFlags>(m_PaneName,
			&vInOutPaneShown , m_PaneFlag, flags))
		{
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
			auto win = ImGui::GetCurrentWindowRead();
			if (win->Viewport->Idx != 0)
				flags |= ImGuiWindowFlags_NoResize;// | ImGuiWindowFlags_NoTitleBar;
			else
				flags = ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoBringToFrontOnFocus |
				ImGuiWindowFlags_MenuBar;
#endif
			if (ProjectFile::Instance()->IsLoaded())
			{
				auto outputSize = SetOrUpdateOutput(m_TextureOutputSlot);

				auto slotPtr = m_TextureOutputSlot.getValidShared();
				if (slotPtr)
				{
					if (ImGui::BeginMenuBar())
					{
						ImGui::RadioButtonLabeled(0.0f, "Mouse", "Can Use Mouse\nBut not when camera is active", &m_CanWeTuneMouse);
						if (ImGui::ContrastedButton("Leave"))
						{
							SetOrUpdateOutput(NodeSlotWeak());
						}

						ImGui::EndMenuBar();
					}

					ct::ivec2 maxSize = ImGui::GetContentRegionAvail();

					if (m_ImGuiTexture.canDisplayPreview)
					{
						m_PreviewRect = ct::GetScreenRectWithRatio<int32_t>(m_ImGuiTexture.ratio, maxSize, false);

						ImVec2 pos = ImVec2((float)m_PreviewRect.x, (float)m_PreviewRect.y);
						ImVec2 siz = ImVec2((float)m_PreviewRect.w, (float)m_PreviewRect.h);

						// faut faire ca avant le dessin de la texture.
						// apres, GetCursorScreenPos ne donnera pas la meme valeur
						ImVec2 org = ImGui::GetCursorScreenPos() + pos;
						ImGui::ImageRect((ImTextureID)&m_ImGuiTexture.descriptor, pos, siz);

						if (ImGui::IsWindowHovered())
						{
							if (ImGui::IsMouseHoveringRect(org, org + siz))
							{
								if (m_CanWeTuneMouse && CanUpdateMouse(true, 0))
								{
									ct::fvec2 norPos = (ImGui::GetMousePos() - org) / siz;
									CommonSystem::Instance()->SetMousePos(norPos, outputSize, GImGui->IO.MouseDown);
								}
							}
						}
					}
				}
			}
		}

		ImGui::End();
	}

	return SourcePane_WidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// DIALOGS //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void View2DPane::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, std::string vUserDatas)
{
	ZoneScoped;

	/*if (ProjectFile::Instance()->IsLoaded())
	{
		ImVec2 maxSize = MainFrame::Instance()->m_DisplaySize;
		ImVec2 minSize = maxSize * 0.5f;
		if (ImGuiFileDialog::Instance()->Display("OpenShaderCode",
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking,
			minSize, maxSize))
		{
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				auto code = FileHelper::Instance()->LoadFileToString(filePathName);
				SetCode(code);
			}
			ImGuiFileDialog::Instance()->Close();
		}
	}*/
}

int View2DPane::DrawWidgets(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas)
{
	return vWidgetId;
}

ct::fvec2 View2DPane::SetOrUpdateOutput(NodeSlotWeak vTextureOutputSlot)
{
	ZoneScoped;

	ct::fvec2 outSize;

	auto slotPtr = vTextureOutputSlot.getValidShared();
	if (slotPtr)
	{
		auto otherNodePtr = std::dynamic_pointer_cast<TextureOutputInterface>(slotPtr->parentNode.getValidShared());
		if (otherNodePtr)
		{
			NodeSlot::sSlotGraphOutputMouseMiddle = vTextureOutputSlot;
			m_TextureOutputSlot = vTextureOutputSlot;
			NodeManager::Instance()->m_RootNodePtr->m_GraphRoot2DNode = slotPtr->parentNode;
			m_ImGuiTexture.SetDescriptor(m_VulkanImGuiRenderer, otherNodePtr->GetDescriptorImageInfo(slotPtr->descriptorBinding, &outSize));
			m_ImGuiTexture.ratio = outSize.ratioXY<float>();
		}
		else
		{
			NodeSlot::sSlotGraphOutputMouseMiddle.reset();
			m_TextureOutputSlot.reset();
			m_ImGuiTexture.ClearDescriptor();
		}
	}
	else
	{
		NodeSlot::sSlotGraphOutputMouseMiddle.reset();
		m_TextureOutputSlot.reset();
		m_ImGuiTexture.ClearDescriptor();
	}

	return outSize;
}

void View2DPane::SetVulkanImGuiRenderer(VulkanImGuiRendererWeak vVulkanImGuiRenderer)
{
	m_VulkanImGuiRenderer = vVulkanImGuiRenderer;
}

bool View2DPane::CanUpdateMouse(bool vWithMouseDown, int vMouseButton)
{
	ZoneScoped;

	bool canUpdateMouse = true;
	//(!MainFrame::g_AnyWindowsHovered) &&
	//(ImGui::GetActiveID() == 0) &&
	//(ImGui::GetHoveredID() == 0);// &&
	//(!ImGui::IsWindowHovered()) &&
	//(!ImGuiFileDialog::Instance()->m_AnyWindowsHovered);
	//if (MainFrame::g_CentralWindowHovered && (ImGui::GetActiveID() != 0) && !ImGuizmo::IsUsing())
	//	canUpdateMouse = true;
	if (vWithMouseDown)
	{
		static bool lastMouseDownState[3] = { false, false, false };
		canUpdateMouse &= lastMouseDownState[vMouseButton] || ImGui::IsMouseDown(vMouseButton);
		lastMouseDownState[vMouseButton] = ImGui::IsMouseDown(vMouseButton);
	}
	return canUpdateMouse;
}
