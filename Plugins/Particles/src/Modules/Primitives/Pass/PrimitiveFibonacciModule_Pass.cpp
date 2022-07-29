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

#include "PrimitiveFibonacciModule_Pass.h"

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
#include <cinttypes>
#include <Base/FrameBuffer.h>
#include <Graph/Base/BaseNode.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////
//// SSAO SECOND PASS : BLUR /////////////////////////////////
//////////////////////////////////////////////////////////////

PrimitiveFibonacciModule_Pass::PrimitiveFibonacciModule_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : Particles Simulation", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	//m_DontUseShaderFilesOnDisk = true;
}

PrimitiveFibonacciModule_Pass::~PrimitiveFibonacciModule_Pass()
{
	Unit();
}

void PrimitiveFibonacciModule_Pass::ActionAfterInitSucceed()
{
	m_DispatchSize.x = (m_UBOComp.count * 3U) / 8U;;
	m_DispatchSize.y = 1U;
	m_DispatchSize.z = 1U;
}

bool PrimitiveFibonacciModule_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	bool change_model = false;
	bool change_ubo = false;

	if (ImGui::ContrastedButton("Reset", nullptr, nullptr, ImGui::GetContentRegionAvail().x))
	{
		m_UBOComp = UBOComp();
		change_model = true;
	}
	change_model |= ImGui::SliderUIntDefaultCompact(0.0f, "Particle Count", &m_UBOComp.count, 1U, 50000U, 2000U);
	change_ubo |= ImGui::SliderFloatDefaultCompact(0.0f, "Particle Life", &m_UBOComp.life, 0.1f, 100.0f, 1.0f);
	change_ubo |= ImGui::SliderFloatDefaultCompact(0.0f, "Particle Speed", &m_UBOComp.speed, 0.1f, 100.0f, 1.0f);
	change_ubo |= ImGui::SliderFloatDefaultCompact(0.0f, "Ball Radius", &m_UBOComp.radius, 0.001f, 100.0f, 1.0f);

	if (change_model)
	{
		NeedNewUBOUpload();
		NeedNewModelUpdate();
	}
	
	if (change_ubo)
	{
		NeedNewUBOUpload();
	}

	return change_model || change_ubo;
}

void PrimitiveFibonacciModule_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);
}

void PrimitiveFibonacciModule_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);
}

vk::Buffer* PrimitiveFibonacciModule_Pass::GetTexelBuffer(const uint32_t& vBindingPoint, ct::uvec2* vOutSize)
{
	ZoneScoped;

	if (m_Particle_pos3_life1_dir3_speed4_color4_buffer_Ptr)
	{
		if (vOutSize)
		{
			vOutSize->x = m_UBOComp.count;
			vOutSize->y = 1U;
		}

		return &m_Particle_pos3_life1_dir3_speed4_color4_buffer_Ptr->buffer;
	}

	return nullptr;
}

vk::BufferView* PrimitiveFibonacciModule_Pass::GetTexelBufferView(const uint32_t& vBindingPoint, ct::uvec2* vOutSize)
{
	ZoneScoped;

	if (m_Particle_pos3_life1_dir3_speed4_color4_buffer_Ptr)
	{
		if (vOutSize)
		{
			vOutSize->x = m_UBOComp.count;
			vOutSize->y = 1U;
		}

		return &m_Particle_pos3_life1_dir3_speed4_color4_buffer_Ptr->bufferView;
	}

	return nullptr;
}

void PrimitiveFibonacciModule_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipeline);

		vCmdBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eVertexInput,
			vk::PipelineStageFlagBits::eComputeShader,
			vk::DependencyFlags(),
			nullptr,
			nullptr,
			nullptr);

		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_PipelineLayout, 0, m_DescriptorSet, nullptr);
		
		vCmdBuffer->dispatch(m_DispatchSize.x, m_DispatchSize.y, m_DispatchSize.z);

		vCmdBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eComputeShader,
			vk::PipelineStageFlagBits::eVertexInput,
			vk::DependencyFlags(),
			nullptr,
			nullptr,
			nullptr);
	}
}

bool PrimitiveFibonacciModule_Pass::BuildModel()
{
	ZoneScoped;

	m_Particle_pos3_life1_dir3_speed4_color4_buffer_Ptr.reset();

	std::vector<ct::fvec4> particles;
	particles.resize(m_UBOComp.count * 3U);
	auto sizeInBytes = particles.size() * sizeof(ct::fvec4);

	m_Particle_pos3_life1_dir3_speed4_color4_buffer_Ptr = VulkanRessource::createTexelBuffer(
		m_VulkanCorePtr, particles.data(), sizeInBytes, vk::Format::eR32G32B32A32Sfloat);

	m_DispatchSize.x = (m_UBOComp.count * 3U) / 8U;

	// vec4 => xyz:pos, w:life
	// vec4 => xyz:dir, w:speed
	// inform observer than a new model is ready
	auto parentNodePtr = GetParentNode().getValidShared();
	if (parentNodePtr)
	{
		parentNodePtr->Notify(NotifyEvent::TexelBufferUpdateDone);
	}

	return true;
}

void PrimitiveFibonacciModule_Pass::DestroyModel(const bool& vReleaseDatas)
{
	m_Particle_pos3_life1_dir3_speed4_color4_buffer_Ptr.reset();
}

bool PrimitiveFibonacciModule_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBO_Comp = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBOComp));
	if (m_UBO_Comp)
	{
		m_DescriptorBufferInfo_Comp.buffer = m_UBO_Comp->buffer;
		m_DescriptorBufferInfo_Comp.range = sizeof(UBOComp);
		m_DescriptorBufferInfo_Comp.offset = 0;
	}

	NeedNewUBOUpload();

	return true;
}

void PrimitiveFibonacciModule_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, *m_UBO_Comp, &m_UBOComp, sizeof(UBOComp));
}

void PrimitiveFibonacciModule_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Comp.reset();
}

bool PrimitiveFibonacciModule_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute);
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute);

	return true;
}

bool PrimitiveFibonacciModule_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();

	writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eStorageTexelBuffer,
		nullptr, nullptr, &m_Particle_pos3_life1_dir3_speed4_color4_buffer_Ptr->bufferView);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer,
		nullptr, &m_DescriptorBufferInfo_Comp);
	
	return true;
}

std::string PrimitiveFibonacciModule_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PrimitiveFibonacciModule_Pass";

	return u8R"(
#version 450

layout(local_size_x = 8, local_size_y = 1, local_size_z = 1) in;

layout(binding = 0, rgba32f) uniform imageBuffer pos3_life1_dir3_speed4_color4_buffer;

layout (std140, binding = 1) uniform UBO_Comp 
{
	vec3 scale; 
	vec3 dir;
	uint count;
	float radius;
	float life;
	float speed;
};

void main() 
{
	const int i_global_index = int(gl_GlobalInvocationID.x);
	const float f_global_index = float(gl_GlobalInvocationID.x);
	
	vec4 pos_life = imageLoad(pos3_life1_dir3_speed4_color4_buffer, i_global_index * 3 + 0);
	vec4 dir_speed = imageLoad(pos3_life1_dir3_speed4_color4_buffer, i_global_index * 3 + 1);
	vec4 color = imageLoad(pos3_life1_dir3_speed4_color4_buffer, i_global_index * 3 + 2);
	
	const float golden_ratio = radians(360.0) / ((1.0 + sqrt(5.0)) * 0.5);
	const float theta = f_global_index * golden_ratio;
	const float phi = acos(1.0 - 2.0 * (f_global_index + 0.5) / float(count));
	
	// xyz:pos, w:life
	pos_life.x = cos(theta) * sin(phi) * radius * scale.x;
	pos_life.y = sin(theta) * sin(phi) * radius * scale.y;
	pos_life.z = cos(phi) * radius * scale.z;
	pos_life.w = life;
	
	// xyz:dir, w:speed
	dir_speed.x = dir.x;
	dir_speed.y = dir.y;
	dir_speed.z = dir.z;
	dir_speed.w = speed;
	
	// rgba:color
	vec3 nor = normalize(pos_life.xyz) * 0.5 + 0.5;
	color.r = nor.x;
	color.g = nor.y;
	color.b = nor.z;
	color.a = 1.0;
	
	imageStore(pos3_life1_dir3_speed4_color4_buffer, i_global_index * 3 + 0, pos_life);
	imageStore(pos3_life1_dir3_speed4_color4_buffer, i_global_index * 3 + 1, dir_speed);
	imageStore(pos3_life1_dir3_speed4_color4_buffer, i_global_index * 3 + 2, color);
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string PrimitiveFibonacciModule_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<count>" + ct::toStr(m_UBOComp.count) + "</count>\n";
	str += vOffset + "<radius>" + ct::toStr(m_UBOComp.radius) + "</radius>\n";
	str += vOffset + "<life>" + ct::toStr(m_UBOComp.life) + "</life>\n";
	str += vOffset + "<speed>" + ct::toStr(m_UBOComp.speed) + "</speed>\n";

	return str;
}

bool PrimitiveFibonacciModule_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
{
	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strParentName == "primitive_fibonacci_module")
	{
		if (strName == "count")
		{
			m_UBOComp.count = ct::uvariant(strValue).GetU();

			NeedNewModelUpdate();
		}
		else if (strName == "radius")
			m_UBOComp.radius = ct::fvariant(strValue).GetF();
		else if (strName == "life")
			m_UBOComp.life = ct::fvariant(strValue).GetF();
		else if (strName == "speed")
			m_UBOComp.speed = ct::fvariant(strValue).GetF();

	}

	return true;
}