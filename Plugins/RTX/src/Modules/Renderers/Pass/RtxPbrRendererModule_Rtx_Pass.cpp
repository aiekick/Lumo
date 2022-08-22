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

void RtxPbrRendererModule_Rtx_Pass::ActionBeforeInit()
{
	ZoneScoped;

	//m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);

}

bool RtxPbrRendererModule_Rtx_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	bool change = false;

	change |= DrawResizeWidget();

	change |= ImGui::SliderFloatDefaultCompact(0.0f, "Name", &m_UBO_Ahit.u_Name, 0.000f, 0.000f, 0.000f, 0.0f, "%.3f");
	change |= ImGui::SliderFloatDefaultCompact(0.0f, "Name", &m_UBO_Chit.u_Name, 0.000f, 0.000f, 0.000f, 0.0f, "%.3f");
	change |= ImGui::SliderFloatDefaultCompact(0.0f, "Name", &m_UBO_Inter.u_Name, 0.000f, 0.000f, 0.000f, 0.0f, "%.3f");
	change |= ImGui::SliderFloatDefaultCompact(0.0f, "Name", &m_UBO_Miss.u_Name, 0.000f, 0.000f, 0.000f, 0.0f, "%.3f");
	change |= ImGui::SliderFloatDefaultCompact(0.0f, "Name", &m_UBO_RGen.u_Name, 0.000f, 0.000f, 0.000f, 0.0f, "%.3f");

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

	m_UBO_Ahit_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Ahit));
	m_UBO_Ahit_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_Ahit_Ptr)
	{
		m_UBO_Ahit_BufferInfos.buffer = m_UBO_Ahit_Ptr->buffer;
		m_UBO_Ahit_BufferInfos.range = sizeof(UBO_Ahit);
		m_UBO_Ahit_BufferInfos.offset = 0;
	}

	m_UBO_Chit_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Chit));
	m_UBO_Chit_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_Chit_Ptr)
	{
		m_UBO_Chit_BufferInfos.buffer = m_UBO_Chit_Ptr->buffer;
		m_UBO_Chit_BufferInfos.range = sizeof(UBO_Chit);
		m_UBO_Chit_BufferInfos.offset = 0;
	}

	m_UBO_Inter_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Inter));
	m_UBO_Inter_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_Inter_Ptr)
	{
		m_UBO_Inter_BufferInfos.buffer = m_UBO_Inter_Ptr->buffer;
		m_UBO_Inter_BufferInfos.range = sizeof(UBO_Inter);
		m_UBO_Inter_BufferInfos.offset = 0;
	}

	m_UBO_Miss_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_Miss));
	m_UBO_Miss_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_Miss_Ptr)
	{
		m_UBO_Miss_BufferInfos.buffer = m_UBO_Miss_Ptr->buffer;
		m_UBO_Miss_BufferInfos.range = sizeof(UBO_Miss);
		m_UBO_Miss_BufferInfos.offset = 0;
	}

	m_UBO_RGen_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBO_RGen));
	m_UBO_RGen_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBO_RGen_Ptr)
	{
		m_UBO_RGen_BufferInfos.buffer = m_UBO_RGen_Ptr->buffer;
		m_UBO_RGen_BufferInfos.range = sizeof(UBO_RGen);
		m_UBO_RGen_BufferInfos.offset = 0;
	}

	NeedNewUBOUpload();

	return true;
}

void RtxPbrRendererModule_Rtx_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_Ahit_Ptr, &m_UBO_Ahit, sizeof(UBO_Ahit));
	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_Chit_Ptr, &m_UBO_Chit, sizeof(UBO_Chit));
	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_Inter_Ptr, &m_UBO_Inter, sizeof(UBO_Inter));
	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_Miss_Ptr, &m_UBO_Miss, sizeof(UBO_Miss));
	VulkanRessource::upload(m_VulkanCorePtr, m_UBO_RGen_Ptr, &m_UBO_RGen, sizeof(UBO_RGen));
}

void RtxPbrRendererModule_Rtx_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Ahit_Ptr.reset();
	m_UBO_Ahit_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	m_UBO_Chit_Ptr.reset();
	m_UBO_Chit_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	m_UBO_Inter_Ptr.reset();
	m_UBO_Inter_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	m_UBO_Miss_Ptr.reset();
	m_UBO_Miss_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

	m_UBO_RGen_Ptr.reset();
	m_UBO_RGen_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
}

bool RtxPbrRendererModule_Rtx_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAnyHitKHR);
	AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eClosestHitKHR);
	AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eIntersectionKHR);
	AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eMissKHR);
	AddOrSetLayoutDescriptor(4U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eRaygenKHR);
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eRaygenKHR); // output
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eAccelerationStructureKHR, vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR); // accel struct
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eRaygenKHR); // camera
	res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR); // model device address
	res &= AddOrSetLayoutDescriptor(4U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR);

	return true;
}

bool RtxPbrRendererModule_Rtx_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, &m_UBO_Ahit_BufferInfos);
	res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_UBO_Chit_BufferInfos);
	res &= AddOrSetWriteDescriptorBuffer(2U, vk::DescriptorType::eUniformBuffer, &m_UBO_Inter_BufferInfos);
	res &= AddOrSetWriteDescriptorBuffer(3U, vk::DescriptorType::eUniformBuffer, &m_UBO_Miss_BufferInfos);
	res &= AddOrSetWriteDescriptorBuffer(4U, vk::DescriptorType::eUniformBuffer, &m_UBO_RGen_BufferInfos);
	auto accelStructurePtr = m_SceneAccelStructure.getValidShared();
	if (accelStructurePtr && 
		accelStructurePtr->GetTLASInfo() && 
		accelStructurePtr->GetBufferAddressInfo())
	{
		res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U)); // output
		res &= AddOrSetWriteDescriptorNext(1U, vk::DescriptorType::eAccelerationStructureKHR, accelStructurePtr->GetTLASInfo()); // accel struct
		res &= AddOrSetWriteDescriptorBuffer(2U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo()); // camera
		res &= AddOrSetWriteDescriptorBuffer(3U, vk::DescriptorType::eStorageBuffer, accelStructurePtr->GetBufferAddressInfo()); // model device address
		res &= AddOrSetWriteDescriptorBuffer(4U, vk::DescriptorType::eStorageBuffer, m_SceneLightGroupDescriptorInfoPtr);
	}

	
	return res;
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
+ CommonSystem::GetBufferObjectStructureHeader(2U) +
u8R"(


layout(std140, binding = 4) uniform UBO_RGen
{
	float u_Name;
};

struct hitPayload
{
	vec4 color;
	vec3 ro;
	vec3 rd;
};

layout(location = 0) rayPayloadEXT hitPayload prd;

vec3 getRayOrigin()
{
	vec3 ro = view[3].xyz + model[3].xyz;
	ro *= mat3(model);
	return -ro;
}

vec3 getRayDirection(vec2 uv)
{
	uv = uv * 2.0 - 1.0;
	vec4 ray_clip = vec4(uv.x, uv.y, -1.0, 0.0);
	vec4 ray_eye = inverse(proj) * ray_clip;
	vec3 rd = normalize(vec3(ray_eye.x, ray_eye.y, -1.0));
	rd *= mat3(view * model);
	return rd;
}

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

	float tmin = 0.001;
	float tmax = 1e32;

	prd.ro = getRayOrigin();		// origin.xyz;
	prd.rd = getRayDirection(uv);	// direction.xyz;
	prd.color = vec4(0.0);

	traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, prd.ro, tmin, prd.rd, tmax, 0);
	
	imageStore(out_color, ivec2(gl_LaunchIDEXT.xy), prd.color);
}
)";
}

std::string RtxPbrRendererModule_Rtx_Pass::GetRayIntersectionShaderCode(std::string& vOutShaderName)
{
	ZoneScoped;

	vOutShaderName = "RtxPbrRendererModule_Rtx_Pass_Inter";
	return u8R"(

layout(std140, binding = 2) uniform UBO_Inter
{
	float u_Name;
};
)";
}

std::string RtxPbrRendererModule_Rtx_Pass::GetRayMissShaderCode(std::string& vOutShaderName)
{
	ZoneScoped;

	vOutShaderName = "RtxPbrRendererModule_Rtx_Pass_Miss";
	return u8R"(
#version 460
#extension GL_EXT_ray_tracing : enable

struct hitPayload
{
	vec4 color;
	vec3 ro;
	vec3 rd;
	float ao;
	float diff;
	float spec;
	float sha;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;
layout(location = 1) rayPayloadEXT bool isShadowed;


layout(std140, binding = 3) uniform UBO_Miss
{
	float u_Name;
};

void main()
{
	prd.color = vec4(0.0, 0.0, 0.0, 1.0);
	prd.diff = 0.0;
	prd.spec = 0.0;
	prd.sha = 0.0;
	isShadowed = false;
}
)";
}

std::string RtxPbrRendererModule_Rtx_Pass::GetRayAnyHitShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "RtxPbrRendererModule_Rtx_Pass_Ahit";
	return u8R"(

layout(std140, binding = 0) uniform UBO_Ahit
{
	float u_Name;
};
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

struct hitPayload
{
	vec4 color;
	vec3 ro;
	vec3 rd;
	float ao;
	float diff;
	float spec;
	float sha;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;
layout(location = 1) rayPayloadEXT bool isShadowed;


layout(std140, binding = 1) uniform UBO_Chit
{
	float u_Name;
};

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
";
+
SceneLightGroup::GetBufferObjectStructureHeader(4U)
+
u8R"(
float ShadowTest(vec3 p, vec3 n, vec3 ld)
{
	if (dot(n, ld) > 0.0)
	{
		p += n * 0.1;
		uint flags  = 
			gl_RayFlagsTerminateOnFirstHitEXT | 
			gl_RayFlagsOpaqueEXT | 
			gl_RayFlagsSkipClosestHitShaderEXT;
		isShadowed = true; 
		traceRayEXT(tlas,        // acceleration structure
		            flags,             // rayFlags
		            0xFF,              // cullMask
		            0,                 // sbtRecordOffset
		            0,                 // sbtRecordStride
		            0,                 // missIndex
		            p,            	   // ray origin
		            0.1,              // ray min range
		            ld,            // ray direction
		            1e32,              // ray max range
		            1                  // payload (location = 1)
		);
		if (isShadowed)
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

	vec3 normal = 
		vec3(v0.nx, v0.ny, v0.nz) * barycentrics.x + 
		vec3(v1.nx, v1.ny, v1.nz) * barycentrics.y + 
		vec3(v2.nx, v2.ny, v2.nz) * barycentrics.z;
    
	// Transforming the normal to world space
	normal = normalize(vec3(normal * gl_WorldToObjectEXT)); 
	
	prd.color = vec4(normal * 0.5 + 0.5, 1.0); // return normal
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

	str += vOffset + "<name>" + ct::toStr(m_UBO_Ahit.u_Name) + "</name>\n";
	str += vOffset + "<name>" + ct::toStr(m_UBO_Chit.u_Name) + "</name>\n";
	str += vOffset + "<name>" + ct::toStr(m_UBO_Inter.u_Name) + "</name>\n";
	str += vOffset + "<name>" + ct::toStr(m_UBO_Miss.u_Name) + "</name>\n";
	str += vOffset + "<name>" + ct::toStr(m_UBO_RGen.u_Name) + "</name>\n";

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

		if (strName == "name")
			m_UBO_Ahit.u_Name = ct::fvariant(strValue).GetF();
		else if (strName == "name")
			m_UBO_Chit.u_Name = ct::fvariant(strValue).GetF();
		else if (strName == "name")
			m_UBO_Inter.u_Name = ct::fvariant(strValue).GetF();
		else if (strName == "name")
			m_UBO_Miss.u_Name = ct::fvariant(strValue).GetF();
		else if (strName == "name")
			m_UBO_RGen.u_Name = ct::fvariant(strValue).GetF();
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
