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
#include <vkFramework/vkFramework.h>
#include <vkFramework/ImGuiTexture.h>
#include <Panes/Abstract/AbstractPane.h>
#include <vkFramework/VulkanFrameBuffer.h>
#include <Interfaces/GuiInterface.h>
#include <Graph/Graph.h>

class ProjectFile;
class VulkanImGuiRenderer;
class View3DPane : public AbstractPane
{
private:
	NodeSlotWeak m_TextureOutputSlot;
	ImGuiTexture m_ImGuiTexture;
	ct::irect m_PreviewRect;
	VulkanImGuiRendererWeak m_VulkanImGuiRenderer;

	uint32_t m_PreviewBufferId = 0;
	bool m_CanWeTuneCamera = true;
	bool m_CanWeTuneMouse = true;
	bool m_DisplayPictureByRatio = true;
	bool m_CanWeTuneGizmo = false;
	float m_DisplayQuality = 1.0f;
	ct::fvec2 m_CurrNormalizedMousePos;
	ct::fvec2 m_LastNormalizedMousePos;
	bool m_MouseDrag = false;
	bool m_UINeedRefresh = false;
	uint32_t m_MaxBuffers = 0;

	//for send the resize event to nodes
	ct::ivec2 m_PaneSize;

public:
	bool Init() override;
	void Unit() override;
	int DrawPanes(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas, PaneFlags& vInOutPaneShown) override;
	void DrawDialogsAndPopups(const uint32_t& vCurrentFrame, std::string vUserDatas) override;
	int DrawWidgets(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas) override;

	ct::fvec2 SetOrUpdateOutput(NodeSlotWeak vTextureOutputSlot);
	void SetVulkanImGuiRenderer(VulkanImGuiRendererWeak vVulkanImGuiRenderer);

private:
	void SetDescriptor(vkApi::VulkanFrameBufferAttachment* vVulkanFrameBufferAttachment);
	bool CanUpdateMouse(bool vWithMouseDown, int vMouseButton);
	void UpdateCamera(ImVec2 vOrg, ImVec2 vSize);

public: // singleton
	static std::shared_ptr<View3DPane> Instance()
	{
		static std::shared_ptr<View3DPane> _instance = std::make_shared<View3DPane>();
		return _instance;
	}

public:
	View3DPane(); // Prevent construction
	View3DPane(const View3DPane&) = default; // Prevent construction by copying
	View3DPane& operator =(const View3DPane&) { return *this; }; // Prevent assignment
	~View3DPane(); // Prevent unwanted destruction;
};
