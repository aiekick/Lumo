#include "SlotEditor.h"

#include <Gui/MainFrame.h>

#include <ImWidgets/ImWidgets.h>
#include <Graph/Base/BaseNode.h>

#include <Graph/Base/NodeSlotInput.h>
#include <Graph/Base/NodeSlotOutput.h>

#include <Graph/Slots/NodeSlotModelInput.h>
#include <Graph/Slots/NodeSlotModelOutput.h>

#include <Graph/Slots/NodeSlotTextureInput.h>
#include <Graph/Slots/NodeSlotTextureOutput.h>

#include <Graph/Slots/NodeSlotVariableInput.h>
#include <Graph/Slots/NodeSlotVariableOutput.h>

#include <Graph/Slots/NodeSlotLightGroupInput.h>
#include <Graph/Slots/NodeSlotLightGroupOutput.h>

#include <Graph/Slots/NodeSlotTexelBufferInput.h>
#include <Graph/Slots/NodeSlotTexelBufferOutput.h>

#include <Graph/Slots/NodeSlotTextureGroupOutput.h>
#include <Graph/Slots/NodeSlotTextureGroupInput.h>

#include <Graph/Slots/NodeSlotTextureCubeOutput.h>
#include <Graph/Slots/NodeSlotTextureCubeInput.h>

#include <Graph/Slots/NodeSlotStorageBufferInput.h>
#include <Graph/Slots/NodeSlotStorageBufferOutput.h>

#include <Graph/GeneratorCommon.h>
#include <Graph/GeneratorNodeSlotInput.h>
#include <Graph/GeneratorNodeSlotOutput.h>

void SlotEditor::SelectSlot(NodeSlotWeak vNodeSlot)
{
	auto slotPtr = vNodeSlot.getValidShared();
	if (slotPtr)
	{
		m_SlotDisplayNameInputText.SetText(slotPtr->name);
		m_CustomTypeInputText.SetText(slotPtr->slotType);

		auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(slotPtr);
		if (slotDatasPtr)
		{
			m_InputType = slotDatasPtr->editorSlotTypeIndex;
		}
	}
}

NodeSlotWeak SlotEditor::DrawSlotCreationPane(const ImVec2& vSize, BaseNodeWeak vNode, NodeSlotWeak vNodeSlot, const NodeSlot::PlaceEnum& vPlace)
{
	NodeSlotWeak res;

	ImGui::PushID(ImGui::IncPUSHID());

	bool framedGroupOpened = false;

	if (vPlace == NodeSlot::PlaceEnum::INPUT)
	{
		framedGroupOpened = ImGui::BeginChild("Input", vSize);
	}
	else if (vPlace == NodeSlot::PlaceEnum::OUTPUT)
	{
		framedGroupOpened = ImGui::BeginChild("Output", vSize);
	}

	if (framedGroupOpened)
	{
		auto nodePtr = vNode.getValidShared();
		if (nodePtr)
		{
			if (ImGui::ContrastedButton("New Slot"))
			{
				if (vPlace == NodeSlot::PlaceEnum::INPUT)
				{
					res = std::dynamic_pointer_cast<GeneratorNodeSlotInput>(nodePtr->AddInput(GeneratorNodeSlotInput::Create("New Slot"), false, false).getValidShared());
					NodeSlot::sSlotGraphOutputMouseLeft = res;
				}
				else if (vPlace == NodeSlot::PlaceEnum::OUTPUT)
				{
					res = std::dynamic_pointer_cast<GeneratorNodeSlotOutput>(nodePtr->AddOutput(GeneratorNodeSlotOutput::Create("New Slot"), false, false).getValidShared());
					NodeSlot::sSlotGraphOutputMouseRight = res;
				}

				ImGui::EndChild();

				ImGui::PopID();

				return res;
			}
		}

		auto slotPtr = vNodeSlot.getValidShared();
		if (slotPtr)
		{
			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(slotPtr);
			if (slotDatasPtr)
			{
				res = vNodeSlot;

				ImGui::SameLine();

				if (ImGui::ContrastedButton("Delete the Slot"))
				{
					nodePtr->DestroySlotOfAnyMap(vNodeSlot);
				}

				if (m_SlotDisplayNameInputText.DisplayInputText(vSize.x, "Slot Name :", "New Slot"))
				{
					slotPtr->name = m_SlotDisplayNameInputText.GetText();
				}

				m_InputType = (int32_t)slotDatasPtr->editorSlotTypeIndex;
				m_SelectedType = m_BaseTypes.m_TypeArray[m_InputType];

				bool _typeChanged = false;
				if (ImGui::ContrastedComboVectorDefault(vSize.x, "Slot Type", 
					&m_InputType, m_BaseTypes.m_TypeArray, 0U))
				{
					slotDatasPtr->editorSlotTypeIndex = (uint32_t)m_InputType;
					m_SelectedType = m_BaseTypes.m_TypeArray[slotDatasPtr->editorSlotTypeIndex];
					if (m_InputType == BASE_TYPE_Custom)
					{
						if (m_SelectedSubType.empty())
						{
							m_SelectedSubType = MainFrame::Instance()->GetCustomTypeInputText().GetText();
						}
						m_CustomTypeInputText.SetText(m_SelectedSubType);
					}
					_typeChanged = true;
				}

				if (m_InputType == BASE_TYPE_Variable)
				{
					m_InputSubType = (int32_t)slotDatasPtr->editorSlotSubTypeIndex;
					m_SelectedSubType = m_BaseTypes.m_VariableTypeArray[m_InputSubType];

					if (ImGui::ContrastedComboVectorDefault(vSize.x, "Variable Slot Type",
						&m_InputSubType, m_BaseTypes.m_VariableTypeArray, 0U))
					{
						slotDatasPtr->editorSlotSubTypeIndex = (uint32_t)m_InputSubType;
						m_SelectedSubType = m_BaseTypes.m_VariableTypeArray[slotDatasPtr->editorSlotSubTypeIndex];
						_typeChanged = true;
					}
				}
				else if (m_InputType == BASE_TYPE_Custom)
				{
					if (m_CustomTypeInputText.DisplayInputText(vSize.x, "Custom Type :", "SceneCustom"))
					{
						m_SelectedSubType = m_CustomTypeInputText.GetText();
						_typeChanged = true;
					}
				}
				else
				{
					m_SelectedSubType = "";
				}

				if (_typeChanged)
				{
					if (vPlace == NodeSlot::PlaceEnum::INPUT)
					{
						res = std::dynamic_pointer_cast<GeneratorNodeSlotInput>(
							ChangeInputSlotType(vNode, m_SelectedType, 
								m_SelectedSubType, vNodeSlot).getValidShared());
					}
					else if (vPlace == NodeSlot::PlaceEnum::OUTPUT)
					{
						res = std::dynamic_pointer_cast<GeneratorNodeSlotOutput>(
							ChangeOutputSlotType(vNode, m_SelectedType, 
								m_SelectedSubType, vNodeSlot).getValidShared());
					}
				}

				ImGui::CheckBoxBoolDefault("Hide Name", &slotPtr->hideName, false);
				ImGui::CheckBoxBoolDefault("Show widget", &slotPtr->showWidget, false);

				if (m_SelectedType == "StorageBuffer" || 
					m_SelectedType == "TexelBuffer" ||
					m_SelectedType == "Texture" ||
					m_SelectedType == "TextureCube" ||
					m_SelectedType == "TextureGroup")
				{
					ImGui::InputUIntDefault(0.0f, "Descriptor Binding", &slotPtr->descriptorBinding, 1U, 2U, 0U);
				}
				else if (m_SelectedType == "Variable")
				{
					ImGui::InputUIntDefault(0.0f, "Variable Index", &slotPtr->variableIndex, 1U, 2U, 0U);
				}
			}
		}
	}

	ImGui::EndChild();

	ImGui::PopID();

	return res;
}

NodeSlotWeak SlotEditor::ChangeInputSlotType(BaseNodeWeak vRootNode, const std::string& vType, const std::string& vSubType, const NodeSlotWeak& vSlot)
{
	auto nodePtr = vRootNode.getValidShared();
	if (nodePtr)
	{
		auto slotPtr = vSlot.getValidShared();
		if (slotPtr)
		{
			slotPtr->slotType = vSubType;

			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(slotPtr);
			if (slotDatasPtr)
			{
				NodeSlotInputPtr resPtr = nullptr;

				if (vType == "None" || vType == "Custom")
				{
					resPtr = NodeSlotInput::Create(slotPtr->name, slotPtr->slotType);
				}
				else if (vType == "LightGroup")
				{
					resPtr = NodeSlotLightGroupInput::Create(slotPtr->name);
				}
				else if (vType == "Model")
				{
					resPtr = NodeSlotModelInput::Create(slotPtr->name);
				}
				else if (vType == "StorageBuffer")
				{
					if (slotPtr->slotType.empty())
						slotPtr->slotType = "NONE";
					resPtr = NodeSlotStorageBufferInput::Create(slotPtr->name, slotPtr->slotType);
				}
				else if (vType == "TexelBuffer")
				{
					if (slotPtr->slotType.empty())
						slotPtr->slotType = "NONE";
					resPtr = NodeSlotTexelBufferInput::Create(slotPtr->name, slotPtr->slotType);
				}
				else if (vType == "Texture")
				{
					resPtr = NodeSlotTextureInput::Create(slotPtr->name, slotPtr->descriptorBinding);
				}
				else if (vType == "TextureCube")
				{
					resPtr = NodeSlotTextureCubeInput::Create(slotPtr->name, slotPtr->descriptorBinding);
				}
				else if (vType == "TextureGroup")
				{
					resPtr = NodeSlotTextureGroupInput::Create(slotPtr->name);
				}
				else if (vType == "Variable")
				{
					if (slotPtr->slotType.empty())
						slotPtr->slotType = "NONE";
					resPtr = NodeSlotVariableInput::Create(slotPtr->name, slotPtr->slotType, slotPtr->variableIndex);
				}

				if (resPtr)
				{
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

NodeSlotWeak SlotEditor::ChangeOutputSlotType(BaseNodeWeak vRootNode, const std::string& vType, const std::string& vSubType, const NodeSlotWeak& vSlot)
{
	auto nodePtr = vRootNode.getValidShared();
	if (nodePtr)
	{
		auto slotPtr = vSlot.getValidShared();
		if (slotPtr)
		{
			slotPtr->slotType = vSubType;

			auto slotDatasPtr = std::dynamic_pointer_cast<GeneratorNodeSlotDatas>(slotPtr);
			if (slotDatasPtr)
			{
				NodeSlotOutputPtr resPtr = nullptr;

				if (vType == "None" || vType == "Custom")
				{
					resPtr = NodeSlotOutput::Create(slotPtr->name, slotPtr->slotType);
				}
				else if (vType == "LightGroup")
				{
					resPtr = NodeSlotLightGroupOutput::Create(slotPtr->name);
				}
				else if (vType == "Model")
				{
					resPtr = NodeSlotModelOutput::Create(slotPtr->name);
				}
				else if (vType == "StorageBuffer")
				{
					if (slotPtr->slotType.empty())
						slotPtr->slotType = "NONE";
					resPtr = NodeSlotStorageBufferOutput::Create(slotPtr->name, slotPtr->slotType);
				}
				else if (vType == "TexelBuffer")
				{
					if (slotPtr->slotType.empty())
						slotPtr->slotType = "NONE";
					resPtr = NodeSlotTexelBufferOutput::Create(slotPtr->name, slotPtr->slotType);
				}
				else if (vType == "Texture")
				{
					resPtr = NodeSlotTextureOutput::Create(slotPtr->name, slotPtr->descriptorBinding);
				}
				else if (vType == "TextureCube")
				{
					resPtr = NodeSlotTextureCubeOutput::Create(slotPtr->name, slotPtr->descriptorBinding);
				}
				else if (vType == "TextureGroup")
				{
					resPtr = NodeSlotTextureGroupOutput::Create(slotPtr->name);
				}
				else if (vType == "Variable")
				{
					if (slotPtr->slotType.empty())
						slotPtr->slotType = "NONE";
					resPtr = NodeSlotVariableOutput::Create(slotPtr->name, slotPtr->slotType, slotPtr->descriptorBinding);
				}

				if (resPtr)
				{
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