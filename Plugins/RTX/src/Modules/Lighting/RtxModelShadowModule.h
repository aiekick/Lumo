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

#include <LumoBackend/Headers/LumoBackendDefs.h>

#include <vulkan/vulkan.hpp>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Base/BaseRenderer.h>

#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>
#include <Gaia/Core/vk_mem_alloc.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/SceneGraph/SceneMesh.hpp>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/TaskInterface.h>
#include <LumoBackend/Interfaces/ShaderInterface.h>
#include <LumoBackend/Interfaces/Texture2DInputInterface.h>
#include <LumoBackend/Interfaces/Texture2DOutputInterface.h>
#include <LumoBackend/Interfaces/ResizerInterface.h>
#include <LumoBackend/Interfaces/SerializationInterface.h>
#include <LumoBackend/Interfaces/TextureGroupInputInterface.h>
#include <LumoBackend/Interfaces/LightGroupInputInterface.h>
#include <LumoBackend/Interfaces/ModelInputInterface.h>
#include <Interfaces/AccelStructureInputInterface.h>

class ModelShadow_Rtx_Pass;
class RtxModelShadowModule : public BaseRenderer,
                             public TaskInterface,
                             public Texture2DOutputInterface,
                             public LightGroupInputInterface,
                             public AccelStructureInputInterface {
public:
    static std::shared_ptr<RtxModelShadowModule> Create(GaiApi::VulkanCoreWeak vVulkanCore);

private:
    std::shared_ptr<ModelShadow_Rtx_Pass> m_ModelShadow_Rtx_Pass_Ptr = nullptr;

public:
    RtxModelShadowModule(GaiApi::VulkanCoreWeak vVulkanCore);
    ~RtxModelShadowModule() override;

    bool Init();

    bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr, void* vUserDatas = nullptr) override;
    void SetAccelStructure(SceneAccelStructureWeak vSceneAccelStructure) override;
    void SetLightGroup(SceneLightGroupWeak vSceneLightGroup = SceneLightGroupWeak()) override;
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};