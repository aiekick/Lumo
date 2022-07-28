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

#include "ParticlesPointRenderer_Pass.h"

#include <functional>
#include <Gui/MainFrame.h>
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

ParticlesPointRenderer_Pass::ParticlesPointRenderer_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Quad Pass 1 : Particles", MESH_SHADER_PASS_DEBUG_COLOR);

	//m_DontUseShaderFilesOnDisk = true;
}

ParticlesPointRenderer_Pass::~ParticlesPointRenderer_Pass()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void ParticlesPointRenderer_Pass::ActionBeforeInit()
{
	m_TexelBufferViews[0] = VK_NULL_HANDLE;

	m_PrimitiveTopology = vk::PrimitiveTopology::ePointList;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void ParticlesPointRenderer_Pass::DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	ZoneScoped;

	if (!m_Loaded) return;

	if (vCmdBuffer && m_TexelBuffers[0] && m_TexelBufferViewsSize[0].x)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
		{
			VKFPScoped(*vCmdBuffer, "Particles Point Renderer", "DrawModel");

			vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout, 0, m_DescriptorSet, nullptr);

			vk::DeviceSize offsets = 0;
			vCmdBuffer->bindVertexBuffers(0, m_TexelBuffers[0], offsets);

			vCmdBuffer->draw(m_TexelBufferViewsSize[0].x, 1, 0, 0);
		}
	}
}

bool ParticlesPointRenderer_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	bool change = false;

	if (change)
	{
		NeedNewSBOUpload();
	}

	return change;
}

void ParticlesPointRenderer_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

}

void ParticlesPointRenderer_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

}

void ParticlesPointRenderer_Pass::SetTexelBuffer(const uint32_t& vBinding, vk::Buffer* vTexelBuffer, ct::uvec2* vTexelBufferSize)
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
		}
	}
	else
	{
		m_TexelBuffers[vBinding] = VK_NULL_HANDLE;
	}
}

void ParticlesPointRenderer_Pass::SetTexelBufferView(const uint32_t& vBinding, vk::BufferView* vTexelBufferView, ct::uvec2* vTexelBufferSize)
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
		}
	}
	else
	{
		m_TexelBufferViews[vBinding] = VK_NULL_HANDLE;
	}
}

vk::DescriptorImageInfo* ParticlesPointRenderer_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

bool ParticlesPointRenderer_Pass::CreateUBO()
{
	ZoneScoped;

	NeedNewUBOUpload();

	return true;
}

void ParticlesPointRenderer_Pass::UploadUBO()
{
	ZoneScoped;
}

void ParticlesPointRenderer_Pass::DestroyUBO()
{
	ZoneScoped;
}

bool ParticlesPointRenderer_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eUniformTexelBuffer, 1, vk::ShaderStageFlagBits::eVertex);
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);

	return true;
}

bool ParticlesPointRenderer_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();
	writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eUniformTexelBuffer, nullptr, nullptr, &m_TexelBufferViews[0]);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, CommonSystem::Instance()->GetBufferInfo());

	return true;
}

void ParticlesPointRenderer_Pass::SetInputStateBeforePipelineCreation()
{
	ZoneScoped;

	m_InputState.binding.binding = 0;
	m_InputState.binding.stride = sizeof(ct::fvec4);
	m_InputState.binding.inputRate = vk::VertexInputRate::eVertex;

	uint32_t offset = 0;

	// P3
	m_InputState.attributes.resize(1);

	{
		// vertex pos vec3
		auto& attrib = m_InputState.attributes[0];
		attrib.binding = 0;
		attrib.location = 0;
		attrib.format = vk::Format::eR32G32B32A32Sfloat;
		attrib.offset = 0;
		offset += sizeof(ct::fvec4);
	}

	m_InputState.state = vk::PipelineVertexInputStateCreateInfo(
		vk::PipelineVertexInputStateCreateFlags(),
		1,
		&m_InputState.binding,
		static_cast<uint32_t>(m_InputState.attributes.size()),
		m_InputState.attributes.data()
	);
}

std::string ParticlesPointRenderer_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ParticlesPointRenderer_Vertex";

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 particle_position;
)"
+ CommonSystem::GetBufferObjectStructureHeader(1U) +
u8R"(
layout(location = 1) out flat vec4 particleColor;

void main() 
{
	gl_PointSize = 1.0;

	particleColor = normalize(particle_position) * 0.5 + 0.5;
	gl_Position = cam * particle_position;
}
)";
}

std::string ParticlesPointRenderer_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ParticlesPointRenderer_Fragment";

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

layout(location = 1) in flat vec4 particleColor;

void main() 
{
	fragColor = particleColor;
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ParticlesPointRenderer_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	return str;
}

bool ParticlesPointRenderer_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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
