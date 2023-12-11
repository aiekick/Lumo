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
#include <LumoBackend/Interfaces/CameraInterface.h>
#include <LumoBackend/Interfaces/ModelInputInterface.h>
#include <LumoBackend/Interfaces/Texture2DInputInterface.h>
#include <LumoBackend/Interfaces/Texture2DOutputInterface.h>
#include <LumoBackend/Interfaces/ResizerInterface.h>

class MeshBuffersModule_Mesh_Pass : public ShaderPass,

                                    public ModelInputInterface,
                                    public Texture2DInputInterface<1U>,
                                    public Texture2DOutputInterface {
private:
    VulkanBufferObjectPtr m_UBOVertPtr = nullptr;
    vk::DescriptorBufferInfo m_DescriptorBufferInfo_Vert;

    struct UBOVert {
        alignas(16) glm::mat4x4 transform = glm::mat4x4(1.0f);
    } m_UBOVert;

    VulkanBufferObjectPtr m_UBOFragPtr = nullptr;
    vk::DescriptorBufferInfo m_DescriptorBufferInfo_Frag;

    struct UBOFrag {
        alignas(4) float use_sampler_mask = 0.0f;
    } m_UBOFrag;

public:
    MeshBuffersModule_Mesh_Pass(GaiApi::VulkanCoreWeak vVulkanCore);
    ~MeshBuffersModule_Mesh_Pass() override;

    void DrawModel(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    void SetModel(SceneModelWeak vSceneModel = SceneModelWeak()) override;
    void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;
    vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;

private:
    void DestroyModel(const bool& vReleaseDatas = false) override;

    bool CreateUBO() override;
    void UploadUBO() override;
    void DestroyUBO() override;

    bool UpdateLayoutBindingInRessourceDescriptor() override;
    bool UpdateBufferInfoInRessourceDescriptor() override;

    void SetInputStateBeforePipelineCreation() override;

    std::string GetVertexShaderCode(std::string& vOutShaderName) override;
    std::string GetFragmentShaderCode(std::string& vOutShaderName) override;
};
