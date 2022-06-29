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

#include "ModelSSSModule_Pass.h"

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

ModelSSSModule_Pass::ModelSSSModule_Pass(vkApi::VulkanCore* vVulkanCore)
	: QuadShaderPass(vVulkanCore, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Quad Pass 1 : Model SSS", QUAD_SHADER_PASS_DEBUG_COLOR);
}

ModelSSSModule_Pass::~ModelSSSModule_Pass()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ModelSSSModule_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	bool change = false;

	if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen))
	{
		change |= ImGui::SliderFloatDefaultCompact(0.0f, "Bias", &m_UBOFrag.u_bias, 0.0f, 0.02f, 0.01f);
		change |= ImGui::SliderFloatDefaultCompact(0.0f, "Noise Scale", &m_UBOFrag.u_poisson_scale, 1.0f, 10000.0f, 5000.0f);

		if (change)
		{
			NeedNewUBOUpload();
		}
	}

	DrawInputTexture(m_VulkanCore, "Input Position", 0U, m_OutputRatio);
	DrawInputTexture(m_VulkanCore, "Input SSS Map", 1U, m_OutputRatio);

	return change;
}

void ModelSSSModule_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{

}

void ModelSSSModule_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{

}

void ModelSSSModule_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
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

vk::DescriptorImageInfo* ModelSSSModule_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	ZoneScoped;

	if (m_FrameBufferPtr)
	{
		return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

void ModelSSSModule_Pass::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	ZoneScoped;

	m_SceneLightGroup = vSceneLightGroup;

	m_NeedLightGroupUpdate = true;
}

void ModelSSSModule_Pass::SetLighViewMatrix(const glm::mat4& vLightViewMatrix)
{
	m_UBOFrag.u_light_cam = vLightViewMatrix;

	NeedNewUBOUpload();
}

void ModelSSSModule_Pass::UpdateShaders(const std::set<std::string>& vFiles)
{
	bool needReCompil = false;

	if (vFiles.find("shaders/SSAOModule.vert") != vFiles.end())
	{
		auto shader_path = FileHelper::Instance()->GetAppPath() + "/shaders/SSAOModule.vert";
		if (FileHelper::Instance()->IsFileExist(shader_path))
		{
			m_VertexShaderCode = FileHelper::Instance()->LoadFileToString(shader_path);
			needReCompil = true;
		}

	}
	else if (vFiles.find("shaders/BlurModule_Pass.frag") != vFiles.end())
	{
		auto shader_path = FileHelper::Instance()->GetAppPath() + "/shaders/BlurModule_Pass.frag";
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
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ModelSSSModule_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<bias>" + ct::toStr(m_UBOFrag.u_bias) + "</bias>\n";
	str += vOffset + "<noise_scale>" + ct::toStr(m_UBOFrag.u_poisson_scale) + "</noise_scale>\n";

	return str;
}

bool ModelSSSModule_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strParentName == "shadow_map_module")
	{
		if (strName == "bias")
			m_UBOFrag.u_bias = ct::fvariant(strValue).GetF();
		else if (strName == "noise_scale")
			m_UBOFrag.u_poisson_scale = ct::fvariant(strValue).GetF();

		NeedNewUBOUpload();
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ModelSSSModule_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBO_Frag = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBOFrag));
	m_DescriptorBufferInfo_Frag.buffer = m_UBO_Frag->buffer;
	m_DescriptorBufferInfo_Frag.range = sizeof(UBOFrag);
	m_DescriptorBufferInfo_Frag.offset = 0;

	m_EmptyTexturePtr = Texture2D::CreateEmptyTexture(m_VulkanCore, ct::uvec2(1, 1), vk::Format::eR8G8B8A8Unorm);

	for (auto& a : m_SamplerImageInfos)
	{
		a = m_EmptyTexturePtr->m_DescriptorImageInfo;
	}

	NeedNewUBOUpload();

	return true;
}

void ModelSSSModule_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCore, *m_UBO_Frag, &m_UBOFrag, sizeof(UBOFrag));
}

void ModelSSSModule_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Frag.reset();
	m_EmptyTexturePtr.reset();
}

bool ModelSSSModule_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(3U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);

	return true;
}

bool ModelSSSModule_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();
	writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, CommonSystem::Instance()->GetBufferInfo());
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Frag);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_SamplerImageInfos[0], nullptr);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 3U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_SamplerImageInfos[1], nullptr);

	return true;
}

void ModelSSSModule_Pass::UpdateRessourceDescriptor()
{
	ZoneScoped;

	m_Device.waitIdle();

	auto lightGroupPtr = m_SceneLightGroup.getValidShared();
	if (lightGroupPtr && !lightGroupPtr->empty())
	{
		auto lightPtr = lightGroupPtr->Get(0).getValidShared();
		if (lightPtr)
		{
			if (m_NeedLightGroupUpdate)
			{
				lightPtr->NeedUpdateCamera();

				m_UBOFrag.u_light_cam = lightPtr->lightDatas.lightView;

				// va provoquer une nouvel upload dans 
				// MixedMeshRenderer::UpdateRessourceDescriptor();
				// juste apres
				NeedNewUBOUpload();

				m_NeedLightGroupUpdate = false;
			}
		}
	}

	ShaderPass::UpdateRessourceDescriptor();
}

std::string ModelSSSModule_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ModelSSSModule_Pass_Vertex";

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

std::string ModelSSSModule_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ModelSSSModule_Pass";

	auto shader_path = FileHelper::Instance()->GetAppPath() + "/shaders/" + vOutShaderName + ".frag";

	if (FileHelper::Instance()->IsFileExist(shader_path))
	{
		m_FragmentShaderCode = FileHelper::Instance()->LoadFileToString(shader_path);
	}
	else
	{
		m_FragmentShaderCode = u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragSSS;
layout(location = 0) in vec2 v_uv;
)"
+ CommonSystem::Instance()->GetBufferObjectStructureHeader(0U) +
u8R"(
layout (std140, binding = 1) uniform UBO_Frag 
{ 
	mat4 u_light_cam;
	float u_bias;
	float u_poisson_scale;
	float use_sampler_pos;
	float use_sampler_shadow_map;
};
layout(binding = 2) uniform sampler2D position_map_sampler;
layout(binding = 3) uniform sampler2D light_shadow_map_sampler;

const vec2 poissonDisk[16] = vec2[]
( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

float random(vec3 seed, int i)
{
	vec4 seed4 = vec4(seed, i);
	float dot_product = dot(seed4, vec4(12.9898, 78.233, 45.164, 94.673));
	return fract(sin(dot_product) * 43758.5453);
}

void main() 
{
	fragSSS = vec4(0);
	
	if (use_sampler_pos > 0.5 && 
		use_sampler_shadow_map > 0.5)
	{
		vec3 pos = texture(position_map_sampler, v_uv).xyz;
		if (dot(pos, pos) > 0.0)
		{
			vec4 shadowCoord = u_light_cam * vec4(pos, 1.0);
			shadowCoord.xyz /= shadowCoord.w;
			shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;

			const float poisson_scale = max(u_poisson_scale, 1.0); // for div by zero
			const float bias = u_bias * 0.01;

			float sha_vis = 1.0;
			float sha_step = 1.0 / 16.0;
			for (int i=0;i<8;i++)
			{
				int index = int(16.0 * random(gl_FragCoord.xyy, i)) % 16;
				float sha = texture(light_shadow_map_sampler, shadowCoord.xy + poissonDisk[index] / poisson_scale).r;
				if (sha * cam_far < (shadowCoord.z - bias)/shadowCoord.w)
				{
					sha_vis -= sha_step * (1.0 - sha);
				}
			}

			fragSSS = vec4(sha_vis);
		}
		else
		{
			discard;
		}
	}	
	else
	{
		discard;
	}
}
)";
		FileHelper::Instance()->SaveStringToFile(m_FragmentShaderCode, shader_path);
	}

	return m_FragmentShaderCode;
}
