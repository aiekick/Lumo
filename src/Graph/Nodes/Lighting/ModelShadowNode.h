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
#include <Graph/Base/BaseNode.h>
#include <Interfaces/TextureInputInterface.h>
#include <Interfaces/TextureGroupInputInterface.h>
#include <Interfaces/TextureOutputInterface.h>
#include <Interfaces/LightGroupInputInterface.h>
#include <Interfaces/ShaderUpdateInterface.h>

class ModelShadowModule;
class ModelShadowNode : 
	public BaseNode,
	public TextureInputInterface<0U>,
	public TextureGroupInputInterface<0U>,
	public TextureOutputInterface,
	public LightGroupInputInterface,
	public ShaderUpdateInterface
{
public:
	static std::shared_ptr<ModelShadowNode> Create(vkApi::VulkanCorePtr vVulkanCorePtr);

private:
	std::shared_ptr<ModelShadowModule> m_ModelShadowModulePtr = nullptr;

public:
	ModelShadowNode();
	~ModelShadowNode() override;
	bool Init(vkApi::VulkanCorePtr vVulkanCorePtr) override;
	void Unit() override;
	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;
	void SetTextures(const uint32_t& vBinding, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;
	void SetLightGroup(SceneLightGroupWeak vSceneLightGroup = SceneLightGroupWeak()) override;
	void Notify(const NotifyEvent& vEvent, const NodeSlotWeak& vEmitterSlot = NodeSlotWeak(), const NodeSlotWeak& vReceiverSlot = NodeSlotWeak()) override;
	void NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers) override;
	void JustConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot) override;
	void JustDisConnectedBySlots(NodeSlotWeak vStartSlot, NodeSlotWeak vEndSlot) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
	void UpdateShaders(const std::set<std::string>& vFiles) override;
};