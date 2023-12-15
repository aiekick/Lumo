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

#include "AudioTextureModule_Comp_2D_Pass.h"

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
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

AudioTextureModule_Comp_2D_Pass::AudioTextureModule_Comp_2D_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : ShaderPass(vVulkanCore) {
    ZoneScoped;

    SetRenderDocDebugName("Comp Pass : Audio Texture", COMPUTE_SHADER_PASS_DEBUG_COLOR);

    // m_DontUseShaderFilesOnDisk = true;
}

AudioTextureModule_Comp_2D_Pass::~AudioTextureModule_Comp_2D_Pass() {
    ZoneScoped;

    Unit();
}

void AudioTextureModule_Comp_2D_Pass::ActionBeforeInit() {
    ZoneScoped;

    // m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);
}

bool AudioTextureModule_Comp_2D_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    bool change = false;

    change |= DrawResizeWidget();

    return change;
}

bool AudioTextureModule_Comp_2D_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

bool AudioTextureModule_Comp_2D_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SCENEAUDIART INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void AudioTextureModule_Comp_2D_Pass::SetSceneAudiArt(const std::string& vName, SceneAudiArtWeak vSceneAudiArt) {
    ZoneScoped;

    m_SceneAudiArts[vName] = vSceneAudiArt;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* AudioTextureModule_Comp_2D_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    ZoneScoped;
    if (m_ComputeBufferPtr) {
        if (vOutSize) {
            *vOutSize = m_ComputeBufferPtr->GetOutputSize();
        }

        return m_ComputeBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AudioTextureModule_Comp_2D_Pass::WasJustResized() {
    ZoneScoped;
}

void AudioTextureModule_Comp_2D_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber) {
    if (vCmdBuffer) {
        vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
        {
            vCmdBuffer->bindDescriptorSets(
                vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
            Dispatch(vCmdBuffer, "Compute");
        }
    }
}

bool AudioTextureModule_Comp_2D_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);

    return res;
}

bool AudioTextureModule_Comp_2D_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U));  // output

    return res;
}

std::string AudioTextureModule_Comp_2D_Pass::GetComputeShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "AudioTextureModule_Comp_2D_Pass_Compute";

    SetLocalGroupSize(ct::uvec3(1U, 1U, 1U));

    return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform image2D colorBuffer;


void main()
{
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

	vec4 color = vec4(coords, 0, 1);

	imageStore(colorBuffer, coords, color); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string AudioTextureModule_Comp_2D_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;

    std::string str;

    str += ShaderPass::getXml(vOffset, vUserDatas);

    return str;
}

bool AudioTextureModule_Comp_2D_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    ShaderPass::setFromXml(vElem, vParent, vUserDatas);

    if (strParentName == "audio_texture_module") {
    }

    return true;
}

void AudioTextureModule_Comp_2D_Pass::AfterNodeXmlLoading() {
    ZoneScoped;

    // code to do after end of the xml loading of this node
    // by ex :
    NeedNewUBOUpload();
}
