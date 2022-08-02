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

#include <string>
#include <memory>
#include <cstdint>
#include <imgui/imgui.h>
#include <ctools/cTools.h>
#include <vulkan/vulkan.hpp>
#include <vkFramework/ImGuiTexture.h>
#include <Panes/Abstract/AbstractPane.h>
#include <vkFramework/VulkanFrameBuffer.h>
#include <ImGuiColorTextEdit/TextEditor.h>
#include <Modules/Output/Output3DModule.h>
#include <Interfaces/GuiInterface.h>

class RenderTask;
class ProjectFile;
class VulkanImGuiRenderer;
class View3DPane : public AbstractPane
{
private:
	Output3DModuleWeak m_Output3DModule;
	ImGuiTexture m_ImGuiTexture;

	ct::irect m_PreviewRect;
	uint32_t m_PreviewBufferId = 0;

	bool m_CanWeTuneCamera = true;
	bool m_CanWeTuneMouse = true;
	bool m_CanWeTuneGizmo = false;
	bool m_ShowDepth = false;
	float m_DisplayQuality = 1.0f;
	ct::fvec2 m_CurrNormalizedMousePos;
	ct::fvec2 m_LastNormalizedMousePos;
	bool m_MouseDrag = false;
	bool m_UINeedRefresh = false;
	uint32_t m_MaxBuffers = 0;

	VulkanImGuiRendererPtr m_VulkanImGuiRenderer = nullptr;

public:
	bool Init() override;
	void Unit() override;
	int DrawPanes(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas) override;
	void DrawDialogsAndPopups(const uint32_t& vCurrentFrame, std::string vUserDatas) override;
	int DrawWidgets(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas) override;

	ct::fvec2 SetOrUpdateOutput(ct::cWeak<Output3DModule> vOutput3DModule);
	void SetVulkanImGuiRenderer(VulkanImGuiRendererPtr vVulkanImGuiRenderer);

private:
	void SetDescriptor(vkApi::VulkanFrameBufferAttachment* vVulkanFrameBufferAttachment);
	bool CanUpdateMouse(bool vWithMouseDown, int vMouseButton);
	void UpdateCamera(ImVec2 vOrg, ImVec2 vSize);

public: // singleton
	static View3DPane* Instance()
	{
		static View3DPane _instance;
		return &_instance;
	}

protected:
	View3DPane(); // Prevent construction
	View3DPane(const View3DPane&) = default; // Prevent construction by copying
	View3DPane& operator =(const View3DPane&) { return *this; }; // Prevent assignment
	~View3DPane(); // Prevent unwanted destruction;
};
