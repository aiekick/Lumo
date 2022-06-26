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
#include <Modules/Output/OutputModule.h>
#include <Interfaces/GuiInterface.h>

class RenderTask;
class ProjectFile;
class VulkanImGuiRenderer;
class View3DPane : public AbstractPane
{
private:
	OutputModuleWeak m_OutputModule;
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

	VulkanImGuiRenderer* m_VulkanImGuiRenderer = nullptr;

public:
	bool Init() override;
	void Unit() override;
	int DrawPanes(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas) override;
	void DrawDialogsAndPopups(const uint32_t& vCurrentFrame, std::string vUserDatas) override;
	int DrawWidgets(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas) override;

	void SetOrUpdateOutput(ct::cWeak<OutputModule> vOutputModule);
	void SetVulkanImGuiRenderer(VulkanImGuiRenderer* vVulkanImGuiRenderer);

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
