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

#include "PosToDepthModule_Quad_Pass.h"

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
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

PosToDepthModule_Quad_Pass::PosToDepthModule_Quad_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : QuadShaderPass(vVulkanCore, MeshShaderPassType::PIXEL) {
    SetRenderDocDebugName("Quad Pass 1 : Pos To Depth", QUAD_SHADER_PASS_DEBUG_COLOR);

    m_DontUseShaderFilesOnDisk = true;
}

PosToDepthModule_Quad_Pass::~PosToDepthModule_Quad_Pass() {
    Unit();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool PosToDepthModule_Quad_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    DrawInputTexture(m_VulkanCore, "Position", 0U, m_OutputRatio);

    return false;
}

bool PosToDepthModule_Quad_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool PosToDepthModule_Quad_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void PosToDepthModule_Quad_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) {
    ZoneScoped;

    if (m_Loaded) {
        if (vBindingPoint < m_ImageInfos.size()) {
            if (vImageInfo) {
                m_ImageInfos[vBindingPoint] = *vImageInfo;

                if ((&m_UBOFrag.use_sampler_pos)[vBindingPoint] < 1.0f) {
                    (&m_UBOFrag.use_sampler_pos)[vBindingPoint] = 1.0f;
                    NeedNewUBOUpload();
                }
            } else {
                if ((&m_UBOFrag.use_sampler_pos)[vBindingPoint] > 0.0f) {
                    (&m_UBOFrag.use_sampler_pos)[vBindingPoint] = 0.0f;
                    NeedNewUBOUpload();
                }

                auto corePtr = m_VulkanCore.lock();
                assert(corePtr != nullptr);

                m_ImageInfos[vBindingPoint] = *corePtr->getEmptyTexture2DDescriptorImageInfo();
            }
        }
    }
}

vk::DescriptorImageInfo* PosToDepthModule_Quad_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize) {
    ZoneScoped;

    if (m_FrameBufferPtr) {
        if (vOutSize) {
            *vOutSize = m_FrameBufferPtr->GetOutputSize();
        }

        return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string PosToDepthModule_Quad_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;

    return str;
}

bool PosToDepthModule_Quad_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PosToDepthModule_Quad_Pass::CreateUBO() {
    ZoneScoped;

    auto size_in_bytes = sizeof(UBOFrag);
    m_UBOFragPtr = VulkanRessource::createUniformBufferObject(m_VulkanCore, size_in_bytes, "PosToDepthModule_Quad_Pass");
    m_DescriptorBufferInfo_Frag.buffer = m_UBOFragPtr->buffer;
    m_DescriptorBufferInfo_Frag.range = size_in_bytes;
    m_DescriptorBufferInfo_Frag.offset = 0;

    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);

    for (auto& info : m_ImageInfos) {
        info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
    }

    NeedNewUBOUpload();

    return true;
}

void PosToDepthModule_Quad_Pass::UploadUBO() {
    ZoneScoped;

    VulkanRessource::upload(m_VulkanCore, m_UBOFragPtr, &m_UBOFrag, sizeof(UBOFrag));
}

void PosToDepthModule_Quad_Pass::DestroyUBO() {
    ZoneScoped;

    m_UBOFragPtr.reset();
}

bool PosToDepthModule_Quad_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
    return res;
}

bool PosToDepthModule_Quad_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo());
    res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_DescriptorBufferInfo_Frag);
    res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]);  // position
    return res;
}

std::string PosToDepthModule_Quad_Pass::GetVertexShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "PosToDepthModule_Quad_Pass_Vertex";

    return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 vertPosition;
layout(location = 1) in vec2 vertUv;
layout(location = 0) out vec2 v_uv;

void main() 
{
	v_uv = vertUv;
	gl_Position = vec4(vertPosition, 0.0, 1.0);
}
)";
}

std::string PosToDepthModule_Quad_Pass::GetFragmentShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "PosToDepthModule_Quad_Pass_Fragment";

    return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 v_uv;
)" + CommonSystem::GetBufferObjectStructureHeader(0U) +
           u8R"(
layout (std140, binding = 1) uniform UBO_Vert { 
	float use_sampler_pos;
};
layout(binding = 2) uniform sampler2D pos_map_sampler;

vec3 getRayOrigin() {
	vec3 ro = view[3].xyz + model[3].xyz;
	ro *= mat3(view * model);
	return -ro;
}

void main() {
	vec3 pos = texture(pos_map_sampler, v_uv).xyz;
	if (use_sampler_pos > 0.5) {
		if (dot(pos, pos) > 0.0) {
			vec3 ro = getRayOrigin();
			float dep = length(ro - pos) / cam_far;
			fragColor = vec4(vec3(dep), 1.0);
		} else {
			fragColor = vec4(0.0);
		}
	} else {
		fragColor = vec4(0.0);
	}
}

)";
}
