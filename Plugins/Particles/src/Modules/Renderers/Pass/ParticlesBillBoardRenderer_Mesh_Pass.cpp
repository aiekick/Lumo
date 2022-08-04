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

#include "ParticlesBillBoardRenderer_Mesh_Pass.h"

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

ParticlesBillBoardRenderer_Mesh_Pass::ParticlesBillBoardRenderer_Mesh_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: QuadShaderPass(vVulkanCorePtr, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Quad Pass 1 : Particles", MESH_SHADER_PASS_DEBUG_COLOR);

	//m_DontUseShaderFilesOnDisk = true;
}

ParticlesBillBoardRenderer_Mesh_Pass::~ParticlesBillBoardRenderer_Mesh_Pass()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void ParticlesBillBoardRenderer_Mesh_Pass::ActionBeforeInit()
{
	m_TexelBufferViews[0] = VK_NULL_HANDLE;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ParticlesBillBoardRenderer_Mesh_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	bool change = false;

	if (change)
	{
		NeedNewSBOUpload();
	}

	return change;
}

void ParticlesBillBoardRenderer_Mesh_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

}

void ParticlesBillBoardRenderer_Mesh_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

}

void ParticlesBillBoardRenderer_Mesh_Pass::SetTexelBuffer(const uint32_t& vBinding, vk::Buffer* vTexelBuffer, ct::uvec2* vTexelBufferSize)
{
	ZoneScoped;

	if (m_Loaded && vBinding < m_TexelBuffers.size())
	{
		if (vTexelBuffer)
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
}

void ParticlesBillBoardRenderer_Mesh_Pass::SetTexelBufferView(const uint32_t& vBinding, vk::BufferView* vTexelBufferView, ct::uvec2* vTexelBufferSize)
{
	ZoneScoped;

	if (m_Loaded && vBinding < m_TexelBufferViews.size())
	{
		if (vTexelBufferView)
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
}

vk::DescriptorImageInfo* ParticlesBillBoardRenderer_Mesh_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

bool ParticlesBillBoardRenderer_Mesh_Pass::CreateUBO()
{
	ZoneScoped;

	NeedNewUBOUpload();

	return true;
}

void ParticlesBillBoardRenderer_Mesh_Pass::UploadUBO()
{
	ZoneScoped;
}

void ParticlesBillBoardRenderer_Mesh_Pass::DestroyUBO()
{
	ZoneScoped;
}

bool ParticlesBillBoardRenderer_Mesh_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eUniformTexelBuffer, 1, vk::ShaderStageFlagBits::eVertex);
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);

	return true;
}

bool ParticlesBillBoardRenderer_Mesh_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();
	writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eUniformTexelBuffer, nullptr, nullptr, &m_TexelBufferViews[0]);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, CommonSystem::Instance()->GetBufferInfo());

	return true;
}

std::string ParticlesBillBoardRenderer_Mesh_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ParticlesBillBoardRenderer_Vertex";

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 vertPosition;
layout(location = 1) in vec2 vertUv;

layout(binding = 0) uniform samplerBuffer posBuffer;
)"
+ CommonSystem::GetBufferObjectStructureHeader(1U) +
u8R"(
layout(location = 0) out vec2 uv_map;
layout(location = 1) out vec4 particleColor;

void main() 
{
	uv_map = vertUv;

	vec4 particle_pos_alpha = texelFetch(posBuffer, gl_InstanceIndex);

	vec4 pos = cam * vec4(particle_pos_alpha.xyz, 1.0);
	vec4 quad = proj * view * vec4(vertPosition, 0.0, 1.0);

	particleColor = vec4(particle_pos_alpha.w);
	gl_Position = vec4(pos.xyz + quad.xyz, 1.0);
}
)";
}

std::string ParticlesBillBoardRenderer_Mesh_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ParticlesBillBoardRenderer_Fragment";

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec2 uv_map;
layout(location = 1) in vec4 particleColor;

void main() 
{
	fragColor = vec4(uv_map, 0.0, 1.0);
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ParticlesBillBoardRenderer_Mesh_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	return str;
}

bool ParticlesBillBoardRenderer_Mesh_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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
