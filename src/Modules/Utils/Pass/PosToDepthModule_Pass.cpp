/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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
#include <Generic/FrameBuffer.h>
using namespace vkApi;

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

PosToDepthModule_Pass::PosToDepthModule_Pass(vkApi::VulkanCore* vVulkanCore)
	: QuadShaderPass(vVulkanCore, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Quad Pass 1 : Pos To Depth", QUAD_SHADER_PASS_DEBUG_COLOR);
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
	DrawInputTexture(m_VulkanCore, "Position", 0U, m_OutputRatio);

	return false;
}

void PosToDepthModule_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{

}

void PosToDepthModule_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{

}

void PosToDepthModule_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding < m_SamplerImageInfos.size())
		{
			if (vImageInfo)
			{
				m_SamplerImageInfos[vBinding] = *vImageInfo;

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

				if (m_EmptyTexturePtr)
				{
					m_SamplerImageInfos[vBinding] = m_EmptyTexturePtr->m_DescriptorImageInfo;
				}
				else
				{
					CTOOL_DEBUG_BREAK;
				}
			}

			m_NeedSamplerUpdate = true;
		}
	}
}

vk::DescriptorImageInfo* PosToDepthModule_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	ZoneScoped;

	if (m_FrameBufferPtr)
	{
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

void PosToDepthModule_Pass::UpdateShaders(const std::set<std::string>& vFiles)
{
	bool needReCompil = false;

	if (vFiles.find("shaders/PosToDepthModule_Pass.vert") != vFiles.end())
	{
		auto shader_path = FileHelper::Instance()->GetAppPath() + "/shaders/PosToDepthModule_Pass.vert";
		if (FileHelper::Instance()->IsFileExist(shader_path))
		{
			m_VertexShaderCode = FileHelper::Instance()->LoadFileToString(shader_path);
			needReCompil = true;
		}

	}
	else if (vFiles.find("shaders/PosToDepthModule_Pass.frag") != vFiles.end())
	{
		auto shader_path = FileHelper::Instance()->GetAppPath() + "/shaders/PosToDepthModule_Pass.frag";
		if (FileHelper::Instance()->IsFileExist(shader_path))
		{
			m_FragmentShaderCode = FileHelper::Instance()->LoadFileToString(shader_path);
			needReCompil = true;
		}
	}

	if (needReCompil)
	{
		ReCompil();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PosToDepthModule_Pass::CreateUBO()
{
	ZoneScoped;

	auto size_in_bytes = sizeof(UBOFrag);
	m_UBO_Frag = VulkanRessource::createUniformBufferObject(m_VulkanCore, size_in_bytes);
	m_DescriptorBufferInfo_Frag.buffer = m_UBO_Frag->buffer;
	m_DescriptorBufferInfo_Frag.range = size_in_bytes;
	m_DescriptorBufferInfo_Frag.offset = 0;

	m_EmptyTexturePtr = Texture2D::CreateEmptyTexture(m_VulkanCore, ct::uvec2(1, 1), vk::Format::eR8G8B8A8Unorm);

	for (auto& a : m_SamplerImageInfos)
	{
		a = m_EmptyTexturePtr->m_DescriptorImageInfo;
	}

	NeedNewUBOUpload();

	return true;
}

void PosToDepthModule_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCore, *m_UBO_Frag, &m_UBOFrag, sizeof(UBOFrag));
}

void PosToDepthModule_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Frag.reset();
	m_EmptyTexturePtr.reset();
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
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_SamplerImageInfos[0], nullptr); // position

	return true;
}

std::string PosToDepthModule_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PosToDepthModule_Pass_Vertex";

	auto shader_path = FileHelper::Instance()->GetAppPath() + "/shaders/PosToDepthModule_Pass.vert";

	if (FileHelper::Instance()->IsFileExist(shader_path))
	{
		m_VertexShaderCode = FileHelper::Instance()->LoadFileToString(shader_path);
	}
	else
	{
		m_VertexShaderCode = u8R"(#version 450
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
		FileHelper::Instance()->SaveStringToFile(m_VertexShaderCode, shader_path);
	}

	return m_VertexShaderCode;
}

std::string PosToDepthModule_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PosToDepthModule_Pass_Fragment";

	auto shader_path = FileHelper::Instance()->GetAppPath() + "/shaders/PosToDepthModule_Pass.frag";

	if (FileHelper::Instance()->IsFileExist(shader_path))
	{
		m_FragmentShaderCode = FileHelper::Instance()->LoadFileToString(shader_path);
	}
	else
	{
		m_FragmentShaderCode = u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 v_uv;
)"
+ CommonSystem::Instance()->GetBufferObjectStructureHeader(0U) +
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
		FileHelper::Instance()->SaveStringToFile(m_FragmentShaderCode, shader_path);
	}

	return m_FragmentShaderCode;
}
