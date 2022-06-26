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

#include "MainToolBar.h"

#include <FontIcons/CustomFontToolBar.cpp>
#include <FontIcons/CustomFontToolBar.h>
#include <ImWidgets/ImWidgets.h>

#include <Panes/Manager/LayoutManager.h>
#include <Gui/MainFrame.h>
#include <imgui/imgui.h>
#ifndef IMGUI_DEFINE_MATH_OPERATORS

#endif
#include <imgui/imgui_internal.h>

ImFont* MainToolBar::puFont = nullptr;

bool MainToolBar::Init()
{
	static ImFontConfig icons_config3; icons_config3.MergeMode = false; icons_config3.PixelSnapH = true;
	static const ImWchar icons_ranges3[] = { ICON_MIN_NDPTB, ICON_MAX_NDPTB, 0 };
	MainToolBar::puFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_NDPTB, 20.0f, &icons_config3, icons_ranges3);
	return (MainToolBar::puFont != nullptr);
}

void MainToolBar::Unit()
{
}

void MainToolBar::DrawToolBar()
{
	/*
	auto NeedOneFrameUpdate = false;

	if (MainToolBar::puFont)
	{
		if (ImGui::BeginLeftToolBar(40.0f))
		{
			const auto aw = ImGui::GetContentRegionAvail().x;

			/////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////

			const auto colBtn = ImGui::ColorButton("##BackGround", BackEnd::Instance()->puBackgroundColor, 0, ImVec2(aw, aw));
			if (colBtn)
			{
				// Store current color and open a picker
				auto& g = *GImGui;
				const auto window = ImGui::GetCurrentWindow();
				g.ColorPickerRef = BackEnd::Instance()->puBackgroundColor;
				ImGui::OpenPopup("picker");
				ImGui::SetNextWindowPos(window->DC.LastItemRect.GetBL() + ImVec2(-1, g.Style.ItemSpacing.y));
				NeedOneFrameUpdate = true;
			}
			if (ImGui::BeginPopup("picker"))
			{
				const auto picker_flags_to_forward = ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__PickerMask | ImGuiColorEditFlags__InputMask | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
				const auto picker_flags = (0 & picker_flags_to_forward) | ImGuiColorEditFlags__DisplayMask | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
				auto& g = *GImGui;
				NeedOneFrameUpdate = ImGui::ColorPicker4("##picker", &BackEnd::Instance()->puBackgroundColor.x, picker_flags, &g.ColorPickerRef.x);
				ImGui::EndPopup();
			}

			/////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////

			ImGui::RadioButtonLabeled_BitWize<PaneFlags>(
				aw, ICON_NDPTB_VIEW_LIST "##Config", "Config",
				&LayoutManager::Instance()->puPane_Shown, PANE_CONFIG,
				false, true, 0, false, MainToolBar::puFont);
			ImGui::RadioButtonLabeled_BitWize<PaneFlags>(
				aw, ICON_NDPTB_TUNE "##Uniforms", "Tuning",
				&LayoutManager::Instance()->puPane_Shown, PANE_UNIFORMS,
				false, true, 0, false, MainToolBar::puFont);
#ifdef USE_NODEGRPAH
			ImGui::RadioButtonLabeled_BitWize<PaneFlags>(
				aw, ICON_NDPTB_NODEGRAPH "##Nodes", "Nodegraph",
				&LayoutManager::Instance()->puPane_Shown, PANE_NODES,
				false, true, 0, false, MainToolBar::puFont);
#endif
			ImGui::RadioButtonLabeled_BitWize<PaneFlags>(
				aw, ICON_NDPTB_VECTOR_CURVE "##TimeLine", "TimeLine",
				&LayoutManager::Instance()->puPane_Shown, PANE_TIMELINE,
				false, true, 0, false, MainToolBar::puFont);
			ImGui::RadioButtonLabeled_BitWize<PaneFlags>(
				aw, ICON_NDPTB_LAYERS_TRIPLE "##TuningSwitcher", "Tuning Switcher",
				&LayoutManager::Instance()->puPane_Shown, PANE_CONFIG_SWITCHER,
				false, true, 0, false, MainToolBar::puFont);
			ImGui::RadioButtonLabeled_BitWize<PaneFlags>(
				aw, ICON_NDPTB_FILE_DOCUMENT_EDIT "##Code", "Code",
				&LayoutManager::Instance()->puPane_Shown, PANE_CODE,
				false, true, 0, false, MainToolBar::puFont);
			ImGui::RadioButtonLabeled_BitWize<PaneFlags>(
				aw, ICON_NDPTB_FILE_DOCUMENT_BOX "##Note", "Note",
				&LayoutManager::Instance()->puPane_Shown, PANE_INFOS,
				false, true, 0, false, MainToolBar::puFont);
#ifdef USE_HELP_IN_APP
			ImGui::RadioButtonLabeled_BitWize<PaneFlags>(
				aw, ICON_NDPTB_NFC_SEARCH_VARIANT "##Help", "Help",
				&LayoutManager::Instance()->puPane_Shown, PANE_HELP,
				false, true, 0, false, MainToolBar::puFont);
#endif
			ImGui::RadioButtonLabeled_BitWize<PaneFlags>(
				aw, ICON_NDPTB_CLIPBOARD_PULSE "##Profiler", "Profiler",
				&LayoutManager::Instance()->puPane_Shown, PANE_PROFILER,
				false, true, 0, false, MainToolBar::puFont);
#ifdef _DEBUG
			ImGui::RadioButtonLabeled_BitWize<PaneFlags>(
				aw, ICON_NDPTB_ATOM_VARIANT "##Inspector", "Inspector",
				&LayoutManager::Instance()->puPane_Shown, PANE_INSPECTOR,
				false, true, 0, false, MainToolBar::puFont);
#endif
#ifdef USE_OPTIMIZER_SYSTEM
			if (ImGui::RadioButtonLabeled_BitWize<PaneFlags>(
				aw, ICON_NDPTB_CLIPBOARD_PULSE "##Opti", "Optimzer",
				&LayoutManager::Instance()->puPane_Shown, PANE_OPTIMIZER))
			{
				if (LayoutManager::Instance()->puPane_Shown & PANE_OPTIMIZER)
				{
					OptimizerSystem::Instance()->InitView(BackEnd::Instance()->puDisplay_RenderPack);
				}
			}
#endif
			MessagePane::Instance()->DrawToolBarButtons(aw, MainToolBar::puFont);

			ImGui::Separator();

			/////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////

			ImGui::RadioButtonLabeled(aw, ICON_NDPTB_CAMCORDER "##Camera", "Camera", &BackEnd::Instance()->puCanWeTuneCamera, false, MainToolBar::puFont);
			ImGui::RadioButtonLabeled(aw, ICON_NDPTB_MOUSE "##Mouse", "Mouse", &BackEnd::Instance()->puCanWeTuneMouse, false, MainToolBar::puFont);
			if (ImGui::RadioButtonLabeled(aw, ICON_NDPTB_AXIS_ARROW "##Gizmo", "Gizmo", &GizmoSystem::Instance()->puActivated, false, MainToolBar::puFont))
			{
				GizmoSystem::Instance()->SetActivation(GizmoSystem::Instance()->puActivated);
				NeedOneFrameUpdate = true;
			}
			NeedOneFrameUpdate |= ImGui::RadioButtonLabeled(aw, ICON_NDPTB_GOOGLE_CONTROLLER "##GamePad", "GamePad", &GamePadSystem::Instance()->puActivated, false, MainToolBar::puFont);

			BackEnd::Instance()->NeedRefresh(NeedOneFrameUpdate);

			/////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////

			MainFrame::sAnyWindowsHovered |= ImGui::IsWindowHovered();

			ImGui::EndLeftToolBar();
		}
	}
	*/
}