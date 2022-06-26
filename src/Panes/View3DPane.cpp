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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "View3DPane.h"

#include <cinttypes> // printf zu
#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <Project/ProjectFile.h>
#include <ImWidgets/ImWidgets.h>
#include <imgui/imgui_internal.h>
#include <Systems/CommonSystem.h>
#include <Graph/Manager/NodeManager.h>
#include <Panes/Manager/LayoutManager.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <vkFramework/VulkanImGuiRenderer.h>
#include <ImGuizmo/ImGuizmo.h>

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

static int SourcePane_WidgetId = 0;

///////////////////////////////////////////////////////////////////////////////////
//// CONSTRUCTORS /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

View3DPane::View3DPane() = default;
View3DPane::~View3DPane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// INIT/UNIT ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool View3DPane::Init()
{
	ZoneScoped;

	return true;
}

void View3DPane::Unit()
{
	ZoneScoped;
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

int View3DPane::DrawPanes(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas)
{
	ZoneScoped;

	SourcePane_WidgetId = vWidgetId;

	if (LayoutManager::Instance()->m_Pane_Shown & m_PaneFlag)
	{
		static ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_MenuBar;
		if (ImGui::Begin<PaneFlags>(m_PaneName,
			&LayoutManager::Instance()->m_Pane_Shown, m_PaneFlag, flags))
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
				SetOrUpdateOutput(m_OutputModule);

				auto outputModulePtr = m_OutputModule.getValidShared();
				if (outputModulePtr)
				{
					if (ImGui::BeginMenuBar())
					{
						//SceneManager::Instance()->DrawBackgroundColorMenu();

						ImGui::RadioButtonLabeled(0.0f, "Camera", "Can Use Camera\nCan be used with alt key if not selected", &m_CanWeTuneCamera);
						ImGui::RadioButtonLabeled(0.0f, "Mouse", "Can Use Mouse\nBut not when camera is active", &m_CanWeTuneMouse);
						if (ImGui::RadioButtonLabeled(0.0f, "Gizmo", "Can Use Gizmo", &m_CanWeTuneGizmo))
						{
							ImGuizmo::Enable(m_CanWeTuneGizmo);
						}
						//ImGui::RadioButtonLabeled(0.0f, "Depth", "Show epth", &m_ShowDepth);

						ImGui::EndMenuBar();
					}

					ct::ivec2 maxSize = ImGui::GetContentRegionAvail();

					ct::fvec2 outputSize = outputModulePtr->GetOutputSize();
					
					if (m_ImGuiTexture.canDisplayPreview)
					{
						m_PreviewRect = ct::GetScreenRectWithRatio<int32_t>(m_ImGuiTexture.ratio, maxSize, false);

						ImVec2 pos = ImVec2((float)m_PreviewRect.x, (float)m_PreviewRect.y);
						ImVec2 siz = ImVec2((float)m_PreviewRect.w, (float)m_PreviewRect.h);

						// faut faire ca avant le dessin de la texture.
						// apres, GetCursorScreenPos ne donnera pas la meme valeur
						ImVec2 org = ImGui::GetCursorScreenPos() + pos;
						ImGui::ImageRect((ImTextureID)&m_ImGuiTexture.descriptor, pos, siz);

						NodeManager::Instance()->DrawOverlays(vCurrentFrame, ct::frect(org, siz));

						if (ImGui::IsWindowHovered())
						{
							if (ImGui::IsMouseHoveringRect(org, org + siz))
							{
								if (m_CanWeTuneMouse && CanUpdateMouse(true, 0))
								{
									ct::fvec2 norPos = (ImGui::GetMousePos() - org) / siz;
									CommonSystem::Instance()->SetMousePos(norPos, outputSize, GImGui->IO.MouseDown);
								}

								UpdateCamera(org, siz);
							}
						}

						if (CommonSystem::Instance()->UpdateIfNeeded(ct::uvec2((uint32_t)siz.x, (uint32_t)siz.y)))
						{
							CommonSystem::Instance()->SetScreenSize(ct::uvec2((uint32_t)siz.x, (uint32_t)siz.y));
						}
					}

					// on test ca car si le vue n'est pas visible cad si maxSize.y est inferieur a 0 alors
					// on a affaire a un resize constant, car outputsize n'est jamais maj, tant que la vue n'a pas rendu
					// et ca casse le fps de l'ui, rendu son utilisation impossible ( deltatime superieur 32ms d'apres tracy sur un gtx 1050 Ti)
					if (maxSize.x > 0 && maxSize.y > 0)
					{
						if (outputSize.x != (uint32_t)maxSize.x ||
							outputSize.y != (uint32_t)maxSize.y)
						{
							outputModulePtr->NeedResize(&maxSize, nullptr);
							CommonSystem::Instance()->SetScreenSize(ct::uvec2(maxSize.x, maxSize.y));
							CommonSystem::Instance()->NeedCamChange();
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

void View3DPane::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, std::string vUserDatas)
{
	ZoneScoped;

	/*
	if (ProjectFile::Instance()->IsLoaded())
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
	}
	*/
}

int View3DPane::DrawWidgets(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas)
{
	return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void View3DPane::SetOrUpdateOutput(ct::cWeak<OutputModule> vOutputModule)
{
	ZoneScoped;

	m_OutputModule = vOutputModule;

	auto outputModulePtr = m_OutputModule.getValidShared();
	if (outputModulePtr)
	{
		m_ImGuiTexture.SetDescriptor(m_VulkanImGuiRenderer, outputModulePtr->GetDescriptorImageInfo(0U));
		m_ImGuiTexture.ratio = outputModulePtr->GetOutputSize().ratioXY<float>();
	}
	else
	{
		m_ImGuiTexture.ClearDescriptor();
	}
}

void View3DPane::SetVulkanImGuiRenderer(VulkanImGuiRenderer* vVulkanImGuiRenderer)
{ 
	m_VulkanImGuiRenderer = vVulkanImGuiRenderer; 
}

void View3DPane::SetDescriptor(vkApi::VulkanFrameBufferAttachment* vVulkanFrameBufferAttachment)
{
	m_ImGuiTexture.SetDescriptor(m_VulkanImGuiRenderer, vVulkanFrameBufferAttachment);
}

bool View3DPane::CanUpdateMouse(bool vWithMouseDown, int vMouseButton)
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

void View3DPane::UpdateCamera(ImVec2 vOrg, ImVec2 vSize)
{
	ZoneScoped;

	bool canTuneCamera = !ImGuizmo::IsUsing() && (m_CanWeTuneCamera || ImGui::IsKeyPressed(ImGuiKey_LeftAlt));

	// update mesher camera // camera of renderpack
	if (CanUpdateMouse(true, 0)) // left mouse rotate
	{
		if (canTuneCamera)// && !ImGuizmo::IsUsing())
		{
			ct::fvec2 pos = ct::fvec2(ImGui::GetMousePos()) / m_DisplayQuality;
			m_CurrNormalizedMousePos.x = (ImGui::GetMousePos().x - vOrg.x) / vSize.x;
			m_CurrNormalizedMousePos.y = (ImGui::GetMousePos().y - vOrg.y) / vSize.y;

			if (!m_MouseDrag)
				m_LastNormalizedMousePos = m_CurrNormalizedMousePos; // first diff to 0
			m_MouseDrag = true;

			ct::fvec2 diff = m_CurrNormalizedMousePos - m_LastNormalizedMousePos;
			CommonSystem::Instance()->IncRotateXYZ(ct::fvec3(diff * 6.28318f, 0.0f));

			if (!diff.emptyAND()) m_UINeedRefresh |= true;
			m_LastNormalizedMousePos = m_CurrNormalizedMousePos;
		}
	}
	else if (CanUpdateMouse(true, 1)) // right mouse zoom
	{
		if (canTuneCamera)// && !ImGuizmo::IsUsing())
		{
			ct::fvec2 pos = ct::fvec2(ImGui::GetMousePos()) / m_DisplayQuality;
			m_CurrNormalizedMousePos.x = (ImGui::GetMousePos().x - vOrg.x) / vSize.x;
			m_CurrNormalizedMousePos.y = (ImGui::GetMousePos().y - vOrg.y) / vSize.y;

			if (!m_MouseDrag)
				m_LastNormalizedMousePos = m_CurrNormalizedMousePos; // first diff to 0
			m_MouseDrag = true;

			ct::fvec2 diff = m_CurrNormalizedMousePos - m_LastNormalizedMousePos;
			CommonSystem::Instance()->IncZoom(diff.y * 50.0f);
			CommonSystem::Instance()->IncRotateXYZ(ct::fvec3(0.0f, 0.0f, diff.x * 6.28318f));

			if (!diff.emptyAND()) m_UINeedRefresh |= true;
			m_LastNormalizedMousePos = m_CurrNormalizedMousePos;
		}
	}
	else if (CanUpdateMouse(true, 2)) // middle mouse, translate
	{
		if (canTuneCamera)// && !ImGuizmo::IsUsing())
		{
			ct::fvec2 pos = ct::fvec2(ImGui::GetMousePos()) / m_DisplayQuality;
			m_CurrNormalizedMousePos.x = (ImGui::GetMousePos().x - vOrg.x) / vSize.x;
			m_CurrNormalizedMousePos.y = (ImGui::GetMousePos().y - vOrg.y) / vSize.y;

			if (!m_MouseDrag)
				m_LastNormalizedMousePos = m_CurrNormalizedMousePos; // first diff to 0
			m_MouseDrag = true;

			ct::fvec2 diff = m_CurrNormalizedMousePos - m_LastNormalizedMousePos;
			CommonSystem::Instance()->IncTranslateXY(diff * 10.0f);

			if (!diff.emptyAND()) m_UINeedRefresh |= true;
			m_LastNormalizedMousePos = m_CurrNormalizedMousePos;
		}
	}
	else
	{
		m_MouseDrag = false;
	}
}