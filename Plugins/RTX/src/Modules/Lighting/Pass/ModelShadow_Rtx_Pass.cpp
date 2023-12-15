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

#include "ModelShadow_Rtx_Pass.h"

#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImGuiPack.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <Gaia/Core/VulkanCommandBuffer.h>
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

#define RTX_SHADER_PASS_DEBUG_COLOR ct::fvec4(0.6f, 0.2f, 0.9f, 0.5f)

ModelShadow_Rtx_Pass::ModelShadow_Rtx_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : RtxShaderPass(vVulkanCore) {
    ZoneScoped;

    SetRenderDocDebugName("Rtx Pass : Model Shadow", RTX_SHADER_PASS_DEBUG_COLOR);

    m_DontUseShaderFilesOnDisk = true;
}

ModelShadow_Rtx_Pass::~ModelShadow_Rtx_Pass() {
    ZoneScoped;

    Unit();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void ModelShadow_Rtx_Pass::ActionBeforeCompilation() {
    AddShaderCode(CompilShaderCode(vk::ShaderStageFlagBits::eRaygenKHR, "main"), "main");
    AddShaderCode(CompilShaderCode(vk::ShaderStageFlagBits::eMissKHR, "main"), "main");
    AddShaderCode(CompilShaderCode(vk::ShaderStageFlagBits::eClosestHitKHR, "main"), "main");
}

bool ModelShadow_Rtx_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    bool change = false;

    change |=
        ImGui::CheckBoxFloatDefault("Enable Light Attenuation", &m_UBO_Rtx.u_enable_light_attenuation, m_Default_UBO_Rtx.u_enable_light_attenuation);
    if (m_UBO_Rtx.u_enable_light_attenuation > 0.5f) {
        change |= ImGui::SliderFloatDefaultCompact(
            0.0f, "Attenuation Factor", &m_UBO_Rtx.u_light_intensity_factor, 0.0f, 1000.0f, m_Default_UBO_Rtx.u_light_intensity_factor);
    }
    change |= ImGui::CheckBoxFloatDefault("Enable Light Color", &m_UBO_Rtx.u_enable_light_color, m_Default_UBO_Rtx.u_enable_light_color);
    change |=
        ImGui::SliderFloatDefaultCompact(0.0f, "Shadow Strength", &m_UBO_Rtx.u_shadow_strength, 0.0f, 1.0f, m_Default_UBO_Rtx.u_shadow_strength);

    if (change) {
        NeedNewUBOUpload();
    }

    return false;
}

bool ModelShadow_Rtx_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool ModelShadow_Rtx_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

vk::DescriptorImageInfo* ModelShadow_Rtx_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    ZoneScoped;

    if (m_ComputeBufferPtr) {
        if (vOutSize) {
            *vOutSize = m_ComputeBufferPtr->GetOutputSize();
        }

        return m_ComputeBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
    }

    return nullptr;
}

void ModelShadow_Rtx_Pass::SetAccelStructure(SceneAccelStructureWeak vSceneAccelStructure) {
    ZoneScoped;

    m_SceneAccelStructure = vSceneAccelStructure;

    UpdateBufferInfoInRessourceDescriptor();
}

void ModelShadow_Rtx_Pass::SetLightGroup(SceneLightGroupWeak vSceneLightGroup) {
    ZoneScoped;

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

std::string ModelShadow_Rtx_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;

    std::string str;

    str +=
        vOffset + "<enable_light_attenuation>" + (m_UBO_Rtx.u_enable_light_attenuation > 0.5f ? "true" : "false") + "</enable_light_attenuation>\n";
    str += vOffset + "<enable_light_color>" + (m_UBO_Rtx.u_enable_light_color > 0.5f ? "true" : "false") + "</enable_light_color>\n";
    str += vOffset + "<light_intensity_factor>" + ct::toStr(m_UBO_Rtx.u_light_intensity_factor) + "</light_intensity_factor>\n";
    str += vOffset + "<shadow_strength>" + ct::toStr(m_UBO_Rtx.u_shadow_strength) + "</shadow_strength>\n";

    return str;
}

bool ModelShadow_Rtx_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (strParentName == "rtx_model_shadow_module") {
        if (strName == "enable_light_attenuation")
            m_UBO_Rtx.u_enable_light_attenuation = ct::ivariant(strValue).GetB() ? 1.0f : 0.0f;
        else if (strName == "enable_light_color")
            m_UBO_Rtx.u_enable_light_color = ct::ivariant(strValue).GetB() ? 1.0f : 0.0f;
        else if (strName == "light_intensity_factor")
            m_UBO_Rtx.u_light_intensity_factor = ct::fvariant(strValue).GetF();
        else if (strName == "shadow_strength")
            m_UBO_Rtx.u_shadow_strength = ct::fvariant(strValue).GetF();
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ModelShadow_Rtx_Pass::CreateUBO() {
    ZoneScoped;

    m_UBO_Rtx_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBO_Rtx), "ModelShadow_Rtx_Pass");
    m_UBO_Rtx_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
    if (m_UBO_Rtx_Ptr) {
        m_UBO_Rtx_BufferInfos.buffer = m_UBO_Rtx_Ptr->buffer;
        m_UBO_Rtx_BufferInfos.range = sizeof(UBO_Rtx);
        m_UBO_Rtx_BufferInfos.offset = 0;
    }

    NeedNewUBOUpload();

    return true;
}

void ModelShadow_Rtx_Pass::UploadUBO() {
    ZoneScoped;

    VulkanRessource::upload(m_VulkanCore, m_UBO_Rtx_Ptr, &m_UBO_Rtx, sizeof(UBO_Rtx));
}

void ModelShadow_Rtx_Pass::DestroyUBO() {
    ZoneScoped;

    m_UBO_Rtx_Ptr.reset();
    m_UBO_Rtx_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
}

bool ModelShadow_Rtx_Pass::CanRender() {
    ZoneScoped;

    if (!m_SceneAccelStructure.expired()) {
        return true;
    }

    return false;
}

bool ModelShadow_Rtx_Pass::CanUpdateDescriptors() {
    ZoneScoped;

    if (!m_SceneAccelStructure.expired()) {
        auto accelStructurePtr = m_SceneAccelStructure.lock();
        if (accelStructurePtr) {
            return accelStructurePtr->IsOk();
        }
    }

    return false;
}

bool ModelShadow_Rtx_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;

    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eRaygenKHR);
    res &= AddOrSetLayoutDescriptor(
        1U, vk::DescriptorType::eAccelerationStructureKHR, vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR);
    res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eRaygenKHR);
    res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR);
    res &= AddOrSetLayoutDescriptor(4U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR);
    res &= AddOrSetLayoutDescriptor(5U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eClosestHitKHR);

    return true;
}

bool ModelShadow_Rtx_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    bool res = false;

    auto accelStructurePtr = m_SceneAccelStructure.lock();
    if (accelStructurePtr && accelStructurePtr->GetTLASInfo() && accelStructurePtr->GetBufferAddressInfo()) {
        res = true;

        res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U));
        res &= AddOrSetWriteDescriptorNext(1U, vk::DescriptorType::eAccelerationStructureKHR, accelStructurePtr->GetTLASInfo());
        res &= AddOrSetWriteDescriptorBuffer(2U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo());  // camera
        res &=
            AddOrSetWriteDescriptorBuffer(3U, vk::DescriptorType::eStorageBuffer, accelStructurePtr->GetBufferAddressInfo());  // model device address
        res &= AddOrSetWriteDescriptorBuffer(4U, vk::DescriptorType::eStorageBuffer, m_SceneLightGroupDescriptorInfoPtr);
        res &= AddOrSetWriteDescriptorBuffer(5U, vk::DescriptorType::eUniformBuffer, &m_UBO_Rtx_BufferInfos);

        return res;  // pas de maj si pas de structure acceleratrice
    }

    return res;
}

std::string ModelShadow_Rtx_Pass::GetHitPayLoadCode() {
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

std::string ModelShadow_Rtx_Pass::GetRayGenerationShaderCode(std::string& vOutShaderName) {
    ZoneScoped;

    vOutShaderName = "ModelShadow_Rtx_Pass";
    return u8R"(
#version 460
#extension GL_EXT_ray_tracing : enable

layout(binding = 0, rgba32f) uniform writeonly image2D out_color;
layout(binding = 1) uniform accelerationStructureEXT tlas;
)" + CommonSystem::GetBufferObjectStructureHeader(2U) +
           GetHitPayLoadCode() +
           u8R"(
layout(location = 0) rayPayloadEXT hitPayload prd;

void main()
{
	const vec2 p = vec2(gl_LaunchIDEXT.xy);
	const vec2 s = vec2(gl_LaunchSizeEXT.xy);

	const vec2 pc = p + 0.5;			// pixel center
	const vec2 uv = pc / s;				// uv [0:1]
	const vec2 uvc = uv * 2.0 - 1.0;	// uv centered [-1:1]
	
	mat4 imv = inverse(model * view);
	mat4 ip = inverse(proj);

	vec4 origin    = imv * vec4(0, 0, 0, 1);
	vec4 target    = ip * vec4(uvc.x, uvc.y, 1, 1);
	vec4 direction = imv * vec4(normalize(target.xyz), 0);

	float tmin = 0.001;
	float tmax = 1e32;

	prd.sha = 0.0;
	prd.color = vec4(0.0);

	uint flags  = gl_RayFlagsOpaqueEXT;

	traceRayEXT(tlas,				// acceleration structure
		        flags,				// rayFlags
		        0xFF,				// cullMask
		        0,					// sbtRecordOffset
		        0,					// sbtRecordStride
		        0,					// missIndex
		        origin.xyz,			// ray origin
		        tmin,				// ray min range
		        direction.xyz,		// ray direction
		        tmax,				// ray max range
		        0					// payload (location = 0)
	);
	
	imageStore(out_color, ivec2(gl_LaunchIDEXT.xy), prd.color);
}
)";
}

std::string ModelShadow_Rtx_Pass::GetRayIntersectionShaderCode(std::string& vOutShaderName) {
    ZoneScoped;

    vOutShaderName = "ModelShadow_Rtx_Pass";
    return u8R"()";
}

std::string ModelShadow_Rtx_Pass::GetRayMissShaderCode(std::string& vOutShaderName) {
    ZoneScoped;

    vOutShaderName = "ModelShadow_Rtx_Pass";
    return u8R"(
#version 460
#extension GL_EXT_ray_tracing : enable
)" + GetHitPayLoadCode() +
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

std::string ModelShadow_Rtx_Pass::GetRayAnyHitShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "ModelShadow_Rtx_Pass";
    return u8R"()";
}

std::string ModelShadow_Rtx_Pass::GetRayClosestHitShaderCode(std::string& vOutShaderName) {
    ZoneScoped;

    vOutShaderName = "ModelShadow_Rtx_Pass";
    return u8R"(#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
)" + GetHitPayLoadCode() +
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
)" + SceneLightGroup::GetBufferObjectStructureHeader(4U) +
           u8R"(
layout(std140, binding = 5) uniform UBO_Rtx
{
	float u_light_intensity_factor;
	float u_enable_light_attenuation;
	float u_enable_light_color;
	float u_shadow_strength;
};

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
		return 0.5;
	}	
	
	return 1.0;
}

void main()
{
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

	// interpolated pos on triangle
	vec3 pos = 
		vec3(v0.px, v0.py, v0.pz) * barycentrics.x + 
		vec3(v1.px, v1.py, v1.pz) * barycentrics.y + 
		vec3(v2.px, v2.py, v2.pz) * barycentrics.z;
	
	uint count = lightsCount % 8; // maxi 8 lights in this system
	for (uint lid = 0 ; lid < count ; ++lid)
	{
		if (lightDatas[lid].lightActive > 0.5)
		{
			vec3 lp = lightDatas[lid].lightGizmo[3].xyz;
			float li = lightDatas[lid].lightIntensity;
			vec4 lc = vec4(1.0);
			vec3 ld = lp - pos;
			float len = length(ld);
			ld /= len; //normalized
			float atten = 1.0;

			prd.sha = getShadowValue(pos, ld);

			if (u_enable_light_attenuation > 0.5)
				atten = li * u_light_intensity_factor / (len * len);
			if (u_enable_light_color > 0.5)
				lc = lightDatas[lid].lightColor;

			prd.color += atten * lc * prd.sha;
		}
	}

	prd.color.rgb /= float(lightsCount);
	prd.color.a = 1.0;
}
)";
}