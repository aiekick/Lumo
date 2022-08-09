#include "SlotEditor.h"

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

#include <Graph/Slots/NodeSlotStorageBufferInput.h>
#include <Graph/Slots/NodeSlotStorageBufferOutput.h>

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
					res = std::dynamic_pointer_cast<NodeSlotInput>(nodePtr->AddInput(NodeSlotInput::Create("New Slot"), false, false).getValidShared());
				}
				else if (vPlace == NodeSlot::PlaceEnum::OUTPUT)
				{
					res = std::dynamic_pointer_cast<NodeSlotOutput>(nodePtr->AddOutput(NodeSlotOutput::Create("New Slot"), false, false).getValidShared());
				}

				ImGui::EndChild();

				ImGui::PopID();

				return res;
			}
		}

		auto slotPtr = vNodeSlot.getValidShared();
		if (slotPtr)
		{
			res = vNodeSlot;

			ImGui::SameLine();

			if (ImGui::ContrastedButton("Delete the Slot"))
			{
				nodePtr->DestroySlotOfAnyMap(vNodeSlot);
			}

			ct::SetBuffer(m_SlotNameBuffer, 255, slotPtr->name);
			ImGui::Text("Slot Name :");
			ImGui::SameLine();
			ImGui::PushItemWidth(vSize.x);
			if (ImGui::InputText("##SlotName", m_SlotNameBuffer, 255U))
			{
				slotPtr->name = std::string(m_SlotNameBuffer, strlen(m_SlotNameBuffer));
			}
			ImGui::PopItemWidth();

			m_InputType = (int32_t)slotPtr->editorSlotTypeIndex;
			m_SelectedType = m_TypeArray[m_InputType];

			if (ImGui::ContrastedComboVectorDefault(vSize.x, "Slot Type", &m_InputType, m_TypeArray, 0U))
			{
				slotPtr->editorSlotTypeIndex = (uint32_t)m_InputType;
				m_SelectedType = m_TypeArray[m_InputType];

				if (vPlace == NodeSlot::PlaceEnum::INPUT)
				{
					res = std::dynamic_pointer_cast<NodeSlotInput>(ChangeInputSlotType(vNode, m_SelectedType, vNodeSlot).getValidShared());
				}
				else if (vPlace == NodeSlot::PlaceEnum::OUTPUT)
				{
					res = std::dynamic_pointer_cast<NodeSlotOutput>(ChangeOutputSlotType(vNode, m_SelectedType, vNodeSlot).getValidShared());
				}
			}

			ImGui::CheckBoxBoolDefault("Hide Name", &slotPtr->hideName, false);
			ImGui::CheckBoxBoolDefault("Show widget", &slotPtr->showWidget, false);

			if (m_SelectedType == "TexelBuffer" ||
				m_SelectedType == "Texture" ||
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

	ImGui::EndChild();

	ImGui::PopID();

	return res;
}

NodeSlotWeak SlotEditor::ChangeInputSlotType(BaseNodeWeak vRootNode, const std::string& vType, const NodeSlotWeak& vSlot)
{
	auto nodePtr = vRootNode.getValidShared();
	if (nodePtr)
	{
		auto slotPtr = vSlot.getValidShared();
		if (slotPtr)
		{
			NodeSlotInputPtr resPtr = nullptr;

			if (vType == "None")
			{
				resPtr = NodeSlotInput::Create(slotPtr->name);
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
				resPtr = NodeSlotStorageBufferInput::Create(slotPtr->name, slotPtr->slotType);
			}
			else if (vType == "TexelBuffer")
			{
				resPtr = NodeSlotTexelBufferInput::Create(slotPtr->name, slotPtr->slotType);
			}
			else if (vType == "Texture")
			{
				resPtr = NodeSlotTextureInput::Create(slotPtr->name, slotPtr->descriptorBinding);
			}
			else if (vType == "TextureGroup")
			{
				resPtr = NodeSlotTextureGroupInput::Create(slotPtr->name);
			}
			else if (vType == "Variable")
			{
				resPtr = NodeSlotVariableInput::Create(slotPtr->name, slotPtr->slotType, slotPtr->variableIndex);
			}

			if (resPtr)
			{
				NodeSlot::sSlotGraphOutputMouseLeft = resPtr;

				resPtr->slotPlace = slotPtr->slotPlace;
				resPtr->pinID = slotPtr->pinID;
				resPtr->index = slotPtr->index;
				resPtr->help = slotPtr->help;
				resPtr->stamp = slotPtr->stamp;
				resPtr->type = slotPtr->type;
				resPtr->linkedSlots = slotPtr->linkedSlots;
				resPtr->parentNode = slotPtr->parentNode;
				resPtr->uniform = slotPtr->uniform;
				resPtr->fboAttachmentId = slotPtr->fboAttachmentId;
				resPtr->channelId = slotPtr->channelId;
				resPtr->idAlreadySetbyXml = slotPtr->idAlreadySetbyXml;
				resPtr->acceptManyInputs = slotPtr->acceptManyInputs;
				resPtr->highLighted = slotPtr->highLighted;
				resPtr->connected = slotPtr->connected;
				resPtr->hideName = slotPtr->hideName;
				resPtr->showWidget = slotPtr->showWidget;
				resPtr->hidden = slotPtr->hidden;
				resPtr->virtualUniform = slotPtr->virtualUniform;
				resPtr->editorSlotTypeIndex = slotPtr->editorSlotTypeIndex;

				uint32_t id = (uint32_t)slotPtr->pinID.Get();
				nodePtr->m_Inputs[id] = resPtr;

				return nodePtr->m_Inputs[id];
			}
		}
	}

	return NodeSlotWeak();
}

NodeSlotWeak SlotEditor::ChangeOutputSlotType(BaseNodeWeak vRootNode, const std::string& vType, const NodeSlotWeak& vSlot)
{
	auto nodePtr = vRootNode.getValidShared();
	if (nodePtr)
	{
		auto slotPtr = vSlot.getValidShared();
		if (slotPtr)
		{
			NodeSlotOutputPtr resPtr = nullptr;

			if (vType == "None")
			{
				resPtr = NodeSlotOutput::Create(slotPtr->name);
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
				resPtr = NodeSlotStorageBufferOutput::Create(slotPtr->name, slotPtr->slotType);
			}
			else if (vType == "TexelBuffer")
			{
				resPtr = NodeSlotTexelBufferOutput::Create(slotPtr->name, slotPtr->slotType);
			}
			else if (vType == "Texture")
			{
				resPtr = NodeSlotTextureOutput::Create(slotPtr->name, slotPtr->descriptorBinding);
			}
			else if (vType == "TextureGroup")
			{
				resPtr = NodeSlotTextureGroupOutput::Create(slotPtr->name);
			}
			else if (vType == "Variable")
			{
				resPtr = NodeSlotVariableOutput::Create(slotPtr->name, slotPtr->slotType, slotPtr->descriptorBinding);
			}

			if (resPtr)
			{
				NodeSlot::sSlotGraphOutputMouseRight = resPtr;

				resPtr->slotPlace = slotPtr->slotPlace;
				resPtr->pinID = slotPtr->pinID;
				resPtr->index = slotPtr->index;
				resPtr->help = slotPtr->help;
				resPtr->stamp = slotPtr->stamp;
				resPtr->type = slotPtr->type;
				resPtr->linkedSlots = slotPtr->linkedSlots;
				resPtr->parentNode = slotPtr->parentNode;
				resPtr->uniform = slotPtr->uniform;
				resPtr->fboAttachmentId = slotPtr->fboAttachmentId;
				resPtr->channelId = slotPtr->channelId;
				resPtr->idAlreadySetbyXml = slotPtr->idAlreadySetbyXml;
				resPtr->acceptManyInputs = slotPtr->acceptManyInputs;
				resPtr->highLighted = slotPtr->highLighted;
				resPtr->connected = slotPtr->connected;
				resPtr->hideName = slotPtr->hideName;
				resPtr->showWidget = slotPtr->showWidget;
				resPtr->hidden = slotPtr->hidden;
				resPtr->virtualUniform = slotPtr->virtualUniform;
				resPtr->editorSlotTypeIndex = slotPtr->editorSlotTypeIndex;

				uint32_t id = (uint32_t)slotPtr->pinID.Get();
				nodePtr->m_Outputs[id] = resPtr;

				return nodePtr->m_Outputs[id];
			}
		}
	}

	return NodeSlotWeak();
}