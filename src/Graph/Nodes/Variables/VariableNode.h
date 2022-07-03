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

#include <Graph/Graph.h>
#include <Graph/Base/BaseNode.h>
#include <Interfaces/VariableOutputInterface.h>

class VariableModule;
class VariableNode : 
	public BaseNode,
	public VariableOutputInterface
{
public:
	static std::shared_ptr<VariableNode> Create(vkApi::VulkanCorePtr vVulkanCorePtr, const NodeTypeEnum& vNodeType);

private:
	std::shared_ptr<VariableModule> m_VariableModulePtr = nullptr;

public:
	VariableNode(const NodeTypeEnum& vNodeType);
	~VariableNode() override;
	bool Init(vkApi::VulkanCorePtr vVulkanCorePtr) override;
	bool Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void DisplayInfosOnTopOfTheNode(BaseNodeStateStruct* vCanvasState) override;
	void Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot = NodeSlotWeak(), const NodeSlotWeak& vReceiverSlot = NodeSlotWeak()) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
	void DrawOutputWidget(BaseNodeStateStruct* vCanvasState, NodeSlotWeak vSlot) override;
	SceneVariableWeak GetVariable() override;
};