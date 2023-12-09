/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

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

#include <Gaia/gaia.h>
#include <functional>
#include <ctools/cTools.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <ctools/ConfigAbstract.h>
#include <LumoBackend/Interfaces/CameraInterface.h>
#include <LumoBackend/Interfaces/ModelOutputInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>

class MeshModule : public conf::ConfigAbstract, public NodeInterface, public ModelOutputInterface, public GuiInterface {
public:
    static std::shared_ptr<MeshModule> Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode);

private:
    std::weak_ptr<MeshModule> m_This;
    GaiApi::VulkanCoreWeak m_VulkanCore;

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
    MeshModule(GaiApi::VulkanCoreWeak vVulkanCore);
    virtual ~MeshModule();

    bool Init();
    void Unit();

    std::string GetFileName() {
        return m_FileName;
    }

    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    SceneModelWeak GetModel() override;

private:
    void LoadMesh(const std::string& vFilePathName);
    void MeshLoadingFinished();

public:
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};
