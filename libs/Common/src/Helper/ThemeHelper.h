/*
Copyright 2022-2022 Stephane Cuillerdier (aka aiekick)

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

#pragma once

#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <ctools/ConfigAbstract.h>
#include <imgui/imgui.h>
#include <Common/Globals.h>
#include <string>
#include <map>
#include <ImGuiColorTextEdit/TextEditor.h>
#ifdef USE_NODEGRAPH
#include <imgui_node_editor/NodeEditor/Source/imgui_node_editor_internal.h>
#endif
 //#define USE_SHADOW

class COMMON_API ThemeHelper : public conf::ConfigAbstract
{
public:
#ifdef USE_SHADOW
	static float puShadowStrength; // low value is darker than higt (0.0f - 2.0f)
	static bool puUseShadow;
	static bool puUseTextureForShadow;
#endif
	bool puShowImGuiStyleEdtor = false;
	bool puShowTextEditorStyleEditor = false;

private:
	std::map<std::string, IGFD::FileStyle> prFileTypeInfos;
	ImGuiStyle prImGuiStyle;
#ifdef USE_NODEGRAPH
	ax::NodeEditor::Style prNodeGraphStyle;
#endif
	TextEditor::Palette prTextEditorPalette = {};

public:
	void Draw();
	void DrawMenu();
	void ShowCustomImGuiStyleEditor(bool* vOpen, ImGuiStyle* ref = nullptr);
	void ShowCustomTextEditorStyleEditor(bool* vOpen);
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

	ImGuiStyle GetImGuiStyle() { return prImGuiStyle; }

	void ApplyStyle();

#ifdef USE_NODEGRAPH
	ax::NodeEditor::Style GetNodeGraphStyle() { return prNodeGraphStyle; }
#endif
	TextEditor::Palette GetTextEditorStyle() { return prTextEditorPalette; }

private:
	void ApplyStyleColorsDefault();
	void ApplyStyleColorsOrangeBlue();
	void ApplyStyleColorsGreenBlue();
	void ApplyStyleColorsClassic();
	void ApplyStyleColorsDark();
	void ApplyStyleColorsLight();
	void ApplyStyleColorsDarcula();
	void ApplyStyleColorsRedDark();

	void ApplyPalette(TextEditor::Palette vPalette);
	TextEditor::Palette GetPalette();

	void ApplyFileTypeColors();

	// ImGuiColorTextEdit Palette
	const char* Get_ImGuiColorTextEditPalette_NameFromCol(TextEditor::PaletteIndex idx);
	TextEditor::PaletteIndex Get_ImGuiColorTextEditPalette_ColFromName(const std::string& vName);
	std::string GetStyleColorName(ImGuiCol idx);
	int GetImGuiColFromName(const std::string& vName);

public: // singleton
	static ThemeHelper* Instance()
	{
		static ThemeHelper _instance;
		return &_instance;
	}

protected:
	ThemeHelper(); // Prevent construction
	ThemeHelper(const ThemeHelper&) = default; // Prevent construction by copying
	ThemeHelper& operator =(const ThemeHelper&) { return *this; }; // Prevent assignment
	~ThemeHelper(); // Prevent unwanted destruction
};
