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

using namespace vkApi;

#define COUNT_BUFFERS 2
#define PARTICLES_COUNT 2000

//////////////////////////////////////////////////////////////
//// SSAO SECOND PASS : BLUR /////////////////////////////////
//////////////////////////////////////////////////////////////

ParticlesSimulationModule_Pass::ParticlesSimulationModule_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : ParticlesSimulation", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	//m_DontUseShaderFilesOnDisk = true;
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

	m_DispatchSize.x = 2000U;
	m_DispatchSize.y = 1U;
	m_DispatchSize.z = 1U;
}

bool ParticlesSimulationModule_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	bool change = false;

	if (ImGui::ContrastedButton("Reset"))
	{
		change = true;
	}

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

void ParticlesSimulationModule_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				m_ImageInfos[vBinding] = *vImageInfo;
			}
			else
			{
				m_ImageInfos[vBinding] = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
			}
		}
	}
}

vk::Buffer* ParticlesSimulationModule_Pass::GetTexelBuffer(const uint32_t& vBindingPoint, ct::uvec2* vOutSize)
{
	ZoneScoped;

	if (m_ParticleTexelBufferPtr)
	{
		if (vOutSize)
		{
			vOutSize->x = PARTICLES_COUNT;
			vOutSize->y = 0U;
		}

		return &m_ParticleTexelBufferPtr->buffer;
	}

	return nullptr;
}

vk::BufferView* ParticlesSimulationModule_Pass::GetTexelBufferView(const uint32_t& vBindingPoint, ct::uvec2* vOutSize)
{
	ZoneScoped;

	if (m_ParticleTexelBufferPtr)
	{
		if (vOutSize)
		{
			vOutSize->x = PARTICLES_COUNT;
			vOutSize->y = 0U;
		}

		return &m_ParticleTexelBufferPtr->bufferView;
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
		
		m_PushConstants.DeltaTime = ct::GetTimeInterval();
		vCmdBuffer->pushConstants(m_PipelineLayout,
			vk::ShaderStageFlagBits::eCompute,
			0, sizeof(PushConstants), &m_PushConstants);

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

bool ParticlesSimulationModule_Pass::CreateSBO()
{
	ZoneScoped;

	for (auto& info : m_ImageInfos)
	{
		info = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
	}

	std::vector<ct::fvec4> particles;
	particles.resize(PARTICLES_COUNT);

	uint32_t idx = 0U;
	float astep = 3.14159f * 2.0f / 70.0f;
	for (auto& p : particles)
	{
		float a = astep * float(idx++);
		ct::fvec2 d = a * ct::fvec2(cos(a), sin(a));
		p.x = d.x;
		p.y = d.y;
		p.z = 0.0f;
		p.w = 1.0f;
	}

	m_ParticleTexelBufferPtr.reset();

	auto sizeInBytes = particles.size() * sizeof(ct::fvec4);
	m_ParticleTexelBufferPtr = VulkanRessource::createTexelBuffer(
		m_VulkanCorePtr,
		particles.data(),
		sizeInBytes, 
		vk::Format::eR32G32B32A32Sfloat);
	
	NeedNewSBOUpload();

	return true;
}

void ParticlesSimulationModule_Pass::DestroySBO()
{
	m_ParticleTexelBufferPtr.reset();
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
		nullptr, nullptr, &m_ParticleTexelBufferPtr->bufferView);
	
	return true;
}

std::string ParticlesSimulationModule_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ParticlesSimulationModule_Pass";

	return u8R"(
#version 450

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout(binding = 0, rgba32f) uniform imageBuffer posBuffer;

layout(push_constant) uniform TimeState 
{
	float DeltaTime;
	uint particlesCount;
};

void main() 
{
	if( gl_GlobalInvocationID.x < particlesCount ) 
	{
		vec4 position = imageLoad(posBuffer, int(gl_GlobalInvocationID.x));
		position.w = 1.0;

		vec3 speed = normalize( cross( vec3( 0.0, 1.0, 0.0 ), position.xyz ) ) * 1.0;
    
		position.xyz += speed * DeltaTime;
    
		imageStore(posBuffer, int(gl_GlobalInvocationID.x), position);
	}
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