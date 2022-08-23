/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "RtxPbrRendererModule_Rtx_Pass.h"

#include <cinttypes>
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
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

RtxPbrRendererModule_Rtx_Pass::RtxPbrRendererModule_Rtx_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: RtxShaderPass(vVulkanCorePtr)
{
	ZoneScoped;

	SetRenderDocDebugName("Rtx Pass : Rtx Pbr Renderer", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	//m_DontUseShaderFilesOnDisk = true;
}

RtxPbrRendererModule_Rtx_Pass::~RtxPbrRendererModule_Rtx_Pass()
{
	ZoneScoped;

	Unit();
}

void RtxPbrRendererModule_Rtx_Pass::ActionBeforeCompilation()
{
	AddShaderCode(CompilShaderCode(vk::ShaderStageFlagBits::eRaygenKHR, "main"), "main");
	AddShaderCode(CompilShaderCode(vk::ShaderStageFlagBits::eMissKHR, "main"), "main");
	AddShaderCode(CompilShaderCode(vk::ShaderStageFlagBits::eClosestHitKHR, "main"), "main");
}

void RtxPbrRendererModule_Rtx_Pass::ActionBeforeInit()
{
	ZoneScoped;

	//m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);

	for (auto& info : m_ImageInfos)
	{
		info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
	}
}

bool RtxPbrRendererModule_Rtx_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	bool change = false;

	change |= DrawResizeWidget();

	change |= ImGui::SliderFloatDefaultCompact(0.0f, "Diffuse Factor", &m_UBO_Chit.u_diffuse_factor, 0.0f, 1.0f, 1.0f, 0.0f, "%.3f");
	change |= ImGui::SliderFloatDefaultCompact(0.0f, "Metallic Factor", &m_UBO_Chit.u_metallic_factor, 0.0f, 1.0f, 1.0f, 0.0f, "%.3f");
	change |= ImGui::SliderFloatDefaultCompact(0.0f, "Rugosity Factor", &m_UBO_Chit.u_rugosity_factor, 0.0f, 1.0f, 1.0f, 0.0f, "%.3f");
	change |= ImGui::SliderFloatDefaultCompact(0.0f, "AO Factor", &m_UBO_Chit.u_ao_factor, 0.000f, 1.0f, 1.000f, 0.0f, "%.3f");
	change |= ImGui::SliderFloatDefaultCompact(0.0f, "Light Intensity Factor", &m_UBO_Chit.u_light_intensity_factor, 0.0f, 200.0f, 100.0f, 0.0f, "%.3f");
	change |= ImGui::SliderFloatDefaultCompact(0.0f, "Shadow Strength", &m_UBO_Chit.u_shadow_strength, 0.000f, 1.000f, 0.5f, 0.0f, "%.3f");

	if (change)
	{
		NeedNewUBOUpload();
	}

	return change;
}

void RtxPbrRendererModule_Rtx_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
}

void RtxPbrRendererModule_Rtx_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// ACCEL STRUCTURE INPUT ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void RtxPbrRendererModule_Rtx_Pass::SetAccelStructure(SceneAccelStructureWeak vSceneAccelStructure)
{	
	ZoneScoped;

	m_SceneAccelStructure = vSceneAccelStructure;

	UpdateBufferInfoInRessourceDescriptor();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// LIGHT GROUP INPUT ///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void RtxPbrRendererModule_Rtx_Pass::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{	
	ZoneScoped;

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

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void RtxPbrRendererModule_Rtx_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBindingPoint < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				m_ImageInfos[vBindingPoint] = *vImageInfo;

				if ((&m_UBO_Chit.u_use_albedo_map)[vBindingPoint] < 1.0f)
				{
					(&m_UBO_Chit.u_use_albedo_map)[vBindingPoint] = 1.0f;

					NeedNewUBOUpload();
				}
			}
			else
			{
				if ((&m_UBO_Chit.u_use_albedo_map)[vBindingPoint] < 1.0f)
				{
					(&m_UBO_Chit.u_use_albedo_map)[vBindingPoint] = 1.0f;

					NeedNewUBOUpload();
				}

				m_ImageInfos[vBindingPoint] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* RtxPbrRendererModule_Rtx_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;
	if (m_ComputeBufferPtr)
	{
		if (vOutSize)
		{
			*vOutSize = m_ComputeBufferPtr->GetOutputSize();
		}

		return m_ComputeBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RtxPbrRendererModule_Rtx_Pass::WasJustResized()
{
	ZoneScoped;
}

bool RtxPbrRendererModule_Rtx_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBO_Chit_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Chit));
	m_UBO_Chit_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_Chit_Ptr)
	{
		m_UBO_Chit_BufferInfos.buffer = m_UBO_Chit_Ptr->buffer;
		m_UBO_Chit_BufferInfos.range = sizeof(UBO_Chit);
		m_UBO_Chit_BufferInfos.offset = 0;
	}

	NeedNewUBOUpload();

	return true;
}

void RtxPbrRendererModule_Rtx_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_Chit_Ptr, &m_UBO_Chit, sizeof(UBO_Chit));
}

void RtxPbrRendererModule_Rtx_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Chit_Ptr.reset();
	m_UBO_Chit_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
}

bool RtxPbrRendererModule_Rtx_Pass::CanUpdateDescriptors()
{
	ZoneScoped;

	if (!m_SceneAccelStructure.expired())
	{
		auto accelStructurePtr = m_SceneAccelStructure.getValidShared();
		if (accelStructurePtr)
		{
			return accelStructurePtr->IsOk();
		}
	}

	return false;
}

bool RtxPbrRendererModule_Rtx_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eRaygenKHR);
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eAccelerationStructureKHR, vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR);
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eRaygenKHR);
	res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR);
	res &= AddOrSetLayoutDescriptor(4U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR);
	res &= AddOrSetLayoutDescriptor(5U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eClosestHitKHR);
	res &= AddOrSetLayoutDescriptor(6U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eClosestHitKHR); // albedo
	res &= AddOrSetLayoutDescriptor(7U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eClosestHitKHR); // ao
	res &= AddOrSetLayoutDescriptor(8U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eClosestHitKHR); // longlat
	
	return res;
}

bool RtxPbrRendererModule_Rtx_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;

	if (res)
	{
		res = false;

		auto accelStructurePtr = m_SceneAccelStructure.getValidShared();
		if (accelStructurePtr &&
			accelStructurePtr->GetTLASInfo() &&
			accelStructurePtr->GetBufferAddressInfo())
		{
			res = true;

			res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U));
			res &= AddOrSetWriteDescriptorNext(1U, vk::DescriptorType::eAccelerationStructureKHR, accelStructurePtr->GetTLASInfo());
			res &= AddOrSetWriteDescriptorBuffer(2U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo()); // camera
			res &= AddOrSetWriteDescriptorBuffer(3U, vk::DescriptorType::eStorageBuffer, accelStructurePtr->GetBufferAddressInfo()); // model device address
			res &= AddOrSetWriteDescriptorBuffer(4U, vk::DescriptorType::eStorageBuffer, m_SceneLightGroupDescriptorInfoPtr);
			res &= AddOrSetWriteDescriptorBuffer(5U, vk::DescriptorType::eUniformBuffer, &m_UBO_Chit_BufferInfos);
			res &= AddOrSetWriteDescriptorImage(6U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]); // albedo
			res &= AddOrSetWriteDescriptorImage(7U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[1]); // ao
			res &= AddOrSetWriteDescriptorImage(8U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[2]); // longlat
		}
	}

	return res;
}

std::string RtxPbrRendererModule_Rtx_Pass::GetHitPayLoadCode()
{
	return u8R"(
struct hitPayload
{
	float sha;
	vec3 ro;
	vec3 rd;
	vec4 color;
};
)";
}

std::string RtxPbrRendererModule_Rtx_Pass::GetRayGenerationShaderCode(std::string& vOutShaderName)
{
	ZoneScoped;

	vOutShaderName = "RtxPbrRendererModule_Rtx_Pass_RGen";
	return u8R"(
#version 460
#extension GL_EXT_ray_tracing : enable

layout(binding = 0, rgba32f) uniform writeonly image2D out_color;
layout(binding = 1) uniform accelerationStructureEXT tlas;
)"
+ 
CommonSystem::GetBufferObjectStructureHeader(2U) 
+
GetHitPayLoadCode()
+
u8R"(
layout(location = 0) rayPayloadEXT hitPayload prd;

void main()
{
	const vec2 p = vec2(gl_LaunchIDEXT.xy);
	const vec2 s = vec2(gl_LaunchSizeEXT.xy);

	const vec2 pc = p + 0.5; // pixel center
	const vec2 uv = pc / s;
	const vec2 uvc = uv * 2.0 - 1.0;
	
	mat4 imv = inverse(model * view);
	mat4 ip = inverse(proj);

	vec4 origin    = imv * vec4(0, 0, 0, 1);
	vec4 target    = ip * vec4(uvc.x, uvc.y, 1, 1);
	vec4 direction = imv * vec4(normalize(target.xyz), 0);

	prd.ro = origin.xyz;
	prd.rd = direction.xyz;
	prd.color = vec4(0.0);
	
	float tmin = 0.001;
	float tmax = 1e32;

	uint flags = gl_RayFlagsOpaqueEXT;

	traceRayEXT(tlas,			// acceleration structure
		        flags,			// rayFlags
		        0xFF,			// cullMask
		        0,				// sbtRecordOffset
		        0,				// sbtRecordStride
		        0,				// missIndex
		        prd.ro,			// ray origin
		        tmin,			// ray min range
		        prd.rd,			// ray direction
		        tmax,			// ray max range
		        0				// payload location
	);

	imageStore(out_color, ivec2(gl_LaunchIDEXT.xy), prd.color);
}
)";
}

std::string RtxPbrRendererModule_Rtx_Pass::GetRayIntersectionShaderCode(std::string& vOutShaderName)
{
	ZoneScoped;

	vOutShaderName = "RtxPbrRendererModule_Rtx_Pass_Inter";
	return u8R"(

)";
}

std::string RtxPbrRendererModule_Rtx_Pass::GetRayMissShaderCode(std::string& vOutShaderName)
{
	ZoneScoped;

	vOutShaderName = "RtxPbrRendererModule_Rtx_Pass_Miss";
	return u8R"(
#version 460
#extension GL_EXT_ray_tracing : enable
)"
+
GetHitPayLoadCode()
+
u8R"(
layout(location = 0) rayPayloadInEXT hitPayload prd;
layout(location = 1) rayPayloadEXT bool isShadowed;

void main()
{
	prd.sha = 0.0;
	isShadowed = false;
}
)";
}

std::string RtxPbrRendererModule_Rtx_Pass::GetRayAnyHitShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "RtxPbrRendererModule_Rtx_Pass_Ahit";
	return u8R"(

)";
}

std::string RtxPbrRendererModule_Rtx_Pass::GetRayClosestHitShaderCode(std::string& vOutShaderName)
{
	ZoneScoped;

	vOutShaderName = "RtxPbrRendererModule_Rtx_Pass_Chit";
	return u8R"(
#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
)"
+
GetHitPayLoadCode()
+
u8R"(
layout(location = 0) rayPayloadInEXT hitPayload prd;
layout(location = 1) rayPayloadEXT bool isShadowed;

hitAttributeEXT vec3 attribs;

struct V3N3T3B3T2C4 
{
	float px, py, pz;
	float nx, ny, nz;
	float tax, tay, taz;
	float btax, btay, btaz;
	float tx, ty;
	float cx, cy, cz, cw;
};

layout(buffer_reference, scalar) readonly buffer Vertices
{
	V3N3T3B3T2C4 vdatas[];
};

layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Indices
{
	uvec3 idatas[];
};

layout(binding = 1) uniform accelerationStructureEXT tlas; // same as raygen shader

struct SceneMeshBuffers
{
	uint64_t vertices;
	uint64_t indices;
};

layout(binding = 3) buffer ModelAddresses 
{ 
	SceneMeshBuffers datas[]; 
} sceneMeshBuffers;
)"
+
SceneLightGroup::GetBufferObjectStructureHeader(4U)
+
u8R"(
layout(std140, binding = 5) uniform UBO_Chit
{
	float u_diffuse_factor;
	float u_metallic_factor;
	float u_rugosity_factor;
	float u_ao_factor;
	float u_use_albedo_map;
	float u_use_ao_map;
	float u_use_longlat_map;
	float u_light_intensity_factor;
	float u_shadow_strength;
};

layout(binding = 6) uniform sampler2D albedo_map_sampler;
layout(binding = 7) uniform sampler2D ao_map_sampler;
layout(binding = 8) uniform sampler2D longlat_map_sampler;

float getShadowValue(vec3 p, vec3 ld)
{
	isShadowed = true; 

	const float tmin = 0.001;
	const float tmax = 1e32;

	uint flags  = 
		gl_RayFlagsTerminateOnFirstHitEXT | 
		gl_RayFlagsOpaqueEXT | 
		gl_RayFlagsSkipClosestHitShaderEXT;

	traceRayEXT(tlas,		// acceleration structure
		        flags,		// rayFlags
		        0xFF,		// cullMask
		        0,			// sbtRecordOffset
		        0,			// sbtRecordStride
		        0,			// missIndex
		        p,			// ray origin
		        tmin,		// ray min range
		        ld,			// ray direction
		        tmax,		// ray max range
		        1			// payload (location = 1)
	);

	if (isShadowed)
	{
		return clamp(1.0 - u_shadow_strength, 0.0, 1.0);
	}	
	
	return 1.0;
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

void main()
{
	const vec2 p = vec2(gl_LaunchIDEXT.xy);
	const vec2 s = vec2(gl_LaunchSizeEXT.xy);

	const vec2 pc = p + 0.5; // pixel center
	const vec2 uv = pc / s;
	const vec2 uvc = uv * 2.0 - 1.0;
	
	// albedo // diffuse
	vec3 albedo_color = vec3(1.0);
	if (u_use_albedo_map > 0.5)
		albedo_color = texture(albedo_map_sampler, uv).rgb;
	albedo_color *= u_diffuse_factor;
	
	// Ambiant Occlusion
	float ao_value = 1.0;
	if (u_use_ao_map > 0.5)
		ao_value = texture(ao_map_sampler, uv).r;
	ao_value *= u_ao_factor;
	
	//vec3 albedo_color = texture(longlat_map_sampler, uv).rgb;
	
	// When contructing the TLAS, we stored the model id in InstanceCustomIndexEXT, so the
	// the instance can quickly have access to the data

	// Object data
	SceneMeshBuffers meshRes = sceneMeshBuffers.datas[gl_InstanceCustomIndexEXT];
	Indices indices = Indices(meshRes.indices);
	Vertices vertices = Vertices(meshRes.vertices);

	// Indices of the triangle
	uvec3 ind = indices.idatas[gl_PrimitiveID];

	// Vertex of the triangle
	V3N3T3B3T2C4 v0 = vertices.vdatas[ind.x];
	V3N3T3B3T2C4 v1 = vertices.vdatas[ind.y];
	V3N3T3B3T2C4 v2 = vertices.vdatas[ind.z];

	// Barycentric coordinates of the triangle
	const vec3 barycentrics = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);

	vec3 model_pos = 
		vec3(v0.px, v0.py, v0.pz) * barycentrics.x + 
		vec3(v1.px, v1.py, v1.pz) * barycentrics.y + 
		vec3(v2.px, v2.py, v2.pz) * barycentrics.z;
    vec3 model_normal = 
		vec3(v0.nx, v0.ny, v0.nz) * barycentrics.x + 
		vec3(v1.nx, v1.ny, v1.nz) * barycentrics.y + 
		vec3(v2.nx, v2.ny, v2.nz) * barycentrics.z;
    vec3 model_color = 
		vec3(v0.cx, v0.cy, v0.cz) * barycentrics.x + 
		vec3(v1.cx, v1.cy, v1.cz) * barycentrics.y + 
		vec3(v2.cx, v2.cy, v2.cz) * barycentrics.z;
    
	// world normal to object space
	vec3 N = normalize(vec3(model_normal * gl_ObjectToWorldEXT)); 
	//vec3 N = normalize(vec3(model_normal * gl_WorldToObjectEXT)); 

	vec3 ambient = 0.03 * albedo_color * ao_value;
	vec3 Lo = ambient;
	
	uint count = lightsCount % 8; // maxi 8 lights in this system
	for (uint lid = 0 ; lid < count ; ++lid)
	{
		if (lightDatas[lid].lightActive > 0.5)
		{
			vec3 lp = lightDatas[lid].lightGizmo[3].xyz;
			float li = lightDatas[lid].lightIntensity;
			vec4 lc = lightDatas[lid].lightColor;
			vec3 ld = lp - model_pos;
			float len = length(ld);
			vec3 L = ld / len;
			float atten = li * u_light_intensity_factor / (len * len);

			prd.sha = getShadowValue(model_pos, L);
			
			vec3 V = -prd.rd;
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
			
			Lo += cc * prd.sha;
		}
	}

	prd.color /= float(lightsCount);
	prd.color.a = 1.0;
	
	prd.color.rgb = Lo / float(count);
	prd.color.rgb = prd.color.rgb / (prd.color.rgb + vec3(1.0));
	prd.color.rgb = pow(prd.color.rgb, vec3(1.0/2.2)); 
	prd.color.a = 1.0;
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string RtxPbrRendererModule_Rtx_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += ShaderPass::getXml(vOffset, vUserDatas);

	str += vOffset + "<diffuse_factor>" + ct::toStr(m_UBO_Chit.u_diffuse_factor) + "</diffuse_factor>\n";
	str += vOffset + "<metallic_factor>" + ct::toStr(m_UBO_Chit.u_metallic_factor) + "</metallic_factor>\n";
	str += vOffset + "<rugosity_factor>" + ct::toStr(m_UBO_Chit.u_rugosity_factor) + "</rugosity_factor>\n";
	str += vOffset + "<ao_factor>" + ct::toStr(m_UBO_Chit.u_ao_factor) + "</ao_factor>\n";
	str += vOffset + "<light_intensity_factor>" + ct::toStr(m_UBO_Chit.u_light_intensity_factor) + "</light_intensity_factor>\n";
	str += vOffset + "<shadow_strength>" + ct::toStr(m_UBO_Chit.u_shadow_strength) + "</shadow_strength>\n";
	
	return str;
}

bool RtxPbrRendererModule_Rtx_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	ZoneScoped;

	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	ShaderPass::setFromXml(vElem, vParent, vUserDatas);

	if (strParentName == "rtx_pbr_renderer_module")
	{
		if (strName == "diffuse_factor")
			m_UBO_Chit.u_diffuse_factor = ct::fvariant(strValue).GetF();
		else if (strName == "metallic_factor")
			m_UBO_Chit.u_metallic_factor = ct::fvariant(strValue).GetF();
		else if (strName == "rugosity_factor")
			m_UBO_Chit.u_rugosity_factor = ct::fvariant(strValue).GetF();
		else if (strName == "ao_factor")
			m_UBO_Chit.u_ao_factor = ct::fvariant(strValue).GetF();
		else if (strName == "light_intensity_factor")
			m_UBO_Chit.u_light_intensity_factor = ct::fvariant(strValue).GetF();
		else if (strName == "shadow_strength")
			m_UBO_Chit.u_shadow_strength = ct::fvariant(strValue).GetF();
	}

	return true;
}

void RtxPbrRendererModule_Rtx_Pass::AfterNodeXmlLoading()
{
	ZoneScoped;

	// code to do after end of the xml loading of this node
	// by ex :
	NeedNewUBOUpload();
}
