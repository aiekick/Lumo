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

#include "DepthConvModule_Quad_Pass.h"

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

DepthConvModule_Quad_Pass::DepthConvModule_Quad_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : QuadShaderPass(vVulkanCore, MeshShaderPassType::PIXEL) {
    SetRenderDocDebugName("Quad Pass 1 : Depth Conversion", QUAD_SHADER_PASS_DEBUG_COLOR);

    m_DontUseShaderFilesOnDisk = true;
}

DepthConvModule_Quad_Pass::~DepthConvModule_Quad_Pass() {
    Unit();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool DepthConvModule_Quad_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    DrawInputTexture(m_VulkanCore, "Input Depth", 0U, m_OutputRatio);

    return false;
}

bool DepthConvModule_Quad_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool DepthConvModule_Quad_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void DepthConvModule_Quad_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) {
    ZoneScoped;

    if (m_Loaded) {
        if (vBindingPoint < m_ImageInfos.size()) {
            if (vImageInfo) {
                m_ImageInfos[vBindingPoint] = *vImageInfo;

                if ((&m_UBOFrag.use_sampler_dep)[vBindingPoint] < 1.0f) {
                    (&m_UBOFrag.use_sampler_dep)[vBindingPoint] = 1.0f;
                    NeedNewUBOUpload();
                }
            } else {
                if ((&m_UBOFrag.use_sampler_dep)[vBindingPoint] > 0.0f) {
                    (&m_UBOFrag.use_sampler_dep)[vBindingPoint] = 0.0f;
                    NeedNewUBOUpload();
                }

                auto corePtr = m_VulkanCore.lock();
                assert(corePtr != nullptr);

                m_ImageInfos[vBindingPoint] = *corePtr->getEmptyTexture2DDescriptorImageInfo();
            }
        }
    }
}

vk::DescriptorImageInfo* DepthConvModule_Quad_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize) {
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

std::string DepthConvModule_Quad_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;

    return str;
}

bool DepthConvModule_Quad_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

bool DepthConvModule_Quad_Pass::CreateUBO() {
    ZoneScoped;

    auto size_in_bytes = sizeof(UBOFrag);
    m_UBOFragPtr = VulkanRessource::createUniformBufferObject(m_VulkanCore, size_in_bytes, "DepthConvModule_Quad_Pass");
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

void DepthConvModule_Quad_Pass::UploadUBO() {
    ZoneScoped;

    VulkanRessource::upload(m_VulkanCore, m_UBOFragPtr, &m_UBOFrag, sizeof(UBOFrag));
}

void DepthConvModule_Quad_Pass::DestroyUBO() {
    ZoneScoped;

    m_UBOFragPtr.reset();
}

bool DepthConvModule_Quad_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
    return res;
}

bool DepthConvModule_Quad_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo());
    res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_DescriptorBufferInfo_Frag);
    res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]);  // depth
    return res;
}

std::string DepthConvModule_Quad_Pass::GetVertexShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "DepthConvModule_Quad_Pass_Vertex";

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

std::string DepthConvModule_Quad_Pass::GetFragmentShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "DepthConvModule_Quad_Pass_Fragment";

    return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragPos;
layout(location = 1) out vec4 fragNor;
layout(location = 0) in vec2 v_uv;
)" + CommonSystem::GetBufferObjectStructureHeader(0U) +
           u8R"(
layout (std140, binding = 1) uniform UBO_Vert 
{ 
	float use_sampler_dep;
};

layout(binding = 2) uniform sampler2D depth_map_sampler;

vec3 getPostFromDepth(vec2 uv) {
	float dep = DeLinearizeDepth(texture(depth_map_sampler, uv).r);
	vec2 uvc = uv * 2.0 - 1.0;
	vec4 ndc = inverse(cam) * vec4(uvc, dep, 1.0);
	return ndc.xyz / ndc.w;
}

void main() 
{
	fragPos = vec4(0.0);
	fragNor = vec4(0.0);

	if (use_sampler_dep > 0.5)
	{
		float dep = DeLinearizeDepth(texture(depth_map_sampler, v_uv).r);
		if (dep > 0.0)
		{
			// pos
			vec3 pos = getPostFromDepth(v_uv);
			fragPos = vec4(pos, 1.0);

			// normal
			/*vec2 depth_size = vec2(textureSize(depth_map_sampler, 0));	
			vec3 P0 = getPostFromDepth(v_uv);
			vec3 P1 = getPostFromDepth(v_uv + vec2(1,0) / depth_size.x);
			vec3 P2 = getPostFromDepth(v_uv + vec2(0,1) / depth_size.y);
			fragNor = vec4(normalize(cross(P2 - P0, P1 - P0)), 1.0);*/
			fragNor = vec4(normalize(cross(dFdx(pos), dFdy(pos))), 1.0);
		}
	}
}

)";
}