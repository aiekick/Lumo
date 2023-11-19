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

#include "ConwayModule_Comp_Pass.h"

#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <cinttypes>
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
//// SSAO SECOND PASS : BLUR /////////////////////////////////
//////////////////////////////////////////////////////////////

ConwayModule_Comp_Pass::ConwayModule_Comp_Pass(GaiApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : Conway", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

ConwayModule_Comp_Pass::~ConwayModule_Comp_Pass()
{
	Unit();
}

void ConwayModule_Comp_Pass::ActionBeforeInit()
{
	m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);
}

bool ConwayModule_Comp_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	ZoneScoped;

	ImGui::SetCurrentContext(vContextPtr);

	const float aw = ImGui::GetContentRegionAvail().x;

	bool change = false;

	change |= DrawResizeWidget();

	ImGui::Header("Pass Iterations##ConwayModule_Comp_Pass");

	change |= ImGui::SliderUIntDefaultCompact(aw, "Iterations##ConwayModule_Comp_Pass", &m_CountIterations.w, m_CountIterations.x, m_CountIterations.y, m_CountIterations.z);

	ImGui::Header("Mouse##ConwayModule_Comp_Pass");

	change |= ImGui::SliderFloatDefaultCompact(aw, "Mouse Radius##ConwayModule_Comp_Pass", &m_UBOComp.mouse_radius, 0.0f, 100.0f, 10.0f);
	if (ImGui::RadioButtonLabeled(aw, "Mouse Inversion##ConwayModule_Comp_Pass", m_UBOComp.mouse_inversion > 0.5f, false))
	{
		m_UBOComp.mouse_inversion = (m_UBOComp.mouse_inversion > 0.5f) ? 0.0f : 1.0f;
		change = true;
	}

	ImGui::Header("Motion Blur##ConwayModule_Comp_Pass");

	change |= ImGui::SliderFloatDefaultCompact(aw, "Coef##ConwayModule_Comp_Pass", &m_UBOComp.motion_blur_coef, 0.0f, 1.0f, 0.95f);
	
	ImGui::Header("Clear##ConwayModule_Comp_Pass");

	if (ImGui::ContrastedButton("Reset Substances##ConwayModule_Comp_Pass", nullptr, nullptr, aw))
	{
		m_UBOComp.reset_substances = 1.0f;
		change = true;
	}

	if (change)
	{
		NeedNewUBOUpload();
	}

	return change;
}

bool ConwayModule_Comp_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool ConwayModule_Comp_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void ConwayModule_Comp_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBindingPoint < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				if (vTextureSize)
				{
					m_ImageInfosSize[vBindingPoint] = *vTextureSize;
				}

				m_ImageInfos[vBindingPoint] = *vImageInfo;

				m_UBOComp.use_noise_input = 1.0f;
			}
			else
			{
				m_ImageInfos[vBindingPoint] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();

				m_UBOComp.use_noise_input = 0.0f;
			}
		}
	}
}

vk::DescriptorImageInfo* ConwayModule_Comp_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / CONWAY //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ConwayModule_Comp_Pass::WasJustResized()
{
	ZoneScoped;

	if (m_UBOComp.image_size.x != m_OutputSize.x ||
		m_UBOComp.image_size.y != m_OutputSize.y)
	{
		m_UBOComp.image_size = m_OutputSize;

		NeedNewUBOUpload();
	}
}

void ConwayModule_Comp_Pass::Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber)
{
	if (vCmdBufferPtr)
	{
		vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);

		vCmdBufferPtr->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);

		for (uint32_t iter = 0; iter < m_CountIterations.w; iter++)
		{
			Dispatch(vCmdBufferPtr);
		}

		// ca c'est les bouton et ca doit etre un one shot 
		// donc on fait ca pour le reset et re provoquer un update
		if (m_UBOComp.reset_substances > 0.5f)
		{
			m_UBOComp.reset_substances = 0.0f;

			NeedNewUBOUpload();
		}
	}
}

bool ConwayModule_Comp_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBOCompPtr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBOComp));
	m_UBOComp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBOCompPtr)
	{
		m_UBOComp_BufferInfos.buffer = m_UBOCompPtr->buffer;
		m_UBOComp_BufferInfos.range = sizeof(UBOComp);
		m_UBOComp_BufferInfos.offset = 0;
	}

	for (auto& info : m_ImageInfos)
	{
		info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
	}

	NeedNewUBOUpload();

	return true;
}

void ConwayModule_Comp_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBOCompPtr, &m_UBOComp, sizeof(UBOComp));
}

void ConwayModule_Comp_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBOCompPtr.reset();
	m_UBOComp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
}

bool ConwayModule_Comp_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);
	return res;
}

bool ConwayModule_Comp_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U)); // output
	res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo()); // common system
	res &= AddOrSetWriteDescriptorBuffer(2U, vk::DescriptorType::eUniformBuffer, &m_UBOComp_BufferInfos);
	res &= AddOrSetWriteDescriptorImage(3U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]); // input 0
	return res;
}

std::string ConwayModule_Comp_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ConwayModule_Comp_Pass";

	// with 8 i have some slow down maybe due the fact than i read an write from same image2D
	SetLocalGroupSize(ct::uvec3(8U, 8U, 1U));

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform image2D outColor;
)"
+ CommonSystem::GetBufferObjectStructureHeader(1U) +
u8R"(
layout(std140, binding = 2) uniform UBO_Comp
{
	float mouse_radius;
	float mouse_inversion;
	float reset_substances;
	float motion_blur_coef;
	float use_noise_input;
	ivec2 image_size;
};

layout(binding = 3) uniform sampler2D input_noise;

vec4 getNoise(ivec2 g, int x, int y)
{
	const ivec2 v = (g + ivec2(x,y)) % image_size;
	return texelFetch(input_noise, v, 0);
}

vec4 getPixel(ivec2 g, int x, int y)
{
	const ivec2 v = (g + ivec2(x,y)) % image_size;
	return imageLoad(outColor, v);
}

int IsPixelActivated(ivec2 g, int x, int y)
{
    return getPixel(g, x, y).b > 0.5 ? 1 : 0;
}

vec4 conWay(ivec2 g, vec4 mo)
{
	int neighbors = 0;
    neighbors += IsPixelActivated(g, -1,  0);
	neighbors += IsPixelActivated(g, -1,  1);
	neighbors += IsPixelActivated(g,  0,  1);
	neighbors += IsPixelActivated(g,  1,  1);
	neighbors += IsPixelActivated(g,  1,  0);
	neighbors += IsPixelActivated(g,  1, -1);
	neighbors += IsPixelActivated(g,  0, -1);
	neighbors += IsPixelActivated(g, -1, -1);

	int current_cell = IsPixelActivated(g,  0,  0);
	
	vec4 color = getPixel(g, 0, 0);

	// conway rules
	color.b = (current_cell == 1 && neighbors == 2 || neighbors == 3) ? 1.0 : 0.0;

	// just for color variations
	color.r = (neighbors == 3) ? 1.0 : 0.0;

	// motion blur
	color.g = color.g * motion_blur_coef + color.b * (1.0 - motion_blur_coef);
		
	// after simulation

	if (mo.z > 0.) 
	{
		if (length(vec2(g) - mo.xy * vec2(image_size)) < mouse_radius) 
		{
			if (mouse_inversion < 0.5)
			{
				color.b = 1.0;
			}
			else
			{
				color.b = 0.0;
			}
		}
	}

	if (reset_substances > 0.5)
	{
		color = vec4(0.0);

		if (use_noise_input > 0.5)
		{
			color = getNoise(g, 0, 0);
		}
	}

	color.a = 1.0;

	return color;
}

void main()
{
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

	vec4 color = conWay(coords, left_mouse);

	imageStore(outColor, coords, color); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ConwayModule_Comp_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += ShaderPass::getXml(vOffset, vUserDatas);
	str += vOffset + "<mouse_radius>" + ct::toStr(m_UBOComp.mouse_radius) + "</mouse_radius>\n";
	str += vOffset + "<mouse_inversion>" + ct::toStr(m_UBOComp.mouse_inversion) + "</mouse_inversion>\n";
	str += vOffset + "<motion_blur_coef>" + ct::toStr(m_UBOComp.motion_blur_coef) + "</motion_blur_coef>\n";
	str += vOffset + "<iterations_count>" + ct::toStr(m_CountIterations.w) + "</iterations_count>\n";

	return str;
}

bool ConwayModule_Comp_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "conway_sim")
	{
		if (strName == "mouse_radius")
			m_UBOComp.mouse_radius = ct::fvariant(strValue).GetF();
		else if (strName == "mouse_inversion")
			m_UBOComp.mouse_inversion = ct::fvariant(strValue).GetF();
		else if (strName == "motion_blur_coef")
			m_UBOComp.motion_blur_coef = ct::fvariant(strValue).GetF();
	}

	return true;
}