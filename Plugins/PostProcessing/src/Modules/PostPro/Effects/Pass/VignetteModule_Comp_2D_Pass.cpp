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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "VignetteModule_Comp_2D_Pass.h"

#include <cinttypes>
#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Gaia/Buffer/FrameBuffer.h>

using namespace GaiApi;

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<VignetteModule_Comp_2D_Pass> VignetteModule_Comp_2D_Pass::Create(const ct::uvec2& vSize, GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res_ptr = std::make_shared<VignetteModule_Comp_2D_Pass>(vVulkanCore);
    res_ptr->AllowResizeOnResizeEvents(false);
    res_ptr->AllowResizeByHandOrByInputs(true);
    if (!res_ptr->InitCompute2D(vSize, 1U, false, vk::Format::eR32G32B32A32Sfloat)) {
        res_ptr.reset();
    }
    return res_ptr;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

VignetteModule_Comp_2D_Pass::VignetteModule_Comp_2D_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : EffectPass(vVulkanCore) {
    SetRenderDocDebugName("Comp Pass : Vignette", COMPUTE_SHADER_PASS_DEBUG_COLOR);
    m_DontUseShaderFilesOnDisk = true;
}

VignetteModule_Comp_2D_Pass::~VignetteModule_Comp_2D_Pass() {
    Unit();
}

void VignetteModule_Comp_2D_Pass::ActionBeforeInit() {
    // m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);
    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);

    for (auto& info : m_ImageInfos) {
        info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
    }
}

bool VignetteModule_Comp_2D_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    ZoneScoped;

    bool change = false;

    if (ImGui::CollapsingHeader_CheckBox("Vignette##VignetteModule", -1.0f, false, true, IsEffectEnabled())) {
        //change |= DrawResizeWidget();
        change |= ImGui::SliderFloatDefaultCompact(0.0f, "Width", &m_UBOComp.u_Width, 0.000f, 0.500f, 0.250f, 0.0f, "%.3f");
        change |= ImGui::SliderFloatDefaultCompact(0.0f, "Intensity", &m_UBOComp.u_Intensity, 0.000f, 30.000f, 15.000f, 0.0f, "%.3f");

        if (change) {
            NeedNewUBOUpload();
        }
    }

    return change;
}

bool VignetteModule_Comp_2D_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool VignetteModule_Comp_2D_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void VignetteModule_Comp_2D_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) {
    ZoneScoped;

    if (m_Loaded) {
        if (vBindingPoint < m_ImageInfos.size()) {
            if (vImageInfo) {
                if (vTextureSize) {
                    m_ImageInfosSize[vBindingPoint] = *vTextureSize;
                    NeedResizeByHandIfChanged(m_ImageInfosSize[vBindingPoint]);
                }

                m_ImageInfos[vBindingPoint] = *vImageInfo;
            } else {
                auto corePtr = m_VulkanCore.lock();
                assert(corePtr != nullptr);

                m_ImageInfos[vBindingPoint] = *corePtr->getEmptyTexture2DDescriptorImageInfo();
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* VignetteModule_Comp_2D_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize) {
    ZoneScoped;
    if (m_ComputeBufferPtr) {
        AutoResizeBuffer(std::dynamic_pointer_cast<OutputSizeInterface>(m_ComputeBufferPtr).get(), vOutSize);
        return m_ComputeBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void VignetteModule_Comp_2D_Pass::WasJustResized() {
    ZoneScoped;
}

void VignetteModule_Comp_2D_Pass::Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) {
    if (vCmdBufferPtr) {
        vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
        {
            // VKFPScoped(*vCmdBufferPtr, "Vignette", "Compute");

            vCmdBufferPtr->bindDescriptorSets(
                vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);

            for (uint32_t iter = 0; iter < m_CountIterations.w; iter++) {
                Dispatch(vCmdBufferPtr);
            }
        }
    }
}

bool VignetteModule_Comp_2D_Pass::CreateUBO() {
    ZoneScoped;

    m_UBOComp_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBO_1_Comp), "VignetteModule_Comp_2D_Pass");
    m_UBOComp_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
    if (m_UBOComp_Ptr) {
        m_UBOComp_BufferInfos.buffer = m_UBOComp_Ptr->buffer;
        m_UBOComp_BufferInfos.range = sizeof(UBO_1_Comp);
        m_UBOComp_BufferInfos.offset = 0;
    }

    NeedNewUBOUpload();

    return true;
}

void VignetteModule_Comp_2D_Pass::UploadUBO() {
    ZoneScoped;
    assert(IsEffectEnabled() != nullptr);
    m_UBOComp.u_enabled = (*IsEffectEnabled()) ? 1.0f : 0.0f;
    VulkanRessource::upload(m_VulkanCore, m_UBOComp_Ptr, &m_UBOComp, sizeof(UBO_1_Comp));
}

void VignetteModule_Comp_2D_Pass::DestroyUBO() {
    ZoneScoped;
    m_UBOComp_Ptr.reset();
    m_UBOComp_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
}

bool VignetteModule_Comp_2D_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
    res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);
    return res;
}

bool VignetteModule_Comp_2D_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    if (m_ComputeBufferPtr) {
        bool res = true;
        res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U));  // output
        res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_UBOComp_BufferInfos);
        res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]);
        return res;
    }

    return false;
}
std::string VignetteModule_Comp_2D_Pass::GetComputeShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "VignetteModule_Comp_2D_Pass_Compute";

    SetLocalGroupSize(ct::uvec3(8U, 8U, 1U));

    return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform image2D outColor;

layout(std140, binding = 1) uniform UBO_1_Comp
{
	float u_Width;
	float u_Intensity;
	uint u_enabled; // default is 1
};

layout(binding = 2) uniform sampler2D input_mask;

void main() {
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	vec4 res = texelFetch(input_mask, coords, 0);
    if (u_enabled > 0.5) {
        vec2 uv = vec2(coords) / textureSize(input_mask, 0);
        uv *= 1.0 - uv.yx;
	    res *= pow(uv.x * uv.y * u_Intensity, u_Width);
    }
	imageStore(outColor, coords, res); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string VignetteModule_Comp_2D_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;

    str += vOffset + "<vignette_pass>\n";
    str += ShaderPass::getXml(vOffset + "\t", vUserDatas);
    str += vOffset + "\t<width>" + ct::toStr(m_UBOComp.u_Width) + "</width>\n";
    str += vOffset + "\t<intensity>" + ct::toStr(m_UBOComp.u_Intensity) + "</intensity>\n";
    str += vOffset + "\t<enabled>" + ct::toStr(m_UBOComp.u_enabled) + "</enabled>\n";
    str += vOffset + "</vignette_pass>\n";
    return str;
}

bool VignetteModule_Comp_2D_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    ZoneScoped;

    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "vignette_pass") {
        ShaderPass::setFromXml(vElem, vParent, vUserDatas);
        if (strName == "width") {
            m_UBOComp.u_Width = ct::fvariant(strValue).GetF();
        } else if (strName == "intensity") {
            m_UBOComp.u_Intensity = ct::fvariant(strValue).GetF();
        } else if (strName == "enabled") {
            m_UBOComp.u_enabled = ct::fvariant(strValue).GetF();
            *IsEffectEnabled() = m_UBOComp.u_enabled;
        }
    }

    return true;
}

void VignetteModule_Comp_2D_Pass::AfterNodeXmlLoading() {
    ZoneScoped;
    NeedNewUBOUpload();
}
