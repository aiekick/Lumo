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

#include <set>
#include <array>
#include <string>
#include <memory>



#include <Gaia/gaia.h>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>
#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/TaskInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/VariableOutputInterface.h>

class VariableModule_Pass;
class VariableModule :
	public conf::ConfigAbstract,
	public GuiInterface,
	public TaskInterface,
	public NodeInterface,
	public VariableOutputInterface
{
public:
	static std::shared_ptr<VariableModule> Create(const std::string& vNodeType, BaseNodeWeak vParentNode);

private:
	SceneVariablePtr m_VariablePtr = nullptr;

public:
	VariableModule();
	~VariableModule();

	bool Init(const std::string& vNodeType);
	void Unit();

	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
	bool DrawNodeWidget(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr) override;
	SceneVariableWeak GetVariable(const uint32_t& vVariableIndex) override;
};