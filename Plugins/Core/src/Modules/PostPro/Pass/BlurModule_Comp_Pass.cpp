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

#include "BlurModule_Comp_Pass.h"

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



//////////////////////////////////////////////////////////////
//// SSAO SECOND PASS : BLUR /////////////////////////////////
//////////////////////////////////////////////////////////////

BlurModule_Comp_Pass::BlurModule_Comp_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : Blur", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

BlurModule_Comp_Pass::~BlurModule_Comp_Pass()
{
	Unit();
}

void BlurModule_Comp_Pass::ActionBeforeInit()
{
	m_Pipelines.resize(2U);
	m_DescriptorSets.resize(2U);
}

void BlurModule_Comp_Pass::ActionBeforeCompilation()
{
	ClearShaderEntryPoints();
	AddShaderEntryPoints(vk::ShaderStageFlagBits::eCompute, "blur_H");
	AddShaderEntryPoints(vk::ShaderStageFlagBits::eCompute, "blur_V");
}

bool BlurModule_Comp_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (ImGui::CollapsingHeader("Blur", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool change = false;

		if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen))
		{
			change |= ImGui::SliderUIntDefaultCompact(0.0f, "Radius", &m_UBOComp.u_blur_radius, 1U, 32U, 4U);
			
			if (change)
			{
				m_UBOComp.u_blur_radius = ct::maxi(m_UBOComp.u_blur_radius, 1U);
				NeedNewUBOUpload();
			}
		}

		DrawInputTexture(m_VulkanCorePtr, "Input", 0U, m_OutputRatio);
		//DrawInputTexture(m_VulkanCorePtr, "Output Blur", 0U, m_OutputRatio);

		return change;
	}

	return false;
}

void BlurModule_Comp_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void BlurModule_Comp_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void BlurModule_Comp_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
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
				m_ImageInfos[vBinding] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* BlurModule_Comp_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
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

void BlurModule_Comp_Pass::SwapMultiPassFrontBackDescriptors()
{
	m_DescriptorSets[0].m_WriteDescriptorSets[0].pImageInfo = m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U); // output
}

bool BlurModule_Comp_Pass::CanUpdateDescriptors()
{
	return (m_ComputeBufferPtr != nullptr);
}

void BlurModule_Comp_Pass::Compute_Blur_H(vk::CommandBuffer* vCmdBuffer)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
		Dispatch(vCmdBuffer);
	}

}

void BlurModule_Comp_Pass::Compute_Blur_V(vk::CommandBuffer* vCmdBuffer)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[1].m_Pipeline);
		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[1].m_DescriptorSet, nullptr);
		Dispatch(vCmdBuffer);
	}
}

void BlurModule_Comp_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (vCmdBuffer)
	{

		Compute_Blur_H(vCmdBuffer);

		#undef MemoryBarrier
		vCmdBuffer->pipelineBarrier(
			vk::PipelineStageFlagBits::eComputeShader,
			vk::PipelineStageFlagBits::eComputeShader,
			vk::DependencyFlags(),
			vk::MemoryBarrier(vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead),
			nullptr,
			nullptr);

		Compute_Blur_V(vCmdBuffer);
	}
}

bool BlurModule_Comp_Pass::CreateUBO()
{
	ZoneScoped;

	auto size_in_bytes = sizeof(UBOComp);
	m_UBOCompPtr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, size_in_bytes);
	m_DescriptorBufferInfo_Comp.buffer = m_UBOCompPtr->buffer;
	m_DescriptorBufferInfo_Comp.range = size_in_bytes;
	m_DescriptorBufferInfo_Comp.offset = 0;

	for (auto& info : m_ImageInfos)
	{
		info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
	}

	NeedNewUBOUpload();

	return true;
}

void BlurModule_Comp_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBOCompPtr, &m_UBOComp, sizeof(UBOComp));
}

void BlurModule_Comp_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBOCompPtr.reset();
}

bool BlurModule_Comp_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	if (m_ComputeBufferPtr)
	{
		m_DescriptorSets[0].m_LayoutBindings.clear();
		m_DescriptorSets[0].m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute);
		m_DescriptorSets[0].m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute);
		m_DescriptorSets[0].m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute);

		m_DescriptorSets[1].m_LayoutBindings.clear();
		m_DescriptorSets[1].m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute);
		m_DescriptorSets[1].m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute);
		m_DescriptorSets[1].m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute);

		return true;
	}

	return false;
}

bool BlurModule_Comp_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	if (m_ComputeBufferPtr)
	{
		// blur H
		m_DescriptorSets[0].m_WriteDescriptorSets.clear();
		m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 0U, 0, 1,
			vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(1U), nullptr); // output
		m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 1U, 0, 1,
			vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Comp);
		m_DescriptorSets[0].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[0].m_DescriptorSet, 2U, 0, 1,
			vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], nullptr); // image to blur

		// blur V
		m_DescriptorSets[1].m_WriteDescriptorSets.clear();
		m_DescriptorSets[1].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[1].m_DescriptorSet, 0U, 0, 1,
			vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U), nullptr); // output
		m_DescriptorSets[1].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[1].m_DescriptorSet, 1U, 0, 1,
			vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Comp);
		m_DescriptorSets[1].m_WriteDescriptorSets.emplace_back(m_DescriptorSets[1].m_DescriptorSet, 2U, 0, 1,
			vk::DescriptorType::eCombinedImageSampler, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(1U), nullptr); // image to blur

		return true;
	}

	return false;
}

std::string BlurModule_Comp_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "BlurModule_Compute_Pass";

	SetLocalGroupSize(ct::uvec3(8U, 8U, 1U));

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColor;

layout(std140, binding = 1) uniform UBO_Comp
{
	uint u_blur_radius; // default is 4
};

layout(binding = 2) uniform sampler2D input_map_sampler;

void blur_H()
{
	vec4 res = vec4(0.0);
	
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	const int blur_radius = int(u_blur_radius);

	ivec2 iv = coords;
	for(int idx = -blur_radius; idx < blur_radius; ++idx)
	{
		iv.x = coords.x + idx;
		res += texelFetch(input_map_sampler, iv, 0);
	}

	res /= float(u_blur_radius * 2);

	imageStore(outColor, coords, res); 
}

void blur_V()
{
	vec4 res = vec4(0.0);
	
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	const int blur_radius = int(u_blur_radius);

	ivec2 iv = coords;
	for(int idx = -blur_radius; idx < blur_radius; ++idx)
	{
		iv.y = coords.y + idx;
		res += texelFetch(input_map_sampler, iv, 0);
	}

	res /= float(u_blur_radius * 2);


	imageStore(outColor, coords, res); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / PIPELINE ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BlurModule_Comp_Pass::CreateComputePipeline()
{
	ZoneScoped;

	if (m_ShaderCodes[vk::ShaderStageFlagBits::eCompute].empty()) return false;
	if (m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["blur_H"][0].m_SPIRV.empty()) return false;
	if (m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["blur_V"][0].m_SPIRV.empty()) return false;

	std::vector<vk::PushConstantRange> push_constants;
	if (m_Internal_PushConstants.size)
	{
		push_constants.push_back(m_Internal_PushConstants);
	}

	m_Pipelines[0].m_PipelineLayout =
		m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			1, &m_DescriptorSets[0].m_DescriptorSetLayout,
			(uint32_t)push_constants.size(), push_constants.data()
		)); 

	m_Pipelines[1].m_PipelineLayout =
		m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			1, &m_DescriptorSets[1].m_DescriptorSetLayout,
			(uint32_t)push_constants.size(), push_constants.data()
		));

	auto cs_H = vkApi::VulkanCore::sVulkanShader->CreateShaderModule(
		(VkDevice)m_Device, m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["blur_H"][0].m_SPIRV);
	
	auto cs_V = vkApi::VulkanCore::sVulkanShader->CreateShaderModule(
		(VkDevice)m_Device, m_ShaderCodes[vk::ShaderStageFlagBits::eCompute]["blur_V"][0].m_SPIRV);


	m_ShaderCreateInfos = {
	   vk::PipelineShaderStageCreateInfo(
		   vk::PipelineShaderStageCreateFlags(),
		   vk::ShaderStageFlagBits::eCompute,
		   cs_H, "blur_H"
	   ),
	   vk::PipelineShaderStageCreateInfo(
		   vk::PipelineShaderStageCreateFlags(),
		   vk::ShaderStageFlagBits::eCompute,
		   cs_V, "blur_V"
	   )
	};

	vk::ComputePipelineCreateInfo computePipeInfo_H = vk::ComputePipelineCreateInfo()
		.setStage(m_ShaderCreateInfos[0]).setLayout(m_Pipelines[0].m_PipelineLayout);
	m_Pipelines[0].m_Pipeline = m_Device.createComputePipeline(nullptr, computePipeInfo_H).value;
	
	vk::ComputePipelineCreateInfo computePipeInfo_V = vk::ComputePipelineCreateInfo()
		.setStage(m_ShaderCreateInfos[1]).setLayout(m_Pipelines[1].m_PipelineLayout);
	m_Pipelines[1].m_Pipeline = m_Device.createComputePipeline(nullptr, computePipeInfo_V).value;

	vkApi::VulkanCore::sVulkanShader->DestroyShaderModule((VkDevice)m_Device, cs_H);
	vkApi::VulkanCore::sVulkanShader->DestroyShaderModule((VkDevice)m_Device, cs_V);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string BlurModule_Comp_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<blur_radius>" + ct::toStr(m_UBOComp.u_blur_radius) + "</blur_radius>\n";
	
	return str;
}

bool BlurModule_Comp_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

		NeedNewUBOUpload();
	}

	return true;
}
