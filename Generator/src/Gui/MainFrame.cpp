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
#include <Graph/Layout/GraphLayout.h>
#include <Project/ProjectFile.h>

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
	m_VulkanCorePtr = vVulkanCorePtr;

	SetAppTitle();

	ThemeHelper::Instance(); // default
	LoadConfigFile("generatorConfig.xml");

	ThemeHelper::Instance()->ApplyStyle();

	NodeSlot::sGetSlotColors()->AddSlotColor("NONE", ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("MESH", ImVec4(0.5f, 0.5f, 0.9f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("MESH_GROUP", ImVec4(0.1f, 0.1f, 0.8f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("LIGHT_GROUP", ImVec4(0.9f, 0.9f, 0.1f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("ENVIRONMENT", ImVec4(0.1f, 0.9f, 0.1f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("MERGED", ImVec4(0.1f, 0.5f, 0.9f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("TEXTURE_2D", ImVec4(0.9f, 0.5f, 0.1f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("TEXTURE_2D_GROUP", ImVec4(0.2f, 0.9f, 0.2f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("TEXTURE_3D", ImVec4(0.9f, 0.8f, 0.3f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("MIXED", ImVec4(0.3f, 0.5f, 0.1f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("WIDGET_BOOLEAN", ImVec4(0.8f, 0.7f, 0.6f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("WIDGET_UINT", ImVec4(0.8f, 0.7f, 0.6f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("WIDGET_INT", ImVec4(0.8f, 0.7f, 0.6f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("WIDGET_FLOAT", ImVec4(0.8f, 0.7f, 0.6f, 1.0f));
	NodeSlot::sGetSlotColors()->AddSlotColor("DEPTH", ImVec4(0.2f, 0.7f, 0.6f, 1.0f));

	using namespace std::placeholders;
	BaseNode::sSelectCallback = std::bind(&MainFrame::SelectNode, this, _1);
	BaseNode::sSelectSlotCallback = std::bind(&MainFrame::SelectSlot, this, _1, _2);
	NodeSlot::sSlotGraphOutputMouseLeftColor = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);
	NodeSlot::sSlotGraphOutputMouseRightColor = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);

	m_RootNodePtr = BaseNode::Create(vVulkanCorePtr);
	if (m_RootNodePtr && m_VulkanCorePtr)
	{
		return true;
	}

	return false;
}

void MainFrame::Unit()
{
	ProjectFile::Instance()->Save();
	SaveConfigFile("generatorConfig.xml");
}

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
		ProjectFile::Instance()->Clear();
		m_NeedToCloseProject = false;
	}
}

//////////////////////////////////////////////////////////////////////////////
//// DRAW ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void MainFrame::Display(const uint32_t& vCurrentFrame, ct::ivec4 vViewport)
{
	if (m_NeedToApplyLayout && m_RootNodePtr)
	{
		m_NeedToApplyLayout = false;
		GraphLayout::Instance()->ApplyLayout(m_RootNodePtr);
	}

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
			ImGuiWindowFlags_MenuBar |
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
	if (ImGuiFileDialog::Instance()->Display("GenerateToPath"))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

			m_NodeGenerator.GenerateNodeClasses(filePath, m_GeneratorDatas);
		}

		ImGuiFileDialog::Instance()->Close();
	}
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
	if (ImGui::BeginTabBar("##tools"))
	{
		/*if (ImGui::BeginTabItem("Plugin Creation"))
		{
		

			ImGui::EndTabItem();
		}*/

		if (ImGui::BeginTabItem("Node Creation"))
		{
			DrawNodeCreationPane();

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

void MainFrame::DrawGraph()
{
	if (m_RootNodePtr)
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::MenuItem("Layout", "apply Layout"))
			{
				GraphLayout::Instance()->ApplyLayout(m_RootNodePtr);
			}

			m_RootNodePtr->DrawToolMenu();

			if (m_RootNodePtr->m_BaseNodeState.m_NodeGraphContext)
			{
				nd::SetCurrentEditor(m_RootNodePtr->m_BaseNodeState.m_NodeGraphContext);
				if (nd::GetSelectedObjectCount())
				{
					if (ImGui::BeginMenu("Selection"))
					{
						if (ImGui::MenuItem("Zoom on Selection"))
						{
							m_RootNodePtr->ZoomToSelection();
						}

						if (ImGui::MenuItem("Center on Selection"))
						{
							m_RootNodePtr->NavigateToSelection();
						}

						ImGui::EndMenu();
					}
				}

				if (ImGui::BeginMenu("Content"))
				{
					if (ImGui::MenuItem("Zoom on Content"))
					{
						m_RootNodePtr->ZoomToContent();
					}

					if (ImGui::MenuItem("Center on Content"))
					{
						m_RootNodePtr->NavigateToContent();
					}

					ImGui::EndMenu();
				}
			}

			if (ImGui::BeginMenu("Style"))
			{
				m_RootNodePtr->DrawStyleMenu();
				GraphLayout::Instance()->DrawSettings();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		m_RootNodePtr->DrawGraph();
	}
}

void MainFrame::DrawPluginCreationPane()
{

}

void MainFrame::DrawNodeCreationPane()
{
	const float aw = ImGui::GetContentRegionAvail().x;

	if (ImGui::ContrastedButton("New Node"))
	{
		m_RootNodePtr->ClearGraph();
		m_GeneratorDatas.m_SelectedNode = m_RootNodePtr->AddChildNode(BaseNode::Create(m_VulkanCorePtr));
		auto nodePtr = m_GeneratorDatas.m_SelectedNode.getValidShared();
		if (nodePtr)
		{
			nodePtr->name = "New Node";
		}
		m_NeedToApplyLayout = true;
	}

	auto nodePtr = m_GeneratorDatas.m_SelectedNode.getValidShared();
	if (nodePtr)
	{
		ImGui::SameLine();

		if (ImGui::ContrastedButton("Delete the Node"))
		{
			m_RootNodePtr->DestroyChildNode(m_GeneratorDatas.m_SelectedNode);
		}

		if (m_NodeDisplayNameInputText.DisplayInputText(aw * 0.5f, "Node Display Name :", "New Node"))
		{
			m_GeneratorDatas.m_NodeDisplayName = m_NodeDisplayNameInputText.GetText();
			nodePtr->name = m_GeneratorDatas.m_NodeDisplayName;
		}

		if (m_NodeCreationNameInputText.DisplayInputText(aw * 0.5f, "Node Creation Name :", "NEW_NODE"))
		{
			m_GeneratorDatas.m_NodeCreationName = m_NodeCreationNameInputText.GetText();
			ct::replaceString(m_GeneratorDatas.m_ClassName, " ", "_");
			m_NodeCreationNameInputText.SetText(m_GeneratorDatas.m_NodeCreationName);
		}

		ImGui::Separator();

		m_SelectedNodeSlotInput = std::dynamic_pointer_cast<NodeSlotInput>(
			m_InputSlotEditor.DrawSlotCreationPane(ImVec2(aw * 0.5f, ImGui::GetFrameHeight() * 7.0f), 
				m_GeneratorDatas.m_SelectedNode,
				m_SelectedNodeSlotInput,
				NodeSlot::PlaceEnum::INPUT).getValidShared());

		ImGui::SameLine();

		m_SelectedNodeSlotOutput = std::dynamic_pointer_cast<NodeSlotOutput>(
			m_OutputSlotEditor.DrawSlotCreationPane(ImVec2(aw * 0.5f, ImGui::GetFrameHeight() * 7.0f),
				m_GeneratorDatas.m_SelectedNode,
				m_SelectedNodeSlotOutput,
				NodeSlot::PlaceEnum::OUTPUT).getValidShared());

		ImGui::Separator();

		ImGui::Text("Classes");

		if (m_ClassNameInputText.DisplayInputText(aw * 0.5f, "Name :", "NewClass"))
		{
			m_GeneratorDatas.m_ClassName = m_ClassNameInputText.GetText();
			ct::replaceString(m_GeneratorDatas.m_ClassName, " ", "");
			m_ClassNameInputText.SetText(m_GeneratorDatas.m_ClassName);
		}

		ImGui::Separator();

		ImGui::CheckBoxBoolDefault("Generate a Module ?", &m_GeneratorDatas.m_GenerateAModule, true);

		if (m_GeneratorDatas.m_GenerateAModule)
		{
			ImGui::Text("Renderer Type");

			if (ImGui::RadioButtonLabeled(0.0f, "Pixel 2D", m_GeneratorDatas.m_RendererType == "Pixel 2D", false))
				m_GeneratorDatas.m_RendererType = "Pixel 2D";
			ImGui::SameLine();
			if (ImGui::RadioButtonLabeled(0.0f, "Compute 1D", m_GeneratorDatas.m_RendererType == "Compute 1D", false))
				m_GeneratorDatas.m_RendererType = "Compute 1D";
			ImGui::SameLine();
			if (ImGui::RadioButtonLabeled(0.0f, "Compute 2D", m_GeneratorDatas.m_RendererType == "Compute 2D", false))
				m_GeneratorDatas.m_RendererType = "Compute 2D";
			ImGui::SameLine();
			if (ImGui::RadioButtonLabeled(0.0f, "Compute 3D", m_GeneratorDatas.m_RendererType == "Compute 3D", false))
				m_GeneratorDatas.m_RendererType = "Compute 3D";
			ImGui::SameLine();
			if (ImGui::RadioButtonLabeled(0.0f, "Rtx", m_GeneratorDatas.m_RendererType == "Rtx", false))
				m_GeneratorDatas.m_RendererType = "Rtx";
		}


		ImGui::CheckBoxBoolDefault("Generate a Pass ?", &m_GeneratorDatas.m_GenerateAPass, true);

		if (ImGui::ContrastedButton("Generate"))
		{
			ImGuiFileDialog::Instance()->OpenDialog("GenerateToPath", "Generate To Path", nullptr, ".");
		}
	}
}

void MainFrame::SelectNode(const BaseNodeWeak& vNode)
{
	m_GeneratorDatas.m_SelectedNode = vNode;
}

void MainFrame::SelectSlot(const NodeSlotWeak& vSlot, const ImGuiMouseButton& vButton)
{
	if (m_RootNodePtr)
	{
		if (vButton == ImGuiMouseButton_Left)
		{
			auto slotPtr = vSlot.getValidShared();
			if (slotPtr)
			{
				if (slotPtr->IsAnInput())
				{
					m_SelectedNodeSlotInput = std::dynamic_pointer_cast<NodeSlotInput>(vSlot.getValidShared());
					NodeSlot::sSlotGraphOutputMouseLeft = m_SelectedNodeSlotInput;
					m_InputSlotEditor.SelectSlot(m_SelectedNodeSlotInput);
				}
				else if (slotPtr->IsAnOutput())
				{
					m_SelectedNodeSlotOutput = std::dynamic_pointer_cast<NodeSlotOutput>(vSlot.getValidShared());
					NodeSlot::sSlotGraphOutputMouseRight = m_SelectedNodeSlotOutput;
					m_OutputSlotEditor.SelectSlot(m_SelectedNodeSlotInput);
				}
			}
		}
		else if (vButton == ImGuiMouseButton_Middle)
		{

		}
		else if (vButton == ImGuiMouseButton_Right)
		{

		}
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

	if (strName == "bookmarks")
		ImGuiFileDialog::Instance()->DeserializeBookmarks(strValue);
	else if (strName == "project")
	{
		m_NeedToLoadProject = true;
		m_FilePathNameToLoad = strValue;
	}
	else if (strName == "showaboutdialog")
		m_ShowAboutDialog = ct::ivariant(strValue).GetB();
	else if (strName == "showimgui")
		m_ShowImGui = ct::ivariant(strValue).GetB();
	else if (strName == "showmetric")
		m_ShowMetric = ct::ivariant(strValue).GetB();

	return true;
}