#pragma once

#include <Graph/Graph.h>
#include <Graph/Base/NodeSlot.h>
#include <ImWidgets/ImWidgets.h>
#include <Graph/GeneratorNode.h>

class SlotEditor
{
private:
	BaseTypes m_BaseTypes;
	std::string m_SelectedType;
	int m_InputType = 0U;
	int m_InputSubType = 0U;
	std::string m_SelectedSubType;
	ImWidgets::InputText m_SlotDisplayNameInputText;
	ImWidgets::InputText m_CustomTypeInputText;

public:
	void SelectSlot(NodeSlotWeak vNodeSlot);
	NodeSlotWeak DrawSlotCreationPane(const ImVec2& vSize, BaseNodeWeak vNode, NodeSlotWeak vNodeSlot, const NodeSlot::PlaceEnum& vPlace);

private:
	NodeSlotWeak ChangeInputSlotType(BaseNodeWeak vRootNode, const std::string& vType, const std::string& vSubType, const NodeSlotWeak& vSlot);
	NodeSlotWeak ChangeOutputSlotType(BaseNodeWeak vRootNode, const std::string& vType, const std::string& vSubType, const NodeSlotWeak& vSlot);
};