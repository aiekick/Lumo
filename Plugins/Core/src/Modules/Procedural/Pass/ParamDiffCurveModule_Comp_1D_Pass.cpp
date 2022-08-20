/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "ParamDiffCurveModule_Comp_1D_Pass.h"

#include <cinttypes>
#include <functional>
#include <Gui/MainFrame.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanSubmitter.h>
#include <utils/Mesh/VertexStruct.h>
#include <Base/FrameBuffer.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

ParamDiffCurveModule_Comp_1D_Pass::ParamDiffCurveModule_Comp_1D_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : Param Diff Curve", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	//m_DontUseShaderFilesOnDisk = true;
}

ParamDiffCurveModule_Comp_1D_Pass::~ParamDiffCurveModule_Comp_1D_Pass()
{
	Unit();
}

void ParamDiffCurveModule_Comp_1D_Pass::ActionBeforeInit()
{
	//m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);

}

bool ParamDiffCurveModule_Comp_1D_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	ZoneScoped;

	bool change = false;

	//change |= DrawResizeWidget();

	change |= ImGui::InputFloatDefault(0.0f, "start_pos x", &m_UBO_0_Comp.u_start_pos.x, 0.001f);
	change |= ImGui::InputFloatDefault(0.0f, "start_pos y", &m_UBO_0_Comp.u_start_pos.y, 0.001f);
	change |= ImGui::InputFloatDefault(0.0f, "start_pos z", &m_UBO_0_Comp.u_start_pos.z, 0.001f);
	change |= ImGui::SliderUIntDefaultCompact(0.0f, "step_count", &m_UBO_0_Comp.u_step_count, 0U, 10000U, 5000U);
	change |= ImGui::SliderFloatDefaultCompact(0.0f, "step_size", &m_UBO_0_Comp.u_step_size, 0.000f, 0.020f, 0.010f, 0.0f, "%.3f");
	if (change)
	{
		NeedNewUBOUpload();
	}


	return change;
}

void ParamDiffCurveModule_Comp_1D_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	ZoneScoped;
}

void ParamDiffCurveModule_Comp_1D_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	ZoneScoped;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL OUTPUT ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

SceneModelWeak ParamDiffCurveModule_Comp_1D_Pass::GetModel()
{	
	ZoneScoped;

	return m_SceneModelPtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ParamDiffCurveModule_Comp_1D_Pass::WasJustResized()
{
	ZoneScoped;
}

void ParamDiffCurveModule_Comp_1D_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
		{
			VKFPScoped(*vCmdBuffer, "Param Diff Curve", "Compute");

			vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);

			for (uint32_t iter = 0; iter < m_CountIterations.w; iter++)
			{
				Dispatch(vCmdBuffer);
			}
		}
	}
}


bool ParamDiffCurveModule_Comp_1D_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBO_0_Comp_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_0_Comp));
	m_UBO_0_Comp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_0_Comp_Ptr)
	{
		m_UBO_0_Comp_BufferInfos.buffer = m_UBO_0_Comp_Ptr->buffer;
		m_UBO_0_Comp_BufferInfos.range = sizeof(UBO_0_Comp);
		m_UBO_0_Comp_BufferInfos.offset = 0;
	}


	NeedNewUBOUpload();

	return true;
}

void ParamDiffCurveModule_Comp_1D_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_0_Comp_Ptr, &m_UBO_0_Comp, sizeof(UBO_0_Comp));

}

void ParamDiffCurveModule_Comp_1D_Pass::DestroyUBO()
{
	ZoneScoped;
	m_UBO_0_Comp_Ptr.reset();
	m_UBO_0_Comp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

}

/*bool ParamDiffCurveModule_Comp_1D_Pass::BuildModel()
{
	auto meshPtr = m_InputMesh.getValidShared();
	if (meshPtr)
	{
		m_SBO_Normals_Compute_Helper.reset();

		m_NormalDatas.clear();
		m_NormalDatas.resize(meshPtr->GetVerticesCount() * 3U);
		const auto sizeInBytes = sizeof(int) * m_NormalDatas.size();
		memset(m_NormalDatas.data(), 0U, sizeInBytes);
		m_SBO_Normals_Compute_Helper = VulkanRessource::createGPUOnlyStorageBufferObject(m_VulkanCorePtr, m_NormalDatas.data(), sizeInBytes);
		if (m_SBO_Normals_Compute_Helper->buffer)
		{
			m_SBO_Normals_Compute_Helper_BufferInfos = vk::DescriptorBufferInfo{m_SBO_Normals_Compute_Helper->buffer, 0, sizeInBytes};
		}
		else
		{
			m_SBO_Normals_Compute_Helper_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
		}
	}

	return true;
}

void ParamDiffCurveModule_Comp_1D_Pass::DestroyModel(const bool& vReleaseDatas)
{
	m_NormalDatas.clear();
	m_SBO_Normals_Compute_Helper.reset();
	m_SBO_Normals_Compute_Helper_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
}*/

bool ParamDiffCurveModule_Comp_1D_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_DescriptorSets[0].m_LayoutBindings.clear();
	m_DescriptorSets[0].m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute);
	m_DescriptorSets[0].m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);

	return true;
}

bool ParamDiffCurveModule_Comp_1D_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	m_DescriptorSets[0].m_WriteDescriptorSets.clear();

	assert(m_ComputeBufferPtr);
	m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(
		m_DescriptorSets[0].m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_UBO_0_Comp_BufferInfos);
	/*m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eStorageBuffer,
		nullptr, outputMeshPtr->GetVerticesBufferInfo());
	m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eStorageBuffer,
		nullptr, outputMeshPtr->GetIndicesBufferInfo());*/

	return true;
}

std::string ParamDiffCurveModule_Comp_1D_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ParamDiffCurveModule_Comp_1D_Pass_Compute";

	SetLocalGroupSize(ct::uvec3(1U, 1U, 1U));

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform image2D colorBuffer;

layout(std140, binding = 0) uniform UBO_0_Comp
{
	vec3 u_start_pos;
	uint u_step_count;
	float u_step_size;
};

layout(binding = 1) uniform sampler2D input_mask;

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

std::string ParamDiffCurveModule_Comp_1D_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += ShaderPass::getXml(vOffset, vUserDatas);

	str += vOffset + "<start_pos>" + m_UBO_0_Comp.u_start_pos.string() + "</start_pos>\n";
	str += vOffset + "<step_count>" + ct::toStr(m_UBO_0_Comp.u_step_count) + "</step_count>\n";
	str += vOffset + "<step_size>" + ct::toStr(m_UBO_0_Comp.u_step_size) + "</step_size>\n";
	return str;
}

bool ParamDiffCurveModule_Comp_1D_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
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

	if (strParentName == "param_diff_curve_module")
	{
		if (strName == "start_pos")
			m_UBO_0_Comp.u_start_pos = ct::fvariant(strValue).GetV3();
		else if (strName == "step_count")
			m_UBO_0_Comp.u_step_count = ct::uvariant(strValue).GetU();
		else if (strName == "step_size")
			m_UBO_0_Comp.u_step_size = ct::fvariant(strValue).GetF();
	}

	return true;
}

void ParamDiffCurveModule_Comp_1D_Pass::AfterNodeXmlLoading()
{
	// code to do after end of the xml loading of this node
}
