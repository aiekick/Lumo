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

#include "MatcapRenderer_Mesh_Pass.h"

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

MatcapRenderer_Mesh_Pass::MatcapRenderer_Mesh_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Mesh Pass 1 : Matcap", MESH_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

MatcapRenderer_Mesh_Pass::~MatcapRenderer_Mesh_Pass()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void MatcapRenderer_Mesh_Pass::DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	ZoneScoped;

	if (!m_Loaded) return;

	if (vCmdBuffer)
	{
		auto modelPtr = m_SceneModel.getValidShared();
		if (!modelPtr || modelPtr->empty()) return;

		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
		{
			VKFPScoped(*vCmdBuffer, "MatcapRenderer_Mesh_Pass", "DrawModel");

			vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout, 0, m_DescriptorSet, nullptr);

			for (auto meshPtr : *modelPtr)
			{
				if (meshPtr)
				{
					vk::DeviceSize offsets = 0;
					vCmdBuffer->bindVertexBuffers(0, meshPtr->GetVerticesBuffer(), offsets);

					if (meshPtr->GetIndicesCount())
					{
						vCmdBuffer->bindIndexBuffer(meshPtr->GetIndicesBuffer(), 0, vk::IndexType::eUint32);
						vCmdBuffer->drawIndexed(meshPtr->GetIndicesCount(), 1, 0, 0, 0);
					}
					else
					{
						vCmdBuffer->draw(meshPtr->GetVerticesCount(), 1, 0, 0);
					}
				}
			}
		}
	}
}

bool MatcapRenderer_Mesh_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	bool change = false;

	change |= ImGui::CheckBoxFloatDefault("Show Face Normal", &m_UBOFrag.show_face_normal, false);

	if (change)
	{
		NeedNewUBOUpload();
	}

	DrawInputTexture(m_VulkanCorePtr, "Input Matcap", 0U, m_OutputRatio);

	return change;
}

void MatcapRenderer_Mesh_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

}

void MatcapRenderer_Mesh_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

}

void MatcapRenderer_Mesh_Pass::SetModel(SceneModelWeak vSceneModel)
{
	ZoneScoped;

	m_SceneModel = vSceneModel;
}

void MatcapRenderer_Mesh_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				m_ImageInfos[vBinding] = *vImageInfo;
			}
			else
			{
				m_ImageInfos[vBinding] = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* MatcapRenderer_Mesh_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

void MatcapRenderer_Mesh_Pass::DestroyModel(const bool& vReleaseDatas)
{
	ZoneScoped;

	if (vReleaseDatas)
	{
		m_SceneModel.reset();
	}
}

bool MatcapRenderer_Mesh_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBO_Vert = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBOVert));
	if (m_UBO_Vert)
	{
		m_DescriptorBufferInfo_Vert.buffer = m_UBO_Vert->buffer;
		m_DescriptorBufferInfo_Vert.range = sizeof(UBOVert);
		m_DescriptorBufferInfo_Vert.offset = 0;
	}

	m_UBO_Frag = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBOFrag));
	if (m_UBO_Frag)
	{
		m_DescriptorBufferInfo_Frag.buffer = m_UBO_Frag->buffer;
		m_DescriptorBufferInfo_Frag.range = sizeof(UBOFrag);
		m_DescriptorBufferInfo_Frag.offset = 0;
	}

	for (auto& info : m_ImageInfos)
	{
		info = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
	}

	NeedNewUBOUpload();

	return true;
}

void MatcapRenderer_Mesh_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, *m_UBO_Vert, &m_UBOVert, sizeof(UBOVert));
	VulkanRessource::upload(m_VulkanCorePtr, *m_UBO_Frag, &m_UBOFrag, sizeof(UBOFrag));
}

void MatcapRenderer_Mesh_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Vert.reset();
	m_UBO_Frag.reset();
}

bool MatcapRenderer_Mesh_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(3U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);

	return true;
}

bool MatcapRenderer_Mesh_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();
	writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, CommonSystem::Instance()->GetBufferInfo());
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Vert);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Frag);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 3U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], nullptr); // matcap

	return true;
}

void MatcapRenderer_Mesh_Pass::SetInputStateBeforePipelineCreation()
{
	VertexStruct::P3_N3_TA3_BTA3_T2_C4::GetInputState(m_InputState);
}

std::string MatcapRenderer_Mesh_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "MatcapRenderer_Mesh_Pass_Vertex";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBiTangent;
layout(location = 4) in vec2 aUv;
layout(location = 5) in vec4 aColor;

layout(location = 0) out vec3 vertPosition;
layout(location = 1) out vec3 vertNormal;
layout(location = 2) out vec2 matcapNormal2D;
)"
+ CommonSystem::GetBufferObjectStructureHeader(0U) +
u8R"(
layout (std140, binding = 1) uniform UBO_Vert 
{ 
	mat4 transform;
};

vec2 getMatCap(vec3 pos, vec3 nor)
{
	mat4 modelViewMatrix = view * model;
	mat4 normalMatrix = transpose(inverse(modelViewMatrix));
	vec4 pp = modelViewMatrix * vec4(pos, 1. );
	vec4 nn = normalMatrix * vec4(nor, 1.);
	vec3 rd = normalize( pp.xyz );
	vec3 n = normalize( nn.xyz );
	vec3 r = reflect(rd, n);
	r.y *= -1.0;
	float m = 2. * sqrt(
		pow( r.x, 2. ) +
		pow( r.y, 2. ) +
		pow( r.z + 1., 2. )
	);
	return r.xy / m + .5;
}

void main() 
{
	vertPosition = aPosition;
	vertNormal = aNormal;
	matcapNormal2D = getMatCap(vertPosition, aNormal);
	
	gl_Position = cam * transform * vec4(vertPosition, 1.0);
}
)";
}

std::string MatcapRenderer_Mesh_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "MatcapRenderer_Mesh_Pass_Fragment";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec2 matcapNormal2D;
)"
+ CommonSystem::GetBufferObjectStructureHeader(0U) +
u8R"(
layout(std140, binding = 2) uniform UBO_Frag 
{ 
	float show_face_normal;
};

layout(binding = 3) uniform sampler2D sampler_matcap;

vec2 getMatCap(vec3 pos, vec3 nor)
{
	mat4 modelViewMatrix = view * model;
	mat4 normalMatrix = transpose(inverse(modelViewMatrix));
	vec4 pp = modelViewMatrix * vec4(pos, 1. );
	vec4 nn = normalMatrix * vec4(nor, 0.);
	vec3 rd = normalize( pp.xyz );
	vec3 n = normalize( nn.xyz );
	vec3 r = reflect(rd, n);
	float m = 2. * sqrt(
		pow( r.x, 2. ) +
		pow( r.y, 2. ) +
		pow( r.z + 1., 2. )
	);
	return r.xy / m + .5;
}

void main() 
{
	fragColor = vec4(0);

	if (dot(vertPosition, vertPosition) > 0.0)
	{
		vec3 vertice_normal = vertNormal;
		if (show_face_normal > 0.5)
		{
			vertice_normal = normalize(-cross(dFdx(vertPosition), dFdy(vertPosition)));
		}

		vec2 tn = matcapNormal2D;
		tn = getMatCap(vertPosition, vertice_normal);
		fragColor = texture(sampler_matcap, tn);
		fragColor.a = 1.0;
	}
	else
	{
		//discard;
	}
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MatcapRenderer_Mesh_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<show_face_normal>" + (m_UBOFrag.show_face_normal > 0.5f ? "true" : "false") + "</show_face_normal>\n";

	return str;
}

bool MatcapRenderer_Mesh_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strParentName == "matcap_renderer")
	{
		if (strName == "show_face_normal")
			m_UBOFrag.show_face_normal = ct::ivariant(strValue).GetB() ? 1.0f : 0.0f;

		NeedNewUBOUpload();
	}

	return true;
}
