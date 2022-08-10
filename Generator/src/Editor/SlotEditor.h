#pragma once

#include <Graph/Graph.h>
#include <Graph/Base/NodeSlot.h>
#include <Generator/NodeGenerator.h>
#include <ImWidgets/ImWidgets.h>

class SlotEditor
{
private:
	BaseTypes m_BaseTypes;
	std::string m_SelectedType;
	int m_InputType = 0U;
	ImWidgets::InputText m_SlotDisplayNameInputText;

public:
	void SelectSlot(NodeSlotWeak vNodeSlot);
	NodeSlotWeak DrawSlotCreationPane(const ImVec2& vSize, BaseNodeWeak vNode, NodeSlotWeak vNodeSlot, const NodeSlot::PlaceEnum& vPlace);

private:
	NodeSlotWeak ChangeInputSlotType(BaseNodeWeak vRootNode, const std::string& vType, const NodeSlotWeak& vSlot);
	NodeSlotWeak ChangeOutputSlotType(BaseNodeWeak vRootNode, const std::string& vType, const NodeSlotWeak& vSlot);
};