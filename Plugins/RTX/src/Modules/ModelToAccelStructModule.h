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

#include <Headers/Globals.h>

#include <vulkan/vulkan.hpp>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <Base/BaseRenderer.h>

#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanDevice.h>
#include <vkFramework/vk_mem_alloc.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/ImGuiTexture.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/VulkanFrameBuffer.h>

#include <SceneGraph/SceneMesh.h>

#include <Interfaces/TaskInterface.h>
#include <Interfaces/ModelInputInterface.h>
#include <Interfaces/AccelStructureOutputInterface.h>

class ModelToAccelStructModule_Pass;
class ModelToAccelStructModule : 
	public BaseRenderer,
	public TaskInterface,
	public ModelInputInterface,
	public AccelStructureOutputInterface
{
public:
	static std::shared_ptr<ModelToAccelStructModule> Create(vkApi::VulkanCorePtr vVulkanCorePtr);

private:
	std::shared_ptr<ModelToAccelStructModule_Pass> m_ModelToAccelStructModule_Pass_Ptr = nullptr;

public:
	ModelToAccelStructModule(vkApi::VulkanCorePtr vVulkanCorePtr);
	~ModelToAccelStructModule() override;

	bool Init();

	bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
	void SetModel(SceneModelWeak vSceneModel) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
	SceneModelWeak GetAccelStruct() override;
};