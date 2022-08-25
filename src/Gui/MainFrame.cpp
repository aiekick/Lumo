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

#include <Headers/LumoBuild.h>

#include <Gui/MainFrame.h>
#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <cctype>
#include <GLFW/glfw3.h>
#include <Helper/Messaging.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <ImWidgets/ImWidgets.h>
#include <Panes/Manager/LayoutManager.h>
#include <Helper/ThemeHelper.h>
#include <Project/ProjectFile.h>
#include <FontIcons/CustomFont.h>
#include <FontIcons/CustomFont.h>
#include <Graph/Manager/NodeManager.h>

#include <Panes/DebugPane.h>
#include <Panes/View2DPane.h>
#include <Panes/View3DPane.h>
#include <Panes/ScenePane.h>
#include <Panes/TuningPane.h>
#include <Panes/GraphPane.h>
#include <Panes/ProfilerPane.h>
#include <Panes/CodePane.h>

#include <imgui/imgui_internal.h>

#define USE_PROFILER

MainFrame::MainFrame(GLFWwindow* vWin)
{
	m_Window = vWin;
}

MainFrame::~MainFrame()
{
}

void MainFrame::Init()
{
	SetAppTitle();

	DebugPane::Instance()->Init();
	View2DPane::Instance()->Init();
	View3DPane::Instance()->Init();
	TuningPane::Instance()->Init();
	GraphPane::Instance()->Init();
	ScenePane::Instance()->Init();
	CodePane::Instance()->Init();
#ifdef USE_PROFILER
	ProfilerPane::Instance()->Init();
#endif

	ThemeHelper::Instance(); // default
	LoadConfigFile("config.xml");

	ThemeHelper::Instance()->ApplyStyle();
	LayoutManager::Instance()->Init("Layouts", "Default Layout");

	LayoutManager::Instance()->SetPaneDisposalSize(PaneDisposal::LEFT, 300.0f);
	LayoutManager::Instance()->SetPaneDisposalSize(PaneDisposal::RIGHT, 200.0f);

	LayoutManager::Instance()->AddPane(DebugPane::Instance(), "Debug", (1 << 1), PaneDisposal::RIGHT, false, false);
	LayoutManager::Instance()->AddPane(ScenePane::Instance(), "Scene", (1 << 2), PaneDisposal::RIGHT, false, false);

	LayoutManager::Instance()->AddPane(GraphPane::Instance(), "Graph", (1 << 3), PaneDisposal::CENTRAL, true, false);
	LayoutManager::Instance()->AddPane(TuningPane::Instance(), "Tuning", (1 << 4), PaneDisposal::RIGHT, true, false);
	LayoutManager::Instance()->AddPane(View3DPane::Instance(), "View3D", (1 << 5), PaneDisposal::LEFT, true, true);
	LayoutManager::Instance()->AddPane(View2DPane::Instance(), "View2D", (1 << 6), PaneDisposal::LEFT, false, false);
	LayoutManager::Instance()->AddPane(CodePane::Instance(), "Code", (1 << 7), PaneDisposal::RIGHT, false, false);

#ifdef USE_PROFILER
	LayoutManager::Instance()->AddPane(ProfilerPane::Instance(), "Profiler", (1 << 8), PaneDisposal::TOP, true, false);
#endif

	using namespace std::placeholders;
	BaseNode::sSelectCallback = std::bind(&MainFrame::SelectNode, this, _1);
	BaseNode::sSelectForGraphOutputCallback = std::bind(&MainFrame::SelectNodeForGraphOutput, this, _1, _2);
	NodeSlot::sSlotGraphOutputMouseLeftColor = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);
	NodeSlot::sSlotGraphOutputMouseMiddleColor = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);
}

void MainFrame::Unit()
{
	SaveProject();

	SaveConfigFile("config.xml");

	DebugPane::Instance()->Unit();
	View2DPane::Instance()->Unit();
	View3DPane::Instance()->Unit();
	TuningPane::Instance()->Unit();
	GraphPane::Instance()->Unit();
	ScenePane::Instance()->Unit();
	CodePane::Instance()->Unit();

#ifdef USE_PROFILER
	ProfilerPane::Instance()->Unit();
#endif
}

void MainFrame::SelectNode(const BaseNodeWeak& vNode)
{
	TuningPane::Instance()->Select(vNode);
	DebugPane::Instance()->Select(vNode);
}

void MainFrame::SelectNodeForGraphOutput(const NodeSlotWeak& vSlot, const ImGuiMouseButton& vButton)
{
	if (NodeManager::Instance()->m_RootNodePtr)
	{
		if (vButton == ImGuiMouseButton_Left)
		{
			View3DPane::Instance()->SetOrUpdateOutput(vSlot);
		}
		else if (vButton == ImGuiMouseButton_Middle)
		{
			View2DPane::Instance()->SetOrUpdateOutput(vSlot);
		}
		else if (vButton == ImGuiMouseButton_Right)
		{

		}
	}
}

void MainFrame::NeedToNewProject(const std::string& vFilePathName)
{
	m_NeedToNewProject = true;
	m_FilePathNameToLoad = vFilePathName;
}

void MainFrame::NeedToLoadProject(const std::string& vFilePathName)
{
	m_NeedToLoadProject = true;
	m_FilePathNameToLoad = vFilePathName;
}

void MainFrame::NeedToCloseProject()
{
	m_NeedToCloseProject = true;
}

bool MainFrame::SaveProject()
{
	return ProjectFile::Instance()->Save();
}

void MainFrame::SaveAsProject(const std::string& vFilePathName)
{
	ProjectFile::Instance()->SaveAs(vFilePathName);

	if (m_NeedToCloseApp)
	{
		glfwSetWindowShouldClose(m_Window, 1); // close app
	}
}

// actions to do after rendering
void MainFrame::PostRenderingActions()
{
	if (m_NeedToNewProject)
	{
		ProjectFile::Instance()->New(m_FilePathNameToLoad);
		SetAppTitle(m_FilePathNameToLoad);

		m_FilePathNameToLoad.clear();
		m_NeedToNewProject = false;
	}

	if (m_NeedToLoadProject)
	{
		if (ProjectFile::Instance()->LoadAs(m_FilePathNameToLoad))
		{
			SetAppTitle(m_FilePathNameToLoad);
			ProjectFile::Instance()->SetProjectChange(false);
		}
		else
		{
			Messaging::Instance()->AddError(true, nullptr, nullptr,
				"Failed to load project %s", m_FilePathNameToLoad.c_str());
		}

		m_FilePathNameToLoad.clear();
		m_NeedToLoadProject = false;
	}

	if (m_NeedToCloseProject)
	{
		NodeManager::Instance()->Clear();
		ProjectFile::Instance()->Clear();
		m_NeedToCloseProject = false;
	}
}

//////////////////////////////////////////////////////////////////////////////
//// DRAW ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void MainFrame::Display(const uint32_t& vCurrentFrame, ct::ivec4 vViewport)
{
	m_DisplayPos = ImVec2((float)vViewport.x, (float)vViewport.y);
	m_DisplaySize = ImVec2((float)vViewport.z, (float)vViewport.w);

	const auto context = ImGui::GetCurrentContext();
	if (context)
	{
		if (ImGui::BeginMainMenuBar())
		{
			DrawMainMenuBar();

			// ImGui Infos
			auto io = ImGui::GetIO();
			const auto label = ct::toStr("Dear ImGui %s (Docking)", ImGui::GetVersion());
			const auto size = ImGui::CalcTextSize(label.c_str());
			ImGui::Spacing(ImGui::GetContentRegionAvail().x - size.x - ImGui::GetStyle().FramePadding.x * 2.0f);
			ImGui::Text("%s", label.c_str());

			//MainFrame::sAnyWindowsHovered |= ImGui::IsWindowHovered();

			ImGui::EndMainMenuBar();
		}

		if (ImGui::BeginMainStatusBar())
		{
			Messaging::Instance()->Draw();

			// ImGui Infos
			const auto io = ImGui::GetIO();
			const auto fps = ct::toStr("%.1f ms/frame (%.1f fps)", 1000.0f / io.Framerate, io.Framerate);
			const auto size = ImGui::CalcTextSize(fps.c_str());
			ImGui::Spacing(ImGui::GetContentRegionAvail().x - size.x - ImGui::GetStyle().FramePadding.x * 2.0f);
			ImGui::Text("%s", fps.c_str());

			//MainFrame::sAnyWindowsHovered |= ImGui::IsWindowHovered();

			ImGui::EndMainStatusBar();
		}

		if (LayoutManager::Instance()->BeginDockSpace(ImGuiDockNodeFlags_PassthruCentralNode))
		{
			//MainFrame::sCentralWindowHovered |= LayoutManager::Instance()->IsDockSpaceHoleHovered();

			LayoutManager::Instance()->EndDockSpace();
		}

		ImGui::CustomStyle::Instance()->pushId = LayoutManager::Instance()->DisplayPanes(vCurrentFrame, ImGui::CustomStyle::Instance()->pushId);

		DisplayDialogsAndPopups(vCurrentFrame);

		ThemeHelper::Instance()->Draw();
		LayoutManager::Instance()->InitAfterFirstDisplay(ImVec2((float)vViewport.z, (float)vViewport.w));
	}
}

void MainFrame::DrawMainMenuBar()
{
	if (ImGui::BeginMenu(ICON_NDP_PROJECT " Project"))
	{
		if (ImGui::MenuItem(ICON_NDP_FILE " New"))
		{
			Action_Menu_NewProject();
		}

		if (ImGui::MenuItem(ICON_NDP_FOLDER_OPEN " Open"))
		{
			Action_Menu_OpenProject();
		}

		if (ProjectFile::Instance()->IsLoaded())
		{
			ImGui::Separator();

			if (ImGui::MenuItem(ICON_NDP_FOLDER_OPEN " Re Open"))
			{
				Action_Menu_ReOpenProject();
			}

			ImGui::Separator();

			if (ImGui::MenuItem(ICON_NDP_SAVE " Save"))
			{
				Action_Menu_SaveProject();
			}

			if (ImGui::MenuItem(ICON_NDP_SAVE " Save As"))
			{
				Action_Menu_SaveAsProject();
			}

			ImGui::Separator();

			if (ImGui::MenuItem(ICON_NDP_DESTROY " Close"))
			{
				Action_Menu_CloseProject();
			}
		}

		ImGui::Separator();

		if (ImGui::MenuItem(ICON_NDP_QUESTION " About"))
		{
			m_ShowAboutDialog = true;
		}

		ImGui::EndMenu();
	}

	ImGui::Spacing();

	LayoutManager::Instance()->DisplayMenu(m_DisplaySize);

	ImGui::Spacing();

	if (ImGui::BeginMenu(ICON_NDP_COGS " Settings"))
	{
		if (ImGui::MenuItem("Settings"))
		{
			//SettingsDlg::Instance()->OpenDialog();
		}

		if (ImGui::BeginMenu(ICON_NDP_PICTURE_O " Styles"))
		{
			ThemeHelper::Instance()->DrawMenu();

			ImGui::Separator();

			ImGui::MenuItem("Show ImGui", "", &m_ShowImGui);
			ImGui::MenuItem("Show ImGui Metric/Debug", "", &m_ShowMetric);

			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}

	if (ProjectFile::Instance()->IsThereAnyNotSavedChanged())
	{
		ImGui::Spacing(200.0f);

		if (ImGui::MenuItem(ICON_NDP_SAVE " Save"))
		{
			Action_Menu_SaveProject();
		}
	}
}

void MainFrame::OpenAboutDialog()
{
	m_ShowAboutDialog = true;
}

void MainFrame::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame)
{
	m_ActionSystem.RunActions();

	if (ProjectFile::Instance()->IsLoaded())
	{
		LayoutManager::Instance()->DrawDialogsAndPopups(vCurrentFrame);

		//ImVec2 min = MainFrame::Instance()->m_DisplaySize * 0.5f;
		//ImVec2 max = MainFrame::Instance()->m_DisplaySize;
	}

	if (m_ShowImGui)
		ImGui::ShowDemoWindow(&m_ShowImGui);
	if (m_ShowMetric)
		ImGui::ShowMetricsWindow(&m_ShowMetric);
}

void MainFrame::SetAppTitle(const std::string& vFilePathName)
{
	auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
	if (ps.isOk)
	{
		char bufTitle[1024];
		snprintf(bufTitle, 1023, "Lumo Beta %s - Project : %s.lum", Lumo_BuildId, ps.name.c_str());
		glfwSetWindowTitle(m_Window, bufTitle);
	}
	else
	{
		char bufTitle[1024];
		snprintf(bufTitle, 1023, "Lumo Beta %s", Lumo_BuildId);
		glfwSetWindowTitle(m_Window, bufTitle);
	}
}

///////////////////////////////////////////////////////
//// SAVE DIALOG WHEN UN SAVED CHANGES ////////////////
///////////////////////////////////////////////////////

void MainFrame::OpenUnSavedDialog()
{
	// force close dialog if any dialog is opened
	ImGuiFileDialog::Instance()->Close();

	m_SaveDialogIfRequired = true;
}
void MainFrame::CloseUnSavedDialog()
{
	m_SaveDialogIfRequired = false;
}

bool MainFrame::ShowUnSavedDialog()
{
	bool res = false;

	if (m_SaveDialogIfRequired)
	{
		if (ProjectFile::Instance()->IsLoaded())
		{
			if (ProjectFile::Instance()->IsThereAnyNotSavedChanged())
			{
				/*
				Unsaved dialog behavior :
				-	save :
					-	insert action : save project
				-	save as :
					-	insert action : save as project
				-	continue without saving :
					-	quit unsaved dialog
				-	cancel :
					-	clear actions
				*/

				ImGui::CloseCurrentPopup();
				ImGui::OpenPopup("Do you want to save before ?");
				if (ImGui::BeginPopupModal("Do you want to save before ?", (bool*)0,
					ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking))
				{
					if (ImGui::Button("Save"))
					{
						res = Action_UnSavedDialog_SaveProject();
					}
					ImGui::SameLine();
					if (ImGui::Button("Save As"))
					{
						Action_UnSavedDialog_SaveAsProject();
					}

					if (ImGui::Button("Continue without saving"))
					{
						res = true; // quit the action
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel"))
					{
						Action_Cancel();
					}

					ImGui::EndPopup();
				}
			}
		}

		return res; // quit if true, else continue on the next frame
	}

	return true; // quit the action
}

///////////////////////////////////////////////////////
//// ACTIONS //////////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrame::Action_Menu_NewProject()
{
	/*
	new project :
	-	unsaved :
		-	add action : show unsaved dialog
		-	add action : open dialog for new project file name
	-	saved :
		-	add action : open dialog for new project file name
	*/
	m_ActionSystem.Clear();
	Action_OpenUnSavedDialog_IfNeeded();
	m_ActionSystem.Add([this]()
		{
			CloseUnSavedDialog();
			ImGuiFileDialog::Instance()->OpenDialog(
				"NewProjectDlg", "New Project File", ".lum", "", 1 , nullptr, ImGuiFileDialogFlags_Modal);
			return true;
		});
	m_ActionSystem.Add([this]()
		{
			return Display_NewProjectDialog();
		});
}

void MainFrame::Action_Menu_OpenProject()
{
	/*
	open project :
	-	unsaved :
		-	add action : show unsaved dialog
		-	add action : open project
	-	saved :
		-	add action : open project
	*/
	m_ActionSystem.Clear();
	Action_OpenUnSavedDialog_IfNeeded();
	m_ActionSystem.Add([this]()
		{
			CloseUnSavedDialog();
			ImGuiFileDialog::Instance()->OpenDialog(
				"OpenProjectDlg", "Open Project File", ".lum", "", 1, nullptr, ImGuiFileDialogFlags_Modal);
			return true;
		});
	m_ActionSystem.Add([this]()
		{
			return Display_OpenProjectDialog();
		});
}

void MainFrame::Action_Menu_ReOpenProject()
{
	/*
	re open project :
	-	unsaved :
		-	add action : show unsaved dialog
		-	add action : re open project
	-	saved :
		-	add action : re open project
	*/
	m_ActionSystem.Clear();
	Action_OpenUnSavedDialog_IfNeeded();
	m_ActionSystem.Add([this]()
		{
			NeedToLoadProject(ProjectFile::Instance()->GetProjectFilepathName());
			return true;
		});
}

void MainFrame::Action_Menu_SaveProject()
{
	/*
	save project :
	-	never saved :
		-	add action : save as project
	-	saved in a file beofre :
		-	add action : save project
	*/
	m_ActionSystem.Clear();
	m_ActionSystem.Add([this]()
		{
			if (!SaveProject())
			{
				CloseUnSavedDialog();
				ImGuiFileDialog::Instance()->OpenDialog(
					"SaveProjectDlg", "Save Project File", ".lum", "", 1, nullptr, ImGuiFileDialogFlags_Modal);
			}
			return true;
		});
	m_ActionSystem.Add([this]()
		{
			return Display_SaveProjectDialog();
		});
}

void MainFrame::Action_Menu_SaveAsProject()
{
	/*
	save as project :
	-	add action : save as project
	*/
	m_ActionSystem.Clear();
	m_ActionSystem.Add([this]()
		{
			CloseUnSavedDialog();
			ImGuiFileDialog::Instance()->OpenDialog(
				"SaveProjectDlg", "Save Project File", ".lum", "",
				1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal);
			return true;
		});
	m_ActionSystem.Add([this]()
		{
			return Display_SaveProjectDialog();
		});
}

void MainFrame::Action_Menu_CloseProject()
{
	/*
	Close project :
	-	unsaved :
		-	add action : show unsaved dialog
		-	add action : Close project
	-	saved :
		-	add action : Close project
	*/
	m_ActionSystem.Clear();
	Action_OpenUnSavedDialog_IfNeeded();
	m_ActionSystem.Add([this]()
		{
			NeedToCloseProject();
			return true;
		});
}

void MainFrame::Action_Window_CloseApp()
{
	if (m_NeedToCloseApp) return; // block next call to close app when running
/*
Close app :
-	unsaved :
	-	add action : show unsaved dialog
	-	add action : Close app
-	saved :
	-	add action : Close app
*/
	m_NeedToCloseApp = true;

	m_ActionSystem.Clear();
	Action_OpenUnSavedDialog_IfNeeded();
	m_ActionSystem.Add([this]()
		{
			glfwSetWindowShouldClose(m_Window, 1); // close app
			return true;
		});
}

void MainFrame::Action_OpenUnSavedDialog_IfNeeded()
{
	if (ProjectFile::Instance()->IsLoaded() &&
		ProjectFile::Instance()->IsThereAnyNotSavedChanged())
	{
		OpenUnSavedDialog();
		m_ActionSystem.Add([this]()
			{
				return ShowUnSavedDialog();
			});
	}
}

void MainFrame::Action_Cancel()
{
	/*
	-	cancel :
		-	clear actions
	*/
	CloseUnSavedDialog();
	m_ActionSystem.Clear();
	m_NeedToCloseApp = false;
}

bool MainFrame::Action_UnSavedDialog_SaveProject()
{
	bool res = SaveProject();
	if (!res)
	{
		m_ActionSystem.Insert([this]()
			{
				return Display_SaveProjectDialog();
			});
		m_ActionSystem.Insert([this]()
			{
				CloseUnSavedDialog();
				ImGuiFileDialog::Instance()->OpenDialog(
					"SaveProjectDlg", "Save Project File", ".lum",
					".", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal);
				return true;
			});
	}
	return res;
}

void MainFrame::Action_UnSavedDialog_SaveAsProject()
{
	m_ActionSystem.Insert([this]()
		{
			return Display_SaveProjectDialog();
		});
	m_ActionSystem.Insert([this]()
		{
			CloseUnSavedDialog();
			ImGuiFileDialog::Instance()->OpenDialog(
				"SaveProjectDlg", "Save Project File", ".lum",
				".", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite | ImGuiFileDialogFlags_Modal);
			return true;
		});
}

void MainFrame::Action_UnSavedDialog_Cancel()
{
	Action_Cancel();
}

///////////////////////////////////////////////////////
//// DIALOG FUNCS /////////////////////////////////////
///////////////////////////////////////////////////////

bool MainFrame::Display_NewProjectDialog()
{
	// need to return false to continue to be displayed next frame

	ImVec2 min = MainFrame::Instance()->m_DisplaySize * 0.5f;
	ImVec2 max = MainFrame::Instance()->m_DisplaySize;

	if (ImGuiFileDialog::Instance()->Display("NewProjectDlg",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			CloseUnSavedDialog();
			auto file = ImGuiFileDialog::Instance()->GetFilePathName();
			NeedToNewProject(file);
		}
		else // cancel
		{
			Action_Cancel(); // we interrupts all actions
		}

		ImGuiFileDialog::Instance()->Close();

		return true;
	}

	return false;
}

bool MainFrame::Display_OpenProjectDialog()
{
	// need to return false to continue to be displayed next frame

	ImVec2 min = MainFrame::Instance()->m_DisplaySize * 0.5f;
	ImVec2 max = MainFrame::Instance()->m_DisplaySize;

	if (ImGuiFileDialog::Instance()->Display("OpenProjectDlg",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			CloseUnSavedDialog();
			NeedToLoadProject(ImGuiFileDialog::Instance()->GetFilePathName());
		}
		else // cancel
		{
			Action_Cancel(); // we interrupts all actions
		}

		ImGuiFileDialog::Instance()->Close();

		return true;
	}

	return false;
}

bool MainFrame::Display_SaveProjectDialog()
{
	// need to return false to continue to be displayed next frame

	ImVec2 min = MainFrame::Instance()->m_DisplaySize * 0.5f;
	ImVec2 max = MainFrame::Instance()->m_DisplaySize;

	if (ImGuiFileDialog::Instance()->Display("SaveProjectDlg",
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, min, max))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			CloseUnSavedDialog();
			SaveAsProject(ImGuiFileDialog::Instance()->GetFilePathName());
		}
		else // cancel
		{
			Action_Cancel(); // we interrupts all actions
		}

		ImGuiFileDialog::Instance()->Close();

		return true;
	}

	return false;
}

///////////////////////////////////////////////////////
//// APP CLOSING //////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrame::IWantToCloseTheApp()
{
	Action_Window_CloseApp();
}

///////////////////////////////////////////////////////
//// DROP /////////////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrame::JustDropFiles(int count, const char** paths)
{
	assert(0);

	std::map<std::string, std::string> dicoFont;
	std::string prj;

	for (int i = 0; i < count; ++i)
	{
		// file
		auto f = std::string(paths[i]);

		// lower case
		auto f_opt = f;
		for (auto& c : f_opt)
			c = (char)std::tolower((int)c);

		// well known extention
		if (f_opt.find(".ttf") != std::string::npos			// truetype (.ttf)
			|| f_opt.find(".otf") != std::string::npos			// opentype (.otf)
			//||	f_opt.find(".ttc") != std::string::npos		// ttf/otf collection for futur (.ttc)
			)
		{
			dicoFont[f] = f;
		}
		if (f_opt.find(".lum") != std::string::npos)
		{
			prj = f;
		}
	}

	// priority to project file
	if (!prj.empty())
	{
		NeedToLoadProject(prj);
	}
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string MainFrame::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string str;

	str += ThemeHelper::Instance()->getXml(vOffset);
	str += LayoutManager::Instance()->getXml(vOffset, "app");
	str += vOffset + "<bookmarks>" + ImGuiFileDialog::Instance()->SerializeBookmarks() + "</bookmarks>\n";
	str += vOffset + "<showaboutdialog>" + (m_ShowAboutDialog ? "true" : "false") + "</showaboutdialog>\n";
	str += vOffset + "<showimgui>" + (m_ShowImGui ? "true" : "false") + "</showimgui>\n";
	str += vOffset + "<showmetric>" + (m_ShowMetric ? "true" : "false") + "</showmetric>\n";
	str += vOffset + "<project>" + ProjectFile::Instance()->GetProjectFilepathName() + "</project>\n";

	return str;
}

bool MainFrame::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != 0)
		strParentName = vParent->Value();

	ThemeHelper::Instance()->setFromXml(vElem, vParent);
	LayoutManager::Instance()->setFromXml(vElem, vParent, "app");

	if (strName == "bookmarks")
		ImGuiFileDialog::Instance()->DeserializeBookmarks(strValue);
	else if (strName == "project")
		NeedToLoadProject(strValue);
	else if (strName == "showaboutdialog")
		m_ShowAboutDialog = ct::ivariant(strValue).GetB();
	else if (strName == "showimgui")
		m_ShowImGui = ct::ivariant(strValue).GetB();
	else if (strName == "showmetric")
		m_ShowMetric = ct::ivariant(strValue).GetB();

	return true;
}