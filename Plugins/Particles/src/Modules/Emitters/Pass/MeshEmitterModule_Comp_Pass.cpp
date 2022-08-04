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

#include "MeshEmitterModule_Comp_Pass.h"

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

MeshEmitterModule_Comp_Pass::MeshEmitterModule_Comp_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : Particles Simulation", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	//m_DontUseShaderFilesOnDisk = true;
}

MeshEmitterModule_Comp_Pass::~MeshEmitterModule_Comp_Pass()
{
	Unit();
}

void MeshEmitterModule_Comp_Pass::ActionBeforeInit()
{
	vk::PushConstantRange push_constant;
	push_constant.offset = 0;
	push_constant.size = sizeof(PushConstants);
	push_constant.stageFlags = vk::ShaderStageFlagBits::eCompute;

	SetPushConstantRange(push_constant);
}

void MeshEmitterModule_Comp_Pass::ActionAfterInitSucceed()
{
	m_PushConstants.delta_time = ct::GetTimeInterval();
}

bool MeshEmitterModule_Comp_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	bool change_ubo = false;
	bool model_ubo = false;

	ImGui::Text("Delta Time : %.5f", m_PushConstants.delta_time);
	ImGui::Text("Absolute Time : %.5f", m_PushConstants.absolute_time);
	ImGui::Text("Max Particles Count : %u", m_UBOComp.count_particles);
	//ImGui::Text("Current Particles Count : %u", m_UBOComp.count_particles);

	ImGui::Separator();

	if (ImGui::ContrastedButton("Reset particles", nullptr, nullptr, ImGui::GetContentRegionAvail().x))
	{
		m_UBOComp.reset = 1.0f;
		m_PushConstants.absolute_time = 0.0f;
		change_ubo = true;
	}

	change_ubo |= ImGui::SliderFloatDefaultCompact(0.0f, "Spawn Rate", &m_UBOComp.spawn_rate, 0.1f, 1.0f, 0.5f);
	change_ubo |= ImGui::SliderFloatDefaultCompact(0.0f, "Spawn Life", &m_UBOComp.base_life, 0.1f, 10.0f, 1.0f);
	change_ubo |= ImGui::SliderFloatDefaultCompact(0.0f, "Spawn Speed", &m_UBOComp.base_speed, 0.1f, 100.0f, 0.1f);
	model_ubo |= ImGui::SliderUIntDefaultCompact(0.0f, "Count Particles per vertex", &m_UBOComp.count_per_vertex, 1U, 10U, 1U);
	
	if (change_ubo)
	{
		NeedNewUBOUpload();
	}

	if (model_ubo)
	{
		m_UBOComp.count_per_vertex = ct::maxi(m_UBOComp.count_per_vertex, 1U);
		NeedNewModelUpdate();
	}

	return change_ubo || model_ubo;
}

void MeshEmitterModule_Comp_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);
}

void MeshEmitterModule_Comp_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);
}

void MeshEmitterModule_Comp_Pass::SetModel(SceneModelWeak vSceneModel)
{
	ZoneScoped;

	m_SceneModel = vSceneModel;

	auto modelPtr = m_SceneModel.getValidShared();
	if (modelPtr && !modelPtr->empty())
	{
		// only take the first messh
		m_InputMesh = modelPtr->Get(0);
	}
	else
	{
		m_InputMesh.reset();
	}

	NeedNewModelUpdate();
}

vk::Buffer* MeshEmitterModule_Comp_Pass::GetTexelBuffer(const uint32_t& vBindingPoint, ct::uvec2* vOutSize)
{
	ZoneScoped;

	if (m_Particle_pos3_life1_dir3_speed4_color4_buffer_Ptr)
	{
		if (vOutSize)
		{
			vOutSize->x = m_UBOComp.count_particles;
			vOutSize->y = 1U;
		}

		return &m_Particle_pos3_life1_dir3_speed4_color4_buffer_Ptr->buffer;
	}

	return nullptr;
}

vk::BufferView* MeshEmitterModule_Comp_Pass::GetTexelBufferView(const uint32_t& vBindingPoint, ct::uvec2* vOutSize)
{
	ZoneScoped;

	if (m_Particle_pos3_life1_dir3_speed4_color4_buffer_Ptr)
	{
		if (vOutSize)
		{
			vOutSize->x = m_UBOComp.count_particles;
			vOutSize->y = 1U;
		}

		return &m_Particle_pos3_life1_dir3_speed4_color4_buffer_Ptr->bufferView;
	}

	return nullptr;
}

void MeshEmitterModule_Comp_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
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

		m_PushConstants.delta_time = m_VulkanCorePtr->GetDeltaTime();
		m_PushConstants.absolute_time += m_PushConstants.delta_time;
		vCmdBuffer->pushConstants(m_PipelineLayout,
			vk::ShaderStageFlagBits::eCompute,
			0, sizeof(PushConstants), &m_PushConstants);

		Dispatch(vCmdBuffer);

		vCmdBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eComputeShader,
			vk::PipelineStageFlagBits::eVertexInput,
			vk::DependencyFlags(),
			nullptr,
			nullptr,
			nullptr);

		if (m_UBOComp.reset > 0.5f)
		{
			m_UBOComp.reset = 0.0f;
			NeedNewUBOUpload();
		}
	}
}

bool MeshEmitterModule_Comp_Pass::BuildModel()
{
	ZoneScoped;

	m_Particle_pos3_life1_dir3_speed4_color4_buffer_Ptr.reset();

	auto meshPtr = m_InputMesh.getValidShared();
	if (meshPtr)
	{
		m_UBOComp.count_vertexs = meshPtr->GetVerticesCount();
		m_UBOComp.count_particles = m_UBOComp.count_vertexs * m_UBOComp.count_per_vertex;
		NeedNewUBOUpload();

		auto sizeOfStruct = sizeof(ct::fvec4) * 3U;
		auto sizeInBytes = m_UBOComp.count_particles * sizeOfStruct;
		m_Particle_pos3_life1_dir3_speed4_color4_buffer_Ptr = VulkanRessource::createTexelBuffer(
			m_VulkanCorePtr, vk::Format::eR32G32B32A32Sfloat, sizeInBytes);

		SetDispatchSize1D(m_UBOComp.count_particles);

		// inform observer than a new model is ready
		auto parentNodePtr = GetParentNode().getValidShared();
		if (parentNodePtr)
		{
			parentNodePtr->SendFrontNotification(NotifyEvent::TexelBufferUpdateDone);
		}
	}
	
	return true;
}

void MeshEmitterModule_Comp_Pass::DestroyModel(const bool& vReleaseDatas)
{
	m_Particle_pos3_life1_dir3_speed4_color4_buffer_Ptr.reset();
}

bool MeshEmitterModule_Comp_Pass::CreateUBO()
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

void MeshEmitterModule_Comp_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, *m_UBO_Comp, &m_UBOComp, sizeof(UBOComp));
}

void MeshEmitterModule_Comp_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Comp.reset();
}

bool MeshEmitterModule_Comp_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute);
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute);

	return true;
}

bool MeshEmitterModule_Comp_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();

	auto inputMeshPtr = m_InputMesh.getValidShared();
	if (inputMeshPtr && inputMeshPtr->GetVerticesBufferInfo()->range > 0U)
	{
		writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eStorageTexelBuffer,
			nullptr, nullptr, &m_Particle_pos3_life1_dir3_speed4_color4_buffer_Ptr->bufferView);
		writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eStorageBuffer, 
			nullptr, inputMeshPtr->GetVerticesBufferInfo());
	}
	else
	{
		writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eStorageTexelBuffer, 
			nullptr, nullptr, m_VulkanCorePtr->getEmptyBufferView());
		writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eStorageBuffer, 
			nullptr, m_VulkanCorePtr->getEmptyDescriptorBufferInfo());
	}

	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eUniformBuffer,
		nullptr, &m_DescriptorBufferInfo_Comp);
	
	return true;
}

std::string MeshEmitterModule_Comp_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "MeshEmitterModule_Comp_Pass";

	SetLocalGroupSize(ct::uvec3(8U, 1U, 1U));

	return u8R"(
#version 450

layout(local_size_x = 8, local_size_y = 1, local_size_z = 1) in;

layout(binding = 0, rgba32f) uniform imageBuffer pos3_life1_dir3_speed4_color4_buffer;

struct V3N3T3B3T2C4 
{
	float px, py, pz;
	float nx, ny, nz;
	float tax, tay, taz;
	float btax, btay, btaz;
	float tx, ty;
	float cx, cy, cz, cw;
};

layout(std430, binding = 1) readonly buffer VertexInput
{
	V3N3T3B3T2C4 inputVertices[];
};

layout (std140, binding = 2) uniform UBO_Comp 
{
	uint count_particles;
	uint count_vertexs;
	uint count_per_vertex;
	float reset;
	float base_life;
	float base_speed;
	float spawn_rate;
};

layout(push_constant) uniform push_constants 
{
	float absolute_time;
	float delta_time;
};

void ResetParticle(int global_index)
{
	imageStore(pos3_life1_dir3_speed4_color4_buffer, global_index * 3 + 0, vec4(0));
	imageStore(pos3_life1_dir3_speed4_color4_buffer, global_index * 3 + 1, vec4(0));
	imageStore(pos3_life1_dir3_speed4_color4_buffer, global_index * 3 + 2, vec4(0));
}

void EmitParticle(int global_index, int vertex_index)
{
	// current vertex datas
	V3N3T3B3T2C4 current_vertex = inputVertices[vertex_index];

	// xyz:pos, w:base life
	vec4 pos_life;
	pos_life.x = current_vertex.px;
	pos_life.y = current_vertex.py;
	pos_life.z = current_vertex.pz;
	pos_life.w = base_life;
		
	// xyz:dir, w:speed
	vec4 dir_speed;
	vec3 nor = normalize(vec3(current_vertex.nx, current_vertex.ny, current_vertex.nz));
	dir_speed.x = nor.x;
	dir_speed.y = nor.y;
	dir_speed.z = nor.z;
	dir_speed.w = base_speed;
	
	// rgba:color
	vec4 color;
	color.r = current_vertex.cx;
	color.g = current_vertex.cy;
	color.b = current_vertex.cz;
	color.a = current_vertex.cw;

	// for demo
	color.rgb = dir_speed.xyz * 0.5 + 0.5;
	color.a = 1.0;
		
	imageStore(pos3_life1_dir3_speed4_color4_buffer, global_index * 3 + 0, pos_life);
	imageStore(pos3_life1_dir3_speed4_color4_buffer, global_index * 3 + 1, dir_speed);
	imageStore(pos3_life1_dir3_speed4_color4_buffer, global_index * 3 + 2, color);
}

void main() 
{
	const int i_global_index = int(gl_GlobalInvocationID.x);
	const int i_count_vertexs = int(count_vertexs);
	const int i_vertex_index = i_global_index % i_count_vertexs;
			
	vec4 pos_life = imageLoad(pos3_life1_dir3_speed4_color4_buffer, i_global_index * 3 + 0);	

	if (reset > 0.5)
	{
		if (i_global_index == i_vertex_index)
		{
			EmitParticle(i_global_index, i_vertex_index);
		}
		else
		{
			ResetParticle(i_global_index);
		}
	}
	else if (pos_life.w > 0.0)
	{
		vec4 dir_speed = imageLoad(pos3_life1_dir3_speed4_color4_buffer, i_global_index * 3 + 1);

		pos_life.xyz += dir_speed.xyz * dir_speed.w * delta_time;
		pos_life.w -= delta_time;

		imageStore(pos3_life1_dir3_speed4_color4_buffer, i_global_index * 3 + 0, pos_life);
	}
	else // particle ready to be emitted
	{
		ResetParticle(i_global_index);
		
		if (mod(absolute_time, spawn_rate * 2.0) > spawn_rate)
		{
			EmitParticle(i_global_index, i_vertex_index);
		}
	}
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MeshEmitterModule_Comp_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<base_life>" + ct::toStr(m_UBOComp.base_life) + "</base_life>\n";
	str += vOffset + "<base_speed>" + ct::toStr(m_UBOComp.base_speed) + "</base_speed>\n";
	str += vOffset + "<spawn_rate>" + ct::toStr(m_UBOComp.spawn_rate) + "</spawn_rate>\n";

	return str;
}

bool MeshEmitterModule_Comp_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strParentName == "mesh_emitter_module")
	{
		if (strName == "base_life")
			m_UBOComp.base_life = ct::fvariant(strValue).GetF();
		else if (strName == "base_speed")
			m_UBOComp.base_speed = ct::fvariant(strValue).GetF();
		else if (strName == "spawn_rate")
			m_UBOComp.spawn_rate = ct::fvariant(strValue).GetF();
	}

	return true;
}