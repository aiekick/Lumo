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

#include "ParticlesPointRenderer_Mesh_Pass.h"

#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <Base/FrameBuffer.h>
using namespace vkApi;

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

ParticlesPointRenderer_Mesh_Pass::ParticlesPointRenderer_Mesh_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Mesh Pass 1 : Particles", MESH_SHADER_PASS_DEBUG_COLOR);

	//m_DontUseShaderFilesOnDisk = true;
}

ParticlesPointRenderer_Mesh_Pass::~ParticlesPointRenderer_Mesh_Pass()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void ParticlesPointRenderer_Mesh_Pass::ActionBeforeInit()
{
	m_PrimitiveTopology = vk::PrimitiveTopology::ePointList;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void ParticlesPointRenderer_Mesh_Pass::DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	ZoneScoped;

	if (!m_Loaded) return;

	if (vCmdBuffer)
	{
		auto particlesPtr = m_SceneParticles.getValidShared();
		if (particlesPtr)
		{
			auto vertexInputBufferPtr = particlesPtr->GetParticlesVertexInputBuffer();
			if (vertexInputBufferPtr)
			{
				auto indexInputBufferPtr = particlesPtr->GetAliveParticlesIndexInputBuffer();
				if (indexInputBufferPtr)
				{
					auto indirectBufferPtr = particlesPtr->GetDrawIndirectCommandBuffer();
					if (indirectBufferPtr)
					{
						vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipelines[0].m_Pipeline);
						{
							VKFPScoped(*vCmdBuffer, "Particles Point Renderer", "DrawModel Indexed Indirect");

							vk::DeviceSize offset = 0;
							vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
							vCmdBuffer->bindVertexBuffers(0, 1, vertexInputBufferPtr, &offset);
							vCmdBuffer->bindIndexBuffer(*indexInputBufferPtr, 0, vk::IndexType::eUint32);
							vCmdBuffer->drawIndexedIndirect(*indirectBufferPtr, 0, 1, sizeof(VkDrawIndexedIndirectCommand));
						}
					}
				}
			}
		}
	}
}

bool ParticlesPointRenderer_Mesh_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	bool change = false;

	if (change)
	{
		NeedNewSBOUpload();
	}

	return change;
}

void ParticlesPointRenderer_Mesh_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);
}

void ParticlesPointRenderer_Mesh_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);
}

void ParticlesPointRenderer_Mesh_Pass::SetParticles(SceneParticlesWeak vSceneParticles)
{
	ZoneScoped;

	m_SceneParticles = vSceneParticles;
}

vk::DescriptorImageInfo* ParticlesPointRenderer_Mesh_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_FrameBufferPtr)
	{
		if (vOutSize)
		{
			*vOutSize = m_FrameBufferPtr->GetOutputSize();
		}

		return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ParticlesPointRenderer_Mesh_Pass::CreateUBO()
{
	ZoneScoped;

	return true;
}

void ParticlesPointRenderer_Mesh_Pass::UploadUBO()
{
	ZoneScoped;
}

void ParticlesPointRenderer_Mesh_Pass::DestroyUBO()
{
	ZoneScoped;
}

bool ParticlesPointRenderer_Mesh_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_DescriptorSets[0].m_LayoutBindings.clear();
	m_DescriptorSets[0].m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);

	return true;
}

bool ParticlesPointRenderer_Mesh_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	m_DescriptorSets[0].m_WriteDescriptorSets.clear();
	m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, CommonSystem::Instance()->GetBufferInfo());

	return true;
}

void ParticlesPointRenderer_Mesh_Pass::SetInputStateBeforePipelineCreation()
{
	ZoneScoped;

	m_InputState = SceneParticles::GetInputStateBeforePipelineCreation();
}

std::string ParticlesPointRenderer_Mesh_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ParticlesPointRenderer_Vertex";

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable
)" 
+ 
SceneParticles::GetParticlesVertexInputBufferHeader()
+
CommonSystem::GetBufferObjectStructureHeader(0U)
+
u8R"(
layout(location = 1) out flat vec4 out_particle_color;

void main() 
{
	gl_PointSize = 5.0;

	if (particle_life1_max_life1.x > 0.0)
	{
		out_particle_color = particle_color4;
		
		if (particle_life1_max_life1.x < 1.0)
		{
			out_particle_color *= particle_life1_max_life1.x;
		}
		
		gl_Position = cam * vec4(particle_pos3_mass1.xyz, 1.0);
	}
	else
	{
		out_particle_color = vec4(0.0);
		gl_Position = vec4(0.0);
	}
}
)";
}

std::string ParticlesPointRenderer_Mesh_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ParticlesPointRenderer_Fragment";

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

layout(location = 1) in flat vec4 in_particle_color;

void main() 
{
	fragColor = in_particle_color;
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ParticlesPointRenderer_Mesh_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	return str;
}

bool ParticlesPointRenderer_Mesh_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	return true;
}
