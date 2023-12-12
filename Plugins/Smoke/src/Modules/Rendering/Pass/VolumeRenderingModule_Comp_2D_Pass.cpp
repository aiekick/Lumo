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

#include "VolumeRenderingModule_Comp_2D_Pass.h"

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

std::shared_ptr<VolumeRenderingModule_Comp_2D_Pass> VolumeRenderingModule_Comp_2D_Pass::Create(const ct::uvec2& vSize, GaiApi::VulkanCoreWeak vVulkanCore) {
	auto res_ptr = std::make_shared<VolumeRenderingModule_Comp_2D_Pass>(vVulkanCore);
	if (!res_ptr->InitCompute2D(vSize, 1U, false, vk::Format::eR32G32B32A32Sfloat)) {
		res_ptr.reset();
	}
	return res_ptr;
}

//////////////////////////////////////////////////////////////
///// CTOR / DTOR ////////////////////////////////////////////
//////////////////////////////////////////////////////////////

VolumeRenderingModule_Comp_2D_Pass::VolumeRenderingModule_Comp_2D_Pass(GaiApi::VulkanCoreWeak vVulkanCore)
	: ShaderPass(vVulkanCore) {
	ZoneScoped;
	SetRenderDocDebugName("Comp Pass : Volume Rendering", COMPUTE_SHADER_PASS_DEBUG_COLOR);
	m_DontUseShaderFilesOnDisk = true;
}

VolumeRenderingModule_Comp_2D_Pass::~VolumeRenderingModule_Comp_2D_Pass() {
	ZoneScoped;
	Unit();
}

void VolumeRenderingModule_Comp_2D_Pass::ActionBeforeInit() {
	ZoneScoped;
	//m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);
    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);
	for (auto& info : m_Image3DInfos)	{
		info = *corePtr->getEmptyTexture3DDescriptorImageInfo();
	}
}

bool VolumeRenderingModule_Comp_2D_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	bool change = false;
	//change |= DrawResizeWidget();

	change |= ImGui::SliderFloatDefaultCompact(0.0f, "Name", &m_UBO_Comp.u_Name, 0.000f, 0.000f, 0.000f, 0.0f, "%.3f");	
	if (change) {
		NeedNewUBOUpload();
	}

	return change;
}

bool VolumeRenderingModule_Comp_2D_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	return false;
}

bool VolumeRenderingModule_Comp_2D_Pass::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void VolumeRenderingModule_Comp_2D_Pass::SetTexture3D(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImage3DInfo, ct::fvec3* vTextureSize) {	
	ZoneScoped;
	if (m_Loaded) {
		if (vBindingPoint < m_Image3DInfos.size()) {
			if (vImage3DInfo) {
				if (vTextureSize) {
					m_Image3DInfosSize[vBindingPoint] = *vTextureSize;
				}
				m_Image3DInfos[vBindingPoint] = *vImage3DInfo;
			} else {
                auto corePtr = m_VulkanCore.lock();
                assert(corePtr != nullptr);
				m_Image3DInfos[vBindingPoint] = *corePtr->getEmptyTexture3DDescriptorImageInfo();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* VolumeRenderingModule_Comp_2D_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize) {	
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

void VolumeRenderingModule_Comp_2D_Pass::WasJustResized() {
	ZoneScoped;
}

void VolumeRenderingModule_Comp_2D_Pass::Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) {
	if (vCmdBufferPtr) {
		//VKFPScoped(*vCmdBufferPtr, "Volume Rendering", "Compute");
		vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
		vCmdBufferPtr->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
		for (uint32_t iter = 0; iter < m_CountIterations.w; iter++)	{
			Dispatch(vCmdBufferPtr, __FUNCTION__);
		}
	}
}


bool VolumeRenderingModule_Comp_2D_Pass::CreateUBO() {
	ZoneScoped;
	m_UBO_Comp_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBO_Comp), "VolumeRenderingModule_Comp_2D_Pass");
	m_UBO_Comp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_Comp_Ptr) {
		m_UBO_Comp_BufferInfos.buffer = m_UBO_Comp_Ptr->buffer;
		m_UBO_Comp_BufferInfos.range = sizeof(UBO_Comp);
		m_UBO_Comp_BufferInfos.offset = 0;
	}
	NeedNewUBOUpload();
	return true;
}

void VolumeRenderingModule_Comp_2D_Pass::UploadUBO() {
	ZoneScoped;
	VulkanRessource::upload(m_VulkanCore, m_UBO_Comp_Ptr, &m_UBO_Comp, sizeof(UBO_Comp));
}

void VolumeRenderingModule_Comp_2D_Pass::DestroyUBO() {
	ZoneScoped;
	m_UBO_Comp_Ptr.reset();
	m_UBO_Comp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
}

bool VolumeRenderingModule_Comp_2D_Pass::UpdateLayoutBindingInRessourceDescriptor() {
	ZoneScoped;
	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute); // common system
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute); // UBO_Comp
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute); // sampler input
	res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute); // color output
	return res;
}

bool VolumeRenderingModule_Comp_2D_Pass::UpdateBufferInfoInRessourceDescriptor() {
	ZoneScoped;
	bool res = true;
	res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo()); // common system
	res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_UBO_Comp_BufferInfos); // UBO_Comp
	res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_Image3DInfos[0U]);  // sampler input
	res &= AddOrSetWriteDescriptorImage(3U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U)); // output
	return res;
}

std::string VolumeRenderingModule_Comp_2D_Pass::GetComputeShaderCode(std::string& vOutShaderName) {
	vOutShaderName = "VolumeRenderingModule_Comp_2D_Pass_Compute";
	SetLocalGroupSize(ct::uvec3(1U, 1U, 1U));
	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;
")
+CommonSystem::Instance()->GetBufferObjectStructureHeader(0)+u8R"(
layout(std140, binding = 1) uniform UBO_Comp {
	float u_Name; // default is 0
};

layout(binding = 2) uniform sampler3D input_density_map;
layout(binding = 3, rgba32f) uniform image2D outColor; // output

void main() {
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	vec4 color = vec4(coords, 0, 1);
	imageStore(outColor, coords, color); 
}
)";
		}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string VolumeRenderingModule_Comp_2D_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
	ZoneScoped;
	std::string str;
    str += vOffset + "<volume_rendering_pass>\n";
	str += ShaderPass::getXml(vOffset + "\t", vUserDatas);
	str += vOffset + "\t<name>" + ct::toStr(m_UBO_Comp.u_Name) + "</name>\n";
    str += vOffset + "</volume_rendering_pass>\n";
	return str;
}

bool VolumeRenderingModule_Comp_2D_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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
	if (strParentName == "volume_rendering_pass") {
		ShaderPass::setFromXml(vElem, vParent, vUserDatas);
		if (strName == "name") {
			m_UBO_Comp.u_Name = ct::fvariant(strValue).GetF();
		}
	}
	return true;
}

void VolumeRenderingModule_Comp_2D_Pass::AfterNodeXmlLoading() {
	ZoneScoped;
	NeedNewUBOUpload();
}
