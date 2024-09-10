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

#include "DilationModule_Comp_2D_Pass.h"

#include <cinttypes>
#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImGuiPack.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Gaia/Buffer/FrameBuffer.h>

using namespace GaiApi;

#ifdef PROFILER_INCLUDE
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

static ct::fvec3 s_bright_color_default = 1.0f;

//////////////////////////////////////////////////////////////
///// STATIC /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<DilationModule_Comp_2D_Pass> DilationModule_Comp_2D_Pass::Create(const ct::uvec2& vSize, GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res_ptr = std::make_shared<DilationModule_Comp_2D_Pass>(vVulkanCore);
    if (!res_ptr->InitCompute2D(vSize, 1U, false, vk::Format::eR32G32B32A32Sfloat)) {
        res_ptr.reset();
    }
    return res_ptr;
}

//////////////////////////////////////////////////////////////
///// CTOR / DTOR ////////////////////////////////////////////
//////////////////////////////////////////////////////////////

DilationModule_Comp_2D_Pass::DilationModule_Comp_2D_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : EffectPass(vVulkanCore) {
    ZoneScoped;
    SetRenderDocDebugName("Comp Pass : Dilation", COMPUTE_SHADER_PASS_DEBUG_COLOR);
    m_DontUseShaderFilesOnDisk = true;
}

DilationModule_Comp_2D_Pass::~DilationModule_Comp_2D_Pass() {
    ZoneScoped;
    Unit();
}

void DilationModule_Comp_2D_Pass::ActionBeforeInit() {
    ZoneScoped;
    // m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);
    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);

    for (auto& info : m_ImageInfos) {
        info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
    }
}

bool DilationModule_Comp_2D_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    bool change = false;
    // change |= DrawResizeWidget();
    if (ImGui::CollapsingHeader_CheckBox("Dilation##DilationModule_Comp_2D_Pass", -1.0f, false, true, IsEffectEnabled())) {
        change |= ImGui::ContrastedComboVectorDefault(0.0f, "shape", &m_UBO_Comp.u_shape, {"bokeh", "circle", "quad"}, 0);
        change |= ImGui::SliderIntDefaultCompact(0.0f, "size", &m_UBO_Comp.u_size, 0, 10, 5);
        change |= ImGui::SliderFloatDefaultCompact(0.0f, "separation", &m_UBO_Comp.u_separation, 0.0f, 2.0f, 1.0f, 0.0f, "%.3f");
        change |= ImGui::SliderFloatDefaultCompact(0.0f, "threshold x", &m_UBO_Comp.u_threshold.x, 0.0f, 1.0f, 0.0f);
        change |= ImGui::SliderFloatDefaultCompact(0.0f, "threshold y", &m_UBO_Comp.u_threshold.y, 0.0f, 1.0f, 1.0f);
        change |= ImGui::ColorEdit3Default(0.0f, "brightColor", &m_UBO_Comp.u_brightColor.x, &s_bright_color_default.x);
        if (change) {
            NeedNewUBOUpload();
        }
    }
    return change;
}

bool DilationModule_Comp_2D_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool DilationModule_Comp_2D_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void DilationModule_Comp_2D_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
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

vk::DescriptorImageInfo* DilationModule_Comp_2D_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
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

void DilationModule_Comp_2D_Pass::WasJustResized() {
    ZoneScoped;
}

void DilationModule_Comp_2D_Pass::Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) {
    ZoneScoped;
    if (vCmdBufferPtr) {
        vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
        {
            // VKFPScoped(*vCmdBufferPtr, "Dilation", "Compute");

            vCmdBufferPtr->bindDescriptorSets(
                vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);

            for (uint32_t iter = 0; iter < m_CountIterations.w; iter++) {
                Dispatch(vCmdBufferPtr, "Compute");
            }
        }
    }
}

bool DilationModule_Comp_2D_Pass::CreateUBO() {
    ZoneScoped;
    m_UBO_Comp_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBO_Comp), "DilationModule_Comp_2D_Pass");
    m_UBO_Comp_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
    if (m_UBO_Comp_Ptr) {
        m_UBO_Comp_BufferInfos.buffer = m_UBO_Comp_Ptr->buffer;
        m_UBO_Comp_BufferInfos.range = sizeof(UBO_Comp);
        m_UBO_Comp_BufferInfos.offset = 0;
    }
    NeedNewUBOUpload();
    return true;
}

void DilationModule_Comp_2D_Pass::UploadUBO() {
    ZoneScoped;
    assert(IsEffectEnabled() != nullptr);
    m_UBO_Comp.u_enabled = (*IsEffectEnabled()) ? 1.0f : 0.0f;
    VulkanRessource::upload(m_VulkanCore, m_UBO_Comp_Ptr, &m_UBO_Comp, sizeof(UBO_Comp));
}

void DilationModule_Comp_2D_Pass::DestroyUBO() {
    ZoneScoped;
    m_UBO_Comp_Ptr.reset();
    m_UBO_Comp_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
}

bool DilationModule_Comp_2D_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;
    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);  // sampler input
    res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);          // color output
    return res;
}

bool DilationModule_Comp_2D_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;
    bool res = true;
    res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, &m_UBO_Comp_BufferInfos);
    res &= AddOrSetWriteDescriptorImage(1U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0U]);                            // sampler input
    res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U));  // output
    return res;
}

std::string DilationModule_Comp_2D_Pass::GetComputeShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "DilationModule_Comp_2D_Pass_Compute";
    SetLocalGroupSize(ct::uvec3(1U, 1U, 1U));
    return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

layout(std140, binding = 0) uniform UBO_Comp {
	int u_shape; // 0 : bokeh, 1 : circle, 2 : quad | default is 0
    int u_size; // default is 1
	float u_separation; // default is 1
	vec2 u_threshold; // default is 0.2, 0.5
	vec3 u_brightColor; // default is 1,1,1
	float u_enabled; // default is 0.0
};

layout(binding = 1) uniform sampler2D input_color_map;

layout(binding = 2, rgba32f) uniform image2D outColor; // output

void main() {
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	vec4 res = texelFetch(input_color_map, coords, 0);
    if (u_enabled > 0.5 && u_size > 0.0) {
        vec2 texSize   = textureSize(input_color_map, 0);
        vec2 texCoord  = vec2(coords);
        float  mx = 0.0;
        vec4 cmx = vec4(0.0);
        for (int i = -u_size; i <= u_size; ++i) {
            for (int j = -u_size; j <= u_size; ++j) {
                if (u_shape == 0) { // bokeh
                    if (!(abs(i) <= u_size - abs(j))) { continue; }
                } else if (u_shape == 1) { // circle
                    if (!(distance(vec2(i, j), vec2(0, 0)) <= u_size)) { continue; }
                } else if (u_shape == 2) { // quad
                    if (false) { continue; }
                }
                vec4 c = texture(input_color_map, (texCoord + vec2(i, j) * u_separation) / texSize);
                float mxt = dot(c.rgb, u_brightColor);
                if (mxt > mx) {
                    mx = mxt;
                    cmx = c;
                }
            }
        }
        res.rgb = mix(res.rgb, cmx.rgb, smoothstep(u_threshold.x, u_threshold.y, mx));
    }
	imageStore(outColor, coords, res); 
}

)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string DilationModule_Comp_2D_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;
    std::string str;
    str += vOffset + "<dilation_pass>\n";
    str += ShaderPass::getXml(vOffset + "\t", vUserDatas);
    str += vOffset + "\t<shape>" + ct::toStr(m_UBO_Comp.u_shape) + "</shape>\n";
    str += vOffset + "\t<size>" + ct::toStr(m_UBO_Comp.u_size) + "</size>\n";
    str += vOffset + "\t<separation>" + ct::toStr(m_UBO_Comp.u_separation) + "</separation>\n";
    str += vOffset + "\t<threshold>" + m_UBO_Comp.u_threshold.string() + "</threshold>\n";
    str += vOffset + "\t<brightcolor>" + m_UBO_Comp.u_brightColor.string() + "</brightcolor>\n";
    str += vOffset + "\t<enabled>" + ct::toStr(m_UBO_Comp.u_enabled) + "</enabled>\n";
    str += vOffset + "</dilation_pass>\n";
    return str;
}

bool DilationModule_Comp_2D_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    ZoneScoped;
    std::string strName;
    std::string strValue;
    std::string strParentName;
    strName = vElem->Value();
    if (vElem->GetText()) {
        strValue = vElem->GetText();
    }
    if (vParent != nullptr) {
        strParentName = vParent->Value();
    }
    if (strParentName == "dilation_pass") {
        ShaderPass::setFromXml(vElem, vParent, vUserDatas);
        if (strName == "shape") {
            m_UBO_Comp.u_shape = ct::ivariant(strValue).GetI();
        } else if (strName == "size") {
            m_UBO_Comp.u_size = ct::fvariant(strValue).GetI();
        } else if (strName == "separation") {
            m_UBO_Comp.u_separation = ct::fvariant(strValue).GetF();
        } else if (strName == "threshold") {
            m_UBO_Comp.u_threshold = ct::fvariant(strValue).GetV2();
        } else if (strName == "brightcolor") {
            m_UBO_Comp.u_brightColor = ct::fvariant(strValue).GetV3();
        } else if (strName == "enabled") {
            m_UBO_Comp.u_enabled = ct::fvariant(strValue).GetF();
            *IsEffectEnabled() = m_UBO_Comp.u_enabled;
        }
    }
    return true;
}

void DilationModule_Comp_2D_Pass::AfterNodeXmlLoading() {
    ZoneScoped;
    NeedNewUBOUpload();
}
