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

#include <Gaia/gaia.h>
#include <ImGuiPack.h>
#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

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

class SubdivideModule : public NodeInterface,
                        public conf::ConfigAbstract,
                        public ModelInputInterface,
                        public ModelOutputInterface,
                        public GuiInterface {
public:
    static std::shared_ptr<SubdivideModule> Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode);

private:
    std::weak_ptr<SubdivideModule> m_This;
    GaiApi::VulkanCoreWeak m_VulkanCore;
    SceneModelWeak m_InputModel;
    SceneModelPtr m_OutputModelPtr = nullptr;

public:
    SubdivideModule(GaiApi::VulkanCoreWeak vVulkanCore);
    ~SubdivideModule();

    bool Init();
    void Unit();

    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) override;

    // Interfaces Setters
    void SetModel(SceneModelWeak vSceneModel) override;

    // Interfaces Getters
    SceneModelWeak GetModel() override;

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
    void AfterNodeXmlLoading() override;
};
