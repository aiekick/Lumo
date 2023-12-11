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

#include "HeatMapModule_Comp_2D_Pass.h"

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

std::shared_ptr<HeatMapModule_Comp_2D_Pass> HeatMapModule_Comp_2D_Pass::Create(const ct::uvec2& vSize, GaiApi::VulkanCoreWeak vVulkanCore) {
	auto res_ptr = std::make_shared<HeatMapModule_Comp_2D_Pass>(vVulkanCore);
	if (!res_ptr->InitCompute2D(vSize, 1U, false, vk::Format::eR32G32B32A32Sfloat)) {
		res_ptr.reset();
	}
	return res_ptr;
}

//////////////////////////////////////////////////////////////
///// CTOR / DTOR ////////////////////////////////////////////
//////////////////////////////////////////////////////////////

HeatMapModule_Comp_2D_Pass::HeatMapModule_Comp_2D_Pass(GaiApi::VulkanCoreWeak vVulkanCore)
	: EffectPass(vVulkanCore) {
	ZoneScoped;
	SetRenderDocDebugName("Comp Pass : Heat Map", COMPUTE_SHADER_PASS_DEBUG_COLOR);
	m_DontUseShaderFilesOnDisk = true;
	*IsEffectEnabled() = true;
}

HeatMapModule_Comp_2D_Pass::~HeatMapModule_Comp_2D_Pass() {
	ZoneScoped;
	Unit();
}

void HeatMapModule_Comp_2D_Pass::ActionBeforeInit() {
	ZoneScoped;
	//m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);
    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);
	for (auto& info : m_ImageInfos)	{
		info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
    }
    ClearColorBuffer();
    AddColorToBuffer(ct::fvec4(0.32f, 0.00f, 0.32f, 1.00f));
    AddColorToBuffer(ct::fvec4(0.00f, 0.00f, 1.00f, 1.00f));
    AddColorToBuffer(ct::fvec4(0.00f, 1.00f, 0.00f, 1.00f));
    AddColorToBuffer(ct::fvec4(1.00f, 1.00f, 0.00f, 1.00f));
    AddColorToBuffer(ct::fvec4(1.00f, 0.60f, 0.00f, 1.00f));
    AddColorToBuffer(ct::fvec4(1.00f, 0.00f, 0.00f, 1.00f));
}

bool HeatMapModule_Comp_2D_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
    bool changeUBO = false;
    bool changeSBO = false;
	//change |= DrawResizeWidget();
	if (ImGui::CollapsingHeader_CheckBox("Heat Map##HeatMapModule_Comp_2D_Pass", -1.0f, false, true, IsEffectEnabled())) {
        ImGui::Text("Component : ");
        ImGui::SameLine();
        if (ImGui::RadioButtonLabeled(0.0f, "R", m_UBO_Comp.u_channel_idx == 0, false)) {
            changeUBO = true;
            m_UBO_Comp.u_channel_idx = 0;
        }
        ImGui::SameLine();
        if (ImGui::RadioButtonLabeled(0.0f, "G", m_UBO_Comp.u_channel_idx == 1, false)) {
            changeUBO = true;
            m_UBO_Comp.u_channel_idx = 1;
        }
        ImGui::SameLine();
        if (ImGui::RadioButtonLabeled(0.0f, "B", m_UBO_Comp.u_channel_idx == 2, false)) {
            changeUBO = true;
            m_UBO_Comp.u_channel_idx = 2;
        }
        ImGui::SameLine();
        if (ImGui::RadioButtonLabeled(0.0f, "A", m_UBO_Comp.u_channel_idx == 3, false)) {
            changeUBO = true;
            m_UBO_Comp.u_channel_idx = 3;
        }
        changeUBO |= ImGui::SliderFloatDefaultCompact(0.0f, "Min", &m_UBO_Comp.u_min, 0.0f, 1.0f, 0.0f);
        changeUBO |= ImGui::SliderFloatDefaultCompact(0.0f, "Max", &m_UBO_Comp.u_max, 0.0f, 1.0f, 1.0f);

        for (uint32_t i = 0; i < m_UBO_Comp.u_count_colors; ++i) {
            ImGui::PushID(ImGui::IncPUSHID());
            changeSBO |= ImGui::ColorEdit4Default(0.0f, ct::toStr("Color %u", i).c_str(), &m_Colors[i].x, &m_DefaultColors[i].x);
            ImGui::PopID();
        }

        if (changeUBO) {
            NeedNewUBOUpload();
        }

        if (changeSBO) {
            NeedNewSBOUpload();
        }
    }

    return changeUBO || changeSBO;
}

bool HeatMapModule_Comp_2D_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	return false;
}

bool HeatMapModule_Comp_2D_Pass::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void HeatMapModule_Comp_2D_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) {	
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

vk::DescriptorImageInfo* HeatMapModule_Comp_2D_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize) {	
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

void HeatMapModule_Comp_2D_Pass::ClearColorBuffer() {
    m_Colors.clear();
    m_DefaultColors.clear();
}

void HeatMapModule_Comp_2D_Pass::AddColorToBuffer(const ct::fvec4& vColor) {
    m_Colors.push_back(vColor);
    m_DefaultColors.push_back(vColor);
    m_UBO_Comp.u_count_colors = (int32_t)m_Colors.size();
}

void HeatMapModule_Comp_2D_Pass::WasJustResized() {
	ZoneScoped;
}

void HeatMapModule_Comp_2D_Pass::Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) {
	if (vCmdBufferPtr) {
		VKFPScoped(*vCmdBufferPtr, "Heat Map", "Compute");
		vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
		vCmdBufferPtr->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
		for (uint32_t iter = 0; iter < m_CountIterations.w; iter++)	{
			Dispatch(vCmdBufferPtr, __FUNCTION__);
		}
	}
}

bool HeatMapModule_Comp_2D_Pass::CreateSBO() {
    ZoneScoped;

    m_SBO_Colors.reset();

    const auto sizeInBytes = sizeof(ct::fvec4) * m_Colors.size();
    m_SBO_Colors = VulkanRessource::createStorageBufferObject(
        m_VulkanCore, sizeInBytes, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU, "HeatMapModule_Comp_2D_Pass");
    if (m_SBO_Colors && m_SBO_Colors->buffer) {
        m_SBO_ColorsDescriptorBufferInfo = vk::DescriptorBufferInfo{m_SBO_Colors->buffer, 0, sizeInBytes};
    } else {
        m_SBO_ColorsDescriptorBufferInfo = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
    }

    NeedNewSBOUpload();

    return true;
}

void HeatMapModule_Comp_2D_Pass::UploadSBO() {
    if (m_SBO_Colors) {
        const auto sizeInBytes = sizeof(ct::fvec4) * m_Colors.size();
        VulkanRessource::upload(m_VulkanCore, m_SBO_Colors, m_Colors.data(), sizeInBytes);
    }
}

void HeatMapModule_Comp_2D_Pass::DestroySBO() {
    ZoneScoped;

    m_SBO_Colors.reset();
}

bool HeatMapModule_Comp_2D_Pass::CreateUBO() {
	ZoneScoped;
	m_UBO_Comp_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBO_Comp), "HeatMapModule_Comp_2D_Pass");
	m_UBO_Comp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_Comp_Ptr) {
		m_UBO_Comp_BufferInfos.buffer = m_UBO_Comp_Ptr->buffer;
		m_UBO_Comp_BufferInfos.range = sizeof(UBO_Comp);
		m_UBO_Comp_BufferInfos.offset = 0;
	}
	NeedNewUBOUpload();
	return true;
}

void HeatMapModule_Comp_2D_Pass::UploadUBO() {
	ZoneScoped;
    assert(IsEffectEnabled() != nullptr);
    m_UBO_Comp.u_enabled = (*IsEffectEnabled()) ? 1.0f : 0.0f;
	VulkanRessource::upload(m_VulkanCore, m_UBO_Comp_Ptr, &m_UBO_Comp, sizeof(UBO_Comp));
}

void HeatMapModule_Comp_2D_Pass::DestroyUBO() {
	ZoneScoped;
	m_UBO_Comp_Ptr.reset();
	m_UBO_Comp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };}

bool HeatMapModule_Comp_2D_Pass::UpdateLayoutBindingInRessourceDescriptor() {
	ZoneScoped;
	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute); // UBO_Comp
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute); // SBO_Colors
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute); // sampler input
	res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute); // color output
	return res;
}

bool HeatMapModule_Comp_2D_Pass::UpdateBufferInfoInRessourceDescriptor() {
	ZoneScoped;
	bool res = true;
	res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, &m_UBO_Comp_BufferInfos); // UBO_Comp
    res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eStorageBuffer, &m_SBO_ColorsDescriptorBufferInfo); // sbo
	res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0U]);  // sampler input
	res &= AddOrSetWriteDescriptorImage(3U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U)); // output
	return res;
}

std::string HeatMapModule_Comp_2D_Pass::GetComputeShaderCode(std::string& vOutShaderName) {
	vOutShaderName = "HeatMapModule_Comp_2D_Pass_Compute";
	SetLocalGroupSize(ct::uvec3(8U, 8U, 1U));
	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1 ) in;

layout(std140, binding = 0) uniform UBO_Comp {
	uint u_channel_idx; // 0..3
	uint u_color_count; // 0..N
	float u_min; // min value
	float u_max; // max value
	float u_enabled; // default is 0.0 (false)
};

layout(std140, binding = 1) readonly buffer SBO_Frag { 
	vec4 s_colors[]; // N Colors
};

layout(binding = 2) uniform sampler2D input_color_map;
layout(binding = 3, rgba32f) uniform image2D outColor; // output

vec4 HeatMapColor(float value, float minValue, float maxValue)
{
    float ratio = (u_color_count-1.0) * clamp((value-minValue) / (maxValue-minValue), 0.0, 1.0);
    uint indexMin = uint(floor(ratio));
    uint indexMax = min(indexMin+1,u_color_count-1);
    return mix(s_colors[indexMin], s_colors[indexMax], ratio-indexMin);
}

vec4 effect(ivec2 vCoords, vec4 res) {
	if (res[u_channel_idx % 3] > 0.0) {
		res = HeatMapColor(res[u_channel_idx % 3], u_min, u_max);
	}
    return res;
}

void main() {
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	vec4 res = texelFetch(input_color_map, coords, 0);
    if (u_enabled > 0.5) {
        res = effect(coords, res);
    }
	imageStore(outColor, coords, res); 
}
)";
		}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string HeatMapModule_Comp_2D_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
	ZoneScoped;
	std::string str;
    str += vOffset + "<heat_map_pass>\n";
	str += ShaderPass::getXml(vOffset + "\t", vUserDatas);
    str += vOffset + "\t<enabled>" + ct::toStr(m_UBO_Comp.u_enabled) + "</enabled>\n";
    str += vOffset + "\t<min>" + ct::toStr(m_UBO_Comp.u_min) + "</min>\n";
    str += vOffset + "\t<max>" + ct::toStr(m_UBO_Comp.u_max) + "</max>\n";
    str += vOffset + ct::toStr("<component>%u</component>\n", m_UBO_Comp.u_channel_idx);
    str += vOffset + "<color_levels>\n";
    for (auto color : m_Colors) {
        str += vOffset + ct::toStr("\t<color>%s</color>\n", color.string().c_str());
    }
    str += vOffset + "</color_levels>\n";
    str += vOffset + "</heat_map_pass>\n";
	return str;
}

bool HeatMapModule_Comp_2D_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (strParentName == "heat_map_pass") {
        ShaderPass::setFromXml(vElem, vParent, vUserDatas);
        if (strName == "enabled") {
            m_UBO_Comp.u_enabled = ct::fvariant(strValue).GetF();
            *IsEffectEnabled() = m_UBO_Comp.u_enabled;
        } else if (strName == "component") {
            m_UBO_Comp.u_channel_idx = ct::ivariant(strValue).GetI();
        } else if (strName == "min") {
            m_UBO_Comp.u_min = ct::fvariant(strValue).GetF();
        } else if (strName == "max") {
            m_UBO_Comp.u_max = ct::fvariant(strValue).GetF();
        } else if (strName == "color_levels") {
            m_Colors.clear();
        }
    }

    if (strParentName == "color_levels") {
        if (strName == "color") {
            m_Colors.push_back(ct::fvariant(strValue).GetV4());
        }
    }

	return true;
}

void HeatMapModule_Comp_2D_Pass::AfterNodeXmlLoading() {
	ZoneScoped;
	NeedNewUBOUpload();
}
