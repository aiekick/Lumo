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

#include "GrayScottModule_Comp_Pass.h"

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

GrayScottModule_Comp_Pass::GrayScottModule_Comp_Pass(GaiApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : GrayScott", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

GrayScottModule_Comp_Pass::~GrayScottModule_Comp_Pass()
{
	Unit();
}

void GrayScottModule_Comp_Pass::ActionBeforeInit()
{
	m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);

	AddGrayScottConfig("Custom##GrayScottModule_Comp_Pass", 0.0f, 0.0f, 0.0f, 0.0f);
	AddGrayScottConfig("Default##GrayScottModule_Comp_Pass", 0.210f, 0.105f, 0.026f, 0.051f);
	AddGrayScottConfig("Solitons##GrayScottModule_Comp_Pass", 0.210f, 0.105f, 0.03f, 0.062f);
	AddGrayScottConfig("Pulsating solitons##GrayScottModule_Comp_Pass", 0.210f, 0.105f, 0.025f, 0.06f);
	AddGrayScottConfig("Worms##GrayScottModule_Comp_Pass", 0.210f, 0.105f, 0.078f, 0.061f);
	AddGrayScottConfig("Mazes##GrayScottModule_Comp_Pass", 0.210f, 0.105f, 0.029f, 0.057f);
	AddGrayScottConfig("Holes##GrayScottModule_Comp_Pass", 0.210f, 0.105f, 0.039f, 0.058f);
	AddGrayScottConfig("Chaos and holes##GrayScottModule_Comp_Pass", 0.210f, 0.105f, 0.034f, 0.056f);
	AddGrayScottConfig("Moving spots##GrayScottModule_Comp_Pass", 0.210f, 0.105f, 0.014f, 0.054f);
	AddGrayScottConfig("Spots and loops##GrayScottModule_Comp_Pass", 0.210f, 0.105f, 0.018f, 0.051f);
	AddGrayScottConfig("Waves##GrayScottModule_Comp_Pass", 0.210f, 0.105f, 0.014f, 0.045f);
	AddGrayScottConfig("The U-Skate world##GrayScottModule_Comp_Pass", 0.210f, 0.105f, 0.062f, 0.061f);
	AddGrayScottConfig("Herisson##GrayScottModule_Comp_Pass", 0.403f, 0.068f, 0.014f, 0.045f);
}

bool GrayScottModule_Comp_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	ZoneScoped;

	ImGui::SetCurrentContext(vContextPtr);

	const float aw = ImGui::GetContentRegionAvail().x;

	bool change = false;

	change |= DrawResizeWidget();

	ImGui::Header("Pass Iterations##GrayScottModule_Comp_Pass");

	change |= ImGui::SliderUIntDefaultCompact(aw, "Iterations##GrayScottModule_Comp_Pass", &m_CountIterations.w, m_CountIterations.x, m_CountIterations.y, m_CountIterations.z);

	ImGui::Header("Mouse##GrayScottModule_Comp_Pass");

	change |= ImGui::SliderFloatDefaultCompact(aw, "Mouse Radius##GrayScottModule_Comp_Pass", &m_UBOComp.mouse_radius, 0.0f, 100.0f, 10.0f);
	if (ImGui::RadioButtonLabeled(aw, "Mouse Inversion##GrayScottModule_Comp_Pass", m_UBOComp.mouse_inversion > 0.5f, false))
	{
		m_UBOComp.mouse_inversion = (m_UBOComp.mouse_inversion > 0.5f) ? 0.0f : 1.0f;
		change = true;
	}

	ImGui::Header("GrayScott##GrayScottModule_Comp_Pass");

	if (ImGui::ContrastedButton("Fast Diffusion Config##GrayScottModule_Comp_Pass", nullptr, nullptr, aw))
	{
		m_UBOComp.grayscott_diffusion_u = 0.0f;
		m_UBOComp.grayscott_diffusion_v = 1.0f;
		m_SelectedGrayScottConfig = 0;
		m_UBOComp.grayscott_feed = 0.026f;
		m_UBOComp.grayscott_kill = 0.030f;
		change = true;
	}

	change |= ImGui::SliderFloatDefaultCompact(aw, "GrayScott Diffusion u##GrayScottModule_Comp_Pass", &m_UBOComp.grayscott_diffusion_u, 0.0f, 1.0f, 0.2097f);
	change |= ImGui::SliderFloatDefaultCompact(aw, "GrayScott Diffusion v##GrayScottModule_Comp_Pass", &m_UBOComp.grayscott_diffusion_v, 0.0f, 1.0f, 0.105f);

	if (ImGui::ContrastedComboVectorDefault(aw, "Configs##GrayScottModule_Comp_Pass", &m_SelectedGrayScottConfig, m_GrayScottConfigNames, 0))
	{
		if (m_SelectedGrayScottConfig)
		{
			auto fv = m_GrayScottConfigs[m_SelectedGrayScottConfig];
			if (fv.x > 0.0f) { m_UBOComp.grayscott_diffusion_u = fv.x; }
			if (fv.y > 0.0f) { m_UBOComp.grayscott_diffusion_v = fv.y; }
			m_UBOComp.grayscott_feed = fv.z;
			m_UBOComp.grayscott_kill = fv.w;
			change = true;
		}
	}

	change |= ImGui::SliderFloatDefaultCompact(aw, "GrayScott Feed##GrayScottModule_Comp_Pass", &m_UBOComp.grayscott_feed, 0.0f, 0.2f, 0.026f);
	change |= ImGui::SliderFloatDefaultCompact(aw, "GrayScott Kill##GrayScottModule_Comp_Pass", &m_UBOComp.grayscott_kill, 0.0f, 0.2f, 0.051f);

	ImGui::Header("Clear##GrayScottModule_Comp_Pass");

	if (ImGui::ContrastedButton("Reset Substances##GrayScottModule_Comp_Pass", nullptr, nullptr, aw))
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

bool GrayScottModule_Comp_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool GrayScottModule_Comp_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void GrayScottModule_Comp_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
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
			}
			else
			{
				m_ImageInfos[vBindingPoint] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* GrayScottModule_Comp_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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
//// PRIVATE / GRAYSCOTT ///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GrayScottModule_Comp_Pass::ClearGrayScottConfigs()
{
	ZoneScoped;

	m_GrayScottConfigNames.clear();
	m_GrayScottConfigs.clear();
	m_SelectedGrayScottConfig = 0; // default is custom
}

void GrayScottModule_Comp_Pass::AddGrayScottConfig(const std::string& vConfigName, const float& vDiffXValue, const float& vDiffYValue, const float& vFeedValue, const float& vKillValue)
{
	ZoneScoped;

	m_GrayScottConfigNames.push_back(vConfigName);
	m_GrayScottConfigs.push_back(ct::fvec4(vDiffXValue, vDiffYValue, vFeedValue, vKillValue));
}

void GrayScottModule_Comp_Pass::WasJustResized()
{
	ZoneScoped;

	if (m_UBOComp.image_size.x != m_OutputSize.x ||
		m_UBOComp.image_size.y != m_OutputSize.y)
	{
		m_UBOComp.image_size = m_OutputSize;

		NeedNewUBOUpload();
	}
}

void GrayScottModule_Comp_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);

		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);

		for (uint32_t iter = 0; iter < m_CountIterations.w; iter++)
		{
			Dispatch(vCmdBuffer);
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

bool GrayScottModule_Comp_Pass::CreateUBO()
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

void GrayScottModule_Comp_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBOCompPtr, &m_UBOComp, sizeof(UBOComp));
}

void GrayScottModule_Comp_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBOCompPtr.reset();
	m_UBOComp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
}

bool GrayScottModule_Comp_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);
	return res;
}

bool GrayScottModule_Comp_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U)); // output
	res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo()); // common system
	res &= AddOrSetWriteDescriptorBuffer(2U, vk::DescriptorType::eUniformBuffer, &m_UBOComp_BufferInfos);
	res &= AddOrSetWriteDescriptorImage(3U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]); // input 0
	return res;
}

std::string GrayScottModule_Comp_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "GrayScottModule_Comp_Pass";

	// with 8 i have some slow down maybe due the fact than i read an write from same image2D
	SetLocalGroupSize(ct::uvec3(1U, 1U, 1U));

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform image2D colorBuffer;
)"
+ CommonSystem::GetBufferObjectStructureHeader(1U) +
u8R"(
layout(std140, binding = 2) uniform UBO_Comp
{
	float mouse_radius;
	float mouse_inversion;
	float reset_substances;
	float grayscott_diffusion_u;
	float grayscott_diffusion_v;
	float grayscott_feed;
	float grayscott_kill;
	ivec2 image_size;
};

layout(binding = 3) uniform sampler2D input_mask;

vec4 getPixel(ivec2 g, int x, int y)
{
    ivec2 v = (g + ivec2(x,y)) % image_size;
	return imageLoad(colorBuffer, v);
}

/* laplacian corner ratio */	#define lc .2
/* laplacian side ratio */ 		#define ls .8

vec4 grayScott(ivec2 g, vec4 mo)
{
    vec4 l 	= getPixel(g, -1,  0);
	vec4 lt = getPixel(g, -1,  1);
	vec4 t 	= getPixel(g,  0,  1);
	vec4 rt = getPixel(g,  1,  1);
	vec4 r 	= getPixel(g,  1,  0);
	vec4 rb = getPixel(g,  1, -1);
	vec4 b 	= getPixel(g,  0, -1);
	vec4 lb = getPixel(g, -1, -1);
	vec4 c 	= getPixel(g,  0,  0);
	vec4 lap = (l+t+r+b)/4.*ls + (lt+rt+rb+lb)/4.*lc - c; // laplacian

	float re = c.x * c.y * c.y; // reaction
    c.xy += vec2(grayscott_diffusion_u, grayscott_diffusion_v) * lap.xy + 
		vec2(grayscott_feed * (1. - c.x) - re, re - (grayscott_feed + grayscott_kill) * c.y); // grayscott formula
	
	/*if (length(vec2(g - image_size / 2)) < mouse_radius) 
	{
		c = vec4(0,1,0,1);
	}*/

	if (mo.z > 0.) 
	{
		if (length(vec2(g) - mo.xy * vec2(image_size)) < mouse_radius) 
		{
			if (mouse_inversion > 0.5)
			{
				c = vec4(1,0,0,1);
			}
			else
			{
				c = vec4(0,1,0,1);
			}
		}
	}

	if (reset_substances > 0.5)
	{
		c = vec4(1,0,0,1);	
	}

	return vec4(clamp(c.xy, -1e1, 1e1), 0, 1);
}

void main()
{
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

	vec4 color = grayScott(coords, left_mouse);

	imageStore(colorBuffer, coords, color); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GrayScottModule_Comp_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += ShaderPass::getXml(vOffset, vUserDatas);
	str += vOffset + "<mouse_radius>" + ct::toStr(m_UBOComp.mouse_radius) + "</mouse_radius>\n";
	str += vOffset + "<mouse_inversion>" + ct::toStr(m_UBOComp.mouse_inversion) + "</mouse_inversion>\n";
	str += vOffset + "<grayscott_diffusion_u>" + ct::toStr(m_UBOComp.grayscott_diffusion_u) + "</grayscott_diffusion_u>\n";
	str += vOffset + "<grayscott_diffusion_v>" + ct::toStr(m_UBOComp.grayscott_diffusion_v) + "</grayscott_diffusion_v>\n";
	str += vOffset + "<grayscott_feed>" + ct::toStr(m_UBOComp.grayscott_feed) + "</grayscott_feed>\n";
	str += vOffset + "<grayscott_kill>" + ct::toStr(m_UBOComp.grayscott_kill) + "</grayscott_kill>\n";
	str += vOffset + "<iterations_count>" + ct::toStr(m_CountIterations.w) + "</iterations_count>\n";
	str += vOffset + "<simulation_config>" + ct::toStr(m_SelectedGrayScottConfig) + "</simulation_config>\n";

	return str;
}

bool GrayScottModule_Comp_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "gray_scott_sim")
	{
		if (strName == "mouse_radius")
			m_UBOComp.mouse_radius = ct::fvariant(strValue).GetF();
		else if (strName == "mouse_inversion")
			m_UBOComp.mouse_inversion = ct::fvariant(strValue).GetF();
		else if (strName == "grayscott_diffusion_u")
			m_UBOComp.grayscott_diffusion_u = ct::fvariant(strValue).GetF();
		else if (strName == "grayscott_diffusion_v")
			m_UBOComp.grayscott_diffusion_v = ct::fvariant(strValue).GetF();
		else if (strName == "grayscott_feed")
			m_UBOComp.grayscott_feed = ct::fvariant(strValue).GetF();
		else if (strName == "grayscott_kill")
			m_UBOComp.grayscott_kill = ct::fvariant(strValue).GetF();
		else if (strName == "iterations_count")
			m_CountIterations.w = ct::uvariant(strValue).GetU();
		else if (strName == "simulation_config")
			m_SelectedGrayScottConfig = ct::ivariant(strValue).GetI();
	}

	return true;
}