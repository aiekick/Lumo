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

#include "PBRRenderer_Quad_Pass.h"

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

PBRRenderer_Quad_Pass::PBRRenderer_Quad_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: QuadShaderPass(vVulkanCorePtr, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Quad Pass 1 : PBR", QUAD_SHADER_PASS_DEBUG_COLOR);

	//m_DontUseShaderFilesOnDisk = true;
}

PBRRenderer_Quad_Pass::~PBRRenderer_Quad_Pass()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool PBRRenderer_Quad_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	/*DrawInputTexture(m_VulkanCorePtr, "Position", 0U, m_OutputRatio);
	DrawInputTexture(m_VulkanCorePtr, "Normal", 1U, m_OutputRatio);
	DrawInputTexture(m_VulkanCorePtr, "Albedo", 2U, m_OutputRatio);
	DrawInputTexture(m_VulkanCorePtr, "Diffuse", 3U, m_OutputRatio);
	DrawInputTexture(m_VulkanCorePtr, "Specular", 4U, m_OutputRatio);
	DrawInputTexture(m_VulkanCorePtr, "Attenuation", 5U, m_OutputRatio);
	DrawInputTexture(m_VulkanCorePtr, "Mask", 6U, m_OutputRatio);
	DrawInputTexture(m_VulkanCorePtr, "Ao", 7U, m_OutputRatio);
	DrawInputTexture(m_VulkanCorePtr, "shadow", 8U, m_OutputRatio);*/

	return false;
}

void PBRRenderer_Quad_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void PBRRenderer_Quad_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void PBRRenderer_Quad_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				m_ImageInfos[vBinding] = *vImageInfo;

				if ((&m_UBOFrag.use_sampler_position)[vBinding] < 1.0f)
				{
					(&m_UBOFrag.use_sampler_position)[vBinding] = 1.0f;
					NeedNewUBOUpload();
				}
			}
			else
			{
				if ((&m_UBOFrag.use_sampler_position)[vBinding] > 0.0f)
				{
					(&m_UBOFrag.use_sampler_position)[vBinding] = 0.0f;
					NeedNewUBOUpload();
				}
				
				m_ImageInfos[vBinding] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* PBRRenderer_Quad_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

void PBRRenderer_Quad_Pass::SetTextures(const uint32_t& vBinding, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding == 0U)
		{
			if (vImageInfos &&
				vImageInfos->size() == m_ImageGroupInfos.size())
			{
				for (size_t i = 0U; i < vImageInfos->size(); ++i)
				{
					m_ImageGroupInfos[i] = vImageInfos->at(i);
				}

				if (m_UBOFrag.use_sampler_position < 1.0f)
				{
					m_UBOFrag.use_sampler_shadow_maps = 1.0f;

					NeedNewUBOUpload();
				}
			}
			else
			{
				for (auto& info : m_ImageGroupInfos)
				{
					info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
				}

				if (m_UBOFrag.use_sampler_position > 0.0f)
				{
					m_UBOFrag.use_sampler_position = 0.0f;

					NeedNewUBOUpload();
				}
			}
		}
	}
}

void PBRRenderer_Quad_Pass::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	m_SceneLightGroup = vSceneLightGroup;

	m_SceneLightGroupDescriptorInfoPtr = &m_SceneEmptyLightGroupDescriptorInfo;

	auto lightGroupPtr = m_SceneLightGroup.getValidShared();
	if (lightGroupPtr &&
		lightGroupPtr->GetBufferInfo())
	{
		m_SceneLightGroupDescriptorInfoPtr = lightGroupPtr->GetBufferInfo();
	}

	UpdateBufferInfoInRessourceDescriptor();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string PBRRenderer_Quad_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += vOffset + "<bias>" + ct::toStr(m_UBOFrag.u_bias) + "</bias>\n";
	str += vOffset + "<strength>" + ct::toStr(m_UBOFrag.u_shadow_strength) + "</strength>\n";
	str += vOffset + "<noise_scale>" + ct::toStr(m_UBOFrag.u_poisson_scale) + "</noise_scale>\n";
	str += vOffset + "<use_pcf>" + (m_UBOFrag.u_use_pcf > 0.5f ? "true" : "false") + "</use_pcf>\n";

	return str;
}

bool PBRRenderer_Quad_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "pbr_renderer_module")
	{
		if (strName == "bias")
			m_UBOFrag.u_bias = ct::fvariant(strValue).GetF();
		else if (strName == "strength")
			m_UBOFrag.u_shadow_strength = ct::fvariant(strValue).GetF();
		else if (strName == "noise_scale")
			m_UBOFrag.u_poisson_scale = ct::fvariant(strValue).GetF();
		else if (strName == "use_pcf")
			m_UBOFrag.u_use_pcf = ct::ivariant(strValue).GetB() ? 1.0f : 0.0f;

		NeedNewUBOUpload();
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PBRRenderer_Quad_Pass::CreateUBO()
{
	ZoneScoped;

	auto size_in_bytes = sizeof(UBOFrag);
	m_UBOFragPtr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, size_in_bytes);
	m_DescriptorBufferInfo_Frag.buffer = m_UBOFragPtr->buffer;
	m_DescriptorBufferInfo_Frag.range = size_in_bytes;
	m_DescriptorBufferInfo_Frag.offset = 0;

	for (auto& info : m_ImageInfos)
	{
		info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
	}

	for (auto& info : m_ImageGroupInfos)
	{
		info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
	}

	NeedNewUBOUpload();

	return true;
}

void PBRRenderer_Quad_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBOFragPtr, &m_UBOFrag, sizeof(UBOFrag));
}

void PBRRenderer_Quad_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBOFragPtr.reset();
}

bool PBRRenderer_Quad_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(3U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(4U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(5U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(6U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(7U, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eFragment);

	// the shadow maps
	m_LayoutBindings.emplace_back(8U, vk::DescriptorType::eCombinedImageSampler,
		(uint32_t)m_ImageGroupInfos.size(), vk::ShaderStageFlagBits::eFragment);

	return true;
}

bool PBRRenderer_Quad_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();
	writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, CommonSystem::Instance()->GetBufferInfo());
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Frag);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], nullptr); // position
	writeDescriptorSets.emplace_back(m_DescriptorSet, 3U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[1], nullptr); // normal
	writeDescriptorSets.emplace_back(m_DescriptorSet, 4U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[2], nullptr); // albedo
	writeDescriptorSets.emplace_back(m_DescriptorSet, 5U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[3], nullptr); // mask
	writeDescriptorSets.emplace_back(m_DescriptorSet, 6U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[4], nullptr); // ssaao
	writeDescriptorSets.emplace_back(m_DescriptorSet, 7U, 0, 1, vk::DescriptorType::eStorageBuffer, nullptr, m_SceneLightGroupDescriptorInfoPtr);

	// the shadow maps
	writeDescriptorSets.emplace_back(m_DescriptorSet, 8U, 0,
		(uint32_t)m_ImageGroupInfos.size(), vk::DescriptorType::eCombinedImageSampler, m_ImageGroupInfos.data(), nullptr);

	return true;
}

std::string PBRRenderer_Quad_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PBRRenderer_Quad_Pass_Vertex";

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

std::string PBRRenderer_Quad_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "PBRRenderer_Quad_Pass_Fragment";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 v_uv;
)"
+ CommonSystem::GetBufferObjectStructureHeader(0U) +
u8R"(
layout (std140, binding = 1) uniform UBO_Frag 
{ 
	float u_shadow_strength;
	float u_bias;
	float u_poisson_scale;
	float u_use_pcf;
	float use_sampler_position;		// position
	float use_sampler_normal;		// normal
	float use_sampler_albedo;		// albedo
	float use_sampler_mask;			// mask
	float use_sampler_ssao;			// ssao
	float use_sampler_shadow_maps;	// shadow maps
};

layout(binding = 2) uniform sampler2D position_map_sampler;
layout(binding = 3) uniform sampler2D normal_map_sampler;
layout(binding = 4) uniform sampler2D albedo_map_sampler;
layout(binding = 5) uniform sampler2D mask_map_sampler;
layout(binding = 6) uniform sampler2D ssao_map_sampler;
)"
+
SceneLightGroup::GetBufferObjectStructureHeader(7U)
+
u8R"(
layout(binding = 8) uniform sampler2D light_shadow_map_samplers[8]; // binding 8 + 8 => the next binding is 16

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

float getShadowPCF(uint lid, vec3 pos, vec3 nor)
{
	if (lightDatas[lid].lightActive > 0.5)
	{
		vec4 shadowCoord = lightDatas[lid].lightView * vec4(pos, 1.0);
		shadowCoord.xyz /= shadowCoord.w;
		shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;

		const float poisson_scale = max(u_poisson_scale, 1.0); // for div by zero
		const float bias = u_bias * 0.01;

		float sha_vis = 0.0;
		float sha_step = 1.0 / 16.0;
		for (int i=0;i<16;i++)
		{
			int index = int(16.0 * random(gl_FragCoord.xyy, i)) % 16;
			float sha = texture(light_shadow_map_samplers[lid], shadowCoord.xy + poissonDisk[index] / poisson_scale, 0.0).r;// * cam_far;
			if (sha < (shadowCoord.z - bias)/shadowCoord.w)
			{
				sha_vis += sha_step * (1.0 - sha);
			}
		}
		
		vec3 ld = normalize(lightDatas[lid].lightGizmo[3].xyz);
		float li = dot(ld, nor) * lightDatas[lid].lightIntensity * u_shadow_strength;
				
		return (sha_vis) * li;
	}

	return 0.0;
}

float getShadowSimple(uint lid, vec3 pos, vec3 nor)
{
	if (lightDatas[lid].lightActive > 0.5)
	{
		vec4 shadowCoord = lightDatas[lid].lightView * vec4(pos, 1.0);
		shadowCoord.xyz /= shadowCoord.w;
		shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;

		const float poisson_scale = max(u_poisson_scale, 1.0); // for div by zero
		const float bias = u_bias * 0.01;
		
		float sha = texture(light_shadow_map_samplers[lid], shadowCoord.xy, 0.0).r;
		if (sha < shadowCoord.z - bias)
		{
			vec3 ld = normalize(lightDatas[lid].lightGizmo[3].xyz);
			float li = dot(ld, nor) * lightDatas[lid].lightIntensity * u_shadow_strength;
			return (1.0 - sha) * li;
		}
	}
	
	return 0.0;
}

vec3 getRayOrigin()
{
	vec3 ro = view[3].xyz + model[3].xyz;
	ro *= mat3(view * model);
	return -ro;
}

void main() 
{
	fragColor = vec4(0);
	
	vec3 pos = vec3(0);
	vec3 nor = vec3(0);
	vec4 col = vec4(1);
	float ssao = 1.0;
	
	if (use_sampler_position > 0.5)
		pos = texture(position_map_sampler, v_uv).xyz;
	
	if (dot(pos, pos) > 0.0)
	{
		if (use_sampler_normal > 0.5)
			nor = normalize(texture(normal_map_sampler, v_uv).xyz * 2.0 - 1.0);
		if (use_sampler_albedo > 0.5)
			col = texture(albedo_map_sampler, v_uv);
		if (use_sampler_ssao > 0.5)
			ssao = texture(ssao_map_sampler, v_uv).r;
		
		// ray pos, ray dir
		vec3 ro = getRayOrigin();
		vec3 rd = normalize(ro - pos);
				
		uint count = uint(lightsCount) % 8; // maxi 8 lights in this system
		for (uint lid = 0 ; lid < count ; ++lid)
		{
			if (lightDatas[lid].lightActive > 0.5)
			{
				// light
				vec3 light_pos = lightDatas[lid].lightGizmo[3].xyz;
				float light_intensity = lightDatas[lid].lightIntensity;
				vec4 light_col = lightDatas[lid].lightColor;
				vec3 light_dir = normalize(light_pos - pos);
				
				// diffuse
				vec4 diff = min(max(dot(nor, light_dir), 0.0) * light_intensity, 1.0) * light_col;
				
				// specular
				vec3 refl = reflect(-light_dir, nor);  
				vec4 spec = min(pow(max(dot(rd, refl), 0.0), 8.0) * light_intensity, 1.0) * light_col;
				
				// shadow
				float sha = 1.0;
				if (u_use_pcf > 0.5) sha -= getShadowPCF(lid, pos, nor);
				else sha -= getShadowSimple(lid, pos, nor);
				
				fragColor += (col * diff * ssao + spec) * sha;
			}
		}

		if (use_sampler_mask > 0.5)
		{
			float mask =  texture(mask_map_sampler, v_uv).r;
			if (mask < 0.5)
			{
				discard; // a faire en dernier
			}
		}
	}
	else
	{
		discard;
	}		
}
)";
}
