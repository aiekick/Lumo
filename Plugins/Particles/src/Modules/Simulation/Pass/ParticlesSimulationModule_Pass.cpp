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

#include "ParticlesSimulationModule_Pass.h"

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

ParticlesSimulationModule_Pass::ParticlesSimulationModule_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : Particles Simulation", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

ParticlesSimulationModule_Pass::~ParticlesSimulationModule_Pass()
{
	Unit();
}

void ParticlesSimulationModule_Pass::ActionBeforeInit()
{
	vk::PushConstantRange push_constant;
	push_constant.offset = 0;
	push_constant.size = sizeof(PushConstants);
	push_constant.stageFlags = vk::ShaderStageFlagBits::eCompute;

	SetPushConstantRange(push_constant);
}

void ParticlesSimulationModule_Pass::ActionAfterInitSucceed()
{
	m_PushConstants.DeltaTime = ct::GetTimeInterval();
}

bool ParticlesSimulationModule_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	bool change = false;

	return change;
}

void ParticlesSimulationModule_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);
}

void ParticlesSimulationModule_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);
}

void ParticlesSimulationModule_Pass::SetTexelBuffer(const uint32_t& vBinding, vk::Buffer* vTexelBuffer, ct::uvec2* vTexelBufferSize)
{
	ZoneScoped;

	if (m_Loaded &&
		vTexelBuffer &&
		vBinding < m_TexelBuffers.size())
	{
		m_TexelBuffers[vBinding] = *vTexelBuffer;

		if (vTexelBufferSize)
		{
			m_TexelBufferViewsSize[vBinding] = *vTexelBufferSize;
			SetDispatchSize1D(vTexelBufferSize->x);
		}
	}
	else
	{
		m_TexelBuffers[vBinding] = VK_NULL_HANDLE;
	}
}

void ParticlesSimulationModule_Pass::SetTexelBufferView(const uint32_t& vBinding, vk::BufferView* vTexelBufferView, ct::uvec2* vTexelBufferSize)
{
	ZoneScoped;

	if (m_Loaded &&
		vTexelBufferView &&
		vBinding < m_TexelBufferViews.size())
	{
		m_TexelBufferViews[vBinding] = *vTexelBufferView;

		if (vTexelBufferSize)
		{
			m_TexelBufferViewsSize[vBinding] = *vTexelBufferSize;
			SetDispatchSize1D(vTexelBufferSize->x);
		}
	}
	else
	{
		m_TexelBufferViews[vBinding] = VK_NULL_HANDLE;
	}
}
vk::Buffer* ParticlesSimulationModule_Pass::GetTexelBuffer(const uint32_t& vBindingPoint, ct::uvec2* vOutSize)
{
	ZoneScoped;

	if (m_TexelBuffers[0])
	{
		if (vOutSize)
		{
			vOutSize->x = m_TexelBufferViewsSize[0].x;
			vOutSize->y = 0U;
		}

		return &m_TexelBuffers[0];
	}

	return nullptr;
}

vk::BufferView* ParticlesSimulationModule_Pass::GetTexelBufferView(const uint32_t& vBindingPoint, ct::uvec2* vOutSize)
{
	ZoneScoped;

	if (m_TexelBufferViews[0])
	{
		if (vOutSize)
		{
			vOutSize->x = m_TexelBufferViewsSize[0].x;
			vOutSize->y = 0U;
		}

		return &m_TexelBufferViews[0];
	}

	return nullptr;
}

void ParticlesSimulationModule_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
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
		
		m_PushConstants.DeltaTime = m_VulkanCorePtr->GetDeltaTime();
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
	}
}

bool ParticlesSimulationModule_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute);

	return true;
}

bool ParticlesSimulationModule_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();

	writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eStorageTexelBuffer,
		nullptr, nullptr, &m_TexelBufferViews[0]);
	
	return true;
}

std::string ParticlesSimulationModule_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ParticlesSimulationModule_Pass";

	SetLocalGroupSize(ct::uvec3(8U, 1U, 1U));

	return u8R"(
#version 450

layout(local_size_x = 8, local_size_y = 1, local_size_z = 1) in;

layout(binding = 0, rgba32f) uniform imageBuffer pos3_life1_dir3_speed4_color4_buffer;

layout(push_constant) uniform TimeState 
{
	float DeltaTime;
};

void main() 
{
	const int i_global_index = int(gl_GlobalInvocationID.x);
	const float f_global_index = float(gl_GlobalInvocationID.x);
	
	vec4 pos_life = imageLoad(pos3_life1_dir3_speed4_color4_buffer, i_global_index * 3 + 0);
	vec4 dir_speed = imageLoad(pos3_life1_dir3_speed4_color4_buffer, i_global_index * 3 + 1);
	//vec4 color = imageLoad(pos3_life1_dir3_speed4_color4_buffer, i_global_index * 3 + 2);
			
	// rotation around y axis
	dir_speed.xyz = normalize(normalize(cross(vec3( 0.0, 1.0, 0.2 ), normalize(pos_life.xyz))) + normalize(dir_speed.xyz));
	
	pos_life.xyz += dir_speed.xyz * dir_speed.w;
	pos_life.w -= DeltaTime;
	
	imageStore(pos3_life1_dir3_speed4_color4_buffer, i_global_index * 3 + 0, pos_life);
	imageStore(pos3_life1_dir3_speed4_color4_buffer, i_global_index * 3 + 1, dir_speed);
	//imageStore(pos3_life1_dir3_speed4_color4_buffer, i_global_index * 3 + 2, color);
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ParticlesSimulationModule_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	//str += vOffset + "<blur_radius>" + ct::toStr(m_UBOComp.u_blur_radius) + "</blur_radius>\n";
	
	return str;
}

bool ParticlesSimulationModule_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strParentName == "diffuse_module")
	{
		//if (strName == "blur_radius")
		//	m_UBOComp.u_blur_radius = ct::uvariant(strValue).GetU();

		NeedNewUBOUpload();
	}

	return true;
}