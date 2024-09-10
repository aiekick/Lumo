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
#include <ImGuiPack.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <cinttypes>
#include <Gaia/Buffer/FrameBuffer.h>
#include <LumoBackend/Graph/Base/BaseNode.h>

using namespace GaiApi;

//////////////////////////////////////////////////////////////
//// SSAO SECOND PASS : BLUR /////////////////////////////////
//////////////////////////////////////////////////////////////

MeshEmitterModule_Comp_Pass::MeshEmitterModule_Comp_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : ShaderPass(vVulkanCore) {
    SetRenderDocDebugName("Comp Pass : Particles Simulation", COMPUTE_SHADER_PASS_DEBUG_COLOR);

    m_ParticlesPtr = SceneParticles::Create(m_VulkanCore);

    // m_DontUseShaderFilesOnDisk = true;
}

MeshEmitterModule_Comp_Pass::~MeshEmitterModule_Comp_Pass() {
    m_ParticlesPtr.reset();

    Unit();
}

void MeshEmitterModule_Comp_Pass::ActionBeforeInit() {
    vk::PushConstantRange push_constant;
    push_constant.offset = 0;
    push_constant.size = sizeof(PushConstants);
    push_constant.stageFlags = vk::ShaderStageFlagBits::eCompute;

    SetPushConstantRange(push_constant);
}

void MeshEmitterModule_Comp_Pass::ActionAfterInitSucceed() {
    m_PushConstants.delta_time = ct::GetTimeInterval();
}

bool MeshEmitterModule_Comp_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    bool change_ubo = false;
    bool model_ubo = false;

    ImGui::Separator();
    ImGui::Text("Delta time          : %.5f", m_PushConstants.delta_time);
    ImGui::Text("Absolute time       : %.5f", m_PushConstants.absolute_time);
    ImGui::Text("Max particles count : %u", m_UBOComp.max_particles_count);

    if (m_IndexedIndirectCommandPtr) {
        ImGui::Separator();
        ImGui::Text("DrawIndexedIndirectCommand Content :");
        ImGui::Text("indexCount    : %u", m_IndexedIndirectCommandPtr->indexCount);
        ImGui::Text("instanceCount : %u", m_IndexedIndirectCommandPtr->instanceCount);
        ImGui::Text("firstIndex    : %u", m_IndexedIndirectCommandPtr->firstIndex);
        ImGui::Text("vertexOffset  : %u", m_IndexedIndirectCommandPtr->vertexOffset);
        ImGui::Text("firstInstance : %u", m_IndexedIndirectCommandPtr->firstInstance);
    }

    if (m_CountersPtr) {
        ImGui::Separator();
        ImGui::Text("Counters Content :");
        ImGui::Text("alive_particles_count  : %u", m_CountersPtr->alive_particles_count);
        ImGui::Text("pending_emission_count : %u", m_CountersPtr->pending_emission_count);
    }

    ImGui::Separator();

    if (ImGui::ContrastedButton("Reset particles", nullptr, nullptr, ImGui::GetContentRegionAvail().x)) {
        m_UBOComp.reset = 1.0f;
        m_PushConstants.absolute_time = 0.0f;
        change_ubo = true;
    }

    model_ubo |= ImGui::SliderUIntDefaultCompact(0.0f, "Max particles count", &m_UBOComp.max_particles_count, 1000U, 1000000U, 100000U);
    change_ubo |= ImGui::SliderUIntDefaultCompact(0.0f, "emission count", &m_UBOComp.emission_count, 0U, m_UBOComp.max_particles_count, 100U);
    change_ubo |= ImGui::SliderFloatDefaultCompact(0.0f, "Spawn mass", &m_UBOComp.spawn_mass, 0.1f, 1.0f, 0.5f);
    change_ubo |= ImGui::SliderFloatDefaultCompact(0.0f, "Spawn rate", &m_UBOComp.spawn_rate, 0.1f, 1.0f, 0.5f);
    change_ubo |= ImGui::SliderFloatDefaultCompact(0.0f, "Spawn min life", &m_UBOComp.base_min_life, 0.1f, 10.0f, 1.0f);
    change_ubo |= ImGui::SliderFloatDefaultCompact(0.0f, "Spawn max life", &m_UBOComp.base_max_life, 0.1f, 10.0f, 1.0f);
    change_ubo |= ImGui::SliderFloatDefaultCompact(0.0f, "Spawn speed", &m_UBOComp.base_speed, 0.1f, 100.0f, 0.1f);

    if (change_ubo) {
        NeedNewUBOUpload();
    }

    if (model_ubo) {
        NeedNewUBOUpload();
        NeedNewModelUpdate();
    }

    return change_ubo || model_ubo;
}

bool MeshEmitterModule_Comp_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool MeshEmitterModule_Comp_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void MeshEmitterModule_Comp_Pass::SetModel(SceneModelWeak vSceneModel) {
    ZoneScoped;

    m_SceneModel = vSceneModel;

    auto modelPtr = m_SceneModel.lock();
    if (modelPtr && !modelPtr->empty()) {
        // only take the first mesh
        m_InputMesh = modelPtr->Get(0);
    } else {
        m_InputMesh.reset();
    }

    NeedNewModelUpdate();
}

SceneParticlesWeak MeshEmitterModule_Comp_Pass::GetParticles() {
    ZoneScoped;

    return m_ParticlesPtr;
}

void MeshEmitterModule_Comp_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber) {
    if (vCmdBuffer) {
        auto corePtr = m_VulkanCore.lock();
        assert(corePtr != nullptr);

        vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);

        vCmdBuffer->pipelineBarrier(
            vk::PipelineStageFlagBits::eVertexInput, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(), nullptr, nullptr, nullptr);

        vCmdBuffer->bindDescriptorSets(
            vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);

        m_PushConstants.delta_time = corePtr->GetDeltaTime();
        m_PushConstants.absolute_time += m_PushConstants.delta_time;
        SetDispatchSize1D(1U);
        ComputePass(vCmdBuffer, 0U);  // reset

        vCmdBuffer->pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(), nullptr, nullptr, nullptr);

        SetDispatchSize1D(m_UBOComp.max_particles_count);
        ComputePass(vCmdBuffer, 1U);  // emit

        vCmdBuffer->pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eVertexInput, vk::DependencyFlags(), nullptr, nullptr, nullptr);

        if (m_UBOComp.reset > 0.5f) {
            m_UBOComp.reset = 0.0f;
            NeedNewUBOUpload();
        }

        if (m_ParticlesPtr) {
            m_CountersPtr = m_ParticlesPtr->GetCountersFromGPU();
            m_IndexedIndirectCommandPtr = m_ParticlesPtr->GetDrawIndirectCommandFromGPU();
        }
    }
}

void MeshEmitterModule_Comp_Pass::ComputePass(vk::CommandBuffer* vCmd, const uint32_t& vPassNumber) {
    ZoneScoped;

    vCmd->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);

    m_PushConstants.pass_number = vPassNumber;
    vCmd->pushConstants(m_Pipelines[0].m_PipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(PushConstants), &m_PushConstants);

    Dispatch(vCmd, ct::toStr("Iter %u : Compute", vPassNumber).c_str());
}

bool MeshEmitterModule_Comp_Pass::BuildModel() {
    ZoneScoped;

    if (m_ParticlesPtr) {
        m_ParticlesPtr->DestroyBuffers();

        auto meshPtr = m_InputMesh.lock();
        if (meshPtr) {
            m_UBOComp.current_vertexs_count = meshPtr->GetVerticesCount();
            NeedNewUBOUpload();

            if (m_ParticlesPtr->Build(m_UBOComp.max_particles_count)) {
                SetDispatchSize1D(m_UBOComp.current_vertexs_count);

                auto parentNodePtr = GetParentNode().lock();
                if (parentNodePtr) {
                    parentNodePtr->SendFrontNotification(ParticlesUpdateDone);
                }
            }
        }
    }

    return true;
}

void MeshEmitterModule_Comp_Pass::DestroyModel(const bool& vReleaseDatas) {
}

bool MeshEmitterModule_Comp_Pass::CreateUBO() {
    ZoneScoped;

    m_UBOCompPtr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBOComp), "MeshEmitterModule_Comp_Pass");
    if (m_UBOCompPtr && m_UBOCompPtr->buffer) {
        m_DescriptorBufferInfo_Comp.buffer = m_UBOCompPtr->buffer;
        m_DescriptorBufferInfo_Comp.offset = 0;
        m_DescriptorBufferInfo_Comp.range = sizeof(UBOComp);
    }

    NeedNewUBOUpload();

    return true;
}

void MeshEmitterModule_Comp_Pass::UploadUBO() {
    ZoneScoped;

    VulkanRessource::upload(m_VulkanCore, m_UBOCompPtr, &m_UBOComp, sizeof(UBOComp));
}

void MeshEmitterModule_Comp_Pass::DestroyUBO() {
    ZoneScoped;

    m_UBOCompPtr.reset();
}

bool MeshEmitterModule_Comp_Pass::CanUpdateDescriptors() {
    return (m_ParticlesPtr != nullptr);
}

bool MeshEmitterModule_Comp_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    m_DescriptorSets[0].m_LayoutBindings.clear();
    m_DescriptorSets[0].m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);  // Mesh vertexs
    m_DescriptorSets[0].m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute);  // UbO
    m_DescriptorSets[0].m_LayoutBindings.emplace_back(
        2U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);  // Particles datas buffer
    m_DescriptorSets[0].m_LayoutBindings.emplace_back(
        3U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);  // Particles alive index buffer
    m_DescriptorSets[0].m_LayoutBindings.emplace_back(
        4U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);  // Particles counter buffer
    m_DescriptorSets[0].m_LayoutBindings.emplace_back(
        5U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute);  // indirect drawing buffer

    return true;
}

bool MeshEmitterModule_Comp_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    m_DescriptorSets[0].m_WriteDescriptorSets.clear();

    // Mesh vertexs
    auto inputMeshPtr = m_InputMesh.lock();
    if (inputMeshPtr && inputMeshPtr->GetVerticesBufferInfo()->range > 0U) {
        m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(
            m_DescriptorSets[0].m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, inputMeshPtr->GetVerticesBufferInfo());
    } else {
        auto corePtr = m_VulkanCore.lock();
        assert(corePtr != nullptr);

        m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(
            m_DescriptorSets[0].m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, corePtr->getEmptyDescriptorBufferInfo());
    }

    // Ubo
    m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(
        m_DescriptorSets[0].m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Comp);

    // Particles datas buffer
    // not existence == no update nor rendering => secured by CanUpdateDescriptors()
    m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(
        m_DescriptorSets[0].m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, m_ParticlesPtr->GetParticlesDatasBufferInfo());
    m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 3U, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr,
        m_ParticlesPtr->GetAliveParticlesIndexBufferInfo());
    m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(
        m_DescriptorSets[0].m_DescriptorSet, 4U, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, m_ParticlesPtr->GetCountersBufferInfo());
    m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 5U, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr,
        m_ParticlesPtr->GetDrawIndirectCommandBufferInfo());

    return true;
}

std::string MeshEmitterModule_Comp_Pass::GetComputeShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "MeshEmitterModule_Comp_Pass";

    SetLocalGroupSize(ct::uvec3(1U, 1U, 1U));

    return u8R"(
#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

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
	uint current_vertexs_count;
	float reset;
	uint emission_count;
	float base_min_life;
	float base_max_life;
	float base_speed;
	float spawn_rate;
	float spawn_mass;
};

layout(push_constant) uniform push_constants 
{
	uint pass_number;
	float absolute_time;
	float delta_time;
};
)" + SceneParticles::GetParticlesDatasBufferHeader(2U) +
           SceneParticles::GetAliveParticlesIndexBufferHeader(3U) + SceneParticles::GetCounterBufferHeader(4U) +
           SceneParticles::GetDrawIndirectCommandHeader(5U) +
           u8R"(
void ResetParticles()
{
	indexCount = 0;
	instanceCount = 1;
	firstIndex = 0;
	vertexOffset = 0;
	firstInstance = 0;
	
	alive_particles_count = 0;
	pending_emission_count = 0;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash(uint x) 
{
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash(uvec2 v) { return hash( v.x ^ hash(v.y)                         ); }
uint hash(uvec3 v) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash(uvec4 v) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) 
{
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float random(float v) { return floatConstruct(hash(floatBitsToUint(v))); }
float random(vec2 v) { return floatConstruct(hash(floatBitsToUint(v))); }
float random(vec3 v) { return floatConstruct(hash(floatBitsToUint(v))); }
float random(vec4 v) { return floatConstruct(hash(floatBitsToUint(v))); }

uint random_vertex_index() 
{
	vec3 inputs = vec3(float(gl_GlobalInvocationID.x), absolute_time, delta_time);
	float rand = random(inputs); // [0 - 1]
	return max(uint(rand * current_vertexs_count) % (current_vertexs_count - 1), 0);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void EmitParticles()
{
	const int i_global_index = int(gl_GlobalInvocationID.x);						
				
	if (reset > 0.5)
	{
		particleDatas[i_global_index].pos3_mass1 = vec4(0.0, 0.0, 0.0, 0.0);
		particleDatas[i_global_index].dir3_speed1 = vec4(0.0, 0.0, 0.0, 0.0);
		particleDatas[i_global_index].color4 = vec4(0.0, 0.0, 0.0, 0.0);
		particleDatas[i_global_index].life1_max_life1 = vec2(0.0, 0.0);
	}
	else
	{
		if (particleDatas[i_global_index].life1_max_life1.x <= 0.0) // dead particle
		{
			if (mod(absolute_time, spawn_rate) > spawn_rate * 0.5) // emit particles
			{
				uint pending_count = atomicAdd(pending_emission_count, 1);
				if (pending_count < emission_count)
				{	
					uint new_pos = atomicAdd(alive_particles_count, 1);
					if (new_pos < max_particles_count)
					{
						uint u_mesh_index = random_vertex_index();
						
						const V3N3T3B3T2C4 v = vertices[i_global_index];
					
						vec3 pos = vec3(v.px, v.py, v.pz);
						vec3 nor = vec3(v.nx, v.ny, v.nz);
						
						particleDatas[i_global_index].pos3_mass1 = vec4(v.px, v.py, v.pz, spawn_mass);
						particleDatas[i_global_index].dir3_speed1 = vec4(nor, base_speed);
						particleDatas[i_global_index].color4 = vec4(nor * 0.5 + 0.5, 1.0);
						particleDatas[i_global_index].life1_max_life1 = vec2(base_min_life, base_max_life);
						
						// increment the alive particle count for this frame
						alive_pre_sim_buffer[new_pos] = i_global_index; // index of the particle
						
						atomicMax(indexCount, alive_particles_count);
					}
				}
			}
		}
		else // alive
		{
			atomicAdd(alive_particles_count, 1);
			atomicMax(indexCount, alive_particles_count);
		}
		
		// security, since in indirect mode, we can crash the app
		// if indexCount > than the buffer size
		atomicMin(indexCount, max_particles_count - 1);
	}
}

void main() 
{
	switch(pass_number)
	{
	case 0:
		ResetParticles();
		break;
	case 1:
		EmitParticles();
		break;
	};
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MeshEmitterModule_Comp_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/) {
    std::string str;

    str += vOffset + "<base_min_life>" + ct::toStr(m_UBOComp.base_min_life) + "</base_min_life>\n";
    str += vOffset + "<base_max_life>" + ct::toStr(m_UBOComp.base_max_life) + "</base_max_life>\n";
    str += vOffset + "<base_speed>" + ct::toStr(m_UBOComp.base_speed) + "</base_speed>\n";
    str += vOffset + "<spawn_rate>" + ct::toStr(m_UBOComp.spawn_rate) + "</spawn_rate>\n";
    str += vOffset + "<max_particles_count>" + ct::toStr(m_UBOComp.max_particles_count) + "</max_particles_count>\n";
    str += vOffset + "<emission_count>" + ct::toStr(m_UBOComp.emission_count) + "</emission_count>\n";

    return str;
}

bool MeshEmitterModule_Comp_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "mesh_emitter_module") {
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
        else if (strName == "emission_count")
            m_UBOComp.emission_count = ct::uvariant(strValue).GetU();
    }

    return true;
}