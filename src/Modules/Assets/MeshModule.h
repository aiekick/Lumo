/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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

namespace vkApi { class VulkanCore; }
class MeshModule : 
	public conf::ConfigAbstract, 
	public NodeInterface,
	public ModelOutputInterface,
	public GuiInterface
{
public:
	static std::shared_ptr<MeshModule> Create(vkApi::VulkanCore* vVulkanCore, BaseNodeWeak vParentNode);

private:
	ct::cWeak<MeshModule> m_This;

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
	vkApi::VulkanCore* m_VulkanCore = nullptr;

	std::string unique_OpenMeshFileDialog_id;

public:
	MeshModule(vkApi::VulkanCore* vVulkanCore);
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
