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

#include <array>
#include <memory>

#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Base/ShaderPass.h>
#include <LumoBackend/Base/BaseRenderer.h>

#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Core/VulkanDevice.h>
#include <Gaia/Resources/VulkanRessource.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/TaskInterface.h>
#include <LumoBackend/Interfaces/ModelInputInterface.h>
#include <LumoBackend/Interfaces/TextureInputInterface.h>
#include <LumoBackend/Interfaces/TextureOutputInterface.h>
#include <LumoBackend/Interfaces/CameraInterface.h>
#include <LumoBackend/Interfaces/MergedInterface.h>
#include <LumoBackend/Interfaces/ResizerInterface.h>

class MeshBuffersModule_Mesh_Pass;
class MeshBuffersModule : public BaseRenderer,
                          public NodeInterface,

                          public ModelInputInterface,
                          public TextureInputInterface<0U>,
                          public TextureOutputInterface,
                          public TaskInterface {
public:
    static std::shared_ptr<MeshBuffersModule> Create(GaiApi::VulkanCoreWeak vVulkanCore);

private:
    std::shared_ptr<MeshBuffersModule_Mesh_Pass> m_MeshBuffersModule_Mesh_Pass_Ptr = nullptr;

public:
    MeshBuffersModule(GaiApi::VulkanCoreWeak vVulkanCore);
    ~MeshBuffersModule() override;

    bool Init();

    bool ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd = nullptr, BaseNodeState* vBaseNodeState = nullptr) override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers = nullptr) override;
    void SetModel(SceneModelWeak vSceneModel = SceneModelWeak()) override;
    void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;
    vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;
};