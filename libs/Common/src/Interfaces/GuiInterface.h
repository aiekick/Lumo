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
#pragma warning(disable : 4251)

#include <imgui/imgui.h>
#include <ctools/cTools.h>
#include <Common/Globals.h>

class VulkanImGuiRenderer;
class COMMON_API GuiInterface
{
public:
	ImVec4 goodColor = ImVec4(0.2f, 0.8f, 0.2f, 0.8f);
	ImVec4 badColor = ImVec4(0.8f, 0.2f, 0.2f, 0.8f);

public:
	ImVec4 GetUniformLocColor(bool vUsed)
	{
		if (vUsed)
			return goodColor;
		return badColor;
	}

public:
	virtual bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext *vContext = nullptr) = 0;
	virtual void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) = 0;
	virtual void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) = 0;
};