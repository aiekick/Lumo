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

#include "MathModule_Pass.h"

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
#include <cinttypes>
#include <Base/FrameBuffer.h>

using namespace vkApi;

#define COUNT_BUFFERS 2

//////////////////////////////////////////////////////////////
//// SSAO SECOND PASS : BLUR /////////////////////////////////
//////////////////////////////////////////////////////////////

MathModule_Pass::MathModule_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : Math", COMPUTE_SHADER_PASS_DEBUG_COLOR);
}

MathModule_Pass::~MathModule_Pass()
{
	Unit();
}

bool MathModule_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (ImGui::CollapsingHeader("Math", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool change = false;

		if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen))
		{
			change |= ImGui::SliderUIntDefaultCompact(0.0f, "Radius", &m_UBOComp.u_blur_radius, 1U, 10U, 4U);
			change |= ImGui::SliderUIntDefaultCompact(0.0f, "Offset", &m_UBOComp.u_blur_offset, 1U, 10U, 1U);

			if (change)
			{
				m_UBOComp.u_blur_radius = ct::maxi(m_UBOComp.u_blur_radius, 1U);
				m_UBOComp.u_blur_offset = ct::maxi(m_UBOComp.u_blur_offset, 1U);
				NeedNewUBOUpload();
			}
		}

		DrawInputTexture(m_VulkanCorePtr, "Input", 0U, m_OutputRatio);
		//DrawInputTexture(m_VulkanCorePtr, "Output Math", 0U, m_OutputRatio);

		return change;
	}

	return false;
}

void MathModule_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

}

void MathModule_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

}

void MathModule_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
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

vk::DescriptorImageInfo* MathModule_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_ComputeBufferPtr)
	{
		if (vOutSize)
		{
			*vOutSize = m_FrameBufferPtr->GetOutputSize();
		}

		return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

void MathModule_Pass::SwapOutputDescriptors()
{
	writeDescriptorSets[0].pImageInfo = m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U); // output
}

void MathModule_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipeline);
		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_PipelineLayout, 0, m_DescriptorSet, nullptr);
		vCmdBuffer->dispatch(m_DispatchSize.x, m_DispatchSize.y, m_DispatchSize.z);
	}
}

bool MathModule_Pass::CreateUBO()
{
	ZoneScoped;

	auto size_in_bytes = sizeof(UBOComp);
	m_UBO_Comp = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, size_in_bytes);
	m_DescriptorBufferInfo_Comp.buffer = m_UBO_Comp->buffer;
	m_DescriptorBufferInfo_Comp.range = size_in_bytes;
	m_DescriptorBufferInfo_Comp.offset = 0;

	for (auto& info : m_ImageInfos)
	{
		info = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
	}

	NeedNewUBOUpload();

	return true;
}

void MathModule_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, *m_UBO_Comp, &m_UBOComp, sizeof(UBOComp));
}

void MathModule_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Comp.reset();
}

bool MathModule_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute);
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute);

	return true;
}

bool MathModule_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();

	assert(m_ComputeBufferPtr);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetBackDescriptorImageInfo(0U), nullptr); // output
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Comp);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], nullptr); // ssao

	return true;
}

std::string MathModule_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "MathModule_Pass";

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColor;

layout(std140, binding = 1) uniform UBO_Comp
{
	uint u_blur_radius; // default is 4
	float u_blur_offset; // default is 1.0
};

layout(binding = 2) uniform sampler2D input_map_sampler;

void main()
{
	vec4 res = vec4(0.0);
	
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	vec4 tex = texelFetch(input_map_sampler, coords, 0);
	if (dot(tex, tex) > 0.0)
	{
		const uint blur_radius = u_blur_radius;
		const uint blur_radius_radius = u_blur_radius * u_blur_radius;

		const float blur_radius_f = float(blur_radius);

		vec2 texSize = textureSize(input_map_sampler, 0);

		for (uint i = 0 ; i < blur_radius_radius; ++i)
		{
			int x = int(floor(i * u_blur_offset / blur_radius_f) / blur_radius_f);
			int y = int((mod(float(i * u_blur_offset), blur_radius_f)) / blur_radius_f);
			res += texelFetch(input_map_sampler, coords + ivec2(x,y), 0);
		}

		res /= float(blur_radius_radius);
	}

	imageStore(outColor, coords, res); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MathModule_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<blur_radius>" + ct::toStr(m_UBOComp.u_blur_radius) + "</blur_radius>\n";
	str += vOffset + "<blur_offset>" + ct::toStr(m_UBOComp.u_blur_offset) + "</blur_offset>\n";

	return str;
}

bool MathModule_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strParentName == "blur_module")
	{
		if (strName == "blur_radius")
			m_UBOComp.u_blur_radius = ct::uvariant(strValue).GetU();
		else if (strName == "blur_offset")
			m_UBOComp.u_blur_offset = ct::uvariant(strValue).GetU();
		else if (strName == "blur_smooth_inf")

		NeedNewUBOUpload();
	}

	return true;
}