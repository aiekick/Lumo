/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

#pragma once

#include <set>
#include <array>
#include <string>
#include <memory>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Base/BaseRenderer.h>
#include <LumoBackend/Base/QuadShaderPass.h>

#include <ImGuiPack.h>

#include <Gaia/gaia.h>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>

#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/TaskInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/ResizerInterface.h>

#include <LumoBackend/Interfaces/ModelInputInterface.h>
#include <LumoBackend/Interfaces/ModelOutputInterface.h>
#include <LumoBackend/Interfaces/VariableInputInterface.h>

class ModelExporterModule : public NodeInterface,
                            public TaskInterface,
                            public conf::ConfigAbstract,
                            public ModelInputInterface,
                            public ModelOutputInterface,
                            public VariableInputInterface<1U>,
                            public GuiInterface {
public:
    static std::shared_ptr<ModelExporterModule> Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode);

private:
    std::weak_ptr<ModelExporterModule> m_This;
    GaiApi::VulkanCoreWeak m_VulkanCore;
    std::string unique_SaveMeshFileDialog_id;
    std::string unique_SavePathFileDialog_id;
    SceneModelWeak m_InputModel;
    std::string m_FilePathName;
    std::string m_FilePath;
    std::string m_FileName;
    bool m_ExportFrames = false;
    bool m_PreviewStarted = false;
    float m_TimeStep = 0.1f;
    std::string m_SketchFabTimeFrameFileContent;
    uint32_t m_CurrentFrameToExport = 0U;
    uint32_t m_LastSavedFrame = 0U;

private:  // to save
    uint32_t m_FramesCountToExport = 10U;
    ImWidgets::InputText m_InputTextPrefix;
    ImWidgets::InputText m_InputTextSaveFilePathName;
    uint32_t m_FramesCountToJump = 10U;
    uint32_t m_FramesCountPerSec = 60U;
    bool m_GenerateSketchFabTimeFrame = true;
    bool m_ExportVertexColors = true;
    bool m_ExportNormals = true;
    bool m_AutoSaverEnabled = false;
    bool m_AutoSaverPreviewEnabled = false;

public:
    ModelExporterModule(GaiApi::VulkanCoreWeak vVulkanCore);
    ~ModelExporterModule();

    bool Init();
    void Unit();

    bool ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;

    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

    // Interfaces Setters
    void SetModel(SceneModelWeak vSceneModel) override;
    void SetVariable(const uint32_t& vVarIndex, SceneVariableWeak vSceneVariable, void* vUserDatas) override;

    // Interfaces Getters
    SceneModelWeak GetModel() override;

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
    void AfterNodeXmlLoading() override;

    bool isPreviewModeEnabled() {
        return m_AutoSaverPreviewEnabled;
    }
    bool isPreviewModeStarted() {
        return m_PreviewStarted;
    }
    bool isExportModeRunning() {
        return m_ExportFrames;
    }

private:
    void m_SaveModel(const std::string& vFilePathName);
    void m_StartAutoSave();
    void m_StopAutoSave();
    void m_AutoSaveModelIfNeeded(const uint32_t& vCurrentFrame);
};
