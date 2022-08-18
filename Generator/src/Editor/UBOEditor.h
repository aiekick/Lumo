#pragma once

#include <ImWidgets/ImWidgets.h>
#include <ctools/ConfigAbstract.h>

#include <string>
#include <vector>
class UBOItem :
	public conf::ConfigAbstract
{
private:
	std::vector<std::string> m_TypeArray =
	{
		"float",
		"vec2",
		"vec3",
		"vec4",
		"int",
		"ivec2",
		"ivec3",
		"ivec4",
		"uint",
		"uvec2",
		"uvec3",
		"uvec4",
		"bool",
		"bvec2",
		"bvec3",
		"bvec4"
	};
	int m_InputTypeIndex = 0U;
	ImWidgets::InputText m_InputName;
	ImWidgets::InputText m_InputValue_x;
	ImWidgets::InputText m_InputValue_y;
	ImWidgets::InputText m_InputValue_z;
	ImWidgets::InputText m_InputValue_w;
	std::string m_Stage;

public:
	UBOItem() = default;
	void DrawItem(const std::string& vStage);
	std::string GetItemHeader();
	std::string GetItemWidget();
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};

class UBOEditor : 
	public conf::ConfigAbstract
{
private:
	// name, type, default value
	std::vector<UBOItem> m_Items;

public:
	std::vector<std::string> m_StageArray =
	{
		"Common",
		"Vert",
		"Frag",
		"Geom",
		"TessCtrl",
		"tessEval",
		"Comp",
		"RGen",
		"Miss",
		"Ahit",
		"Chit",
		"Inter",
	};
	int m_InputStageIndex = 0U;

public:
	void DrawStageSelection();
	void DrawPane(const ImVec2& vSize);
	std::string GetUBOCode_Widgets();
	std::string GetUBOHeader();
	std::string GetFunction_CreateUBO();
	std::string GetFunction_UploadUBO();
	std::string GetFunction_DestroyUBO();

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};