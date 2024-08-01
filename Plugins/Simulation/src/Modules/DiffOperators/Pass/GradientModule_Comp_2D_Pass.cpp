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

#include "GradientModule_Comp_2D_Pass.h"

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

#include <cinttypes>

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

GradientModule_Comp_2D_Pass::GradientModule_Comp_2D_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : ShaderPass(vVulkanCore) {
    SetRenderDocDebugName("Comp Pass : Normal From Texture", COMPUTE_SHADER_PASS_DEBUG_COLOR);

    m_DontUseShaderFilesOnDisk = true;
}

GradientModule_Comp_2D_Pass::~GradientModule_Comp_2D_Pass() {
    Unit();
}

void GradientModule_Comp_2D_Pass::ActionBeforeInit() {
    ZoneScoped;

    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);

    for (auto& info : m_ImageInfos) {
        info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
    }
}

bool GradientModule_Comp_2D_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    ZoneScoped;

    ImGui::SetCurrentContext(vContextPtr);

    const float aw = ImGui::GetContentRegionAvail().x;

    bool change = false;

    change |= ImGui::ContrastedComboVectorDefault(aw, "Method", &m_UBOComp.method, m_MethodNames, 0);

    change |= ImGui::SliderFloatDefaultCompact(aw, "Smoothness", &m_UBOComp.smoothness, -1.0f, 1.0f, 0.5f);

    if (change) {
        NeedNewUBOUpload();
    }

    return change;
}

bool GradientModule_Comp_2D_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool GradientModule_Comp_2D_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void GradientModule_Comp_2D_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
    ZoneScoped;

    if (m_Loaded) {
        if (vBindingPoint < m_ImageInfos.size()) {
            if (vTextureSize) {
                m_ImageInfosSize[vBindingPoint] = *vTextureSize;

                NeedResizeByHandIfChanged(m_ImageInfosSize[0]);
            }

            if (vImageInfo) {
                m_ImageInfos[vBindingPoint] = *vImageInfo;
            } else {
                auto corePtr = m_VulkanCore.lock();
                assert(corePtr != nullptr);

                m_ImageInfos[vBindingPoint] = *corePtr->getEmptyTexture2DDescriptorImageInfo();
            }
        }
    }
}

vk::DescriptorImageInfo* GradientModule_Comp_2D_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
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

void GradientModule_Comp_2D_Pass::WasJustResized() {
    ZoneScoped;

    if (m_ComputeBufferPtr) {
        m_UBOComp.image_size = m_ComputeBufferPtr->GetOutputSize();

        NeedNewUBOUpload();
    }
}

void GradientModule_Comp_2D_Pass::Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) {
    if (vCmdBufferPtr) {
        vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);

        vCmdBufferPtr->bindDescriptorSets(
            vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
        Dispatch(vCmdBufferPtr, "Compute");
    }
}

bool GradientModule_Comp_2D_Pass::CreateUBO() {
    ZoneScoped;

    m_UBOCompPtr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBOComp), "GradientModule_Comp_2D_Pass");
    m_UBOComp_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
    if (m_UBOCompPtr) {
        m_UBOComp_BufferInfos.buffer = m_UBOCompPtr->buffer;
        m_UBOComp_BufferInfos.range = sizeof(UBOComp);
        m_UBOComp_BufferInfos.offset = 0;
    }

    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);

    for (auto& info : m_ImageInfos) {
        info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
    }

    NeedNewUBOUpload();

    return true;
}

void GradientModule_Comp_2D_Pass::UploadUBO() {
    ZoneScoped;

    VulkanRessource::upload(m_VulkanCore, m_UBOCompPtr, &m_UBOComp, sizeof(UBOComp));
}

void GradientModule_Comp_2D_Pass::DestroyUBO() {
    ZoneScoped;

    m_UBOCompPtr.reset();
    m_UBOComp_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
}

bool GradientModule_Comp_2D_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
    res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);
    return res;
}

bool GradientModule_Comp_2D_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U));  // output
    res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_UBOComp_BufferInfos);
    res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]);  // input 0
    return res;
}

std::string GradientModule_Comp_2D_Pass::GetComputeShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "GradientModule_Comp_2D_Pass";

    SetLocalGroupSize(ct::uvec3(32U, 32U, 1U));

    return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform image2D outColor;

layout(std140, binding = 1) uniform UBO_Comp
{
	int method;			// 0 -> 7
	float smoothness;	// 0 -> 1
	ivec2 image_size;
};

layout(binding = 2) uniform sampler2D inputBuffer;

vec4 getPixel(ivec2 g, int x, int y)
{
    return texelFetch(inputBuffer, g + ivec2(x,y), 0);
}

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

void main()
{
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

	float e = smoothness * 100.0 / min(image_size.x, image_size.y);
	float f = getValue(getPixel(coords, 0, 0));
	float fx = (f-getValue(getPixel(coords, 1, 0)))/e;
	float fy = (f-getValue(getPixel(coords, 0, 1)))/e;
	vec4 color = vec4(normalize(vec3(0,0,1) - vec3(fx,fy,0.0)) * 0.5 + 0.5, 1.0);
	
	imageStore(outColor, coords, color); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GradientModule_Comp_2D_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/) {
    std::string str;

    str += vOffset + "<method>" + ct::toStr(m_UBOComp.method) + "</method>\n";
    str += vOffset + "<smoothness>" + ct::toStr(m_UBOComp.smoothness) + "</smoothness>\n";

    return str;
}

bool GradientModule_Comp_2D_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/) {
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

    if (strParentName == "gradient_module") {
        if (strName == "method")
            m_UBOComp.method = ct::ivariant(strValue).GetI();
        else if (strName == "smoothness")
            m_UBOComp.smoothness = ct::fvariant(strValue).GetF();
    }

    return true;
}

void GradientModule_Comp_2D_Pass::AfterNodeXmlLoading() {
    ZoneScoped;

    // code to do after end of the xml loading of this node
    // by ex :
    NeedNewUBOUpload();
}