#include "UBOEditor.h"

#include <ImWidgets/ImWidgets.h>
#include <Graph/Base/BaseNode.h>

#include <Graph/GeneratorNode.h>
#include <Graph/GeneratorCommon.h>
#include <Graph/GeneratorNodeSlotInput.h>
#include <Graph/GeneratorNodeSlotOutput.h>

///////////////////////////////////////////////////////
//// UBOItem //////////////////////////////////////////
///////////////////////////////////////////////////////

void UBOItem::DrawItem(const std::string& vStage)
{
	m_Stage = vStage;

	const float& aaw = (ImGui::GetContentRegionAvail().x - 150.0f) * 0.5f;

	m_InputName.DisplayInputText(aaw, "", "Name");

	ImGui::SameLine();

	ImGui::ContrastedComboVectorDefault(150.0f, "##Type", &m_InputTypeIndex, m_TypeArray, 0);

	ImGui::SameLine();

	float aw = aaw;
	if (m_InputTypeIndex % 4 == 0U) aw = aaw;
	else if (m_InputTypeIndex % 4 == 1U) aw = aaw / 2.0f;
	else if (m_InputTypeIndex % 4 == 2U) aw = aaw / 3.0f;
	else if (m_InputTypeIndex % 4 == 3U) aw = aaw / 4.0f;

	m_InputValue_x.DisplayInputText(aw, "", "0");

	if (m_InputTypeIndex % 4 > 0U)
	{
		ImGui::SameLine();

		m_InputValue_y.DisplayInputText(aw, "", "0");
	}
		
	if (m_InputTypeIndex % 4 > 1U)
	{
		ImGui::SameLine();

		m_InputValue_z.DisplayInputText(aw, "", "0");
	}

	if (m_InputTypeIndex % 4 > 2U)
	{
		ImGui::SameLine();

		m_InputValue_w.DisplayInputText(aw, "", "0");
	}
}

std::string UBOItem::GetCppItemHeader()
{
	std::string res;
	uint32_t _alignas = 4;

	uint32_t inputIdx = m_InputTypeIndex % 4;
	int baseTypeIdx = m_InputTypeIndex - inputIdx;
	auto baseType = m_TypeArray[baseTypeIdx];
	std::string type = m_TypeArray[m_InputTypeIndex];
	std::string value = m_InputValue_x.GetText(baseType);

	if (m_InputTypeIndex > 0 && m_InputTypeIndex < 4)
		type = "f" + type;

	if (inputIdx == 0)
	{
		if (type == "int" ||
			type == "uint")
			type += "32_t";
	}
	else if (inputIdx == 1)
	{
		_alignas = 8;
		type = "ct::" + type;

		value = type + "(" + value + ", " + m_InputValue_y.GetText(baseType) + ")";
	}
	else if (inputIdx == 2)
	{
		_alignas = 16;
		type = "ct::" + type;
		value = type + "(" + value + ", " + m_InputValue_y.GetText(baseType) + ", " + m_InputValue_z.GetText(baseType) + ")";
	}
	else if (inputIdx == 3)
	{
		_alignas = 16;
		type = "ct::" + type;
		value = type + "(" + value + ", " + m_InputValue_y.GetText(baseType) + ", " + m_InputValue_z.GetText(baseType) + ", " + m_InputValue_w.GetText(baseType) + ")";
	}

	value += ";";

	res += ct::toStr(u8R"(
		alignas(%u) %s u_%s = %s)", _alignas, type.c_str(), m_InputName.GetText().c_str(), value.c_str());

	return res;
}

std::string UBOItem::GetGlslItemHeader()
{
	std::string res;
	
	uint32_t inputIdx = m_InputTypeIndex % 4;
	int baseTypeIdx = m_InputTypeIndex - inputIdx;
	auto baseType = m_TypeArray[baseTypeIdx];
	std::string type = m_TypeArray[m_InputTypeIndex];

	res += ct::toStr(u8R"(
	%s u_%s)", type.c_str(), m_InputName.GetText().c_str());

	return res;
}

std::string UBOItem::GetItemWidget()
{
	std::string res;

	std::string type = m_TypeArray[m_InputTypeIndex];
	uint32_t inputIdx = m_InputTypeIndex % 4;
	int baseTypeIdx = m_InputTypeIndex - inputIdx;
	auto baseType = m_TypeArray[baseTypeIdx];
	std::string value = m_InputValue_x.GetText(baseType);

	if (m_InputTypeIndex > 0 && m_InputTypeIndex < 4)
		type = "f" + type;

	if (type == "float")
	{
		float fv = ct::fvariant(value).GetF();
		res += ct::toStr(u8R"(
	change |= ImGui::SliderFloatDefaultCompact(0.0f, "%s", &m_UBO%s.u_%s, %.3ff, %.3ff, %.3ff, 0.0f, "%%.3f");)",
			m_InputName.GetText().c_str(), m_Stage.c_str(), m_InputName.GetText().c_str(), 0.0f, fv * 2.0f, fv);
	}
	else if (type == "uint")
	{
		uint32_t uv = ct::uvariant(value).GetU();
		res += ct::toStr(u8R"(
	change |= ImGui::SliderUIntDefaultCompact(0.0f, "%s", &m_UBO%s.u_%s, %uU, %uU, %uU);)",
			m_InputName.GetText().c_str(), m_Stage.c_str(), m_InputName.GetText().c_str(), 0U, uv * 2U, uv);
	}
	else if (type == "int")
	{
		int32_t iv = ct::uvariant(value).GetI();
		res += ct::toStr(u8R"(
	change |= ImGui::SliderIntDefaultCompact(0.0f, "%s", &m_UBO%s.u_%s, %i, %i, %i);)",
			m_InputName.GetText().c_str(), m_Stage.c_str(), m_InputName.GetText().c_str(), 0, iv * 2, iv);
	}
	else if (type == "bool")
	{
		bool bv = ct::ivariant(value).GetB();
		res += ct::toStr(u8R"(
	change |= ImGui::CheckBoxBoolDefault("%s", &m_UBO%s.u_%s, %s);)",
			m_InputName.GetText().c_str(), m_Stage.c_str(), m_InputName.GetText().c_str(), bv ? "true" : "false");
	}
	else if (inputIdx == 1) // for help, but not functionnal
	{
		type = "ct::" + type;
		value = type + "(" + value + ", " + m_InputValue_y.GetText(baseType) + ")"; 
		res += ct::toStr(u8R"(
	// change |= %s => ("%s", &m_UBO%s.u_%s => %s);)",
			type.c_str(), m_InputName.GetText().c_str(), m_Stage.c_str(), m_InputName.GetText().c_str(), value.c_str());
	}
	else if (inputIdx == 2) // for help, but not functionnal
	{
		type = "ct::" + type;
		value = type + "(" + value + ", " + m_InputValue_y.GetText(baseType) + ", " + m_InputValue_z.GetText(baseType) + ")";
		res += ct::toStr(u8R"(
	// change |= %s => ("%s", &m_UBO%s.u_%s => %s);)",
			type.c_str(), m_InputName.GetText().c_str(), m_Stage.c_str(), m_InputName.GetText().c_str(), value.c_str());
	}
	else if (inputIdx == 3) // for help, but not functionnal
	{
		type = "ct::" + type;
		value = type + "(" + value + ", " + m_InputValue_y.GetText(baseType) + ", " + m_InputValue_z.GetText(baseType) + ", " + m_InputValue_w.GetText(baseType) + ")";
		res += ct::toStr(u8R"(
	// change |= %s => ("%s", &m_UB%s.u_%s => %s);)",
			type.c_str(), m_InputName.GetText().c_str(), m_Stage.c_str(), m_InputName.GetText().c_str(), value.c_str());
	}

	return res;
}

std::string UBOItem::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string str;

	str += vOffset + ct::toStr("<UBOItem name=\"%s\" typeIndex=\"%u\" vx=\"%s\" vy=\"%s\" vz=\"%s\" vw=\"%s\"/>\n",
		m_InputName.GetText().c_str(), 
		m_InputTypeIndex,
		m_InputValue_x.GetText().c_str(),
		m_InputValue_y.GetText().c_str(),
		m_InputValue_z.GetText().c_str(),
		m_InputValue_w.GetText().c_str());

	return str;
}

bool UBOItem::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strName == "UBOItem")
	{
		for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
		{
			std::string attName = attr->Name();
			std::string attValue = attr->Value();

			if (attName == "name")
				m_InputName.SetText(attValue);
			else if (attName == "typeIndex")
				m_InputTypeIndex = ct::ivariant(attValue).GetI();
			else if (attName == "vx")
				m_InputValue_x.SetText(attValue);
			else if (attName == "vy")
				m_InputValue_y.SetText(attValue);
			else if (attName == "vz")
				m_InputValue_z.SetText(attValue);
			else if (attName == "vw")
				m_InputValue_w.SetText(attValue);
		}
	}

	return false;
}

///////////////////////////////////////////////////////
//// UBOEditor ////////////////////////////////////////
///////////////////////////////////////////////////////

UBOEditor::UBOEditor()
{
	m_InputStageIndex = 0;
}

void UBOEditor::DrawPane(const std::string& vRendererType)
{
	auto stage = UBOEditors::m_StageArray[vRendererType][m_InputStageIndex];

	if (ImGui::ContrastedButton("Add Item"))
	{
		m_Items.push_back(UBOItem());
	}

	ImGui::Indent();

	int32_t idx_to_erase = -1;
	for (size_t idx = 0U; idx < m_Items.size(); ++idx)
	{
		if (ImGui::ContrastedButton("X"))
		{
			idx_to_erase = (int32_t)idx;
		}

		ImGui::SameLine();

		m_Items[idx].DrawItem(stage);
	}

	ImGui::Unindent();

	if (idx_to_erase > -1)
	{
		m_Items.erase(m_Items.begin() + (size_t)idx_to_erase); 
		idx_to_erase = -1;
	}
}

bool UBOEditor::DrawStageSelection(const std::string& vRendererType)
{
	bool change = false;

	if (m_RendererType != vRendererType)
	{
		m_InputStageIndex = 0U;
		change = true;
	}

	m_RendererType = vRendererType;

	change |= ImGui::ContrastedComboVectorDefault(ImGui::GetContentRegionAvail().x, 
		"Stage type", &m_InputStageIndex, UBOEditors::m_StageArray[vRendererType], 0);

	return change;
}

/*std::string UBOEditor::GetUBOCode_Widgets()
{
	std::string res;

	for (auto item : m_Items)
	{
		res += item.GetItemWidget();
	}

	if (!m_Items.empty())
	{
		res += u8R"(
	if (change)
	{
		NeedNewUBOUpload();
	}
)";
	}

	return res;
}

std::string UBOEditor::GetUBOCppHeader()
{
	std::string stage = m_StageArray[m_RendererType][m_InputStageIndex];

	std::string res = ct::toStr(u8R"(
	struct UBO%s {)", stage.c_str());

	for (auto item : m_Items)
	{
		res += item.GetCppItemHeader();
	}

	res += ct::toStr(u8R"(
	} m_UBO%s;
	VulkanBufferObjectPtr m_UBO%s_Ptr = nullptr;
	vk::DescriptorBufferInfo m_UBO_%s_BufferInfos;)", stage.c_str(), stage.c_str(), stage.c_str());

	return res;
}

std::string UBOEditor::GetUBOGlslHeader()
{
	std::string stage = m_StageArray[m_RendererType][m_InputStageIndex];
	
	std::string res = ct::toStr(u8R"(
layout(std140, binding = 0) uniform UBO_%S
{)", stage.c_str());

	for (auto item : m_Items)
	{
		res += item.GetGlslItemHeader();
	}

	res+= ct::toStr(u8R"(
};)");

	return res;
}

std::string UBOEditor::GetFunction_CreateUBO()
{
	std::string stage = m_StageArray[m_RendererType][m_InputStageIndex];

	return ct::toStr(u8R"(
	m_UBO%s_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO%s));
	m_UBO_%s_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO%s_Ptr)
	{
		m_UBO_%s_BufferInfos.buffer = m_UBO%s_Ptr->buffer;
		m_UBO_%s_BufferInfos.range = sizeof(UBO%s);
		m_UBO_%s_BufferInfos.offset = 0;
	}
)", stage.c_str(), stage.c_str(), stage.c_str(), stage.c_str(), stage.c_str(), stage.c_str(), stage.c_str(), stage.c_str(), stage.c_str());
}

std::string UBOEditor::GetFunction_UploadUBO()
{
	std::string stage = m_StageArray[m_RendererType][m_InputStageIndex];

	return ct::toStr(u8R"(
	VulkanRessource::upload(m_VulkanCorePtr, m_UBO%s_Ptr, &m_UBO%s, sizeof(UBO%s));
)", stage.c_str(), stage.c_str(), stage.c_str());
}

std::string UBOEditor::GetFunction_DestroyUBO()
{
	std::string stage = m_StageArray[m_RendererType][m_InputStageIndex];

	return ct::toStr(u8R"(
	m_UBO%s_Ptr.reset();
	m_UBO_%s_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
)", stage.c_str(), stage.c_str());
}*/

std::string UBOEditor::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string str;

	/*str += vOffset + ct::toStr("<UBO stage=\"%u\">\n", m_InputStageIndex);

	for (auto item : m_Items)
	{
		str += item.getXml(vOffset + "\t", vUserDatas);
	}

	str += vOffset + "</UBO>\n";*/

	return str;
}

bool UBOEditor::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	/*if (strName == "UBO")
	{
		for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
		{
			std::string attName = attr->Name();
			std::string attValue = attr->Value();

			if (attName == "stage")
				m_InputStageIndex = ct::ivariant(attValue).GetI();
		}

		RecursParsingConfigChilds(vElem, vUserDatas);
	}

	if (strParentName == "UBO")
	{
		if (strName == "UBOItem")
		{
			UBOItem item;
			item.setFromXml(vElem, vParent, vUserDatas);
			m_Items.push_back(item);
		}
	}*/

	return false;
}

///////////////////////////////////////////////////////
//// UBOEditors : STATIC //////////////////////////////
///////////////////////////////////////////////////////

std::map<std::string, std::vector<std::string>> UBOEditors::m_StageArray;

///////////////////////////////////////////////////////
//// UBOEditors ///////////////////////////////////////
///////////////////////////////////////////////////////

UBOEditors::UBOEditors()
{
	UBOEditors::m_StageArray[RENDERER_TYPE_PIXEL_2D] =
	{
		"Vert",
		"Frag",
		//"Geom",
		//"TessCtrl",
		//"tessEval"
	};
	UBOEditors::m_StageArray[RENDERER_TYPE_COMPUTE_1D] =
		UBOEditors::m_StageArray[RENDERER_TYPE_COMPUTE_2D] =
		UBOEditors::m_StageArray[RENDERER_TYPE_COMPUTE_3D] =
	{
		"Comp"
	};
	UBOEditors::m_StageArray[RENDERER_TYPE_RTX] =
	{
		"RGen",
		"Miss",
		"Ahit",
		"Chit",
		"Inter",
	};
}

void UBOEditors::DrawPane(const std::string& vRendererType)
{
	ImGui::Separator();

	ImGui::CheckBoxBoolDefault("Use A UBO", &m_UseUbos, true);

	if (!m_UseUbos) return;

	if (ImGui::ContrastedButton("Add UBO"))
	{
		if (UBOEditors::m_StageArray.find(vRendererType) != UBOEditors::m_StageArray.end())
		{
			if (!UBOEditors::m_StageArray[vRendererType].empty())
			{
				auto defaultStage = *m_StageArray[vRendererType].begin();
				m_UBOEditors[defaultStage].push_back(UBOEditor());
			}
		}
	}

	uint32_t idx = 0U;
	std::pair<std::string, uint32_t> _editorToMove;
	std::pair<std::string, uint32_t> _editorToDelete;

	ImGui::Indent();
	
	for (auto& uboEditors : m_UBOEditors)
	{
		idx = 0U;
		for (auto& uboEditor : uboEditors.second)
		{
			ImGui::Separator();

			if (ImGui::ContrastedButton("X"))
			{
				_editorToDelete.first = uboEditors.first;
				_editorToDelete.second = idx;
			}

			ImGui::SameLine();

			if (uboEditor.DrawStageSelection(vRendererType))
			{
				_editorToMove.first = uboEditors.first;
				_editorToMove.second = idx;
			}
			else
			{
				uboEditor.DrawPane(vRendererType);
			}

			++idx;
		}
	}

	ImGui::Unindent();

	if (!_editorToMove.first.empty())
	{
		UBOEditor save;

		if (m_UBOEditors.find(_editorToMove.first) != m_UBOEditors.end())
		{
			// save + deletion
			auto& vec = m_UBOEditors[_editorToMove.first];
			if (vec.size() > _editorToMove.second)
			{
				// save
				save = vec[_editorToDelete.second];

				// deletion
				vec.erase(vec.begin() + (size_t)_editorToDelete.second);
			}

			// no more UBo's in the vector, clear the stage
			if (m_UBOEditors.at(_editorToMove.first).size() == 0)
			{
				m_UBOEditors.erase(_editorToMove.first);
			}

			// move
			if (UBOEditors::m_StageArray.find(vRendererType) != UBOEditors::m_StageArray.end())
			{
				if (!UBOEditors::m_StageArray[vRendererType].empty())
				{
					auto stage = UBOEditors::m_StageArray[vRendererType][save.m_InputStageIndex];
					if (!stage.empty())
					{
						m_UBOEditors[stage].push_back(save);
					}
				}
			}
		}

		// clear move situation
		_editorToMove.first.clear();
	}

	if (!_editorToDelete.first.empty())
	{
		// deletion
		if (m_UBOEditors.find(_editorToDelete.first) != m_UBOEditors.end())
		{
			auto& vec = m_UBOEditors[_editorToDelete.first];
			if (vec.size() > _editorToDelete.second)
			{
				// deletion
				vec.erase(vec.begin() + (size_t)_editorToDelete.second);
			}
		}

		// clear delete situation
		_editorToDelete.first.clear();
	}
	
	ImGui::Separator();
}

std::string UBOEditors::Get_Widgets_Header()
{
	return "";
}

std::string UBOEditors::Get_Glsl_Header(const std::string& vStage)
{
	return "";
}

std::string UBOEditors::Get_Cpp_Functions_Imp()
{
	std::string res;

	if (m_UseUbos)
	{
		res += u8R"(

bool PASS_CLASS_NAME::CreateUBO()
{
	ZoneScoped;
)";

		res += Get_Create_Header();

		res += u8R"(

	NeedNewUBOUpload();

	return true;
})";

		res += u8R"(

void PASS_CLASS_NAME::UploadUBO()
{
	ZoneScoped;
)";

		res += Get_Upload_Header();

		res += u8R"(
})";

		res += u8R"(

void PASS_CLASS_NAME::DestroyUBO()
{
	ZoneScoped;)";

		res += Get_Destroy_Header();

		res += u8R"(
})";
	}

	return res;
}

std::string UBOEditors::Get_Cpp_Functions_Header()
{
	std::string res;

	if (m_UseUbos)
	{
		res = u8R"(
	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;
)";
	}

	return res;
}

std::string UBOEditors::Get_Cpp_Header()
{
	return "";
}

std::string UBOEditors::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string str;

	str += vOffset + "<UBOS>\n";

	str += vOffset + "</UBOS>\n";

	return str;
}

bool UBOEditors::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "UBOS")
	{
		if (strName == "UBO")
		{
			/*for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
			{
				std::string attName = attr->Name();
				std::string attValue = attr->Value();

				if (attName == "stage")
					m_InputStageIndex = ct::ivariant(attValue).GetI();
			}

			RecursParsingConfigChilds(vElem, vUserDatas);*/
		}

		if (strName == "UBO")
		{

			UBOEditor editor;
			//editor.setFromXml(vElem, vParent, vUserDatas);
			//m_UBOEditors.push_back(editor);
		}
	}

	return false;
}

std::string UBOEditors::Get_Create_Header()
{
	return "";
}

std::string UBOEditors::Get_Upload_Header()
{
	return "";
}

std::string UBOEditors::Get_Destroy_Header()
{
	return "";
}