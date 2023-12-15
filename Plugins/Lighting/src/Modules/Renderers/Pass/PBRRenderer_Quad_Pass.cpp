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

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Gaia/Buffer/FrameBuffer.h>

using namespace GaiApi;

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

PBRRenderer_Quad_Pass::PBRRenderer_Quad_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : QuadShaderPass(vVulkanCore, MeshShaderPassType::PIXEL) {
    SetRenderDocDebugName("Quad Pass 1 : PBR", QUAD_SHADER_PASS_DEBUG_COLOR);

    m_DontUseShaderFilesOnDisk = true;
}

PBRRenderer_Quad_Pass::~PBRRenderer_Quad_Pass() {
    Unit();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool PBRRenderer_Quad_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    bool change = false;

    ImGui::Header("Maps");

    change |= ImGui::SliderFloatDefaultCompact(0.0f, "Diffuse Factor", &m_UBOFrag.u_diffuse_factor, 0.0f, 1.0f, 1.0f, 0.0f, "%.3f");
    change |= ImGui::SliderFloatDefaultCompact(0.0f, "Metallic Factor", &m_UBOFrag.u_metallic_factor, 0.0f, 1.0f, 1.0f, 0.0f, "%.3f");
    change |= ImGui::SliderFloatDefaultCompact(0.0f, "Rugosity Factor", &m_UBOFrag.u_rugosity_factor, 0.0f, 1.0f, 1.0f, 0.0f, "%.3f");
    change |= ImGui::SliderFloatDefaultCompact(0.0f, "AO Factor", &m_UBOFrag.u_ao_factor, 0.000f, 1.0f, 1.000f, 0.0f, "%.3f");

    ImGui::Header("Lights");

    change |=
        ImGui::SliderFloatDefaultCompact(0.0f, "Light Intensity Factor", &m_UBOFrag.u_light_intensity_factor, 0.0f, 200.0f, 100.0f, 0.0f, "%.3f");

    ImGui::Header("Shadow");

    change |= ImGui::SliderFloatDefaultCompact(0.0f, "Shadow Strength", &m_UBOFrag.u_shadow_strength, 0.000f, 1.000f, 0.5f, 0.0f, "%.3f");
    change |= ImGui::CheckBoxFloatDefault("Use PCF Filtering", &m_UBOFrag.u_use_pcf, true);
    if (m_UBOFrag.u_use_pcf > 0.5f) {
        ImGui::Indent();
        change |= ImGui::SliderFloatDefaultCompact(0.0f, "Bias", &m_UBOFrag.u_bias, 0.0f, 0.02f, 0.01f);
        change |= ImGui::SliderFloatDefaultCompact(0.0f, "Noise Scale", &m_UBOFrag.u_poisson_scale, 1.0f, 10000.0f, 2000.0f);
        ImGui::Unindent();
    }

    /*
    DrawInputTexture(m_VulkanCore, "Position", 0U, m_OutputRatio);
    DrawInputTexture(m_VulkanCore, "Normal", 1U, m_OutputRatio);
    DrawInputTexture(m_VulkanCore, "Albedo", 2U, m_OutputRatio);
    DrawInputTexture(m_VulkanCore, "Diffuse", 3U, m_OutputRatio);
    DrawInputTexture(m_VulkanCore, "Specular", 4U, m_OutputRatio);
    DrawInputTexture(m_VulkanCore, "Attenuation", 5U, m_OutputRatio);
    DrawInputTexture(m_VulkanCore, "Mask", 6U, m_OutputRatio);
    DrawInputTexture(m_VulkanCore, "Ao", 7U, m_OutputRatio);
    DrawInputTexture(m_VulkanCore, "shadow", 8U, m_OutputRatio);
    */

    if (change) {
        NeedNewUBOUpload();
    }

    return false;
}

bool PBRRenderer_Quad_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool PBRRenderer_Quad_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void PBRRenderer_Quad_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
    ZoneScoped;

    if (m_Loaded) {
        if (vBindingPoint < m_ImageInfos.size()) {
            if (vImageInfo) {
                m_ImageInfos[vBindingPoint] = *vImageInfo;

                if ((&m_UBOFrag.use_sampler_position)[vBindingPoint] < 1.0f) {
                    (&m_UBOFrag.use_sampler_position)[vBindingPoint] = 1.0f;
                    NeedNewUBOUpload();
                }
            } else {
                if ((&m_UBOFrag.use_sampler_position)[vBindingPoint] > 0.0f) {
                    (&m_UBOFrag.use_sampler_position)[vBindingPoint] = 0.0f;
                    NeedNewUBOUpload();
                }

                auto corePtr = m_VulkanCore.lock();
                assert(corePtr != nullptr);

                m_ImageInfos[vBindingPoint] = *corePtr->getEmptyTexture2DDescriptorImageInfo();
            }
        }
    }
}

vk::DescriptorImageInfo* PBRRenderer_Quad_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    if (m_FrameBufferPtr) {
        AutoResizeBuffer(std::dynamic_pointer_cast<OutputSizeInterface>(m_FrameBufferPtr).get(), vOutSize);

        return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
    }

    return nullptr;
}

void PBRRenderer_Quad_Pass::SetTextures(const uint32_t& vBindingPoint, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes) {
    ZoneScoped;

    if (m_Loaded) {
        if (vBindingPoint == 0U) {
            if (vImageInfos && vImageInfos->size() == m_ImageGroupInfos.size()) {
                for (size_t i = 0U; i < vImageInfos->size(); ++i) {
                    m_ImageGroupInfos[i] = vImageInfos->at(i);
                }

                if (m_UBOFrag.use_sampler_position < 1.0f) {
                    m_UBOFrag.use_sampler_shadow_maps = 1.0f;

                    NeedNewUBOUpload();
                }
            } else {
                auto corePtr = m_VulkanCore.lock();
                assert(corePtr != nullptr);

                for (auto& info : m_ImageGroupInfos) {
                    info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
                }

                if (m_UBOFrag.use_sampler_position > 0.0f) {
                    m_UBOFrag.use_sampler_position = 0.0f;

                    NeedNewUBOUpload();
                }
            }
        }
    }
}

void PBRRenderer_Quad_Pass::SetLightGroup(SceneLightGroupWeak vSceneLightGroup) {
    m_SceneLightGroup = vSceneLightGroup;

    m_SceneLightGroupDescriptorInfoPtr = &m_SceneEmptyLightGroupDescriptorInfo;

    auto lightGroupPtr = m_SceneLightGroup.lock();
    if (lightGroupPtr && lightGroupPtr->GetBufferInfo()) {
        m_SceneLightGroupDescriptorInfoPtr = lightGroupPtr->GetBufferInfo();
    }

    UpdateBufferInfoInRessourceDescriptor();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string PBRRenderer_Quad_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;

    str += vOffset + "<bias>" + ct::toStr(m_UBOFrag.u_bias) + "</bias>\n";
    str += vOffset + "<strength>" + ct::toStr(m_UBOFrag.u_shadow_strength) + "</strength>\n";
    str += vOffset + "<noise_scale>" + ct::toStr(m_UBOFrag.u_poisson_scale) + "</noise_scale>\n";
    str += vOffset + "<use_pcf>" + (m_UBOFrag.u_use_pcf > 0.5f ? "true" : "false") + "</use_pcf>\n";

    return str;
}

bool PBRRenderer_Quad_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "pbr_renderer_module") {
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

bool PBRRenderer_Quad_Pass::CreateUBO() {
    ZoneScoped;

    auto size_in_bytes = sizeof(UBOFrag);
    m_UBOFragPtr = VulkanRessource::createUniformBufferObject(m_VulkanCore, size_in_bytes, "PBRRenderer_Quad_Pass");
    m_DescriptorBufferInfo_Frag.buffer = m_UBOFragPtr->buffer;
    m_DescriptorBufferInfo_Frag.range = size_in_bytes;
    m_DescriptorBufferInfo_Frag.offset = 0;

    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);

    for (auto& info : m_ImageInfos) {
        info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
    }

    for (auto& info : m_ImageGroupInfos) {
        info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
    }

    NeedNewUBOUpload();

    return true;
}

void PBRRenderer_Quad_Pass::UploadUBO() {
    ZoneScoped;

    VulkanRessource::upload(m_VulkanCore, m_UBOFragPtr, &m_UBOFrag, sizeof(UBOFrag));
}

void PBRRenderer_Quad_Pass::DestroyUBO() {
    ZoneScoped;

    m_UBOFragPtr.reset();
}

bool PBRRenderer_Quad_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(4U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(5U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(6U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(7U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eFragment);

    // the shadow maps
    res &= AddOrSetLayoutDescriptor(
        8U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, (uint32_t)m_ImageGroupInfos.size());

    return res;
}

bool PBRRenderer_Quad_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo());
    res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_DescriptorBufferInfo_Frag);
    res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]);  // position
    res &= AddOrSetWriteDescriptorImage(3U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[1]);  // normal
    res &= AddOrSetWriteDescriptorImage(4U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[2]);  // albedo
    res &= AddOrSetWriteDescriptorImage(5U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[3]);  // mask
    res &= AddOrSetWriteDescriptorImage(6U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[4]);  // ssaao
    res &= AddOrSetWriteDescriptorBuffer(7U, vk::DescriptorType::eStorageBuffer, m_SceneLightGroupDescriptorInfoPtr);

    // the shadow maps
    res &= AddOrSetWriteDescriptorImage(8U, vk::DescriptorType::eCombinedImageSampler, m_ImageGroupInfos.data(), (uint32_t)m_ImageGroupInfos.size());

    return res;
}

std::string PBRRenderer_Quad_Pass::GetVertexShaderCode(std::string& vOutShaderName) {
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

std::string PBRRenderer_Quad_Pass::GetFragmentShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "PBRRenderer_Quad_Pass_Fragment";

    return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 v_uv;
)" + CommonSystem::GetBufferObjectStructureHeader(0U) +
           u8R"(
layout (std140, binding = 1) uniform UBO_Frag 
{ 
	float u_light_intensity_factor;
	float u_shadow_strength;
	float u_bias;
	float u_poisson_scale;
	float u_use_pcf;
	float u_diffuse_factor;
	float u_metallic_factor;
	float u_rugosity_factor;
	float u_ao_factor;
	float use_sampler_position;		// position
	float use_sampler_normal;		// normal
	float use_sampler_albedo;		// albedo
	float use_sampler_mask;			// mask
	float use_sampler_ao;			// ao
	float use_sampler_shadow_maps;	// shadow maps
};

layout(binding = 2) uniform sampler2D position_map_sampler;
layout(binding = 3) uniform sampler2D normal_map_sampler;
layout(binding = 4) uniform sampler2D albedo_map_sampler;
layout(binding = 5) uniform sampler2D mask_map_sampler;
layout(binding = 6) uniform sampler2D ao_map_sampler;
)" + SceneLightGroup::GetBufferObjectStructureHeader(7U) +
           u8R"(
layout(std430, binding = 7) readonly buffer SBO_LightGroup
{
	uint lightsCount;
	LightDatas lightDatas[];
};

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

const float PI = 3.14159265359;

float distributionGGX (vec3 N, vec3 H, float roughness)
{
    float a2    = roughness * roughness * roughness * roughness;
    float NdotH = max (dot (N, H), 0.0);
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float geometrySchlickGGX (float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith (vec3 N, vec3 V, vec3 L, float roughness)
{
    return geometrySchlickGGX (max (dot (N, L), 0.0), roughness) * 
           geometrySchlickGGX (max (dot (N, V), 0.0), roughness);
}

vec3 fresnelSchlick (float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow (1.0 - cosTheta, 5.0);
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

void main() 
{
	fragColor = vec4(0);
	
	vec3 pos = vec3(0);
	vec3 N = vec3(0);
	vec3 albedo_color = vec3(1.0);
	float ao_value = 1.0;
	
	if (use_sampler_position > 0.5)
		pos = texture(position_map_sampler, v_uv).xyz;
	
	if (dot(pos, pos) > 0.0)
	{
		if (use_sampler_normal > 0.5)
			N = normalize(texture(normal_map_sampler, v_uv).xyz * 2.0 - 1.0);
		
		if (use_sampler_albedo > 0.5)
			albedo_color = texture(albedo_map_sampler, v_uv).rgb;
		albedo_color *= u_diffuse_factor;
		
		if (use_sampler_ao > 0.5)
			ao_value = texture(ao_map_sampler, v_uv).r;
		ao_value *= u_ao_factor;
		
		// ray pos, ray dir
		mat4 imv = inverse(model * view);
		mat4 ip = inverse(proj);
		vec2 uvc = v_uv * 2.0 - 1.0;
		vec4 origin = imv * vec4(0, 0, 0, 1);
		vec4 target = ip * vec4(uvc.x, uvc.y, 1, 1);
		vec4 direction = imv * vec4(normalize(target.xyz), 0);

		vec3 ambient = 0.03 * albedo_color * ao_value;
		vec3 Lo = ambient;
	
		uint count = uint(lightsCount) % 8; // maxi 8 lights in this system
		for (uint lid = 0 ; lid < count ; ++lid)
		{
			if (lightDatas[lid].lightActive > 0.5)
			{
				vec3 lp = lightDatas[lid].lightGizmo[3].xyz;
				float li = lightDatas[lid].lightIntensity;
				vec4 lc = lightDatas[lid].lightColor;
				vec3 ld = lp - pos;
				float len = length(ld);
				vec3 L = ld / len;
				float atten = li * 100.0 / (len * len);

				// shadow
				float sha = 1.0;
				if (u_use_pcf > 0.5) sha -= getShadowPCF(lid, pos, N);
				else sha -= getShadowSimple(lid, pos, N);
				
				vec3 V = -direction.xyz;
				vec3 H = normalize(V + L);
				vec3 radiance = lc.rgb * atten; 
				
				vec3 F0 = vec3(0.04); 
				F0 = mix(F0, albedo_color, u_metallic_factor);
				vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
				float NDF = distributionGGX(N, H, u_rugosity_factor);       
				float G = geometrySmith(N, V, L, u_rugosity_factor);       
				vec3 numerator = NDF * G * F;
				float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0)  + 0.0001;
				vec3 specular = numerator / denominator;  
				
				vec3 kS = F;
				vec3 kD = vec3(1.0) - kS;
				  
				kD *= 1.0 - u_metallic_factor;	

				const float PI = 3.14159265359;
			  
				float NdotL = max(dot(N, L), 0.0);        
				vec3 cc = (kD * albedo_color / PI + specular) * radiance * NdotL;
				
				Lo += cc * sha;
			}
		}

		fragColor.rgb = Lo / float(count);
		fragColor.rgb = fragColor.rgb / (fragColor.rgb + vec3(1.0));
		fragColor.rgb = pow(fragColor.rgb, vec3(1.0/2.2)); 
		fragColor.a = 1.0;

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
