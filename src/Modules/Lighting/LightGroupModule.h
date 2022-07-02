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

#include <array>
#include <string>
#include <vector>
#include <memory>
#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <glm/glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ctools/ConfigAbstract.h>
#include <Interfaces/GuiInterface.h>
#include <Interfaces/NodeInterface.h>
#include <Interfaces/GizmoInterface.h>
#include <Interfaces/LightOutputInterface.h>
#include <Interfaces/BufferObjectInterface.h>

class LightGroupModule;
typedef std::shared_ptr<LightGroupModule> LightGroupModulePtr;
typedef ct::cWeak<LightGroupModule> LightGroupModuleWeak;

namespace vkApi { class VulkanCore; }
class LightGroupModule :
	public conf::ConfigAbstract,
	public NodeInterface,
	public GuiInterface,
	public LightOutputInterface,
	public BufferObjectInterface
{
public:
	static LightGroupModulePtr Create(vkApi::VulkanCore* vVulkanCore, BaseNodeWeak vParentNode);
	static std::string GetBufferObjectStructureHeader(const uint32_t& vBinding);

private:
	LightGroupModuleWeak m_This;
	SceneLightGroupPtr m_SceneLightGroupPtr = nullptr;
	vkApi::VulkanCore* m_VulkanCore = nullptr;

private: // imgui
	ct::fvec4 m_DefaultLightColor = 1.0f;

public:
	LightGroupModule(vkApi::VulkanCore* vVulkanCore);
	~LightGroupModule();

	bool Init();
	void Unit();

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;

	SceneLightGroupWeak GetLightGroup();

	///////////////////////////////////////////////////////
	//// BUFFER OBJECTS ///////////////////////////////////
	///////////////////////////////////////////////////////

	void UploadBufferObjectIfDirty(vkApi::VulkanCore* vVulkanCore) override;
	bool CreateBufferObject(vkApi::VulkanCore* vVulkanCore) override;
	void DestroyBufferObject() override;
	vk::DescriptorBufferInfo* GetBufferInfo() override;

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};