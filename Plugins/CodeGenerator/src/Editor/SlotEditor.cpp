#include "SlotEditor.h"

#include <LumoBackend/Graph/Base/BaseNode.h>

#include <LumoBackend/Graph/Base/NodeSlotInput.h>
#include <LumoBackend/Graph/Base/NodeSlotOutput.h>

#include <LumoBackend/Graph/Slots/NodeSlotModelInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotModelOutput.h>

#include <LumoBackend/Graph/Slots/NodeSlotTextureInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureOutput.h>

#include <LumoBackend/Graph/Slots/NodeSlotVariableInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotVariableOutput.h>

#include <LumoBackend/Graph/Slots/NodeSlotLightGroupInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotLightGroupOutput.h>

#include <LumoBackend/Graph/Slots/NodeSlotTexelBufferInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTexelBufferOutput.h>

#include <LumoBackend/Graph/Slots/NodeSlotTextureGroupOutput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureGroupInput.h>

#include <LumoBackend/Graph/Slots/NodeSlotTextureCubeOutput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTextureCubeInput.h>

#include <LumoBackend/Graph/Slots/NodeSlotStorageBufferInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotStorageBufferOutput.h>

#include <LumoBackend/Graph/Slots/NodeSlotShaderPassInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotShaderPassOutput.h>

#include <LumoBackend/Graph/Slots/NodeSlotTaskInput.h>
#include <LumoBackend/Graph/Slots/NodeSlotTaskOutput.h>

#include <Headers/GeneratorCommon.h>
#include <Slots/GeneratorNodeSlotInput.h>
#include <Slots/GeneratorNodeSlotOutput.h>

#include <Panes/CodeGeneratorPane.h>

void SlotEditor::SelectSlot(NodeSlotWeak vNodeSlot) {
    auto slotPtr = vNodeSlot.lock();
    if (slotPtr) {
        m_SlotDisplayNameInputText.SetText(slotPtr->name);
        // m_CustomTypeInputText.SetText(slotPtr->slotType);
        auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(slotPtr);
        if (slotDatasPtr) {
            m_InputType = slotDatasPtr->editorSlotTypeIndex;
        }
    }
}

NodeSlotWeak SlotEditor::DrawSlotCreationPane(
    const ImVec2& vSize, BaseNodeWeak vNode, NodeSlotWeak vNodeSlot, const NodeSlot::PlaceEnum& vPlace, bool& vChange) {
    NodeSlotWeak res;
    ImGui::PushID(ImGui::IncPUSHID());
    bool framedGroupOpened = false;
    if (vPlace == NodeSlot::PlaceEnum::INPUT) {
        framedGroupOpened = ImGui::BeginChild("Input", vSize);
    } else if (vPlace == NodeSlot::PlaceEnum::OUTPUT) {
        framedGroupOpened = ImGui::BeginChild("Output", vSize);
    }
    if (framedGroupOpened) {
        auto nodePtr = vNode.lock();
        if (nodePtr) {
            if (ImGui::ContrastedButton("New Slot")) {
                if (vPlace == NodeSlot::PlaceEnum::INPUT) {
                    res = std::dynamic_pointer_cast<GeneratorNodeSlotInput>(
                        nodePtr->AddInput(GeneratorNodeSlotInput::Create("New Slot"), false, false).lock());
                    NodeSlot::sSlotGraphOutputMouseMiddle = res;
                    auto slot_ptr = res.lock();
                    if (slot_ptr != nullptr) {
                        slot_ptr->descriptorBinding = (uint32_t)nodePtr->m_Inputs.size() - 1U;
                    }
                } else if (vPlace == NodeSlot::PlaceEnum::OUTPUT) {
                    res = std::dynamic_pointer_cast<GeneratorNodeSlotOutput>(
                        nodePtr->AddOutput(GeneratorNodeSlotOutput::Create("New Slot"), false, false).lock());
                    NodeSlot::sSlotGraphOutputMouseRight = res;
                    auto slot_ptr = res.lock();
                    if (slot_ptr != nullptr) {
                        slot_ptr->descriptorBinding = (uint32_t)nodePtr->m_Inputs.size() - 1U;
                    }
                }
                SelectSlot(res);
                ImGui::EndChild();
                ImGui::PopID();
                vChange = true;
                return res;
            }
        }
        auto slotPtr = vNodeSlot.lock();
        if (slotPtr) {
            auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(slotPtr);
            if (slotDatasPtr) {
                res = vNodeSlot;
                ImGui::SameLine();
                if (ImGui::ContrastedButton("Delete the Slot")) {
                    nodePtr->DestroySlotOfAnyMap(vNodeSlot);
                    vChange = true;                }

                if (m_SlotDisplayNameInputText.DisplayInputText(vSize.x, "Slot Name :", "New Slot")) {
                    slotPtr->name = m_SlotDisplayNameInputText.GetText();
                    vChange = true;
                }
                m_InputType = (int32_t)slotDatasPtr->editorSlotTypeIndex;
                m_SelectedType = m_BaseTypes.m_TypeArray[m_InputType];
                bool _typeChanged = false;
                if (ImGui::ContrastedComboVectorDefault(vSize.x, "Slot Type", &m_InputType, m_BaseTypes.m_TypeArray, 0U)) {
                    slotDatasPtr->editorSlotTypeIndex = (uint32_t)m_InputType;
                    m_SelectedType = m_BaseTypes.m_TypeArray[slotDatasPtr->editorSlotTypeIndex];
                    if (m_InputType == BASE_TYPE_Custom) {
                        auto arr = CodeGeneratorPane::Instance()->GetCustomTypeInputTexts();
                        if (m_SelectedSubTypeIndex > -1 && m_SelectedSubTypeIndex < (int)arr.size()) {
                            m_SelectedSubType = arr.at(m_SelectedSubTypeIndex).GetText();
                        }
                    }
                    _typeChanged = true;
                    vChange = true;
                }
                if (m_InputType == BASE_TYPE_Variable) {
                    m_InputSubType = (int32_t)slotDatasPtr->editorSlotSubTypeIndex;
                    m_SelectedSubType = m_BaseTypes.m_VariableTypeArray[m_InputSubType];

                    if (ImGui::ContrastedComboVectorDefault(vSize.x, "Variable Slot Type", &m_InputSubType, m_BaseTypes.m_VariableTypeArray, 0U)) {
                        slotDatasPtr->editorSlotSubTypeIndex = (uint32_t)m_InputSubType;
                        m_SelectedSubType = m_BaseTypes.m_VariableTypeArray[slotDatasPtr->editorSlotSubTypeIndex];
                        _typeChanged = true;
                        vChange = true;
                    }
                } else if (m_InputType == BASE_TYPE_Custom) {
                    auto arr = CodeGeneratorPane::Instance()->GetCustomTypeInputTexts();
                    ImGui::PushID(ImGui::IncPUSHID());
                    bool change = ImGui::ContrastedButton(ICON_SDFM_TRASH_CAN_OUTLINE);
                    if (change) {
                        m_SelectedSubTypeIndex = 0;
                    }
                    ImGui::SameLine();
                    change |= ImGui::ContrastedCombo(
                        0.0f, "##Custom Types", &m_SelectedSubTypeIndex,
                        [](void* data, int idx, const char** out_text) {
                            *out_text = ((const std::vector<ImWidgets::InputText>*)data)->at(idx).GetConstCharPtrText();
                            return true;
                        },
                        (void*)&arr, (int)arr.size(), -1);
                    if (change) {
                        m_SelectedSubType = arr.at(m_SelectedSubTypeIndex).GetText();
                        _typeChanged = true;
                    }
                    vChange |= change;
                    ImGui::PopID();
                } else {
                    m_SelectedSubType = "";
                }
                if (_typeChanged) {
                    if (vPlace == NodeSlot::PlaceEnum::INPUT) {
                        res = std::dynamic_pointer_cast<GeneratorNodeSlotInput>(
                            ChangeInputSlotType(vNode, m_SelectedType, m_SelectedSubType, vNodeSlot, vChange).lock());
                    } else if (vPlace == NodeSlot::PlaceEnum::OUTPUT) {
                        res = std::dynamic_pointer_cast<GeneratorNodeSlotOutput>(
                            ChangeOutputSlotType(vNode, m_SelectedType, m_SelectedSubType, vNodeSlot, vChange).lock());
                    }
                }
                vChange |= ImGui::CheckBoxBoolDefault("Hide Name", &slotPtr->hideName, false);
                vChange |= ImGui::CheckBoxBoolDefault("Show widget", &slotPtr->showWidget, false);
                if (m_SelectedType == "StorageBuffer" || m_SelectedType == "TexelBuffer" || m_SelectedType == "Texture" ||
                    m_SelectedType == "TextureCube" || m_SelectedType == "TextureGroup") {
                    vChange |= ImGui::InputUIntDefault(0.0f, "Descriptor Binding", &slotPtr->descriptorBinding, 1U, 2U, 0U);
                } else if (m_SelectedType == "Variable") {
                    vChange |= ImGui::InputUIntDefault(0.0f, "Variable Index", &slotPtr->variableIndex, 1U, 2U, 0U);
                }
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopID();
    return res;
}

NodeSlotWeak SlotEditor::ChangeInputSlotType(
    BaseNodeWeak vRootNode, const std::string& vType, const std::string& vSubType, const NodeSlotWeak& vSlot, bool& vChange) {
    auto nodePtr = vRootNode.lock();
    if (nodePtr) {
        auto slotPtr = vSlot.lock();
        if (slotPtr) {
            slotPtr->slotType = vSubType;
            auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(slotPtr);
            if (slotDatasPtr) {
                NodeSlotInputPtr resPtr = nullptr;
                if (vType == "None" || vType == "Custom") {
                    resPtr = NodeSlotInput::Create(slotPtr->name, slotPtr->slotType);
                } else if (vType == "LightGroup") {
                    resPtr = NodeSlotLightGroupInput::Create(slotPtr->name);
                } else if (vType == "Model") {
                    resPtr = NodeSlotModelInput::Create(slotPtr->name);
                } else if (vType == "StorageBuffer") {
                    if (slotPtr->slotType.empty())
                        slotPtr->slotType = "NONE";
                    resPtr = NodeSlotStorageBufferInput::Create(slotPtr->name, slotPtr->slotType);
                } else if (vType == "TexelBuffer") {
                    if (slotPtr->slotType.empty())
                        slotPtr->slotType = "NONE";
                    resPtr = NodeSlotTexelBufferInput::Create(slotPtr->name, slotPtr->slotType);
                } else if (vType == "Texture") {
                    resPtr = NodeSlotTextureInput::Create(slotPtr->name, slotPtr->descriptorBinding);
                } else if (vType == "TextureCube") {
                    resPtr = NodeSlotTextureCubeInput::Create(slotPtr->name, slotPtr->descriptorBinding);
                } else if (vType == "TextureGroup") {
                    resPtr = NodeSlotTextureGroupInput::Create(slotPtr->name);
                } else if (vType == "ShaderPass") {
                    resPtr = NodeSlotShaderPassInput::Create(slotPtr->name);
                } else if (vType == "Task") {
                    resPtr = NodeSlotTaskInput::Create(slotPtr->name);
                } else if (vType == "Variable") {
                    if (slotPtr->slotType.empty())
                        slotPtr->slotType = "NONE";
                    resPtr = NodeSlotVariableInput::Create(slotPtr->name, slotPtr->slotType, slotPtr->variableIndex);
                }
                if (resPtr) {
                    NodeSlot::sSlotGraphOutputMouseLeft = slotPtr;

                    slotPtr->slotType = resPtr->slotType;
                    slotPtr->color = resPtr->color;
                    slotPtr->colorIsSet = resPtr->colorIsSet;

                    resPtr.reset();

                    return slotPtr;
                }
            }
        }
    }

    return NodeSlotWeak();
}

NodeSlotWeak SlotEditor::ChangeOutputSlotType(
    BaseNodeWeak vRootNode, const std::string& vType, const std::string& vSubType, const NodeSlotWeak& vSlot, bool& vChange) {
    auto nodePtr = vRootNode.lock();
    if (nodePtr) {
        auto slotPtr = vSlot.lock();
        if (slotPtr) {
            slotPtr->slotType = vSubType;
            auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(slotPtr);
            if (slotDatasPtr) {
                NodeSlotOutputPtr resPtr = nullptr;
                if (vType == "None" || vType == "Custom") {
                    resPtr = NodeSlotOutput::Create(slotPtr->name, slotPtr->slotType);
                } else if (vType == "LightGroup") {
                    resPtr = NodeSlotLightGroupOutput::Create(slotPtr->name);
                } else if (vType == "Model") {
                    resPtr = NodeSlotModelOutput::Create(slotPtr->name);
                } else if (vType == "StorageBuffer") {
                    if (slotPtr->slotType.empty())
                        slotPtr->slotType = "NONE";
                    resPtr = NodeSlotStorageBufferOutput::Create(slotPtr->name, slotPtr->slotType);
                } else if (vType == "TexelBuffer") {
                    if (slotPtr->slotType.empty())
                        slotPtr->slotType = "NONE";
                    resPtr = NodeSlotTexelBufferOutput::Create(slotPtr->name, slotPtr->slotType);
                } else if (vType == "Texture") {
                    resPtr = NodeSlotTextureOutput::Create(slotPtr->name, slotPtr->descriptorBinding);
                } else if (vType == "TextureCube") {
                    resPtr = NodeSlotTextureCubeOutput::Create(slotPtr->name, slotPtr->descriptorBinding);
                } else if (vType == "TextureGroup") {
                    resPtr = NodeSlotTextureGroupOutput::Create(slotPtr->name);
                } else if (vType == "ShaderPass") {
                    resPtr = NodeSlotShaderPassOutput::Create(slotPtr->name);
                } else if (vType == "Task") {
                    resPtr = NodeSlotTaskOutput::Create(slotPtr->name);
                } else if (vType == "Variable") {
                    if (slotPtr->slotType.empty())
                        slotPtr->slotType = "NONE";
                    resPtr = NodeSlotVariableOutput::Create(slotPtr->name, slotPtr->slotType, slotPtr->descriptorBinding);
                }
                if (resPtr) {
                    NodeSlot::sSlotGraphOutputMouseRight = slotPtr;

                    slotPtr->slotType = resPtr->slotType;
                    slotPtr->color = resPtr->color;
                    slotPtr->colorIsSet = resPtr->colorIsSet;

                    resPtr.reset();

                    return slotPtr;
                }
            }
        }
    }

    return NodeSlotWeak();
}