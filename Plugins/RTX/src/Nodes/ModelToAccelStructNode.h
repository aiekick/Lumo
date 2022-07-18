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

#include <Graph/Graph.h>

#include <ctools/cTools.h>

#include <Graph/Base/BaseNode.h>
#include <Interfaces/ModelInputInterface.h>
#include <Interfaces/AccelStructureOutputInterface.h>

class ModelToAccelStructModule;
class ModelToAccelStructNode : 
	public BaseNode, 
	public ModelInputInterface,
	public AccelStructureOutputInterface
{
public:
	static std::shared_ptr<ModelToAccelStructNode> Create(vkApi::VulkanCorePtr vVulkanCorePtr);

private:
	std::shared_ptr<ModelToAccelStructModule> m_ModelToAccelStructModule = nullptr;

public:
	ModelToAccelStructNode();
	~ModelToAccelStructNode() override;
	bool Init(vkApi::VulkanCorePtr vVulkanCorePtr) override;
	void Unit() override;
	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
	void Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmmiterSlot = NodeSlotWeak(), const NodeSlotWeak& vReceiverSlot = NodeSlotWeak()) override;
	void DisplayInfosOnTopOfTheNode(BaseNodeState* vBaseNodeState) override;
	void JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot) override;
	void JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot) override;
	void SetModel(SceneModelWeak vSceneModel = SceneModelWeak()) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
};