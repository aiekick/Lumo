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

#include <LumoBackend/Headers/LumoBackendDefs.h>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Base/BaseRenderer.h>
#include <LumoBackend/Base/QuadShaderPass.h>

#include <vulkan/vulkan.hpp>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>
#include <Gaia/Core/vk_mem_alloc.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/TaskInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/ResizerInterface.h>

#include <LumoBackend/Interfaces/TextureInputInterface.h>
#include <LumoBackend/Interfaces/TextureOutputInterface.h>

class PlanetModule_Water_Mesh_Pass;
class PlanetModule_Ground_Mesh_Pass;
class PlanetModule_Atmosphere_Mesh_Pass;
class PlanetModule :
	public NodeInterface,
	public BaseRenderer,	
	public TaskInterface,
	public TextureInputInterface<0U>,
	public TextureOutputInterface,
	public GuiInterface
{
public:
	static std::shared_ptr<PlanetModule> Create(GaiApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode);

private:
	std::weak_ptr<PlanetModule> m_This;
	std::shared_ptr<PlanetModule_Ground_Mesh_Pass> m_PlanetModule_Ground_Mesh_Pass_Ptr = nullptr;
	std::shared_ptr<PlanetModule_Atmosphere_Mesh_Pass> m_PlanetModule_Atmosphere_Mesh_Pass_Ptr = nullptr;
	std::shared_ptr<PlanetModule_Water_Mesh_Pass> m_PlanetModule_Water_Mesh_Pass_Ptr = nullptr;
	FrameBufferPtr m_FrameBufferPtr = nullptr;

public:
	PlanetModule(GaiApi::VulkanCorePtr vVulkanCorePtr);
	~PlanetModule() override;

	bool Init();

	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
	bool ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
	void RenderShaderPasses(vk::CommandBuffer* vCmdBuffer) override;

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;

	void NeedResizeByHand(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
	void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
	bool ResizeIfNeeded() override;

	// Interfaces Setters
	void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize = nullptr) override;

	// Interfaces Getters
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
	void AfterNodeXmlLoading() override;

private:
	void CreateCube();
};
