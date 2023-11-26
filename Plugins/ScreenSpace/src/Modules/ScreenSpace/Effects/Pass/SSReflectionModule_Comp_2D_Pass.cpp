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

#include "SSReflectionModule_Comp_2D_Pass.h"

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
#ifndef VKFPScoped
#define VKFPScoped
#endif

//////////////////////////////////////////////////////////////
///// STATIC /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<SSReflectionModule_Comp_2D_Pass> SSReflectionModule_Comp_2D_Pass::Create(
    const ct::uvec2& vSize, GaiApi::VulkanCorePtr vVulkanCorePtr) {
    auto res_ptr = std::make_shared<SSReflectionModule_Comp_2D_Pass>(vVulkanCorePtr);
    if (!res_ptr->InitCompute2D(vSize, 1U, false, vk::Format::eR32G32B32A32Sfloat)) {
        res_ptr.reset();
    }
    return res_ptr;
}

//////////////////////////////////////////////////////////////
///// CTOR / DTOR ////////////////////////////////////////////
//////////////////////////////////////////////////////////////

SSReflectionModule_Comp_2D_Pass::SSReflectionModule_Comp_2D_Pass(GaiApi::VulkanCorePtr vVulkanCorePtr) : EffectPass(vVulkanCorePtr) {
    ZoneScoped;
    SetRenderDocDebugName("Comp Pass : SS Reflection", COMPUTE_SHADER_PASS_DEBUG_COLOR);
    //m_DontUseShaderFilesOnDisk = true;
}

SSReflectionModule_Comp_2D_Pass::~SSReflectionModule_Comp_2D_Pass() {
    ZoneScoped;
    Unit();
}

void SSReflectionModule_Comp_2D_Pass::ActionBeforeInit() {
    ZoneScoped;
    // m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);
    for (auto& info : m_ImageInfos) {
        info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
    }
}

bool SSReflectionModule_Comp_2D_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    bool change = false;
    // change |= DrawResizeWidget();
    if (ImGui::CollapsingHeader_CheckBox("SS Reflection##SSReflectionModule_Comp_2D_Pass", -1.0f, false, true, IsEffectEnabled())) {
        change |= ImGui::SliderFloatDefaultCompact(0.0f, "max_distance", &m_UBO_Comp.u_max_distance, 0.000f, 16.000f, 8.000f, 0.0f, "%.3f");
        change |= ImGui::SliderFloatDefaultCompact(0.0f, "resolution", &m_UBO_Comp.u_resolution, 0.000f, 0.600f, 0.300f, 0.0f, "%.3f");
        change |= ImGui::SliderIntDefaultCompact(0.0f, "steps", &m_UBO_Comp.u_steps, 0, 10, 5);
        change |= ImGui::SliderFloatDefaultCompact(0.0f, "thickness", &m_UBO_Comp.u_thickness, 0.000f, 1.000f, 0.500f, 0.0f, "%.3f");
        if (change) {
            NeedNewUBOUpload();
        }
    }
    return change;
}

bool SSReflectionModule_Comp_2D_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool SSReflectionModule_Comp_2D_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void SSReflectionModule_Comp_2D_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) {
    ZoneScoped;
    if (m_Loaded && *IsEffectEnabled()) {
        if (vBindingPoint < m_ImageInfos.size()) {
            if (vImageInfo) {
                if (vTextureSize) {
                    m_ImageInfosSize[vBindingPoint] = *vTextureSize;
                    NeedResizeByHandIfChanged(m_ImageInfosSize[vBindingPoint]);
                }
                m_ImageInfos[vBindingPoint] = *vImageInfo;
                if (vBindingPoint == 3) {  // mask sampler
                    m_UBO_Comp.u_use_mask_sampler = 1.0f;
                    NeedNewUBOUpload();
                }
            } else {
                m_ImageInfos[vBindingPoint] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
                if (vBindingPoint == 3) {  // mask sampler
                    m_UBO_Comp.u_use_mask_sampler = 0.0f;
                    NeedNewUBOUpload();
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* SSReflectionModule_Comp_2D_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize) {
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

void SSReflectionModule_Comp_2D_Pass::WasJustResized() {
    ZoneScoped;
}

void SSReflectionModule_Comp_2D_Pass::Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) {
    if (vCmdBufferPtr) {
        VKFPScoped(*vCmdBufferPtr, "SS Reflection", "Compute");
        vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
        vCmdBufferPtr->bindDescriptorSets(
            vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
        for (uint32_t iter = 0; iter < m_CountIterations.w; iter++) {
            Dispatch(vCmdBufferPtr);
        }
    }
}

bool SSReflectionModule_Comp_2D_Pass::CreateUBO() {
    ZoneScoped;
    m_UBO_Comp_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Comp));
    m_UBO_Comp_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
    if (m_UBO_Comp_Ptr) {
        m_UBO_Comp_BufferInfos.buffer = m_UBO_Comp_Ptr->buffer;
        m_UBO_Comp_BufferInfos.range = sizeof(UBO_Comp);
        m_UBO_Comp_BufferInfos.offset = 0;
    }
    NeedNewUBOUpload();
    return true;
}

void SSReflectionModule_Comp_2D_Pass::UploadUBO() {
    ZoneScoped;
    assert(IsEffectEnabled() != nullptr);
    m_UBO_Comp.u_enabled = (*IsEffectEnabled()) ? 1.0f : 0.0f;
    VulkanRessource::upload(m_VulkanCorePtr, m_UBO_Comp_Ptr, &m_UBO_Comp, sizeof(UBO_Comp));
}

void SSReflectionModule_Comp_2D_Pass::DestroyUBO() {
    ZoneScoped;
    m_UBO_Comp_Ptr.reset();
    m_UBO_Comp_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
}

bool SSReflectionModule_Comp_2D_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;
    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);         // common system
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);         // UBO_Comp
    res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);  // sampler input
    res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);  // sampler input
    res &= AddOrSetLayoutDescriptor(4U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);  // sampler input
    res &= AddOrSetLayoutDescriptor(5U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);  // sampler input
    res &= AddOrSetLayoutDescriptor(6U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);          // color output
    return res;
}

bool SSReflectionModule_Comp_2D_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;
    bool res = true;
    res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo());          // common system
    res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_UBO_Comp_BufferInfos);                            // UBO_Comp
    res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0U]);                            // sampler input
    res &= AddOrSetWriteDescriptorImage(3U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[1U]);                            // sampler input
    res &= AddOrSetWriteDescriptorImage(4U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[2U]);                            // sampler input
    res &= AddOrSetWriteDescriptorImage(5U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[3U]);                            // sampler input
    res &= AddOrSetWriteDescriptorImage(6U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U));  // output
    return res;
}

std::string SSReflectionModule_Comp_2D_Pass::GetComputeShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "SSReflectionModule_Comp_2D_Pass_Compute";
    SetLocalGroupSize(ct::uvec3(8U, 8U, 1U));
    return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1 ) in;
)" 
+ 
CommonSystem::Instance()->GetBufferObjectStructureHeader(0U)
+
u8R"(
layout(std140, binding = 1) uniform UBO_Comp {
	float u_max_distance; // default is 8
	float u_resolution; // default is 0.3
	int u_steps; // default is 5
	float u_thickness; // default is 0.5
	float u_enabled; // default is 0.0 (false)
	float u_use_mask_sampler; // default is 0.0 (false)
};

layout(binding = 2) uniform sampler2D input_color_map;
layout(binding = 3) uniform sampler2D input_position_map;
layout(binding = 4) uniform sampler2D input_normal_map;
layout(binding = 5) uniform sampler2D input_mask_map;

layout(binding = 6, rgba32f) uniform image2D outColor; // output

vec4 getEffect(ivec2 vCoords) {
	vec2 si = textureSize(input_position_map, 0);
	vec2 uv = vec2(vCoords);
	
	vec4 currentColor = texelFetch(input_color_map, vCoords, 0);
	
	vec4 positionFrom     = texelFetch(input_position_map, vCoords, 0);
	vec3 unitPositionFrom = normalize(positionFrom.xyz);
	vec3 normal           = normalize(texelFetch(input_normal_map, vCoords, 0).xyz);
	vec3 pivot            = normalize(reflect(unitPositionFrom, normal));
	
	vec4 positionTo = positionFrom;

	vec4 startView = vec4(positionFrom.xyz + (pivot *            0.0), 1.0);
	vec4 endView   = vec4(positionFrom.xyz + (pivot * u_max_distance), 1.0);

	vec4 startFrag = startView;
	// Project to screen space.
	startFrag = proj * startFrag;
	// Perform the perspective divide.
	startFrag.xyz /= startFrag.w;
	// Convert the screen-space XY coordinates to UV coordinates.
	startFrag.xy = startFrag.xy * 0.5 + 0.5;
	// Convert the UV coordinates to fragment/pixel coordnates.
	
	vec4 endFrag = endView;
	endFrag = proj * endFrag;
	endFrag.xyz /= endFrag.w;
	endFrag.xy = endFrag.xy * 0.5 + 0.5;
	   
	float deltaX = endFrag.x - startFrag.x;
	float deltaY = endFrag.y - startFrag.y;
	
	float useX = abs(deltaX) >= abs(deltaY) ? 1 : 0;
	float delta = mix(abs(deltaY), abs(deltaX), useX) * clamp(u_resolution, 0.0, 1.0);
	
	if (delta != 0 && deltaX != 0 && deltaY != 0) {
		vec2 increment = vec2(deltaX, deltaY) / max(delta, 0.001);
		
		float search0 = 0;
		float search1 = 0;

		int hit0 = 0;
		int hit1 = 0;
		
		float viewDistance = startView.y;
		float depth = u_thickness;
		
		vec2 frag = startFrag.xy;
	   
		for (int i=0; i<u_steps; ++i) {
			frag += increment;
			positionTo = texelFetch(input_position_map, ivec2(frag), 0);
			float searchX = (frag.x - startFrag.x) / deltaX;
			float searchY = (frag.y - startFrag.y) / deltaY;
			search1 = mix(searchY, searchX, useX);
			search1 = clamp(search1, 0.0, 1.0);
			float quo = mix(endView.y, startView.y, search1);
			if (quo != 0) {
				viewDistance = (startView.y * endView.y) / quo;
				depth = viewDistance - positionTo.y;
				if (depth > 0 && depth < u_thickness) {
					hit0 = 1;
					break;
				} else {
					search0 = search1;
				}
			}
		}
		
		search1 = search0 + ((search1 - search0) / 2.0);

		int steps = u_steps * hit0;

		for (int i=0; i<steps; ++i) {
			frag = mix(startFrag.xy, endFrag.xy, search1);
			positionTo = texelFetch(input_position_map, ivec2(frag), 0);
			float quo = mix(endView.y, startView.y, search1);
			if (quo != 0) {
				viewDistance = (startView.y * endView.y) / quo;
				depth = viewDistance - positionTo.y;
				if (depth > 0 && depth < u_thickness) {
					hit1 = 1;
					search1 = search0 + ((search1 - search0) * 0.5);
				} else {
					float temp = search1;
					search1 = search1 + ((search1 - search0) * 0.5);
					search0 = temp;
				}
			}
		}

		float visibility = hit1 * 
			positionTo.w * 
			(1.0 - max ( dot(-unitPositionFrom, pivot), 0.0)) * 
			(1.0 - clamp( depth / max(u_thickness, 1.0), 0.0, 1.0)) *  
			(1.0 - clamp(length(positionTo - positionFrom) / max(u_max_distance, 1.0), 0.0, 1.0)) * 
			(uv.x < 0 || uv.x > 1.0 ? 0.0 : 1.0) * 
			(uv.y < 0 || uv.y > 1.0 ? 0.0 : 1.0);

		float alpha = clamp(visibility, 0.0, 1.0);
		vec4 refectedColor = texelFetch(input_color_map, ivec2(frag), 0);
		
		return mix(currentColor, refectedColor, alpha);
	}
	return currentColor;
}

void main() {
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	vec4 res = texelFetch(input_color_map, coords, 0);
    if (u_enabled > 0.5 &&  res.a > 0.0) {
        res = getEffect(coords);
    }
	imageStore(outColor, coords, res); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string SSReflectionModule_Comp_2D_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;
    std::string str;
    str += vOffset + "<ss_reflection_pass>\n";
    str += ShaderPass::getXml(vOffset + "\t", vUserDatas);
    str += vOffset + "\t<max_distance>" + ct::toStr(m_UBO_Comp.u_max_distance) + "</max_distance>\n";
    str += vOffset + "\t<resolution>" + ct::toStr(m_UBO_Comp.u_resolution) + "</resolution>\n";
    str += vOffset + "\t<steps>" + ct::toStr(m_UBO_Comp.u_steps) + "</steps>\n";
    str += vOffset + "\t<thickness>" + ct::toStr(m_UBO_Comp.u_thickness) + "</thickness>\n";
    str += vOffset + "\t<enabled>" + ct::toStr(m_UBO_Comp.u_enabled) + "</enabled>\n";
    str += vOffset + "</ss_reflection_pass>\n";
    return str;
}

bool SSReflectionModule_Comp_2D_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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
    if (strParentName == "ss_reflection_pass") {
        ShaderPass::setFromXml(vElem, vParent, vUserDatas);
        if (strName == "max_distance") {
            m_UBO_Comp.u_max_distance = ct::fvariant(strValue).GetF();
        } else if (strName == "resolution") {
            m_UBO_Comp.u_resolution = ct::fvariant(strValue).GetF();
        } else if (strName == "steps") {
            m_UBO_Comp.u_steps = ct::ivariant(strValue).GetI();
        } else if (strName == "thickness") {
            m_UBO_Comp.u_thickness = ct::fvariant(strValue).GetF();
        } else if (strName == "enabled") {
            m_UBO_Comp.u_enabled = ct::fvariant(strValue).GetF();
            *IsEffectEnabled() = m_UBO_Comp.u_enabled;
        }
    }
    return true;
}

void SSReflectionModule_Comp_2D_Pass::AfterNodeXmlLoading() {
    ZoneScoped;
    NeedNewUBOUpload();
}
