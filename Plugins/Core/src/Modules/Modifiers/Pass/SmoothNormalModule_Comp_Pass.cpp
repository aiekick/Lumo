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

#include "SmoothNormalModule_Comp_Pass.h"

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

using namespace GaiApi;

//////////////////////////////////////////////////////////////
//// SSAO FIRST PASS : AO ////////////////////////////////////
//////////////////////////////////////////////////////////////

SmoothNormalModule_Comp_Pass::SmoothNormalModule_Comp_Pass(GaiApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : Smooth Normal", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

SmoothNormalModule_Comp_Pass::~SmoothNormalModule_Comp_Pass()
{
	Unit();
}

void SmoothNormalModule_Comp_Pass::ActionBeforeInit()
{
	vk::PushConstantRange push_constant;
	push_constant.offset = 0;
	push_constant.size = sizeof(PushConstants);
	push_constant.stageFlags = vk::ShaderStageFlagBits::eCompute;

	SetPushConstantRange(push_constant);
}

bool SmoothNormalModule_Comp_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext, const std::string& vUserDatas)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	ImGui::Header("Infos");

	auto modelPtr = m_SceneModel.lock();
	if (modelPtr)
	{
		for (auto meshPtr : *modelPtr)
		{
			ImGui::Text("Mesh Vertice Count : %u", (uint32_t)meshPtr->GetVerticesCount());
			if (meshPtr->HasIndices())
			{
				ImGui::Text("Mesh Indice Count : %u", (uint32_t)meshPtr->GetIndicesCount());
			}

			ImGui::Header("Attibutes");

			ImGui::Text("[%s] Normals", meshPtr->HasNormals() ? "x" : " ");
			ImGui::Text("[%s] Tangeants", meshPtr->HasTangeants() ? "x" : " ");
			ImGui::Text("[%s] BiTangeants", meshPtr->HasBiTangeants() ? "x" : " ");
			ImGui::Text("[%s] TextureCoords", meshPtr->HasTextureCoords() ? "x" : " ");
			ImGui::Text("[%s] VertexColors", meshPtr->HasVertexColors() ? "x" : " ");
			ImGui::Text("[%s] Indices", meshPtr->HasIndices() ? "x" : " ");
		}
	}

	return false;
}

bool SmoothNormalModule_Comp_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContext, const std::string& vUserDatas)
{
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
    return false;
}

bool SmoothNormalModule_Comp_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContext, const std::string& vUserDatas) {
	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
    return false;
}

void SmoothNormalModule_Comp_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (!m_Loaded) return;
	if (!m_IsShaderCompiled) return;

	if (vCmdBuffer)
	{
		auto modelPtr = m_SceneModel.lock();
		if (modelPtr)
		{
			for (auto meshPtr : *modelPtr)
			{
				m_InputMesh = meshPtr;
				NeedNewModelUpdate();
				UpdateModel(true);
				UpdateRessourceDescriptor();

				vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);

				vCmdBuffer->pipelineBarrier(
					vk::PipelineStageFlagBits::eVertexInput,
					vk::PipelineStageFlagBits::eComputeShader,
					vk::DependencyFlags(),
					nullptr,
					nullptr,
					nullptr);

				// le dispatch c'est les indices sur 3
				SetDispatchSize1D(meshPtr->GetIndicesCount() / 3U);
				ComputePass(vCmdBuffer, 0U); // add vertex normals

				// le dispatch c'est le nombre de vertexs
				SetDispatchSize1D(meshPtr->GetVerticesCount());
				ComputePass(vCmdBuffer, 1U); // normalize vertex normals

				vCmdBuffer->pipelineBarrier(
					vk::PipelineStageFlagBits::eComputeShader,
					vk::PipelineStageFlagBits::eVertexInput,
					vk::DependencyFlags(),
					nullptr,
					nullptr,
					nullptr);
			}
		}
	}
}

void SmoothNormalModule_Comp_Pass::ComputePass(vk::CommandBuffer* vCmd, const uint32_t& vPassNumber)
{
	vCmd->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);

	m_PushConstants.pass_number = vPassNumber;
	vCmd->pushConstants(m_Pipelines[0].m_PipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(PushConstants), &m_PushConstants);

	Dispatch(vCmd);
}

void SmoothNormalModule_Comp_Pass::SetModel(SceneModelWeak vSceneModel)
{
	ZoneScoped;

	m_SceneModel = vSceneModel;

	NeedNewModelUpdate();
}

SceneModelWeak SmoothNormalModule_Comp_Pass::GetModel()
{
	ZoneScoped;

	return m_SceneModel;
}

bool SmoothNormalModule_Comp_Pass::BuildModel()
{
	auto meshPtr = m_InputMesh.lock();
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
			m_SBO_Normals_Compute_Helper_BufferInfos = vk::DescriptorBufferInfo{ m_SBO_Normals_Compute_Helper->buffer, 0, sizeInBytes };
		}
		else
		{
			m_SBO_Normals_Compute_Helper_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
		}
	}

	return true;
}

void SmoothNormalModule_Comp_Pass::DestroyModel(const bool& vReleaseDatas)
{
	m_NormalDatas.clear();
	m_SBO_Normals_Compute_Helper.reset();
	m_SBO_Normals_Compute_Helper_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
}

bool SmoothNormalModule_Comp_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute);
	return res;
}

bool SmoothNormalModule_Comp_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;

	auto inputMeshPtr = m_InputMesh.lock();
	if (inputMeshPtr && inputMeshPtr->GetVerticesBufferInfo()->range > 0U)
	{
		// VertexStruct::P3_N3_TA3_BTA3_T2_C4
		res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eStorageBuffer, inputMeshPtr->GetVerticesBufferInfo());
		// VertexStruct::I1
		res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eStorageBuffer, inputMeshPtr->GetIndicesBufferInfo());
		// Normals
		res &= AddOrSetWriteDescriptorBuffer(2U, vk::DescriptorType::eStorageBuffer, &m_SBO_Normals_Compute_Helper_BufferInfos);
	}
	else
	{
		// empty version, almost empty because his size is thr size of 1 VertexStruct::P3_N3_TA3_BTA3_T2_C4
		res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eStorageBuffer, m_VulkanCorePtr->getEmptyDescriptorBufferInfo());
		// empty version, almost empty because his size is thr size of 1 VertexStruct::I1
		res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eStorageBuffer, m_VulkanCorePtr->getEmptyDescriptorBufferInfo());
		// empty version, almost empty because his size is thr size of 1 uvec3
		res &= AddOrSetWriteDescriptorBuffer(2U, vk::DescriptorType::eStorageBuffer, m_VulkanCorePtr->getEmptyDescriptorBufferInfo());
	}

	return res;
}

std::string SmoothNormalModule_Comp_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "SmoothNormalModule_Comp_Pass";

	SetLocalGroupSize(1U);

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

struct V3N3T3B3T2C4 
{
	float px, py, pz;
	float nx, ny, nz;
	float tax, tay, taz;
	float btax, btay, btaz;
	float tx, ty;
	float cx, cy, cz, cw;
};

layout(std430, binding = 0) buffer Vertices
{
	V3N3T3B3T2C4 vertices[];
};

layout(std430, binding = 1) buffer Indices
{
	uint indices[];
};

layout(std430, binding = 2) buffer Normals
{
	int normals[];
};

layout(push_constant) uniform constants
{
	uint pass_number;
};

// count indexs
void first_pass_sum_normals(const in uint id)
{
	const uint i0 = indices[id * 3 + 0];
	const uint i1 = indices[id * 3 + 1];
	const uint i2 = indices[id * 3 + 2];

	const V3N3T3B3T2C4 v0 = vertices[i0];
	const V3N3T3B3T2C4 v1 = vertices[i1];
	const V3N3T3B3T2C4 v2 = vertices[i2];

	const vec3 p0 = vec3(v0.px, v0.py, v0.pz);
	const vec3 p1 = vec3(v1.px, v1.py, v1.pz);
	const vec3 p2 = vec3(v2.px, v2.py, v2.pz);

	// cross product of two edges
	const vec3 n = cross(normalize(p1 - p0), normalize(p2 - p0));
	const int i_n_x = int(n.x * 100000.0);
	const int i_n_y = int(n.y * 100000.0);
	const int i_n_z = int(n.z * 100000.0);
	
	// exclusive add float to int
	atomicAdd(normals[i0 * 3], i_n_x);
	atomicAdd(normals[i0 * 3 + 1], i_n_y);
	atomicAdd(normals[i0 * 3 + 2], i_n_z);

	atomicAdd(normals[i1 * 3], i_n_x);
	atomicAdd(normals[i1 * 3 + 1], i_n_y);
	atomicAdd(normals[i1 * 3 + 2], i_n_z);
	
	atomicAdd(normals[i2 * 3], i_n_x);
	atomicAdd(normals[i2 * 3 + 1], i_n_y);
	atomicAdd(normals[i2 * 3 + 2], i_n_z);
}

// count vertexs
void second_pass_normalize_normals(const in uint id)
{
	// only one thread on i0
	// not atomic needed here

	const float nx = float(normals[id * 3]);
	const float ny = float(normals[id * 3 + 1]);
	const float nz = float(normals[id * 3 + 2]);
	const vec3 n = normalize(vec3(nx, ny, nz));
		
	vertices[id].nx = n.x;
	vertices[id].ny = n.y;
	vertices[id].nz = n.z;

	// reste normals to zero for the next call
	normals[id * 3] = 0;	
	normals[id * 3 + 1] = 0;
	normals[id * 3 + 2] = 0;
}

void main()
{
	const uint id = gl_GlobalInvocationID.x;
	switch(pass_number)
	{
	case 0:
		first_pass_sum_normals(id);
		break;
	case 1:
		second_pass_normalize_normals(id);
		break;
	default:
		break;
	}
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string SmoothNormalModule_Comp_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	return str;
}

bool SmoothNormalModule_Comp_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
{
	// The value of this child identifies the name of this element
	/*std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();*/

	return true;
}