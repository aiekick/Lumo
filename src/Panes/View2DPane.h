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
class View2DPane : public AbstractPane
{
private:
	NodeSlotWeak m_TextureOutputSlot;
	ImGuiTexture m_ImGuiTexture;
	ct::irect m_PreviewRect;
	VulkanImGuiRendererWeak m_VulkanImGuiRenderer;

	Texture2DPtr m_BGTexture = nullptr; // pour y afficher le damier
	ImGuiTexture m_ImGuiBGTexture;

	uint32_t m_PreviewBufferId = 0;
	bool m_CanWeTuneMouse = true;
	float m_DisplayQuality = 1.0f;
	ct::fvec2 m_CurrNormalizedMousePos;
	ct::fvec2 m_LastNormalizedMousePos;
	bool m_MouseDrag = false;
	bool m_UINeedRefresh = false;
	uint32_t m_MaxBuffers = 0;

public:
	bool Init() override;
	void Unit() override;
	int DrawPanes(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas) override;
	void DrawDialogsAndPopups(const uint32_t& vCurrentFrame, std::string vUserDatas) override;
	int DrawWidgets(const uint32_t& vCurrentFrame, int vWidgetId, std::string vUserDatas) override;

	ct::fvec2 SetOrUpdateOutput(NodeSlotWeak vTextureOutputSlot);
	void SetVulkanImGuiRenderer(VulkanImGuiRendererWeak vVulkanImGuiRenderer);

private:
	bool CanUpdateMouse(bool vWithMouseDown, int vMouseButton);

public: // singleton
	static std::shared_ptr<View2DPane> Instance()
	{
		static std::shared_ptr<View2DPane> _instance = std::make_shared<View2DPane>();
		return _instance;
	}

public:
	View2DPane(); // Prevent construction
	View2DPane(const View2DPane&) = default; // Prevent construction by copying
	View2DPane& operator =(const View2DPane&) { return *this; }; // Prevent assignment
	~View2DPane(); // Prevent unwanted destruction;
};
