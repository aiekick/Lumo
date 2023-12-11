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
#include <LumoBackend/Interfaces/GuiInterface.h>

#include <set>
#include <string>

#include <Gaia/gaia.h>

#include <ctools/cTools.h>
#include <LumoBackend/Base/QuadShaderPass.h>
#include <ctools/ConfigAbstract.h>
#include <LumoBackend/Interfaces/ShaderInterface.h>
#include <LumoBackend/Interfaces/SerializationInterface.h>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <LumoBackend/SceneGraph/SceneMesh.hpp>
#include <LumoBackend/Interfaces/TaskInterface.h>
#include <LumoBackend/Interfaces/Texture2DInputInterface.h>
#include <LumoBackend/Interfaces/Texture2DOutputInterface.h>
#include <LumoBackend/Interfaces/TextureGroupInputInterface.h>
#include <LumoBackend/Interfaces/LightGroupInputInterface.h>

class ModelShadowModule_Quad_Pass : public QuadShaderPass,

                                    public Texture2DInputInterface<2U>,
                                    public TextureGroupInputInterface<8U>,
                                    public Texture2DOutputInterface,
                                    public LightGroupInputInterface {
protected:
    VulkanBufferObjectPtr m_UBOFragPtr = nullptr;
    vk::DescriptorBufferInfo m_DescriptorBufferInfo_Frag;
    struct UBOFrag {
        alignas(4) float u_shadow_strength = 0.5f;
        alignas(4) float u_bias = 0.01f;
        alignas(4) float u_poisson_scale = 5000.0f;
        alignas(4) float u_use_pcf = 1.0f;
        alignas(4) float use_sampler_pos = 0.0f;
        alignas(4) float use_sampler_nor = 0.0f;
        alignas(4) float use_sampler_shadow_map = 0.0f;
    } m_UBOFrag;

public:
    ModelShadowModule_Quad_Pass(GaiApi::VulkanCoreWeak vVulkanCore);
    ~ModelShadowModule_Quad_Pass() override;

    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;
    void SetTextures(const uint32_t& vBindingPoint, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes) override;
    vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;
    void SetLightGroup(SceneLightGroupWeak vSceneLightGroup) override;
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

private:
    bool CreateUBO() override;
    void UploadUBO() override;
    void DestroyUBO() override;

    bool UpdateLayoutBindingInRessourceDescriptor() override;
    bool UpdateBufferInfoInRessourceDescriptor() override;

    std::string GetVertexShaderCode(std::string& vOutShaderName) override;
    std::string GetFragmentShaderCode(std::string& vOutShaderName) override;
};