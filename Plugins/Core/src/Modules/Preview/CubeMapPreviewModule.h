/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/


#pragma once

#include <set>
#include <array>
#include <string>
#include <memory>

#include <Headers/Globals.h>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <Base/BaseRenderer.h>
#include <Base/QuadShaderPass.h>

#include <vulkan/vulkan.hpp>
#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanDevice.h>
#include <vkFramework/vk_mem_alloc.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/ImGuiTexture.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/VulkanFrameBuffer.h>

#include <Interfaces/GuiInterface.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/NodeInterface.h>
#include <Interfaces/ResizerInterface.h>

#include <Interfaces/TextureCubeInputInterface.h>
#include <Interfaces/TextureOutputInterface.h>

class CubeMapPreview_Quad_Pass;
class CubeMapPreviewModule :
	public NodeInterface,
	public BaseRenderer,	
	public TaskInterface,
	public TextureCubeInputInterface<0U>,
	public TextureOutputInterface,
	public GuiInterface
{
public:
	static std::shared_ptr<CubeMapPreviewModule> Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode);

private:
	ct::cWeak<CubeMapPreviewModule> m_This;

	std::shared_ptr<CubeMapPreview_Quad_Pass> m_CubeMapPreview_Quad_Pass_Ptr = nullptr;

public:
	CubeMapPreviewModule(vkApi::VulkanCorePtr vVulkanCorePtr);
	~CubeMapPreviewModule() override;

	bool Init();

	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
	bool ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;

	void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;

	// Interfaces Setters
	void SetTextureCube(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageCubeInfo, ct::fvec2* vTextureSize = nullptr) override;

	// Interfaces Getters
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};
