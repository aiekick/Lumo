/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

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

#include <Panes/CodeGeneratorPane.h>
#include <Gaia/gaia.h>
#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <LumoBackend/Graph/Layout/GraphLayout.h>
#include <Modules/FilesSaver.h>

#define ICON_SDFM_SHARE u8"\uf496"

using namespace std::placeholders;

#ifdef PROFILER_INCLUDE
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif
#ifndef FrameMark
#define FrameMark
#endif

#include <cinttypes>  // printf zu

CodeGeneratorPane::CodeGeneratorPane() = default;
CodeGeneratorPane::~CodeGeneratorPane() = default;

bool CodeGeneratorPane::Init() {
    return true;
}

void CodeGeneratorPane::Unit() {
    NodeSlot::sSlotGraphOutputMouseLeft.reset();
    NodeSlot::sSlotGraphOutputMouseMiddle.reset();
    NodeSlot::sSlotGraphOutputMouseRight.reset();
}

void CodeGeneratorPane::setVulkanCore(GaiApi::VulkanCoreWeak vVulkanCoreWeak) {
    m_VulkanCore = vVulkanCoreWeak;
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool CodeGeneratorPane::DrawPanes(
    const uint32_t& vCurrentFrame, bool* vOpened, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;

    bool change = false;

    if (vOpened && *vOpened) {
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar;
        if (ImGui::Begin(GetName().c_str(), vOpened, flags)) {
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
            auto win = ImGui::GetCurrentWindowRead();
            if (win->Viewport->Idx != 0)
                flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar;
            else
                flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar;
#endif

            {
                const auto aw = ImGui::GetContentRegionAvail();

                if (ImGui::BeginChild("Parameters", ImVec2(aw.x * 0.5f, aw.y), true,    //
                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |           //
                            ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse |  //
                            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar)) {
                    DrawContent();
                }
                ImGui::EndChild();

                ImGui::SameLine();

                if (ImGui::BeginChild("Graph", ImVec2(aw.x * 0.5f, aw.y), true,         //
                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |           //
                            ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse |  //
                            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar)) {
                    DrawGraph();
                }
                ImGui::EndChild();
            }
        }

        ImGui::End();
    }

    return change;
}

///////////////////////////////////////////////////////////////////////////////////
//// DIALOGS //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool CodeGeneratorPane::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    if (ImGuiFileDialog::Instance()->Display("GenerateToPath")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            FilesSaver::Instance()->GenerateGraphFiles(  //
                m_NodeGraph.lock(),                      //
                ImGuiFileDialog::Instance()->GetCurrentPath());
            SetProjectChange(true);
        }
        ImGuiFileDialog::Instance()->Close();
    }
    return false;
}

bool CodeGeneratorPane::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    bool change = false;

    change |= CommonSystem::Instance()->DrawImGui();

    auto ptr = GetParentNode().lock();
    if (ptr) {
        change |= ptr->DrawWidgets(vCurrentFrame, ImGui::GetCurrentContext(), vUserDatas);
    }

    return change;
}

bool CodeGeneratorPane::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    UNUSED(vCurrentFrame);
    UNUSED(vRect);
    ImGui::SetCurrentContext(vContextPtr);
    UNUSED(vUserDatas);
    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//// GRAPH ////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void CodeGeneratorPane::SetNodeGraph(GeneratorNodeWeak vNodeGraph) {
    m_NodeGraph = vNodeGraph;
    auto ptr = m_NodeGraph.lock();
    if (ptr != nullptr) {
        ptr->name = "GeneratorGraph";
        ptr->m_NodeTypeString = "Graph";
        ptr->SetSelectNodeCallback(std::bind(&CodeGeneratorPane::SelectNode, this, _1));
        ptr->SetSelectSlotCallback(std::bind(&CodeGeneratorPane::SelectSlot, this, _1, _2));
        ptr->SetLoadNodeFromXMLCallback(std::bind(&CodeGeneratorPane::LoadNodeFromXML, this, _1, _2, _3, _4, _5, _6, _7));
        NodeSlot::sSlotGraphOutputMouseLeftColor = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);
        NodeSlot::sSlotGraphOutputMouseRightColor = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//// SELECTOR /////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void CodeGeneratorPane::Select(BaseNodeWeak vObjet) {
    SetParentNode(vObjet);
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE : DISPLAY's //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void CodeGeneratorPane::DrawContent() {
    if (ImGui::BeginTabBar("##tools")) {
        if (ImGui::BeginTabItem("Node Creation")) {
            DrawNodeCreationPane();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Plugin Creation")) {
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void CodeGeneratorPane::DrawGraph() {
    auto ptr = m_NodeGraph.lock();
    if (ptr) {
        bool change = false;
        if (ImGui::BeginMenuBar()) {
            if (ImGui::MenuItem("Layout", "apply Layout")) {
                GraphLayout::Instance()->ApplyLayout(ptr);
                change = true;
            }
            ptr->DrawToolMenu();
            if (ptr->m_BaseNodeState.m_NodeGraphContext) {
                nd::SetCurrentEditor(ptr->m_BaseNodeState.m_NodeGraphContext);
                if (nd::GetSelectedObjectCount()) {
                    if (ImGui::BeginMenu("Selection")) {
                        if (ImGui::MenuItem("Zoom on Selection")) {
                            ptr->ZoomToSelection();
                            change = true;
                        }
                        if (ImGui::MenuItem("Center on Selection")) {
                            ptr->NavigateToSelection();
                            change = true;
                        }
                        ImGui::EndMenu();
                    }
                }
                if (ImGui::BeginMenu("Content")) {
                    if (ImGui::MenuItem("Zoom on Content")) {
                        ptr->ZoomToContent();
                        change = true;
                    }
                    if (ImGui::MenuItem("Center on Content")) {
                        ptr->NavigateToContent();
                        change = true;
                    }
                    ImGui::EndMenu();
                }
            }
            if (ImGui::BeginMenu("Style")) {
                ptr->DrawStyleMenu();
                GraphLayout::Instance()->DrawSettings();
                change = true;
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        change |= ptr->DrawGraph();
        if (change) {
            SetProjectChange(change);
        }
        if (WasJustSaved()) {
            ptr->SetChanged(false);
        }
    }
}

void CodeGeneratorPane::DrawPluginCreationPane() {
}

void CodeGeneratorPane::DrawNodeCreationPane() {
    auto ptr = m_NodeGraph.lock();
    if (ptr) {
        const float aw = ImGui::GetContentRegionAvail().x;

        bool change = false;

        if (ImGui::ContrastedButton("Clear Graph (WARNING, cannot be canceled !)")) {
            ptr->ClearGraph();
            change = true;
        }

        if (ImGui::ContrastedButton("New Node")) {
            SelectNode(std::dynamic_pointer_cast<GeneratorNode>(  //
                ptr->AddChildNode(GeneratorNode::Create(m_VulkanCore.lock())).lock()));
            auto nodePtr = m_SelectedNode.lock();
            if (nodePtr) {
                nodePtr->name = "New Node";
            }
            m_NeedToApplyLayout = true;
            change = true;
        }

        auto nodePtr = m_SelectedNode.lock();
        if (nodePtr) {
            ImGui::SameLine();

            if (ImGui::ContrastedButton("Delete the Node")) {
                ptr->DestroyChildNode(m_SelectedNode);
                change = true;
            }

            ImGui::SameLine();

            if (ImGui::ContrastedButton("Generate Nodes")) {
                IGFD::FileDialogConfig config;
                config.flags = ImGuiFileDialogFlags_Modal;
                config.path = FilesSaver::Instance()->m_GenerationRootPath;
                ImGuiFileDialog::Instance()->OpenDialog("GenerateToPath", "Generate To Path", nullptr, config);
            }

            if (m_NodeDisplayNameInputText.DisplayInputText(aw, "Node Display Name :", "New Node")) {
                nodePtr->m_NodeDisplayName = m_NodeDisplayNameInputText.GetText();
                nodePtr->name = nodePtr->m_NodeDisplayName;

                nodePtr->m_NodeCreationName = ct::toUpper(nodePtr->m_NodeDisplayName);
                ct::replaceString(nodePtr->m_NodeCreationName, " ", "_");
                m_NodeCreationNameInputText.SetText(nodePtr->m_NodeCreationName);

                nodePtr->m_ClassName = nodePtr->m_NodeDisplayName;
                ct::replaceString(nodePtr->m_ClassName, " ", "");
                m_ClassNameInputText.SetText(nodePtr->m_ClassName);

                nodePtr->m_ModuleDisplayName = nodePtr->m_NodeDisplayName;
                m_ModuleDisplayNameInputText.SetText(nodePtr->m_ModuleDisplayName);

                nodePtr->m_ModuleXmlName = ct::toLower(nodePtr->m_NodeCreationName) + "_module";
                nodePtr->m_PassXmlName = ct::toLower(nodePtr->m_NodeCreationName) + "_pass";
                m_ModuleXmlNameInputText.SetText(nodePtr->m_ModuleXmlName);

                change = true;
            }

            if (m_NodeCreationNameInputText.DisplayInputText(aw, "Node Creation Name :", "NEW_NODE")) {
                nodePtr->m_NodeCreationName = m_NodeCreationNameInputText.GetText();
                ct::replaceString(nodePtr->m_NodeCreationName, " ", "_");
                m_NodeCreationNameInputText.SetText(nodePtr->m_NodeCreationName);
                change = true;
            }

            ImGui::Separator();

            change |= ImGui::CheckBoxBoolDefault("Is a Task ?", &nodePtr->m_IsATask, true);
            change |= ImGui::CheckBoxBoolDefault("Is an Effect ?", &nodePtr->m_IsAnEffect, false);

            ImGui::Separator();

            if (ImGui::ContrastedButton("New Custom type")) {
                m_CustomTypeInputTexts.emplace_back();
                change = true;
            }

            auto it_to_erase = m_CustomTypeInputTexts.end();
            for (auto it = m_CustomTypeInputTexts.begin(); it != m_CustomTypeInputTexts.end(); ++it) {
                if (ImGui::ContrastedButton(ICON_SDFM_SHARE)) {
                    it_to_erase = it;
                    change = true;
                }

                ImGui::SameLine();

                it->DisplayInputText(aw, "Custom Type :", "SceneCustom");
            }

            if (it_to_erase != m_CustomTypeInputTexts.end()) {
                m_CustomTypeInputTexts.erase(it_to_erase);
                change = true;
            }

            ImGui::Separator();

            m_SelectedNodeSlotInput =
                std::dynamic_pointer_cast<NodeSlotInput>(m_InputSlotEditor
                                                             .DrawSlotCreationPane(ImVec2(aw * 0.5f, ImGui::GetFrameHeight() * 10.0f), m_SelectedNode,
                                                                 m_SelectedNodeSlotInput, NodeSlot::PlaceEnum::INPUT, change)
                                                             .lock());

            ImGui::SameLine();

            m_SelectedNodeSlotOutput =
                std::dynamic_pointer_cast<NodeSlotOutput>(m_OutputSlotEditor
                                                              .DrawSlotCreationPane(ImVec2(aw * 0.5f, ImGui::GetFrameHeight() * 10.0f),
                                                                  m_SelectedNode, m_SelectedNodeSlotOutput, NodeSlot::PlaceEnum::OUTPUT, change)
                                                              .lock());

            ImGui::Separator();

            ImGui::Text("Classes");

            if (m_ClassNameInputText.DisplayInputText(aw, "Name :", "NewClass")) {
                nodePtr->m_ClassName = m_ClassNameInputText.GetText();
                ct::replaceString(nodePtr->m_ClassName, " ", "");
                m_ClassNameInputText.SetText(nodePtr->m_ClassName);
                change = true;
            }

            if (m_NodeCategoryNameInputText.DisplayInputText(aw, "Category name :", "TestNodes")) {
                nodePtr->m_CategoryName = m_NodeCategoryNameInputText.GetText();
                ct::replaceString(nodePtr->m_CategoryName, " ", "");
                m_NodeCategoryNameInputText.SetText(nodePtr->m_CategoryName);
                change = true;
            }

            ImGui::Separator();

            ImGui::CheckBoxBoolDefault("Generate a Module ?", &nodePtr->m_GenerateAModule, true);

            if (nodePtr->m_GenerateAModule) {
                if (m_ModuleDisplayNameInputText.DisplayInputText(aw, "Module Display Name :", "New Node")) {
                    nodePtr->m_ModuleDisplayName = m_ModuleDisplayNameInputText.GetText();
                    m_ModuleDisplayNameInputText.SetText(nodePtr->m_ModuleDisplayName);
                    change = true;
                }

                if (m_ModuleXmlNameInputText.DisplayInputText(aw, "Module Xml Name :", "toto_module")) {
                    nodePtr->m_ModuleXmlName = m_ModuleXmlNameInputText.GetText();
                    ct::replaceString(nodePtr->m_ModuleXmlName, " ", "");
                    m_ModuleXmlNameInputText.SetText(nodePtr->m_ModuleXmlName);
                    change = true;
                }

                ImGui::Text("Renderer Type");

                ImGui::SameLine();

                if (ImGui::RadioButtonLabeled(0.0f, RENDERER_TYPE_NONE, nodePtr->m_RendererType == RENDERER_TYPE_NONE, false)) {
                    nodePtr->m_RendererType = RENDERER_TYPE_NONE;
                    change = true;
                }
                ImGui::SameLine();
                if (ImGui::RadioButtonLabeled(0.0f, RENDERER_TYPE_PIXEL_2D, nodePtr->m_RendererType == RENDERER_TYPE_PIXEL_2D, false)) {
                    nodePtr->m_RendererType = RENDERER_TYPE_PIXEL_2D;
                    change = true;
                }
                ImGui::SameLine();
                if (ImGui::RadioButtonLabeled(0.0f, RENDERER_TYPE_COMPUTE_1D, nodePtr->m_RendererType == RENDERER_TYPE_COMPUTE_1D, false)) {
                    nodePtr->m_RendererType = RENDERER_TYPE_COMPUTE_1D;
                    change = true;
                }
                ImGui::SameLine();
                if (ImGui::RadioButtonLabeled(0.0f, RENDERER_TYPE_COMPUTE_2D, nodePtr->m_RendererType == RENDERER_TYPE_COMPUTE_2D, false)) {
                    nodePtr->m_RendererType = RENDERER_TYPE_COMPUTE_2D;
                    change = true;
                }
                ImGui::SameLine();
                if (ImGui::RadioButtonLabeled(0.0f, RENDERER_TYPE_COMPUTE_3D, nodePtr->m_RendererType == RENDERER_TYPE_COMPUTE_3D, false)) {
                    nodePtr->m_RendererType = RENDERER_TYPE_COMPUTE_3D;
                    change = true;
                }
                ImGui::SameLine();
                if (ImGui::RadioButtonLabeled(0.0f, RENDERER_TYPE_RTX, nodePtr->m_RendererType == RENDERER_TYPE_RTX, false)) {
                    nodePtr->m_RendererType = RENDERER_TYPE_RTX;
                    change = true;
                }
            }

            ImGui::CheckBoxBoolDefault("Generate a Pass ?", &nodePtr->m_GenerateAPass, true);

            if (nodePtr->m_GenerateAPass) {
                if (nodePtr->m_RendererType == RENDERER_TYPE_PIXEL_2D) {
                    ImGui::Text("Renderer Specialization Type");

                    ImGui::SameLine();

                    if (ImGui::RadioButtonLabeled(0.0f, RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD,
                            nodePtr->m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD, false)) {
                        nodePtr->m_RendererTypePixel2DSpecializationType = RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_QUAD;
                        change = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::RadioButtonLabeled(0.0f, RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH,
                            nodePtr->m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH, false)) {
                        nodePtr->m_RendererTypePixel2DSpecializationType = RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH;
                        change = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::RadioButtonLabeled(0.0f, RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_TESSELATION,
                            nodePtr->m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_TESSELATION, false)) {
                        nodePtr->m_RendererTypePixel2DSpecializationType = RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_TESSELATION;
                        change = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::RadioButtonLabeled(0.0f, RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX,
                            nodePtr->m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX, false)) {
                        nodePtr->m_RendererTypePixel2DSpecializationType = RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_VERTEX;
                        change = true;
                    }
                }

                if (nodePtr->m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_MESH ||
                    nodePtr->m_RendererTypePixel2DSpecializationType == RENDERER_TYPE_PIXEL_2D_SPECIALIZATION_TESSELATION) {
                    change |= ImGui::ContrastedComboVectorDefault(
                        0.0f, "VertexStruct type", &nodePtr->m_VertexStructTypesIndex, nodePtr->m_BaseTypes.m_VertexStructTypes, 0);
                }

                ImGui::Separator();

                ImGui::CheckBoxBoolDefault("Use a CommonSystem UBO", &nodePtr->m_UseCommonSystemUBO, false);

                change |= nodePtr->m_UBOEditors.DrawPane(nodePtr->m_RendererType);

                /*change |= ImGui::CheckBoxBoolDefault("Use A SBO", &nodePtr->m_UseASbo, false);
                if (nodePtr->m_UseASbo)
                {

                }*/
            }
        }
        if (change) {
            SetProjectChange(change);
        }
    }
}

void CodeGeneratorPane::SelectNode(const BaseNodeWeak& vNode) {
    auto currentNodePtr = std::dynamic_pointer_cast<GeneratorNode>(m_SelectedNode.lock());
    auto nodePtr = std::dynamic_pointer_cast<GeneratorNode>(vNode.lock());
    if (nodePtr) {
        m_SelectedNode = nodePtr;

        m_NodeDisplayNameInputText.SetText(nodePtr->m_NodeDisplayName);
        m_NodeCreationNameInputText.SetText(nodePtr->m_NodeCreationName);
        m_NodeCategoryNameInputText.SetText(nodePtr->m_CategoryName);
        m_ClassNameInputText.SetText(nodePtr->m_ClassName);
        m_ModuleDisplayNameInputText.SetText(nodePtr->m_ModuleDisplayName);
        m_ModuleXmlNameInputText.SetText(nodePtr->m_ModuleXmlName);

        if (currentNodePtr && nodePtr != currentNodePtr) {
            NodeSlot::sSlotGraphOutputMouseLeft.reset();
            m_SelectedNodeSlotInput.reset();

            // selection of the first slot
            if (!nodePtr->m_Inputs.empty()) {
                SelectSlot(nodePtr->m_Inputs.begin()->second, ImGuiMouseButton_Left);
            }

            NodeSlot::sSlotGraphOutputMouseRight.reset();
            m_SelectedNodeSlotOutput.reset();

            // selection of the first slot
            if (!nodePtr->m_Outputs.empty()) {
                SelectSlot(nodePtr->m_Outputs.begin()->second, ImGuiMouseButton_Left);
            }
        }
    }
}

void CodeGeneratorPane::SelectSlot(const NodeSlotWeak& vSlot, const ImGuiMouseButton& vButton) {
    if (vButton == ImGuiMouseButton_Left) {
        auto slotPtr = vSlot.lock();
        if (slotPtr) {
            if (slotPtr->IsAnInput()) {
                m_SelectedNodeSlotInput = std::dynamic_pointer_cast<NodeSlotInput>(vSlot.lock());
                NodeSlot::sSlotGraphOutputMouseMiddle = m_SelectedNodeSlotInput;
                m_InputSlotEditor.SelectSlot(m_SelectedNodeSlotInput);
            } else if (slotPtr->IsAnOutput()) {
                m_SelectedNodeSlotOutput = std::dynamic_pointer_cast<NodeSlotOutput>(vSlot.lock());
                NodeSlot::sSlotGraphOutputMouseRight = m_SelectedNodeSlotOutput;
                m_OutputSlotEditor.SelectSlot(m_SelectedNodeSlotOutput);
            }
        }
    } else if (vButton == ImGuiMouseButton_Middle) {
    } else if (vButton == ImGuiMouseButton_Right) {
    }
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string CodeGeneratorPane::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    std::string str;

    if (vUserDatas == "project") {
        str += vOffset + "<custom_types>\n";
        for (const auto& it : m_CustomTypeInputTexts) {
            str += vOffset + "\t<custom_type_name>" + it.GetText() + "</custom_type_name>\n";
        }
        str += vOffset + "</custom_types>\n";
        str += vOffset + "<root_path>" + FilesSaver::Instance()->m_GenerationRootPath + "</root_path>\n";
    }

    return str;
}

bool CodeGeneratorPane::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (vUserDatas == "project") {
        if (strParentName == "custom_types" && strName == "custom_type_name") {
            m_CustomTypeInputTexts.emplace_back(strValue);
        } else if (strParentName == "CodeGenerator" && strName == "root_path") {
            FilesSaver::Instance()->m_GenerationRootPath = strValue;
        }
    }

    return true;
}

bool CodeGeneratorPane::LoadNodeFromXML(BaseNodeWeak vBaseNodeWeak,
    tinyxml2::XMLElement* vElem,
    tinyxml2::XMLElement* vParent,
    const std::string& vNodeName,
    const std::string& vNodeType,
    const ct::fvec2& vPos,
    const size_t& vNodeId) {
    ZoneScoped;
    bool continueXMLParsing = true;
    auto ptr = m_NodeGraph.lock();
    if (ptr) {
        GeneratorNodePtr nodePtr = GeneratorNode::Create(m_VulkanCore.lock());
        if (nodePtr) {
            if (!vNodeName.empty()) {
                nodePtr->name = vNodeName;
            }
            nodePtr->pos = ImVec2(vPos.x, vPos.y);
            nodePtr->nodeID = vNodeId;
            ptr->AddChildNode(nodePtr);
            nd::SetNodePosition(vNodeId, nodePtr->pos);
            nodePtr->RecursParsingConfigChilds(vElem);
            // pour eviter que des slots aient le meme id qu'un nodePtr
            BaseNode::freeNodeId = ct::maxi<uint32_t>(BaseNode::freeNodeId, (uint32_t)vNodeId);
            continueXMLParsing = true;
        }
    }

    return continueXMLParsing;
}
