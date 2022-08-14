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

#include <functional>
#include <ctools/cTools.h>
#include <vulkan/vulkan.hpp>
#include <Graph/Base/BaseNode.h>
#include <ctools/ConfigAbstract.h>
#include <Interfaces/CameraInterface.h>
#include <Interfaces/ModelOutputInterface.h>
#include <Interfaces/NodeInterface.h>
#include <Interfaces/GuiInterface.h>
#include <Utils/Mesh/VertexStruct.h>


class MeshModule : 
	public conf::ConfigAbstract, 
	public NodeInterface,
	public ModelOutputInterface,
	public GuiInterface
{
public:
	static std::shared_ptr<MeshModule> Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode);

private:
	ct::cWeak<MeshModule> m_This;
	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;

	std::string m_FilePathName;
	std::string m_FilePath;
	std::string m_FileName;

	bool m_Loaded = false;
	bool m_NeedResize = false;
	bool m_CanWeRender = true;
	bool m_JustReseted = false;
	bool m_UseDepth = false;
	bool m_NeedToClear = false;
	bool m_NeedModelUpdate = false;

	SceneModelPtr m_SceneModelPtr = nullptr;

	std::string unique_OpenMeshFileDialog_id;

public:
	MeshModule(vkApi::VulkanCorePtr vVulkanCorePtr);
	virtual ~MeshModule();

	bool Init();
	void Unit();

	std::string GetFileName() { return m_FileName; }

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	SceneModelWeak GetModel() override;

private:
	void LoadMesh(const std::string& vFilePathName);
	void MeshLoadingFinished();

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};
