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

#include "PosToDepthModule_Pass.h"

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
#include <Base/FrameBuffer.h>
using namespace vkApi;

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

PosToDepthModule_Pass::PosToDepthModule_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: QuadShaderPass(vVulkanCorePtr, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Quad Pass 1 : Pos To Depth", QUAD_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

PosToDepthModule_Pass::~PosToDepthModule_Pass()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool PosToDepthModule_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	DrawInputTexture(m_VulkanCorePtr, "Position", 0U, m_OutputRatio);

	return false;
}

void PosToDepthModule_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

}

void PosToDepthModule_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

}

void PosToDepthModule_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				m_ImageInfos[vBinding] = *vImageInfo;

				if ((&m_UBOFrag.use_sampler_pos)[vBinding] < 1.0f)
				{
					(&m_UBOFrag.use_sampler_pos)[vBinding] = 1.0f;
					NeedNewUBOUpload();
				}
			}
			else
			{
				if ((&m_UBOFrag.use_sampler_pos)[vBinding] > 0.0f)
				{
					(&m_UBOFrag.use_sampler_pos)[vBinding] = 0.0f;
					NeedNewUBOUpload();
				}

				m_ImageInfos[vBinding] = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* PosToDepthModule_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	ZoneScoped;

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
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string PosToDepthModule_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	return str;
}

bool PosToDepthModule_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PosToDepthModule_Pass::CreateUBO()
{
	ZoneScoped;

	auto size_in_bytes = sizeof(UBOFrag);
	m_UBO_Frag = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, size_in_bytes);
	m_DescriptorBufferInfo_Frag.buffer = m_UBO_Frag->buffer;
	m_DescriptorBufferInfo_Frag.range = size_in_bytes;
	m_DescriptorBufferInfo_Frag.offset = 0;

	for (auto& info : m_ImageInfos)
	{
		info = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
	}

	NeedNewUBOUpload();

	return true;
}

void PosToDepthModule_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, *m_UBO_Frag, &m_UBOFrag, sizeof(UBOFrag));
}

void PosToDepthModule_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Frag.reset();
}

bool PosToDepthModule_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);

	return true;
}

bool PosToDepthModule_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();
	writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, CommonSystem::Instance()->GetBufferInfo());
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Frag);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], nullptr); // position

	return true;
}

std::string PosToDepthModule_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PosToDepthModule_Pass_Vertex";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 vertPosition;
layout(location = 1) in vec2 vertUv;
layout(location = 0) out vec2 v_uv;

void main() 
{
	v_uv = vertUv;
	gl_Position = vec4(vertPosition, 0.0, 1.0);
}
)";
}

std::string PosToDepthModule_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PosToDepthModule_Pass_Fragment";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 v_uv;
)"
+ CommonSystem::GetBufferObjectStructureHeader(0U) +
u8R"(
layout (std140, binding = 1) uniform UBO_Vert 
{ 
	float use_sampler_pos;
};
layout(binding = 2) uniform sampler2D pos_map_sampler;

vec3 getRayOrigin()
{
	vec3 ro = view[3].xyz + model[3].xyz;
	ro *= mat3(view * model);
	return -ro;
}

void main() 
{
	vec3 pos = texture(pos_map_sampler, v_uv).xyz;
	
	if (use_sampler_pos > 0.5)
	{
		if (dot(pos, pos) > 0.0)
		{
			vec3 ro = getRayOrigin();
			float dep = length(ro - pos) / cam_far;
			fragColor = vec4(dep, dep, dep, 1.0);
		}
		else
		{
			fragColor = vec4(0.0);
		}
	}
	else 
	{
		fragColor = vec4(0.0);
	}
}

)";
}
