#pragma once

#include <Graph/Graph.h>
#include <Graph/Base/NodeSlot.h>

class SlotEditor
{
private:
	char m_SlotNameBuffer[255 + 1] = "";
	std::vector<std::string> m_TypeArray =
	{
		"None",
		"LightGroup",
		"Model",
		"StorageBuffer",
		"TexelBuffer",
		"Texture",
		"TextureGroup",
		"Variable",
		"Custom" 
	};
	std::string m_SelectedType;
	int m_InputType = 0U;

public:
	NodeSlotWeak DrawSlotCreationPane(const ImVec2& vSize, BaseNodeWeak vNode, NodeSlotWeak vNodeSlot, const NodeSlot::PlaceEnum& vPlace);

private:
	NodeSlotWeak ChangeInputSlotType(BaseNodeWeak vRootNode, const std::string& vType, const NodeSlotWeak& vSlot);
	NodeSlotWeak ChangeOutputSlotType(BaseNodeWeak vRootNode, const std::string& vType, const NodeSlotWeak& vSlot);
};