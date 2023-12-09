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

#include "SpecularModule_Comp_Pass.h"

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
#include <Modules/Lighting/LightGroupModule.h>

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

SpecularModule_Comp_Pass::SpecularModule_Comp_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : ShaderPass(vVulkanCore) {
    SetRenderDocDebugName("Comp Pass : Specular", COMPUTE_SHADER_PASS_DEBUG_COLOR);

    m_DontUseShaderFilesOnDisk = true;
}

SpecularModule_Comp_Pass::~SpecularModule_Comp_Pass() {
    Unit();
}

bool SpecularModule_Comp_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    bool change = false;

    if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
        change |= ImGui::SliderFloatDefaultCompact(0.0f, "Power", &m_UBOComp.u_pow_coef, 0.0f, 64.0f, 8.0f);

        if (change) {
            NeedNewUBOUpload();
        }
    }

    // DrawInputTexture(m_VulkanCore, "Input Position", 0U, m_OutputRatio);

    return change;
}

bool SpecularModule_Comp_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool SpecularModule_Comp_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void SpecularModule_Comp_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) {
    ZoneScoped;

    if (m_Loaded) {
        if (vBindingPoint < m_ImageInfos.size()) {
            if (vImageInfo) {
                if (vTextureSize) {
                    m_ImageInfosSize[vBindingPoint] = *vTextureSize;

                    NeedResizeByHandIfChanged(m_ImageInfosSize[0]);
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

vk::DescriptorImageInfo* SpecularModule_Comp_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize) {
    if (m_ComputeBufferPtr) {
        AutoResizeBuffer(std::dynamic_pointer_cast<OutputSizeInterface>(m_ComputeBufferPtr).get(), vOutSize);

        return m_ComputeBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
    }

    return nullptr;
}

void SpecularModule_Comp_Pass::SetLightGroup(SceneLightGroupWeak vSceneLightGroup) {
    m_SceneLightGroup = vSceneLightGroup;

    m_SceneLightGroupDescriptorInfoPtr = &m_SceneEmptyLightGroupDescriptorInfo;

    auto lightGroupPtr = m_SceneLightGroup.lock();
    if (lightGroupPtr && lightGroupPtr->GetBufferInfo()) {
        m_SceneLightGroupDescriptorInfoPtr = lightGroupPtr->GetBufferInfo();
    }

    UpdateBufferInfoInRessourceDescriptor();
}

void SpecularModule_Comp_Pass::Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) {
    if (vCmdBufferPtr) {
        vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
        vCmdBufferPtr->bindDescriptorSets(
            vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
        Dispatch(vCmdBufferPtr, "Compute");
    }
}

bool SpecularModule_Comp_Pass::CreateUBO() {
    ZoneScoped;

    m_UBOCompPtr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBOComp), "SpecularModule_Comp_Pass");
    if (m_UBOCompPtr->buffer) {
        m_UBOComp_BufferInfo = vk::DescriptorBufferInfo{m_UBOCompPtr->buffer, 0, sizeof(UBOComp)};
    } else {
        m_UBOComp_BufferInfo = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
    }

    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);

    for (auto& info : m_ImageInfos) {
        info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
    }

    NeedNewUBOUpload();

    return true;
}

void SpecularModule_Comp_Pass::UploadUBO() {
    ZoneScoped;

    VulkanRessource::upload(m_VulkanCore, m_UBOCompPtr, &m_UBOComp, sizeof(UBOComp));
}

void SpecularModule_Comp_Pass::DestroyUBO() {
    ZoneScoped;

    m_UBOCompPtr.reset();
}

bool SpecularModule_Comp_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
    res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
    res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
    res &= AddOrSetLayoutDescriptor(4U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);
    res &= AddOrSetLayoutDescriptor(5U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);
    return res;
}

bool SpecularModule_Comp_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetBackDescriptorImageInfo(0U));  // output
    res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo());         // output
    res &= AddOrSetWriteDescriptorBuffer(2U, vk::DescriptorType::eStorageBuffer, m_SceneLightGroupDescriptorInfoPtr);
    res &= AddOrSetWriteDescriptorBuffer(3U, vk::DescriptorType::eUniformBuffer, &m_UBOComp_BufferInfo);   // output
    res &= AddOrSetWriteDescriptorImage(4U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]);  // pos
    res &= AddOrSetWriteDescriptorImage(5U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[1]);  // nor
    return res;
}

std::string SpecularModule_Comp_Pass::GetComputeShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "SpecularModule_Comp_Pass";

    SetLocalGroupSize(ct::uvec3(8U, 8U, 1U));

    return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColor;
)" + CommonSystem::GetBufferObjectStructureHeader(1U) +
           SceneLightGroup::GetBufferObjectStructureHeader(2U) +
           u8R"(
layout (std140, binding = 3) uniform UBO_Comp
{
	float u_pow_coef;
};

layout(binding = 4) uniform sampler2D pos_map_sampler;
layout(binding = 5) uniform sampler2D nor_map_sampler;

vec3 getRayOrigin()
{
	vec3 ro = view[3].xyz + model[3].xyz;
	ro *= mat3(view * model);
	return -ro;
}

vec4 getLightGroup(uint id, ivec2 coords, vec3 pos)
{
	vec4 spec = vec4(1.0);

	if (lightDatas[id].lightActive > 0.5)
	{
		vec3 light_pos = lightDatas[id].lightGizmo[3].xyz;
		float light_intensity = lightDatas[id].lightIntensity;
		vec4 light_col = lightDatas[id].lightColor;
	
		vec3 normal = normalize(texelFetch(nor_map_sampler, coords, 0).xyz * 2.0 - 1.0);
		vec3 light_dir = normalize(light_pos - pos);
		
		if (lightDatas[id].is_inside > 0.5) // inside mesh
			normal *= - 1.0;

		vec3 ro = getRayOrigin();
		vec3 rd = normalize(ro - pos);
		vec3 refl = reflect(-light_dir, normal);  
		spec = min(pow(max(dot(rd, refl), 0.0), u_pow_coef) * light_intensity, 1.0) * light_col;
	}

	return spec;
}

void main()
{
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	
	vec4 res = vec4(0.0);
	
	vec3 pos = texelFetch(pos_map_sampler, coords, 0).xyz;
	if (dot(pos, pos) > 0.0)
	{
		uint count = uint(lightsCount) % 8; // maxi 8 lights in this system
		for (int i=0;i<count;++i)
		{
			res += getLightGroup(i, coords, pos);
		}
	}
	
	imageStore(outColor, coords, res); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string SpecularModule_Comp_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/) {
    std::string str;

    // str += vOffset + "<blur_radius>" + ct::toStr(m_UBOComp.u_blur_radius) + "</blur_radius>\n";

    return str;
}

bool SpecularModule_Comp_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "diffuse_module") {
        // if (strName == "blur_radius")
        //	m_UBOComp.u_blur_radius = ct::uvariant(strValue).GetU();

        NeedNewUBOUpload();
    }

    return true;
}