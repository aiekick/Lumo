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
#include <ctools/ConfigAbstract.h>

#include <Gaia/gaia.h>

#include <ctools/cTools.h>
#include <LumoBackend/Base/ShaderPass.h>
#include <ctools/ConfigAbstract.h>

#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Gui/ImGuiTexture.h>

#include <LumoBackend/SceneGraph/SceneMesh.hpp>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/TaskInterface.h>
#include <LumoBackend/Interfaces/TextureGroupOutputInterface.h>
#include <LumoBackend/Interfaces/ModelInputInterface.h>
#include <LumoBackend/Interfaces/LightGroupInputInterface.h>
#include <LumoBackend/Interfaces/LightGroupOutputInterface.h>

class ShadowMapModule_Mesh_Pass : public ShaderPass,

                                  public TextureGroupOutputInterface,
                                  public ModelInputInterface,
                                  public LightGroupInputInterface,
                                  public LightGroupOutputInterface {
protected:
    struct PushConstants {
        uint32_t light_id_to_use = 0U;
    } m_PushConstants;

public:
    ShadowMapModule_Mesh_Pass(GaiApi::VulkanCoreWeak vVulkanCore);
    ~ShadowMapModule_Mesh_Pass() override;

    void ActionBeforeInit() override;
    void DrawModel(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    void SetModel(SceneModelWeak vSceneModel = SceneModelWeak()) override;
    DescriptorImageInfoVector* GetDescriptorImageInfos(const uint32_t& vBindingPoint, fvec2Vector* vOutSizes) override;
    void SetLightGroup(SceneLightGroupWeak vSceneLightGroup) override;
    SceneLightGroupWeak GetLightGroup() override;
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

    void SetLightIdToUse(const uint32_t& vLightID);

private:
    void DestroyModel(const bool& vReleaseDatas = false) override;

    bool UpdateLayoutBindingInRessourceDescriptor() override;
    bool UpdateBufferInfoInRessourceDescriptor() override;

    void SetInputStateBeforePipelineCreation() override;

    std::string GetVertexShaderCode(std::string& vOutShaderName) override;
    std::string GetFragmentShaderCode(std::string& vOutShaderName) override;
};