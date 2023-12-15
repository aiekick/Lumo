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

#include "ShapeModule_Comp_3D_Pass.h"

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

std::shared_ptr<ShapeModule_Comp_3D_Pass> ShapeModule_Comp_3D_Pass::Create(const ct::uvec3& vSize, GaiApi::VulkanCoreWeak vVulkanCore) {
	auto res_ptr = std::make_shared<ShapeModule_Comp_3D_Pass>(vVulkanCore);
	if (!res_ptr->InitCompute3D(vSize)) {
		res_ptr.reset();
	}
	return res_ptr;
}

//////////////////////////////////////////////////////////////
///// CTOR / DTOR ////////////////////////////////////////////
//////////////////////////////////////////////////////////////

ShapeModule_Comp_3D_Pass::ShapeModule_Comp_3D_Pass(GaiApi::VulkanCoreWeak vVulkanCore)
	: ShaderPass(vVulkanCore) {
	ZoneScoped;
	SetRenderDocDebugName("Comp Pass : Shape", COMPUTE_SHADER_PASS_DEBUG_COLOR);
	m_DontUseShaderFilesOnDisk = true;
}

ShapeModule_Comp_3D_Pass::~ShapeModule_Comp_3D_Pass() {
	ZoneScoped;
	Unit();
}

void ShapeModule_Comp_3D_Pass::ActionBeforeInit() {
	ZoneScoped;
	//m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);
    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);
}

bool ShapeModule_Comp_3D_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
	ZoneScoped;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	bool change = false;
	//change |= DrawResizeWidget();

	change |= ImGui::ContrastedComboVectorDefault(0.0f, "type", &m_UBO_Comp.u_type, { "cube", " sphere", " torus" }, 0);
	change |= ImGui::SliderFloatDefaultCompact(0.0f, "size", &m_UBO_Comp.u_size, 0.000f, 2.000f, 1.000f, 0.0f, "%.3f");
	change |= ImGui::SliderFloatDefaultCompact(0.0f, "thick", &m_UBO_Comp.u_thick, 0.000f, 0.000f, 0.000f, 0.0f, "%.3f");
	static ct::fvec4 _default_value = 1.0f;
	change |= ImGui::ColorEdit4Default(0.0f, "color", &m_UBO_Comp.u_color.x, &_default_value.x);
	
	if (change) {
		NeedNewUBOUpload();
	}

	return change;
}

bool ShapeModule_Comp_3D_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
	ZoneScoped;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	return false;
}

bool ShapeModule_Comp_3D_Pass::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, void* vUserDatas) {
	ZoneScoped;
	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* ShapeModule_Comp_3D_Pass::GetDescriptorImage3DInfo(const uint32_t& vBindingPoint, ct::fvec3* vOutSize) {	
	ZoneScoped;
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShapeModule_Comp_3D_Pass::WasJustResized() {
	ZoneScoped;
}

void ShapeModule_Comp_3D_Pass::Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) {
	if (vCmdBufferPtr) {
		//VKFPScoped(*vCmdBufferPtr, "Shape", "Compute");
		vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
		vCmdBufferPtr->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
		for (uint32_t iter = 0; iter < m_CountIterations.w; iter++)	{
			Dispatch(vCmdBufferPtr, __FUNCTION__);
		}
	}
}


bool ShapeModule_Comp_3D_Pass::CreateUBO() {
	ZoneScoped;
	m_UBO_Comp_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBO_Comp), "ShapeModule_Comp_3D_Pass");
	m_UBO_Comp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_Comp_Ptr) {
		m_UBO_Comp_BufferInfos.buffer = m_UBO_Comp_Ptr->buffer;
		m_UBO_Comp_BufferInfos.range = sizeof(UBO_Comp);
		m_UBO_Comp_BufferInfos.offset = 0;
	}
	NeedNewUBOUpload();
	return true;
}

void ShapeModule_Comp_3D_Pass::UploadUBO() {
	ZoneScoped;
	VulkanRessource::upload(m_VulkanCore, m_UBO_Comp_Ptr, &m_UBO_Comp, sizeof(UBO_Comp));
}

void ShapeModule_Comp_3D_Pass::DestroyUBO() {
	ZoneScoped;
	m_UBO_Comp_Ptr.reset();
	m_UBO_Comp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
}

bool ShapeModule_Comp_3D_Pass::UpdateLayoutBindingInRessourceDescriptor() {
	ZoneScoped;
	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute); // UBO_Comp
	return res;
}

bool ShapeModule_Comp_3D_Pass::UpdateBufferInfoInRessourceDescriptor() {
	ZoneScoped;
	bool res = true;
	res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, &m_UBO_Comp_BufferInfos); // UBO_Comp
	return res;
}

std::string ShapeModule_Comp_3D_Pass::GetComputeShaderCode(std::string& vOutShaderName) {
	vOutShaderName = "ShapeModule_Comp_3D_Pass_Compute";
	SetLocalGroupSize(ct::uvec3(1U, 1U, 1U));
	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

layout(std140, binding = 0) uniform UBO_Comp {
	int u_type; // default is cube, sphere, torus
	float u_size; // default is 1
	float u_thick; // default is 0
	vec4 u_color; // default is 0, , , 
	float u_Transform; // default is 0
};

layout(binding = 1, rgba32f) uniform image3D outColor; // output

void main() {
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
}
)";
		}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShapeModule_Comp_3D_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
	ZoneScoped;
	std::string str;
    str += vOffset + "<shape_pass>\n";
	str += ShaderPass::getXml(vOffset + "\t", vUserDatas);
	str += vOffset + "\t<type>" + ct::toStr(m_UBO_Comp.u_type) + "</type>\n";
	str += vOffset + "\t<size>" + ct::toStr(m_UBO_Comp.u_size) + "</size>\n";
	str += vOffset + "\t<thick>" + ct::toStr(m_UBO_Comp.u_thick) + "</thick>\n";
	str += vOffset + "\t<color>" + m_UBO_Comp.u_color.string() + "</color>\n";
	str += vOffset + "\t<transform>" + ct::toStr(m_UBO_Comp.u_Transform) + "</transform>\n";
    str += vOffset + "</shape_pass>\n";
	return str;
}

bool ShapeModule_Comp_3D_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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
	if (strParentName == "shape_pass") {
		ShaderPass::setFromXml(vElem, vParent, vUserDatas);
		if (strName == "type") {
			m_UBO_Comp.u_type = ct::ivariant(strValue).GetI();
		} else if (strName == "size") {
			m_UBO_Comp.u_size = ct::fvariant(strValue).GetF();
		} else if (strName == "thick") {
			m_UBO_Comp.u_thick = ct::fvariant(strValue).GetF();
		} else if (strName == "color") {
			m_UBO_Comp.u_color = ct::fvariant(strValue).GetV4();
		} else if (strName == "transform") {
			m_UBO_Comp.u_Transform = ct::fvariant(strValue).GetF();
		}
	}
	return true;
}

void ShapeModule_Comp_3D_Pass::AfterNodeXmlLoading() {
	ZoneScoped;
	NeedNewUBOUpload();
}
