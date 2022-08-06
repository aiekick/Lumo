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
#include <Headers/ParticlesCommon.h>

#include <functional>
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

	m_ParticlesPtr = SceneParticles::Create(m_VulkanCorePtr);

	//m_DontUseShaderFilesOnDisk = true;
}

MeshEmitterModule_Comp_Pass::~MeshEmitterModule_Comp_Pass()
{
	m_ParticlesPtr.reset();

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
	assert(vContext); ImGui::SetCurrentContext(vContext);

	bool change_ubo = false;
	bool model_ubo = false;

	ImGui::Text("Delta time : %.5f", m_PushConstants.delta_time);
	ImGui::Text("Absolute time : %.5f", m_PushConstants.absolute_time);
	ImGui::Text("Max particles count : %u", m_UBOComp.max_particles_count);
	ImGui::Text("Current particles count : %u", m_UBOComp.current_particles_count);

	ImGui::Separator();

	if (ImGui::ContrastedButton("Reset particles", nullptr, nullptr, ImGui::GetContentRegionAvail().x))
	{
		m_UBOComp.reset = 1.0f;
		m_PushConstants.absolute_time = 0.0f;
		change_ubo = true;
	}

	model_ubo |= ImGui::SliderUIntDefaultCompact(0.0f, "Max particles count", &m_UBOComp.max_particles_count, 1000U, 1000000U, 100000U);
	change_ubo |= ImGui::SliderFloatDefaultCompact(0.0f, "Spawn mass", &m_UBOComp.spawn_mass, 0.1f, 1.0f, 0.5f);
	change_ubo |= ImGui::SliderFloatDefaultCompact(0.0f, "Spawn rate", &m_UBOComp.spawn_rate, 0.1f, 1.0f, 0.5f);
	change_ubo |= ImGui::SliderFloatDefaultCompact(0.0f, "Spawn min life", &m_UBOComp.base_min_life, 0.1f, 10.0f, 1.0f);
	change_ubo |= ImGui::SliderFloatDefaultCompact(0.0f, "Spawn max life", &m_UBOComp.base_max_life, 0.1f, 10.0f, 1.0f);
	change_ubo |= ImGui::SliderFloatDefaultCompact(0.0f, "Spawn speed", &m_UBOComp.base_speed, 0.1f, 100.0f, 0.1f);
	
	if (change_ubo)
	{
		NeedNewUBOUpload();
	}

	if (model_ubo)
	{
		NeedNewModelUpdate();
	}

	return change_ubo || model_ubo;
}

void MeshEmitterModule_Comp_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);
}

void MeshEmitterModule_Comp_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);
}

void MeshEmitterModule_Comp_Pass::SetModel(SceneModelWeak vSceneModel)
{
	ZoneScoped;

	m_SceneModel = vSceneModel;

	auto modelPtr = m_SceneModel.getValidShared();
	if (modelPtr && !modelPtr->empty())
	{
		// only take the first mesh
		m_InputMesh = modelPtr->Get(0);
	}
	else
	{
		m_InputMesh.reset();
	}

	NeedNewModelUpdate();
}

SceneParticlesWeak MeshEmitterModule_Comp_Pass::GetParticles()
{
	ZoneScoped;

	return m_ParticlesPtr;
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

	if (m_ParticlesPtr)
	{
		m_ParticlesPtr->DestroyBuffers();

		auto meshPtr = m_InputMesh.getValidShared();
		if (meshPtr)
		{
			m_UBOComp.current_vertexs_count = meshPtr->GetVerticesCount();
			NeedNewUBOUpload();

			if (m_ParticlesPtr->Build(m_UBOComp.max_particles_count))
			{
				SetDispatchSize1D(m_UBOComp.current_vertexs_count);

				auto parentNodePtr = GetParentNode().getValidShared();
				if (parentNodePtr)
				{
					parentNodePtr->SendFrontNotification(ParticlesUpdateDone);
				}
			}
		}
	}
	
	return true;
}

void MeshEmitterModule_Comp_Pass::DestroyModel(const bool& vReleaseDatas)
{
	
}

bool MeshEmitterModule_Comp_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBOCompPtr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBOComp));
	if (m_UBOCompPtr && 
		m_UBOCompPtr->buffer)
	{
		m_DescriptorBufferInfo_Comp.buffer = m_UBOCompPtr->buffer;
		m_DescriptorBufferInfo_Comp.offset = 0;
		m_DescriptorBufferInfo_Comp.range = sizeof(UBOComp);
	}

	NeedNewUBOUpload();

	return true;
}

void MeshEmitterModule_Comp_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBOCompPtr, &m_UBOComp, sizeof(UBOComp));
}

void MeshEmitterModule_Comp_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBOCompPtr.reset();
}

bool MeshEmitterModule_Comp_Pass::CanUpdateDescriptors()
{
	return (m_ParticlesPtr != nullptr);
}

bool MeshEmitterModule_Comp_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);	// Mesh vertexs
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute);	// Ubo
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);	// Particles datas buffer
	m_LayoutBindings.emplace_back(3U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);	// Particles alive pre sim buffer
	m_LayoutBindings.emplace_back(4U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);	// Particles dead buffer
	m_LayoutBindings.emplace_back(5U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);	// Particles counter buffer

	return true;
}

bool MeshEmitterModule_Comp_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();

	// Mesh vertexs
	auto inputMeshPtr = m_InputMesh.getValidShared();
	if (inputMeshPtr && inputMeshPtr->GetVerticesBufferInfo()->range > 0U )
	{
		writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, 
			vk::DescriptorType::eStorageBuffer, 
			nullptr, inputMeshPtr->GetVerticesBufferInfo());
	}
	else
	{
		writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, 
			vk::DescriptorType::eStorageBuffer, 
			nullptr, m_VulkanCorePtr->getEmptyDescriptorBufferInfo());
	}

	// Ubo
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, 
		vk::DescriptorType::eUniformBuffer,
		nullptr, &m_DescriptorBufferInfo_Comp);

	// Particles datas buffer
	// not existence == no udate nor rendering => secured by CanUpdateDescriptors()
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1,
		vk::DescriptorType::eStorageBuffer, nullptr, m_ParticlesPtr->GetParticlesDatasBufferInfo());
	writeDescriptorSets.emplace_back(m_DescriptorSet, 3U, 0, 1,
		vk::DescriptorType::eStorageBuffer, nullptr, m_ParticlesPtr->GetAliveParticlesPreSimBufferInfo());
	writeDescriptorSets.emplace_back(m_DescriptorSet, 4U, 0, 1,
		vk::DescriptorType::eStorageBuffer, nullptr, m_ParticlesPtr->GetDeadParticlesBufferInfo());
	writeDescriptorSets.emplace_back(m_DescriptorSet, 5U, 0, 1,
		vk::DescriptorType::eStorageBuffer, nullptr, m_ParticlesPtr->GetCountersBufferInfo());

	return true;
}

std::string MeshEmitterModule_Comp_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "MeshEmitterModule_Comp_Pass";

	SetLocalGroupSize(ct::uvec3(8U, 1U, 1U));

	return u8R"(
#version 450

layout(local_size_x = 8, local_size_y = 1, local_size_z = 1) in;

struct V3N3T3B3T2C4 
{
	float px, py, pz;
	float nx, ny, nz;
	float tax, tay, taz;
	float btax, btay, btaz;
	float tx, ty;
	float cx, cy, cz, cw;
};

layout(std430, binding = 0) readonly buffer VertexInput
{
	V3N3T3B3T2C4 vertices[];
};

layout (std140, binding = 1) uniform UBO_Comp 
{
	uint max_particles_count;
	uint current_particles_count;
	uint current_vertexs_count;
	float reset;
	float base_min_life;
	float base_max_life;
	float base_speed;
	float spawn_rate;
	float spawn_mass;
};

layout(push_constant) uniform push_constants 
{
	float absolute_time;
	float delta_time;
};
)"
+
SceneParticles::GetParticlesDatasBufferHeader(2U)
+
SceneParticles::GetAliveParticlesPreSimBufferHeader(3U)
+
SceneParticles::GetDeadParticlesBufferHeader(4U)
+
SceneParticles::GetCounterBufferHeader(5U)
+
u8R"(
void main() 
{
	const int i_global_index = int(gl_GlobalInvocationID.x);			
	
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MeshEmitterModule_Comp_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<base_min_life>" + ct::toStr(m_UBOComp.base_min_life) + "</base_min_life>\n";
	str += vOffset + "<base_max_life>" + ct::toStr(m_UBOComp.base_max_life) + "</base_max_life>\n";
	str += vOffset + "<base_speed>" + ct::toStr(m_UBOComp.base_speed) + "</base_speed>\n";
	str += vOffset + "<spawn_rate>" + ct::toStr(m_UBOComp.spawn_rate) + "</spawn_rate>\n";
	str += vOffset + "<max_particles_count>" + ct::toStr(m_UBOComp.max_particles_count) + "</max_particles_count>\n";

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
		if (strName == "base_min_life")
			m_UBOComp.base_min_life = ct::fvariant(strValue).GetF();
		else if (strName == "base_max_life")
			m_UBOComp.base_max_life = ct::fvariant(strValue).GetF();
		else if (strName == "base_speed")
			m_UBOComp.base_speed = ct::fvariant(strValue).GetF();
		else if (strName == "spawn_rate")
			m_UBOComp.spawn_rate = ct::fvariant(strValue).GetF();
		else if (strName == "max_particles_count")
			m_UBOComp.max_particles_count = ct::uvariant(strValue).GetU();
	}

	return true;
}