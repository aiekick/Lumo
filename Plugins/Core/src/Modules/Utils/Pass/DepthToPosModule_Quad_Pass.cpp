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

#include "DepthToPosModule_Quad_Pass.h"

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

DepthToPosModule_Quad_Pass::DepthToPosModule_Quad_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: QuadShaderPass(vVulkanCorePtr, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Quad Pass 1 : Depth To Pos", QUAD_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

DepthToPosModule_Quad_Pass::~DepthToPosModule_Quad_Pass()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool DepthToPosModule_Quad_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	DrawInputTexture(m_VulkanCorePtr, "Input Depth", 0U, m_OutputRatio);

	return false;
}

void DepthToPosModule_Quad_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void DepthToPosModule_Quad_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void DepthToPosModule_Quad_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				m_ImageInfos[vBinding] = *vImageInfo;

				if ((&m_UBOFrag.use_sampler_dep)[vBinding] < 1.0f)
				{
					(&m_UBOFrag.use_sampler_dep)[vBinding] = 1.0f;
					NeedNewUBOUpload();
				}
			}
			else
			{
				if ((&m_UBOFrag.use_sampler_dep)[vBinding] > 0.0f)
				{
					(&m_UBOFrag.use_sampler_dep)[vBinding] = 0.0f;
					NeedNewUBOUpload();
				}

				m_ImageInfos[vBinding] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* DepthToPosModule_Quad_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

std::string DepthToPosModule_Quad_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	return str;
}

bool DepthToPosModule_Quad_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

bool DepthToPosModule_Quad_Pass::CreateUBO()
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

	NeedNewUBOUpload();

	return true;
}

void DepthToPosModule_Quad_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBOFragPtr, &m_UBOFrag, sizeof(UBOFrag));
}

void DepthToPosModule_Quad_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBOFragPtr.reset();
}

bool DepthToPosModule_Quad_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	return res;
}

bool DepthToPosModule_Quad_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo());
	res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_DescriptorBufferInfo_Frag);
	res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]); // depth
	return res;
}

std::string DepthToPosModule_Quad_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "DepthToPosModule_Quad_Pass_Vertex";

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

std::string DepthToPosModule_Quad_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "DepthToPosModule_Quad_Pass_Fragment";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 v_uv;
)"
+ CommonSystem::GetBufferObjectStructureHeader(0U) +
u8R"(
layout (std140, binding = 1) uniform UBO_Vert 
{ 
	float use_sampler_dep;
};

layout(binding = 2) uniform sampler2D depth_map_sampler;

void main() 
{
	fragColor = vec4(0.0);
	if (use_sampler_dep > 0.5)
	{
		float dep = texture(depth_map_sampler, v_uv).r;
		if (dep > 0.0)
		{
			vec2 uvc = v_uv * 2.0 - 1.0;
			vec4 clip_pos = vec4(uvc, dep, 1.0);
			vec4 pos = inverse(cam) * clip_pos;
			fragColor = pos / pos.w;
		}
	}
}

)";
}