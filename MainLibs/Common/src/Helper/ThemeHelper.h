/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <ctools/ConfigAbstract.h>
#include <imgui/imgui.h>
#include <string>
#include <map>
#include <ImGuiColorTextEdit/TextEditor.h>
#ifdef USE_NODEGRAPH
#include <imgui_node_editor/NodeEditor/Source/imgui_node_editor_internal.h>
#endif
 //#define USE_SHADOW

class ThemeHelper : public conf::ConfigAbstract
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
