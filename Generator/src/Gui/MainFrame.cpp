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
	LoadConfigFile("config.xml");

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
	SaveConfigFile("config.xml");
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
		m_SelectedNode = m_RootNodePtr->AddChildNode(BaseNode::Create(m_VulkanCorePtr));
		auto nodePtr = m_SelectedNode.getValidShared();
		if (nodePtr)
		{
			nodePtr->name = "New Node";
		}
		m_NeedToApplyLayout = true;
	}

	auto nodePtr = m_SelectedNode.getValidShared();
	if (nodePtr)
	{
		ImGui::SameLine();

		if (ImGui::ContrastedButton("Delete the Node"))
		{
			m_RootNodePtr->DestroyChildNode(m_SelectedNode);
		}

		static char s_nameBuffer[255 + 1] = "";
		ct::SetBuffer(s_nameBuffer, 255, nodePtr->name);
		ImGui::Text("Name :"); 
		ImGui::SameLine();
		ImGui::PushItemWidth(aw * 0.5f);
		if (ImGui::InputText("##Name", s_nameBuffer, 255U))
		{
			nodePtr->name = std::string(s_nameBuffer, strlen(s_nameBuffer));
		}
		ImGui::PopItemWidth();

		ImGui::Separator();

		m_SelectedNodeSlotInput = std::dynamic_pointer_cast<NodeSlotInput>(
			m_InputSlotEditor.DrawSlotCreationPane(ImVec2(aw * 0.5f, ImGui::GetFrameHeight() * 7.0f), 
				m_SelectedNode, 
				m_SelectedNodeSlotInput, 
				NodeSlot::PlaceEnum::INPUT).getValidShared());

		ImGui::SameLine();

		m_SelectedNodeSlotOutput = std::dynamic_pointer_cast<NodeSlotOutput>(
			m_OutputSlotEditor.DrawSlotCreationPane(ImVec2(aw * 0.5f, ImGui::GetFrameHeight() * 7.0f),
				m_SelectedNode, 
				m_SelectedNodeSlotOutput, 
				NodeSlot::PlaceEnum::OUTPUT).getValidShared());

		ImGui::Separator();

		ImGui::Text("Classes");

		static char s_classNameBuffer[255 + 1] = "";
		ct::SetBuffer(s_classNameBuffer, 255, m_ClassName);
		ImGui::Text("Name :");
		ImGui::SameLine();
		ImGui::PushItemWidth(aw * 0.5f);
		if (ImGui::InputText("##Name", s_classNameBuffer, 255U))
		{
			m_ClassName = std::string(s_classNameBuffer, strlen(s_classNameBuffer));
		}
		ImGui::PopItemWidth();

		ImGui::Separator();

		ImGui::CheckBoxBoolDefault("Generate a Module ?", &m_GenerateAModule, true);

		if (m_GenerateAModule)
		{
			ImGui::Text("Renderer Type");

			if (ImGui::RadioButtonLabeled(0.0f, "Pixel 2D", m_RendererType == "Pixel 2D", false))
				m_RendererType = "Pixel 2D";
			ImGui::SameLine();
			if (ImGui::RadioButtonLabeled(0.0f, "Compute 1D", m_RendererType == "Compute 1D", false))
				m_RendererType = "Compute 1D";
			ImGui::SameLine();
			if (ImGui::RadioButtonLabeled(0.0f, "Compute 2D", m_RendererType == "Compute 2D", false))
				m_RendererType = "Compute 2D";
			ImGui::SameLine();
			if (ImGui::RadioButtonLabeled(0.0f, "Compute 3D", m_RendererType == "Compute 3D", false))
				m_RendererType = "Compute 3D";
			ImGui::SameLine();
			if (ImGui::RadioButtonLabeled(0.0f, "Rtx", m_RendererType == "Rtx", false))
				m_RendererType = "Rtx";
		}


		ImGui::CheckBoxBoolDefault("Generate a Pass ?", &m_GenerateAPass, true);
	}
}

void MainFrame::SelectNode(const BaseNodeWeak& vNode)
{
	m_SelectedNode = vNode;
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
				}
				else if (slotPtr->IsAnOutput())
				{
					m_SelectedNodeSlotOutput = std::dynamic_pointer_cast<NodeSlotOutput>(vSlot.getValidShared());
					NodeSlot::sSlotGraphOutputMouseRight = m_SelectedNodeSlotOutput;
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