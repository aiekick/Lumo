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

#include <Panes/Abstract/AbstractPane.h>

#include <imgui/imgui.h>
#include <vulkan/vulkan.hpp>
#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanFrameBuffer.h>
#include <ImGuiColorTextEdit/TextEditor.h>
#include <Modules/Output/Output2DModule.h>
#include <vkFramework/ImGuiTexture.h>
#include <ctools/cTools.h>
#include <cstdint>
#include <string>
#include <memory>

class UniformWidgetCompute;
class RenderTask;
class ProjectFile;
class View2DPane : public AbstractPane
{
private:
	Output2DModuleWeak m_Output2DModule;
	Texture2DPtr m_BGTexture = nullptr; // pour y afficher le damier
	ImGuiTexture m_ImGuiBGTexture;
	ImGuiTexture m_ImGuiTexture;
	ct::irect m_PreviewRect;
	VulkanImGuiRendererPtr m_VulkanImGuiRenderer = nullptr;

public:
	bool Init() override;
	void Unit() override;
	int DrawPanes(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas) override;
	void DrawDialogsAndPopups(const uint32_t& vCurrentFrame, std::string vUserDatas) override;
	int DrawWidgets(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas) override;

	void SetOrUpdateOutput(ct::cWeak<Output2DModule> vOutput2DModule);
	void SetVulkanImGuiRenderer(VulkanImGuiRendererPtr vVulkanImGuiRenderer);

public: // singleton
	static View2DPane* Instance()
	{
		static View2DPane _instance;
		return &_instance;
	}

protected:
	View2DPane(); // Prevent construction
	View2DPane(const View2DPane&) = default; // Prevent construction by copying
	View2DPane& operator =(const View2DPane&) { return *this; }; // Prevent assignment
	~View2DPane(); // Prevent unwanted destruction;
};
