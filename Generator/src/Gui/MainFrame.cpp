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

#include <Headers/LumoCodeGeneratorBuild.h>

#include <Gui/MainFrame.h>
#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <cctype>
#include <GLFW/glfw3.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <ImWidgets/ImWidgets.h>
#include <FontIcons/CustomFont.h>
#include <FontIcons/CustomFont.h>
#include <Helper/ThemeHelper.h>
#include <Helper/Messaging.h>
#include <Graph/Base/BaseNode.h>
#include <imgui/imgui_internal.h>

MainFrame::MainFrame(GLFWwindow* vWin)
{
	m_Window = vWin;
}

MainFrame::~MainFrame()
{
	m_RootNodePtr.reset();
}

bool MainFrame::Init(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	SetAppTitle();

	ThemeHelper::Instance(); // default
	LoadConfigFile("config.xml");

	ThemeHelper::Instance()->ApplyStyle();

	m_RootNodePtr = BaseNode::Create(vVulkanCorePtr);
	if (m_RootNodePtr)
	{
		return true;
	}

	return false;
}

void MainFrame::Unit()
{
	SaveConfigFile("config.xml");
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

			auto io = ImGui::GetIO();
			const auto label = ct::toStr("Dear ImGui %s (Docking)", ImGui::GetVersion());
			const auto size = ImGui::CalcTextSize(label.c_str());
			ImGui::Spacing(ImGui::GetContentRegionAvail().x - size.x - ImGui::GetStyle().FramePadding.x * 2.0f);
			ImGui::Text("%s", label.c_str());

			ImGui::EndMainMenuBar();
		}

		if (ImGui::BeginMainStatusBar())
		{
			Messaging::Instance()->Draw();

			const auto io = ImGui::GetIO();
			const auto fps = ct::toStr("%.1f ms/frame (%.1f fps)", 1000.0f / io.Framerate, io.Framerate);
			const auto size = ImGui::CalcTextSize(fps.c_str());
			ImGui::Spacing(ImGui::GetContentRegionAvail().x - size.x - ImGui::GetStyle().FramePadding.x * 2.0f);
			ImGui::Text("%s", fps.c_str());

			ImGui::EndMainStatusBar();
		}

		ImGui::SetNextWindowPos(m_DisplayPos + ImVec2(0, ImGui::GetFrameHeight()));
		ImGui::SetNextWindowSize(ImVec2(m_DisplaySize.x * 0.5f, m_DisplaySize.y - ImGui::GetFrameHeight() * 2.0f));

		ImGui::Begin("Parameters", 0, 
			ImGuiWindowFlags_NoMove | 
			ImGuiWindowFlags_NoResize | 
			ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoSavedSettings);

		DrawContent();

		ImGui::End();

		ImGui::SetNextWindowPos(m_DisplayPos + ImVec2(m_DisplaySize.x * 0.5f, ImGui::GetFrameHeight()));
		ImGui::SetNextWindowSize(ImVec2(m_DisplaySize.x * 0.5f, m_DisplaySize.y - ImGui::GetFrameHeight() * 2.0f));

		ImGui::Begin("Graph", 0,
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoSavedSettings);

		DrawGraph();

		ImGui::End();

		DisplayDialogsAndPopups(vCurrentFrame);

		ThemeHelper::Instance()->Draw();
	}
}

void MainFrame::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame)
{
	if (m_ShowImGui)
		ImGui::ShowDemoWindow(&m_ShowImGui);
	if (m_ShowMetric)
		ImGui::ShowMetricsWindow(&m_ShowMetric);
}

void MainFrame::SetAppTitle(const std::string& vFilePathName)
{
	char bufTitle[1024];
	snprintf(bufTitle, 1023, "Lumo Code Generator Beta %s", LumoCodeGenerator_BuildId);
	glfwSetWindowTitle(m_Window, bufTitle);
}

void MainFrame::DrawMainMenuBar()
{
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
}

void MainFrame::DrawContent()
{
	const auto pos = ImGui::GetWindowPos();
	const auto size = ImGui::GetContentRegionAvail();

	ImGui::Text("Lumo Node Creation");
}

void MainFrame::DrawGraph()
{
	if (m_RootNodePtr)
	{
		m_RootNodePtr->DrawGraph();
	}
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string MainFrame::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string str;
	
	//str += vOffset + "<showmetric>" + (m_ShowMetric ? "true" : "false") + "</showmetric>\n";

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
	
	//if (strName == "project")
	//	NeedToLoadProject(strValue);

	return true;
}