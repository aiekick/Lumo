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
	std::string GetGlslItemHeader();
	std::string GetCppItemHeader();
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
	std::string m_RendererType;

public:
	int m_InputStageIndex = 0U;

public:
	UBOEditor();
	bool DrawStageSelection(const std::string& vRendererType);
	void DrawPane(const std::string& vRendererType);
	/*std::string Get_Widgets_Header();
	std::string Get_Glsl_Header();
	std::string Get_Cpp_Header();
	std::string Get_Create_Header();
	std::string Get_Upload_Header();
	std::string Get_Destroy_Header();*/

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};

class UBOEditors :
	public conf::ConfigAbstract
{
public:
	static std::map<std::string, std::vector<std::string>> m_StageArray;

private:
	// ubo stage type, ubos per stage
	std::map<std::string, std::vector<UBOEditor>> m_UBOEditors;
	std::string m_RendererType;

public:
	bool m_UseUbos = false;

public:
	UBOEditors();
	void DrawPane(const std::string& vRendererType);

	std::string Get_Cpp_Functions_Imp();
	std::string Get_Cpp_Functions_Header();
	std::string Get_Widgets_Header();
	std::string Get_Glsl_Header(const std::string& vStage);
	std::string Get_Cpp_Header();

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

private:
	std::string Get_Create_Header();
	std::string Get_Upload_Header();
	std::string Get_Destroy_Header();
};