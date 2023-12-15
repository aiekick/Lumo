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

#include "WidgetColorModule_Pass.h"

#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <cinttypes>
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
//// SSAO SECOND PASS : BLUR /////////////////////////////////
//////////////////////////////////////////////////////////////

WidgetColorModule_Pass::WidgetColorModule_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : ShaderPass(vVulkanCore) {
    SetRenderDocDebugName("Comp Pass : Widget Color", COMPUTE_SHADER_PASS_DEBUG_COLOR);

    m_DontUseShaderFilesOnDisk = true;
}

WidgetColorModule_Pass::~WidgetColorModule_Pass() {
    Unit();
}

bool WidgetColorModule_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (ImGui::ColorEdit4Default(0.0f, "Color", &m_UBOComp.u_color.x, &m_DefaultColor.x)) {
        NeedNewUBOUpload();

        return true;
    }

    return false;
}

bool WidgetColorModule_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool WidgetColorModule_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool WidgetColorModule_Pass::DrawNodeWidget(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    /*if (ImGui::ColorPicker4DefaultForNode(100.0f, "Color", &m_UBOComp.u_color.x, &m_DefaultColor.x))
    {
        NeedNewUBOUpload();

        return true;
    }*/

    return false;
}

vk::DescriptorImageInfo* WidgetColorModule_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    if (m_ComputeBufferPtr) {
        if (vOutSize) {
            *vOutSize = m_ComputeBufferPtr->GetOutputSize();
        }

        return m_ComputeBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
    }

    return nullptr;
}

void WidgetColorModule_Pass::Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) {
    if (vCmdBufferPtr) {
        vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
        vCmdBufferPtr->bindDescriptorSets(
            vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
        Dispatch(vCmdBufferPtr, "Compute");
    }
}

bool WidgetColorModule_Pass::CreateUBO() {
    ZoneScoped;

    m_UBOCompPtr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBOComp), "WidgetColorModule_Pass");
    if (m_UBOCompPtr->buffer) {
        m_UBOComp_BufferInfo = vk::DescriptorBufferInfo{m_UBOCompPtr->buffer, 0, sizeof(UBOComp)};
    } else {
        m_UBOComp_BufferInfo = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
    }

    NeedNewUBOUpload();

    return true;
}

void WidgetColorModule_Pass::UploadUBO() {
    ZoneScoped;

    VulkanRessource::upload(m_VulkanCore, m_UBOCompPtr, &m_UBOComp, sizeof(UBOComp));
}

void WidgetColorModule_Pass::DestroyUBO() {
    ZoneScoped;

    m_UBOCompPtr.reset();
}

bool WidgetColorModule_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
    return res;
}

bool WidgetColorModule_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U));  // output
    res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_UBOComp_BufferInfo);
    return res;
}

std::string WidgetColorModule_Pass::GetComputeShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "WidgetColorModule_Pass";

    SetLocalGroupSize(1U);

    return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColor;

layout (std140, binding = 1) uniform UBO_Comp
{
	vec4 u_color;
};

void main()
{
	imageStore(outColor, ivec2(gl_GlobalInvocationID.xy), u_color); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string WidgetColorModule_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/) {
    std::string str;

    str += vOffset + "<color>" + m_UBOComp.u_color.string() + "</color>\n";

    return str;
}

bool WidgetColorModule_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "widget_color_module") {
        if (strName == "color")
            m_UBOComp.u_color = ct::fvariant(strValue).GetV4();

        NeedNewUBOUpload();
    }

    return true;
}