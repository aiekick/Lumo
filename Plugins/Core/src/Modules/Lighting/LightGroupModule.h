#pragma once

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
#include <Interfaces/TaskInterface.h>
#include <Interfaces/NodeInterface.h>
#include <Interfaces/GizmoInterface.h>
#include <Interfaces/LightGroupOutputInterface.h>
#include <Interfaces/BufferObjectInterface.h>

class LightGroupModule;
typedef std::shared_ptr<LightGroupModule> LightGroupModulePtr;
typedef ct::cWeak<LightGroupModule> LightGroupModuleWeak;


class LightGroupModule :
	public conf::ConfigAbstract,
	public NodeInterface,
	public GuiInterface,
	public LightGroupOutputInterface,
	public TaskInterface
{
public:
	static LightGroupModulePtr Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode);

private:
	LightGroupModuleWeak m_This;
	SceneLightGroupPtr m_SceneLightGroupPtr = nullptr;
	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;

private: // imgui
	ct::fvec4 m_DefaultLightGroupColor = 1.0f;
	// becasue by default there is one light
	// and we must avoid adding a new one for nothing during xml parsing
	// the second time, we check this var, adding a new light is ok
	bool m_FirstXmlLightGroup = true;

public:
	LightGroupModule(vkApi::VulkanCorePtr vVulkanCorePtr);
	~LightGroupModule();

	bool Init();
	void Unit();
	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	SceneLightGroupWeak GetLightGroup() override;

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};