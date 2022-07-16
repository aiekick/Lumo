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

#include "RtxPbrRenderer_Pass.h"

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
#include <vkFramework/VulkanCommandBuffer.h>
#include <utils/Mesh/VertexStruct.h>
#include <Base/FrameBuffer.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

RtxPbrRenderer_Pass::RtxPbrRenderer_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: QuadShaderPass(vVulkanCorePtr, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Quad Pass 1 : PBR", QUAD_SHADER_PASS_DEBUG_COLOR);

	//m_DontUseShaderFilesOnDisk = true;
}

RtxPbrRenderer_Pass::~RtxPbrRenderer_Pass()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool RtxPbrRenderer_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

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

void RtxPbrRenderer_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

}

void RtxPbrRenderer_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

}

void RtxPbrRenderer_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
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
				
				m_ImageInfos[vBinding] = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* RtxPbrRenderer_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

void RtxPbrRenderer_Pass::SetTextures(const uint32_t& vBinding, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes)
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
					info = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
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

void RtxPbrRenderer_Pass::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	m_SceneLightGroup = vSceneLightGroup;

	m_SceneLightGroupDescriptorInfoPtr = &m_SceneLightGroupDescriptorInfo;

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

std::string RtxPbrRenderer_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += vOffset + "<bias>" + ct::toStr(m_UBOFrag.u_bias) + "</bias>\n";
	str += vOffset + "<strength>" + ct::toStr(m_UBOFrag.u_shadow_strength) + "</strength>\n";
	str += vOffset + "<noise_scale>" + ct::toStr(m_UBOFrag.u_poisson_scale) + "</noise_scale>\n";
	str += vOffset + "<use_pcf>" + (m_UBOFrag.u_use_pcf > 0.5f ? "true" : "false") + "</use_pcf>\n";

	return str;
}

bool RtxPbrRenderer_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

bool RtxPbrRenderer_Pass::BuildModel()
{
	// Materials
	ObjMaterial mat_red = { {1, 0, 0}, {1, 1, 1}, 0.0f };
	ObjMaterial mat_green = { {0, 1, 0}, {1, 1, 1}, 0.0f };
	ObjMaterial mat_blue = { {0, 0, 1}, {1, 1, 1}, 0.0f };
	ObjMaterial mat_yellow = { {1, 1, 0}, {1, 1, 1}, 0.0f };
	ObjMaterial mat_cyan = { {0, 1, 1}, {1, 1, 1}, 0.0f };
	ObjMaterial mat_magenta = { {1, 0, 1}, {1, 1, 1}, 0.0f };
	ObjMaterial mat_grey = { {0.7f, 0.7f, 0.7f}, {0.9f, 0.9f, 0.9f}, 0.1f };        // Slightly reflective
	ObjMaterial mat_mirror = { {0.3f, 0.9f, 1.0f}, {0.9f, 0.9f, 0.9f}, 0.9f };        // Mirror Slightly blue

	// Geometries
	auto cube = ObjCube();
	auto plane = ObjPlane();

	// Upload geometries to GPU
	create_model(cube, { mat_red, mat_green, mat_blue, mat_yellow, mat_cyan, mat_magenta });        // 6 color faces
	create_model(plane, { mat_grey });
	create_model(cube, { mat_mirror });

	// Create a buffer holding the address of model buffers (buffer reference)
	create_buffer_references();

	// Create as many bottom acceleration structures (blas) as there are geometries/models
	create_bottom_level_acceleration_structure(obj_models[0]);
	create_bottom_level_acceleration_structure(obj_models[1]);
	create_bottom_level_acceleration_structure(obj_models[2]);

	// Matrices to position the instances
	glm::mat4 m_mirror_back = glm::scale(glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 0.0f, -7.0f)), glm::vec3(5.0f, 5.0f, 0.1f));
	glm::mat4 m_mirror_front = glm::scale(glm::translate(glm::mat4(1.f), glm::vec3(0.0f, 0.0f, 7.0f)), glm::vec3(5.0f, 5.0f, 0.1f));
	glm::mat4 m_plane = glm::scale(glm::translate(glm::mat4(1.f), glm::vec3(0.0f, -1.0f, 0.0f)), glm::vec3(15.0f, 15.0f, 15.0f));
	glm::mat4 m_cube_left = glm::translate(glm::mat4(1.f), glm::vec3(-1.0f, 0.0f, 0.0f));
	glm::mat4 m_cube_right = glm::translate(glm::mat4(1.f), glm::vec3(1.0f, 0.0f, 0.0f));

	// Creating instances of the blas to the top level acceleration structure
	std::vector<VkAccelerationStructureInstanceKHR> blas_instances;
	blas_instances.push_back(create_blas_instance(0, m_cube_left));
	blas_instances.push_back(create_blas_instance(0, m_cube_right));
	blas_instances.push_back(create_blas_instance(1, m_plane));
	blas_instances.push_back(create_blas_instance(2, m_mirror_back));
	blas_instances.push_back(create_blas_instance(2, m_mirror_front));

	// Building the TLAS
	create_top_level_acceleration_structure(blas_instances);
}

void RtxPbrRenderer_Pass::DestroyModel(const bool& vReleaseDatas = false)
{

}

bool RtxPbrRenderer_Pass::CreateUBO()
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

	for (auto& info : m_ImageGroupInfos)
	{
		info = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
	}

	NeedNewUBOUpload();

	return true;
}

void RtxPbrRenderer_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, *m_UBO_Frag, &m_UBOFrag, sizeof(UBOFrag));
}

void RtxPbrRenderer_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Frag.reset();
}

bool RtxPbrRenderer_Pass::UpdateLayoutBindingInRessourceDescriptor()
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

bool RtxPbrRenderer_Pass::UpdateBufferInfoInRessourceDescriptor()
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

std::string RtxPbrRenderer_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "RtxPbrRenderer_Pass_Vertex";

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

std::string RtxPbrRenderer_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "RtxPbrRenderer_Pass_Fragment";

	return u8R"(
#version 450
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
				
		uint count = uint(lightDatas.length() + 1) % 8; // maxi 8 lights in this system
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / RTX /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

// will convert model in accel struct
bool RtxPbrRenderer_Pass::CreateBottomLevelAccelerationStructure()
{
	auto modelPtr = m_SceneModel.getValidShared();
	if (modelPtr && !modelPtr->empty())
	{
		// only take the first messh
		// support only one fully connected mesh
		auto meshPtr = modelPtr->Get(0).getValidShared();
		if (meshPtr)
		{
			auto buffer_usage_flags =
				vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
				vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;

			/*
			vk::TransformMatrixKHR transform_matrix =
				std::array<std::array<float, 4>, 3>
			{
				1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f
			};
			std::unique_ptr<vkb::core::Buffer> transform_matrix_buffer = std::make_unique<vkb::core::Buffer>(get_device(), sizeof(transform_matrix), buffer_usage_flags, VMA_MEMORY_USAGE_CPU_TO_GPU);
			transform_matrix_buffer->update(&transform_matrix, sizeof(transform_matrix));
			*/

			vk::BufferDeviceAddressInfoKHR vertex_buffer_device_address_info{};
			vertex_buffer_device_address_info.buffer = meshPtr->GetVerticesBuffer();
			auto vertex_address = m_Device.getBufferAddressKHR(&vertex_buffer_device_address_info);

			vk::BufferDeviceAddressInfoKHR index_buffer_device_address_info{};
			index_buffer_device_address_info.buffer = meshPtr->GetIndicesBuffer();
			auto index_address = m_Device.getBufferAddressKHR(&index_buffer_device_address_info);

			vk::AccelerationStructureGeometryTrianglesDataKHR triangles;
			triangles.vertexFormat = vk::Format::eR32G32B32Sfloat;
			triangles.vertexData = vertex_address;
			triangles.maxVertex = meshPtr->GetVerticesCount();
			triangles.vertexStride = sizeof(VertexStruct::P3_N3_TA3_BTA3_T2_C4);
			triangles.indexType = vk::IndexType::eUint32;
			triangles.indexData = index_address;
			//triangles.transformData = transform_matrix_device_address;

			// The bottom level acceleration structure contains one set of triangles as the input geometry
			vk::AccelerationStructureGeometryKHR accelStructureGeometry;
			accelStructureGeometry.geometryType = vk::GeometryTypeKHR::eTriangles;
			accelStructureGeometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
			accelStructureGeometry.geometry.triangles = triangles;

			// Get the size requirements for buffers involved in the acceleration structure build process
			vk::AccelerationStructureBuildGeometryInfoKHR accelStructureBuildGeometryInfo;
			accelStructureBuildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
			accelStructureBuildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
			accelStructureBuildGeometryInfo.geometryCount = 1;
			accelStructureBuildGeometryInfo.pGeometries = &accelStructureGeometry;

			const uint32_t triangle_count = meshPtr->GetIndicesCount() / 3U;

			auto accelStructureBuildSizeInfo = m_Device.getAccelerationStructureBuildSizesKHR(
				vk::AccelerationStructureBuildTypeKHR::eDevice,
				accelStructureBuildGeometryInfo, triangle_count);

			// Create a buffer to hold the acceleration structure
			m_AccelStructure_Bottom_Ptr = VulkanRessource::createAccelStructureBufferObject(m_VulkanCorePtr, 
				accelStructureBuildSizeInfo.accelerationStructureSize, 
				VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);

			// Create the acceleration structure
			vk::AccelerationStructureCreateInfoKHR accelStructureCreateInfo;
			accelStructureCreateInfo.buffer = m_AccelStructure_Bottom_Ptr->buffer;
			accelStructureCreateInfo.size = accelStructureBuildSizeInfo.accelerationStructureSize;
			accelStructureCreateInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
			m_AccelStructure_Bottom_Ptr->handle = m_Device.createAccelerationStructureKHR(accelStructureCreateInfo);

			// The actual build process starts here

			// Create a scratch buffer as a temporary storage for the acceleration structure build
			auto scratchBufferPtr = VulkanRessource::createStorageBufferObject(m_VulkanCorePtr,
				accelStructureBuildSizeInfo.accelerationStructureSize,
				vk::BufferUsageFlagBits::eShaderDeviceAddressKHR | vk::BufferUsageFlagBits::eStorageBuffer,
				VMA_MEMORY_USAGE_CPU_TO_GPU);

			vk::BufferDeviceAddressInfoKHR scratchBufferDeviceAddressInfo{};
			scratchBufferDeviceAddressInfo.buffer = scratchBufferPtr->buffer;
			auto scratchBufferAddress = m_Device.getBufferAddressKHR(&scratchBufferDeviceAddressInfo);

			vk::AccelerationStructureBuildGeometryInfoKHR accelBuildGeometryInfo;
			accelBuildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
			accelBuildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
			accelBuildGeometryInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
			accelBuildGeometryInfo.dstAccelerationStructure = m_AccelStructure_Bottom_Ptr->handle;
			accelBuildGeometryInfo.geometryCount = 1;
			accelBuildGeometryInfo.pGeometries = &accelStructureGeometry;
			accelBuildGeometryInfo.scratchData.deviceAddress = scratchBufferAddress;

			vk::AccelerationStructureBuildRangeInfoKHR accelStructureBuildRangeInfo;
			accelStructureBuildRangeInfo.primitiveCount = triangle_count;
			accelStructureBuildRangeInfo.primitiveOffset = 0;
			accelStructureBuildRangeInfo.firstVertex = 0;
			accelStructureBuildRangeInfo.transformOffset = 0;
			std::vector<vk::AccelerationStructureBuildRangeInfoKHR*> accelStructureBuildRangeInfos =
				{ &accelStructureBuildRangeInfo };

			// Build the acceleration structure on the device via a one-time command buffer submission
			auto cmd = VulkanCommandBuffer::beginSingleTimeCommands(m_VulkanCorePtr, true);
			cmd.buildAccelerationStructuresKHR(1, &accelBuildGeometryInfo, accelStructureBuildRangeInfos.data());
			VulkanCommandBuffer::flushSingleTimeCommands(m_VulkanCorePtr, cmd, true);

			//delete_scratch_buffer(scratch_buffer);
			scratchBufferPtr.reset();

			return true;
		}
	}

	return false;
}

void RtxPbrRenderer_Pass::DestroyBottomLevelAccelerationStructure()
{
	m_Device.waitIdle();

	m_AccelStructure_Bottom_Ptr.reset();
}

bool RtxPbrRenderer_Pass::CreateTopLevelAccelerationStructure(std::vector<vk::AccelerationStructureInstanceKHR>& vBlasInstances)
{
	auto instancesBufferPtr = VulkanRessource::createStorageBufferObject(m_VulkanCorePtr, 
		sizeof(vk::AccelerationStructureInstanceKHR) * vBlasInstances.size(),
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR,
		VMA_MEMORY_USAGE_CPU_TO_GPU);
	VulkanRessource::upload(m_VulkanCorePtr, *instancesBufferPtr, 
		vBlasInstances.data(), sizeof(vk::AccelerationStructureInstanceKHR) * vBlasInstances.size());

	vk::BufferDeviceAddressInfoKHR instancesBufferDeviceAddressInfo{};
	instancesBufferDeviceAddressInfo.buffer = instancesBufferPtr->buffer;
	auto instancesBufferAddress = m_Device.getBufferAddressKHR(&instancesBufferDeviceAddressInfo);

	vk::DeviceOrHostAddressConstKHR instance_data_device_address{};
	instance_data_device_address.deviceAddress = instancesBufferAddress;

	// The top level acceleration structure contains (bottom level) instance as the input geometry
	vk::AccelerationStructureGeometryKHR accelStructureGeometry;
	accelStructureGeometry.geometryType = vk::GeometryTypeKHR::eInstances;
	accelStructureGeometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
	accelStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
	accelStructureGeometry.geometry.instances.data = instance_data_device_address;

	// Get the size requirements for buffers involved in the acceleration structure build process
	vk::AccelerationStructureBuildGeometryInfoKHR accelStructureBuildGeometryInfo;
	accelStructureBuildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	accelStructureBuildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	accelStructureBuildGeometryInfo.geometryCount = 1;
	accelStructureBuildGeometryInfo.pGeometries = &accelStructureGeometry;

	const auto primitive_count = static_cast<uint32_t>(vBlasInstances.size());

	auto accelStructureBuildSizeInfo = m_Device.getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice,
		accelStructureBuildGeometryInfo, primitive_count);

	// Create a buffer to hold the acceleration structure
	m_AccelStructure_Top_Ptr = VulkanRessource::createAccelStructureBufferObject(m_VulkanCorePtr,
		accelStructureBuildSizeInfo.accelerationStructureSize,
		VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);

	// Create the acceleration structure
	vk::AccelerationStructureCreateInfoKHR accelStructureCreateInfo;
	accelStructureCreateInfo.buffer = m_AccelStructure_Top_Ptr->buffer;
	accelStructureCreateInfo.size = accelStructureBuildSizeInfo.accelerationStructureSize;
	accelStructureCreateInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	m_AccelStructure_Top_Ptr->handle = m_Device.createAccelerationStructureKHR(accelStructureCreateInfo);
	
	// The actual build process starts here

	// Create a scratch buffer as a temporary storage for the acceleration structure build
	auto scratchBufferPtr = VulkanRessource::createStorageBufferObject(m_VulkanCorePtr,
		accelStructureBuildSizeInfo.accelerationStructureSize,
		vk::BufferUsageFlagBits::eShaderDeviceAddressKHR | vk::BufferUsageFlagBits::eStorageBuffer,
		VMA_MEMORY_USAGE_CPU_TO_GPU);

	vk::BufferDeviceAddressInfoKHR scratchBufferDeviceAddressInfo{};
	scratchBufferDeviceAddressInfo.buffer = scratchBufferPtr->buffer;
	auto scratchBufferAddress = m_Device.getBufferAddressKHR(&scratchBufferDeviceAddressInfo);

	vk::AccelerationStructureBuildGeometryInfoKHR accelBuildGeometryInfo;
	accelBuildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	accelBuildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	accelBuildGeometryInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
	accelBuildGeometryInfo.dstAccelerationStructure = m_AccelStructure_Top_Ptr->handle;
	accelBuildGeometryInfo.geometryCount = 1;
	accelBuildGeometryInfo.pGeometries = &accelStructureGeometry;
	accelBuildGeometryInfo.scratchData.deviceAddress = scratchBufferAddress;

	vk::AccelerationStructureBuildRangeInfoKHR accelStructureBuildRangeInfo;
	accelStructureBuildRangeInfo.primitiveCount = primitive_count;
	accelStructureBuildRangeInfo.primitiveOffset = 0;
	accelStructureBuildRangeInfo.firstVertex = 0;
	accelStructureBuildRangeInfo.transformOffset = 0;
	std::vector<vk::AccelerationStructureBuildRangeInfoKHR*> accelStructureBuildRangeInfos =
	{ &accelStructureBuildRangeInfo };

	// Build the acceleration structure on the device via a one-time command buffer submission
	auto cmd = VulkanCommandBuffer::beginSingleTimeCommands(m_VulkanCorePtr, true);
	cmd.buildAccelerationStructuresKHR(1, &accelBuildGeometryInfo, accelStructureBuildRangeInfos.data());
	VulkanCommandBuffer::flushSingleTimeCommands(m_VulkanCorePtr, cmd, true);

	//delete_scratch_buffer(scratch_buffer);
	scratchBufferPtr.reset();

	return true;
}

void RtxPbrRenderer_Pass::DestroyTopLevelAccelerationStructure()
{
	m_Device.waitIdle();

	m_AccelStructure_Top_Ptr.reset();
}

vk::AccelerationStructureInstanceKHR RtxPbrRenderer_Pass::CreateBlasInstance(const uint32_t& blas_id, glm::mat4& mat)
{
	VkTransformMatrixKHR transform_matrix;
	glm::mat3x4          rtxT = glm::transpose(mat);
	memcpy(&transform_matrix, glm::value_ptr(rtxT), sizeof(VkTransformMatrixKHR));

	AccelerationStructure& blas = bottom_level_acceleration_structure[blas_id];

	// Get the bottom acceleration structure's handle, which will be used during the top level acceleration build
	VkAccelerationStructureDeviceAddressInfoKHR acceleration_device_address_info{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
	acceleration_device_address_info.accelerationStructure = blas.handle;
	auto device_address = vkGetAccelerationStructureDeviceAddressKHR(device->get_handle(), &acceleration_device_address_info);

	VkAccelerationStructureInstanceKHR blas_instance{};
	blas_instance.transform = transform_matrix;
	blas_instance.instanceCustomIndex = blas_id;
	blas_instance.mask = 0xFF;
	blas_instance.instanceShaderBindingTableRecordOffset = 0;
	blas_instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
	blas_instance.accelerationStructureReference = device_address;

	return blas_instance;
}

void RtxPbrRenderer_Pass::CreateShaderBindingTables()
{
	// Index position of the groups in the generated ray tracing pipeline
	// To be generic, this should be pass in parameters
	std::vector<uint32_t> rgen_index{ 0 };
	std::vector<uint32_t> miss_index{ 1, 2 };
	std::vector<uint32_t> hit_index{ 3 };

	const uint32_t handle_size = ray_tracing_pipeline_properties.shaderGroupHandleSize;
	const uint32_t handle_alignment = ray_tracing_pipeline_properties.shaderGroupHandleAlignment;
	const uint32_t handle_size_aligned = aligned_size(handle_size, handle_alignment);

	const VkBufferUsageFlags sbt_buffer_usage_flags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	const VmaMemoryUsage     sbt_memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	// Create binding table buffers for each shader type
	raygen_shader_binding_table = std::make_unique<vkb::core::Buffer>(get_device(), handle_size_aligned * rgen_index.size(), sbt_buffer_usage_flags, sbt_memory_usage, 0);
	miss_shader_binding_table = std::make_unique<vkb::core::Buffer>(get_device(), handle_size_aligned * miss_index.size(), sbt_buffer_usage_flags, sbt_memory_usage, 0);
	hit_shader_binding_table = std::make_unique<vkb::core::Buffer>(get_device(), handle_size_aligned * hit_index.size(), sbt_buffer_usage_flags, sbt_memory_usage, 0);

	// Copy the pipeline's shader handles into a host buffer
	const auto           group_count = static_cast<uint32_t>(rgen_index.size() + miss_index.size() + hit_index.size());
	const auto           sbt_size = group_count * handle_size_aligned;
	std::vector<uint8_t> shader_handle_storage(sbt_size);
	VK_CHECK(vkGetRayTracingShaderGroupHandlesKHR(get_device().get_handle(), pipeline, 0, group_count, sbt_size, shader_handle_storage.data()));

	// Write the handles in the SBT buffer
	auto copyHandles = [&](auto& buffer, std::vector<uint32_t>& indices, uint32_t stride) {
		auto* pBuffer = static_cast<uint8_t*>(buffer->map());
		for (uint32_t index = 0; index < static_cast<uint32_t>(indices.size()); index++)
		{
			auto* pStart = pBuffer;
			// Copy the handle
			memcpy(pBuffer, shader_handle_storage.data() + (indices[index] * handle_size), handle_size);
			pBuffer = pStart + stride;        // Jumping to next group
		}
		buffer->unmap();
	};

	copyHandles(raygen_shader_binding_table, rgen_index, handle_size_aligned);
	copyHandles(miss_shader_binding_table, miss_index, handle_size_aligned);
	copyHandles(hit_shader_binding_table, hit_index, handle_size_aligned);
}

bool RtxPbrRenderer_Pass::CreateRtxPipeline()
{
	// Slot for binding top level acceleration structures to the ray generation shader
	VkDescriptorSetLayoutBinding acceleration_structure_layout_binding{};
	acceleration_structure_layout_binding.binding = 0;
	acceleration_structure_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	acceleration_structure_layout_binding.descriptorCount = 1;
	acceleration_structure_layout_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

	VkDescriptorSetLayoutBinding result_image_layout_binding{};
	result_image_layout_binding.binding = 1;
	result_image_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	result_image_layout_binding.descriptorCount = 1;
	result_image_layout_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	VkDescriptorSetLayoutBinding uniform_buffer_binding{};
	uniform_buffer_binding.binding = 2;
	uniform_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniform_buffer_binding.descriptorCount = 1;
	uniform_buffer_binding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	// Scene description
	VkDescriptorSetLayoutBinding scene_buffer_binding{};
	scene_buffer_binding.binding = 3;
	scene_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	scene_buffer_binding.descriptorCount = 1;
	scene_buffer_binding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

	std::vector<VkDescriptorSetLayoutBinding> bindings = {
		acceleration_structure_layout_binding,
		result_image_layout_binding,
		uniform_buffer_binding,
		scene_buffer_binding,
	};

	VkDescriptorSetLayoutCreateInfo layout_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
	layout_info.pBindings = bindings.data();
	VK_CHECK(vkCreateDescriptorSetLayout(get_device().get_handle(), &layout_info, nullptr, &descriptor_set_layout));

	VkPipelineLayoutCreateInfo pipeline_layout_create_info{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipeline_layout_create_info.setLayoutCount = 1;
	pipeline_layout_create_info.pSetLayouts = &descriptor_set_layout;

	VK_CHECK(vkCreatePipelineLayout(get_device().get_handle(), &pipeline_layout_create_info, nullptr, &pipeline_layout));

	// Ray tracing shaders + buffer reference require SPIR-V 1.5, so we need to set the appropriate target environment for the glslang compiler
	vkb::GLSLCompiler::set_target_environment(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);

	/*
		Setup ray tracing shader groups
		Each shader group points at the corresponding shader in the pipeline
	*/
	std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

	// Ray generation group
	{
		shader_stages.push_back(load_shader("ray_tracing_reflection/raygen.rgen", VK_SHADER_STAGE_RAYGEN_BIT_KHR));
		VkRayTracingShaderGroupCreateInfoKHR raygen_group_ci{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
		raygen_group_ci.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		raygen_group_ci.generalShader = static_cast<uint32_t>(shader_stages.size()) - 1;
		raygen_group_ci.closestHitShader = VK_SHADER_UNUSED_KHR;
		raygen_group_ci.anyHitShader = VK_SHADER_UNUSED_KHR;
		raygen_group_ci.intersectionShader = VK_SHADER_UNUSED_KHR;
		shader_groups.push_back(raygen_group_ci);
	}

	// Ray miss group
	{
		shader_stages.push_back(load_shader("ray_tracing_reflection/miss.rmiss", VK_SHADER_STAGE_MISS_BIT_KHR));
		VkRayTracingShaderGroupCreateInfoKHR miss_group_ci{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
		miss_group_ci.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		miss_group_ci.generalShader = static_cast<uint32_t>(shader_stages.size()) - 1;
		miss_group_ci.closestHitShader = VK_SHADER_UNUSED_KHR;
		miss_group_ci.anyHitShader = VK_SHADER_UNUSED_KHR;
		miss_group_ci.intersectionShader = VK_SHADER_UNUSED_KHR;
		shader_groups.push_back(miss_group_ci);
	}

	// Ray miss (shadow) group
	{
		shader_stages.push_back(load_shader("ray_tracing_reflection/missShadow.rmiss", VK_SHADER_STAGE_MISS_BIT_KHR));
		VkRayTracingShaderGroupCreateInfoKHR miss_group_ci{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
		miss_group_ci.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		miss_group_ci.generalShader = static_cast<uint32_t>(shader_stages.size()) - 1;
		miss_group_ci.closestHitShader = VK_SHADER_UNUSED_KHR;
		miss_group_ci.anyHitShader = VK_SHADER_UNUSED_KHR;
		miss_group_ci.intersectionShader = VK_SHADER_UNUSED_KHR;
		shader_groups.push_back(miss_group_ci);
	}

	// Ray closest hit group
	{
		shader_stages.push_back(load_shader("ray_tracing_reflection/closesthit.rchit", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR));
		VkRayTracingShaderGroupCreateInfoKHR closes_hit_group_ci{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
		closes_hit_group_ci.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		closes_hit_group_ci.generalShader = VK_SHADER_UNUSED_KHR;
		closes_hit_group_ci.closestHitShader = static_cast<uint32_t>(shader_stages.size()) - 1;
		closes_hit_group_ci.anyHitShader = VK_SHADER_UNUSED_KHR;
		closes_hit_group_ci.intersectionShader = VK_SHADER_UNUSED_KHR;
		shader_groups.push_back(closes_hit_group_ci);
	}

	/*
		Create the ray tracing pipeline
	*/
	VkRayTracingPipelineCreateInfoKHR raytracing_pipeline_create_info{ VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR };
	raytracing_pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
	raytracing_pipeline_create_info.pStages = shader_stages.data();
	raytracing_pipeline_create_info.groupCount = static_cast<uint32_t>(shader_groups.size());
	raytracing_pipeline_create_info.pGroups = shader_groups.data();
	raytracing_pipeline_create_info.maxPipelineRayRecursionDepth = 2;
	raytracing_pipeline_create_info.layout = pipeline_layout;
	VK_CHECK(vkCreateRayTracingPipelinesKHR(get_device().get_handle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &raytracing_pipeline_create_info, nullptr, &pipeline));

	return false;
}

void RtxPbrRenderer_Pass::DestroyRtxPipeline()
{

}

bool RtxPbrRenderer_Pass::CreateShaderBindingTable()
{
	return false;
}

void RtxPbrRenderer_Pass::DestroyShaderBindingTable()
{

}

std::string RtxPbrRenderer_Pass::GetRayGenerationShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "RtxPbrRenderer_Pass_RayGen";
	return u8R"(
#version 450
#extension GL_EXT_ray_tracing : enable

layout(set = 0, binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 0, binding = 1, rgba8) uniform image2D image;
layout(set = 0, binding = 2) uniform CameraProperties
{
	mat4 viewInverse;
	mat4 projInverse;
}
cam;

struct hitPayload
{
	vec3 radiance;
	vec3 attenuation;
	int  done;
	vec3 rayOrigin;
	vec3 rayDir;
};

layout(location = 0) rayPayloadEXT hitPayload prd;

void main()
{
	const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
	const vec2 inUV        = pixelCenter / vec2(gl_LaunchSizeEXT.xy);
	vec2       d           = inUV * 2.0 - 1.0;

	vec4 origin    = cam.viewInverse * vec4(0, 0, 0, 1);
	vec4 target    = cam.projInverse * vec4(d.x, d.y, 1, 1);
	vec4 direction = cam.viewInverse * vec4(normalize(target.xyz), 0);

	float tmin = 0.001;
	float tmax = 1e32;

	prd.rayOrigin   = origin.xyz;
	prd.rayDir      = direction.xyz;
	prd.radiance    = vec3(0.0);
	prd.attenuation = vec3(1.0);
	prd.done        = 0;

	vec3 hitValue = vec3(0);

	for (int depth = 0; depth < 64; depth++)
	{
		traceRayEXT(topLevelAS, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, prd.rayOrigin, tmin, prd.rayDir, tmax, 0);
		hitValue += prd.radiance;
		if (prd.done == 1 || length(prd.attenuation) < 0.1)
			break;
	}

	imageStore(image, ivec2(gl_LaunchIDEXT.xy), vec4(hitValue, 0.0));
}
)";
}

std::string RtxPbrRenderer_Pass::GetRayMissShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "RtxPbrRenderer_Pass_RayMiss";
	return u8R"(

#version 460
#extension GL_EXT_ray_tracing : enable

struct hitPayload
{
	vec3 radiance;
	vec3 attenuation;
	int  done;
	vec3 rayOrigin;
	vec3 rayDir;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main()
{
	prd.radiance = vec3(0.3) * prd.attenuation;
	prd.done     = 1;
}

)";
}

std::string RtxPbrRenderer_Pass::GetRayClosestHitShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "RtxPbrRenderer_Pass_RayClosestHit";
	return u8R"(

#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_nonuniform_qualifier : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
struct hitPayload
{
	vec3 radiance;
	vec3 attenuation;
	int  done;
	vec3 rayOrigin;
	vec3 rayDir;
};

layout(location = 0) rayPayloadInEXT hitPayload prd;
layout(location = 1) rayPayloadEXT bool isShadowed;

hitAttributeEXT vec3 attribs;

struct WaveFrontMaterial
{
	vec3  diffuse;
	vec3  specular;
	float shininess;
};

struct Vertex
{
	vec3 pos;
	vec3 nrm;
};

struct ObjBuffers
{
	uint64_t vertices;
	uint64_t indices;
	uint64_t materials;
	uint64_t materialIndices;
};

// clang-format off
layout(buffer_reference, scalar) buffer Vertices {Vertex v[]; }; // Positions of an object
layout(buffer_reference, scalar) buffer Indices {uvec3 i[]; }; // Triangle indices
layout(buffer_reference, scalar) buffer Materials {WaveFrontMaterial m[]; }; // Array of all materials on an object
layout(buffer_reference, scalar) buffer MatIndices {int i[]; }; // Material ID for each triangle

layout(set = 0, binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 0, binding = 3) buffer _scene_desc { ObjBuffers i[]; } scene_desc;
// clang-format on

vec3 computeSpecular(WaveFrontMaterial mat, vec3 V, vec3 L, vec3 N)
{
	const float kPi        = 3.14159265;
	const float kShininess = max(mat.shininess, 4.0);

	// Specular
	const float kEnergyConservation = (2.0 + kShininess) / (2.0 * kPi);
	V                               = normalize(-V);
	vec3  R                         = reflect(-L, N);
	float specular                  = kEnergyConservation * pow(max(dot(V, R), 0.0), kShininess);

	return vec3(mat.specular * specular);
}

void main()
{
	// When contructing the TLAS, we stored the model id in InstanceCustomIndexEXT, so the
	// the instance can quickly have access to the data

	// Object data
	ObjBuffers objResource = scene_desc.i[gl_InstanceCustomIndexEXT];
	MatIndices matIndices  = MatIndices(objResource.materialIndices);
	Materials  materials   = Materials(objResource.materials);
	Indices    indices     = Indices(objResource.indices);
	Vertices   vertices    = Vertices(objResource.vertices);

	// Retrieve the material used on this triangle 'PrimitiveID'
	int               mat_idx = matIndices.i[gl_PrimitiveID];
	WaveFrontMaterial mat     = materials.m[mat_idx];        // Material for this triangle

	// Indices of the triangle
	uvec3 ind = indices.i[gl_PrimitiveID];

	// Vertex of the triangle
	Vertex v0 = vertices.v[ind.x];
	Vertex v1 = vertices.v[ind.y];
	Vertex v2 = vertices.v[ind.z];

	// Barycentric coordinates of the triangle
	const vec3 barycentrics = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);

	// Computing the normal at hit position
	vec3 N = v0.nrm.xyz * barycentrics.x + v1.nrm.xyz * barycentrics.y + v2.nrm.xyz * barycentrics.z;
	N      = normalize(vec3(N.xyz * gl_WorldToObjectEXT));        // Transforming the normal to world space

	// Computing the coordinates of the hit position
	vec3 P = v0.pos.xyz * barycentrics.x + v1.pos.xyz * barycentrics.y + v2.pos.xyz * barycentrics.z;
	P      = vec3(gl_ObjectToWorldEXT * vec4(P, 1.0));        // Transforming the position to world space

	// Hardocded (to) light direction
	vec3 L = normalize(vec3(1, 1, 1));

	float NdotL = dot(N, L);

	// Fake Lambertian to avoid black
	vec3 diffuse  = mat.diffuse * max(NdotL, 0.3);
	vec3 specular = vec3(0);

	// Tracing shadow ray only if the light is visible from the surface
	if (NdotL > 0)
	{
		float tMin   = 0.001;
		float tMax   = 1e32;        // infinite
		vec3  origin = P;
		vec3  rayDir = L;
		uint  flags  = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
		isShadowed   = true;

		traceRayEXT(topLevelAS,        // acceleration structure
		            flags,             // rayFlags
		            0xFF,              // cullMask
		            0,                 // sbtRecordOffset
		            0,                 // sbtRecordStride
		            1,                 // missIndex
		            origin,            // ray origin
		            tMin,              // ray min range
		            rayDir,            // ray direction
		            tMax,              // ray max range
		            1                  // payload (location = 1)
		);

		if (isShadowed)
			diffuse *= 0.3;
		else
			// Add specular only if not in shadow
			specular = computeSpecular(mat, gl_WorldRayDirectionEXT, L, N);
	}
	prd.radiance = (diffuse + specular) * (1 - mat.shininess) * prd.attenuation;

	// Reflect
	vec3 rayDir = reflect(gl_WorldRayDirectionEXT, N);
	prd.attenuation *= vec3(mat.shininess);
	prd.rayOrigin = P;
	prd.rayDir    = rayDir;
}

)";
}

std::string RtxPbrRenderer_Pass::GetRayCallableShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "RtxPbrRenderer_Pass_RayCallable";
	return u8R"(

)";
}