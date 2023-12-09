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

#include "CurlModule_Comp_2D_Pass.h"

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
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

CurlModule_Comp_2D_Pass::CurlModule_Comp_2D_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : ShaderPass(vVulkanCore) {
    ZoneScoped;

    SetRenderDocDebugName("Comp Pass : Curl", COMPUTE_SHADER_PASS_DEBUG_COLOR);

    m_DontUseShaderFilesOnDisk = true;
}

CurlModule_Comp_2D_Pass::~CurlModule_Comp_2D_Pass() {
    ZoneScoped;

    Unit();
}

void CurlModule_Comp_2D_Pass::ActionBeforeInit() {
    ZoneScoped;

    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);

    for (auto& info : m_ImageInfos) {
        info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
    }
}

bool CurlModule_Comp_2D_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (ImGui::CollapsingHeader("Curl", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool change = false;

        if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
            change |= ImGui::ContrastedComboVectorDefault(0.0f, "Method", &m_UBOComp.method, m_MethodNames, 0);
            change |= ImGui::CheckBoxFloatDefault("Discard Zero values", &m_UBOComp.u_discard_zero, false);

            if (change) {
                NeedNewUBOUpload();
            }
        }

        return change;
    }

    return false;
}

bool CurlModule_Comp_2D_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool CurlModule_Comp_2D_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void CurlModule_Comp_2D_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) {
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

vk::DescriptorImageInfo* CurlModule_Comp_2D_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize) {
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

void CurlModule_Comp_2D_Pass::WasJustResized() {
    ZoneScoped;
}

void CurlModule_Comp_2D_Pass::Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) {
    if (vCmdBufferPtr) {
        vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
        {
            // VKFPScoped(*vCmdBufferPtr, "Curl", "Compute");

            vCmdBufferPtr->bindDescriptorSets(
                vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
            Dispatch(vCmdBufferPtr, "Compute");
        }
    }
}

bool CurlModule_Comp_2D_Pass::CreateUBO() {
    ZoneScoped;

    m_UBOComp_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBO_Comp), "CurlModule_Comp_2D_Pass");
    m_UBOComp_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
    if (m_UBOComp_Ptr) {
        m_UBOComp_BufferInfos.buffer = m_UBOComp_Ptr->buffer;
        m_UBOComp_BufferInfos.range = sizeof(UBO_Comp);
        m_UBOComp_BufferInfos.offset = 0;
    }

    NeedNewUBOUpload();

    return true;
}

void CurlModule_Comp_2D_Pass::UploadUBO() {
    ZoneScoped;

    VulkanRessource::upload(m_VulkanCore, m_UBOComp_Ptr, &m_UBOComp, sizeof(UBO_Comp));
}

void CurlModule_Comp_2D_Pass::DestroyUBO() {
    ZoneScoped;

    m_UBOComp_Ptr.reset();
    m_UBOComp_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
}

bool CurlModule_Comp_2D_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
    res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);

    return res;
}

bool CurlModule_Comp_2D_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U));  // output
    res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_UBOComp_BufferInfos);
    res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]);  // ssao

    return res;
}

std::string CurlModule_Comp_2D_Pass::GetComputeShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "CurlModule_Comp_2D_Pass_Compute";

    SetLocalGroupSize(ct::uvec3(1U, 1U, 1U));

    return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform image2D outColor;

layout(std140, binding = 1) uniform UBO_Comp
{
	int method;			// 0 -> 7
	float u_discard_zero; // default is 0.0 (false)
};

layout(binding = 2) uniform sampler2D input_map_sampler;

float getValue(vec4 v) 
{
	switch(method)
	{
	case 0: // r
		return v.r;
	case 1: // g
		return v.g;
	case 2: // b
		return v.b;
	case 3: // a
		return v.a;
	case 4: // length(rg)
		return length(v.rg);
	case 5: // length(rgb)
		return length(v.rgb);
	case 6: // length(rgba)
		return length(v);
	case 7: // median(rgb)
		// https://github.com/Chlumsky/msdfgen
		return max(min(v.r, v.g), min(max(v.r, v.g), v.b));
	}
	return v.r;
}

float getSam(ivec2 co, int x, int y)
{
	return getValue(texelFetch(input_map_sampler, co + ivec2(x,y), 0));
}

float getCurl(ivec2 co)
{
	float dx = getSam(co, -1, 0) - getSam(co, 1, 0); // left - right
	float dy = getSam(co, 0, 1) - getSam(co, 0, -1); // up - down
	return (dx + dy) * 0.5;
}

void main()
{
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

	float c = getSam(coords, 0, 0);
	
	float value = getCurl(coords);

	if (u_discard_zero > 0.5)
	{
		if (dot(c, c) < 0.01)
		{
			value = 0.0;
		}
	}

	imageStore(outColor, coords, vec4(value)); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string CurlModule_Comp_2D_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;

    std::string str;

    str += ShaderPass::getXml(vOffset, vUserDatas);

    str += vOffset + "<method>" + ct::toStr(m_UBOComp.method) + "</method>\n";
    str += vOffset + "<discard_zeros>" + ct::toStr(m_UBOComp.u_discard_zero) + "</discard_zeros>\n";

    return str;
}

bool CurlModule_Comp_2D_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (strParentName == "rotational_module") {
        if (strName == "method")
            m_UBOComp.method = ct::ivariant(strValue).GetI();
        else if (strName == "discard_zeros")
            m_UBOComp.u_discard_zero = ct::ivariant(strValue).GetB();
    }

    return true;
}

void CurlModule_Comp_2D_Pass::AfterNodeXmlLoading() {
    ZoneScoped;

    // code to do after end of the xml loading of this node
    // by ex :
    NeedNewUBOUpload();
}
