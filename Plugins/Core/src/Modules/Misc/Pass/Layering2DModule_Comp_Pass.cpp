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

#include "Layering2DModule_Comp_Pass.h"

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

Layering2DModule_Comp_Pass::Layering2DModule_Comp_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : 2D Layering", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

Layering2DModule_Comp_Pass::~Layering2DModule_Comp_Pass()
{
	Unit();
}

bool Layering2DModule_Comp_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	ZoneScoped;

	ImGui::SetCurrentContext(vContext);

	const float aw = ImGui::GetContentRegionAvail().x;

	bool change = false;

	change |= ImGui::ContrastedComboVectorDefault(aw, "Value method", &m_UBOComp.method, m_MethodNames, 0);

	change |= ImGui::SliderFloatDefaultCompact(aw, "Value smoothness", &m_UBOComp.smoothness, -1.0f, 1.0f, 0.5f);

	change |= ImGui::SliderUIntDefaultCompact(aw, "Layers count ", &m_UBOComp.layer_count, 1U, 200U, 50U);

	change |= ImGui::SliderFloatDefaultCompact(aw, "Layers step scale", &m_UBOComp.step_scale, 0.0f, 0.1f, 0.005f);

	// hidden for the moment because need a slot for define the center and need so to ass some new type of nodes
	//change |= ImGui::SliderFloatDefaultCompact(aw, "Center offset step", &m_UBOComp.center_offset_step, 0.0f, 50.0f, 2.0f);

	if (change)
	{
		NeedNewUBOUpload();
	}

	return change;
}

void Layering2DModule_Comp_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void Layering2DModule_Comp_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void Layering2DModule_Comp_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBindingPoint < m_ImageInfos.size())
		{
			if (vTextureSize)
			{
				if (vTextureSize)
				{
					m_ImageInfosSize[vBindingPoint] = *vTextureSize;

					NeedResizeByHandIfChanged(m_ImageInfosSize[0]);
				}
			}

			if (vImageInfo)
			{
				m_ImageInfos[vBindingPoint] = *vImageInfo;

				if (vBindingPoint == 1U)
				{
					m_UBOComp.use_input_color_buffer = 1.0f;
				}
			}
			else
			{
				if (vBindingPoint == 1U)
				{
					m_UBOComp.use_input_color_buffer = 0.0f;
				}

				m_ImageInfos[vBindingPoint] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* Layering2DModule_Comp_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Layering2DModule_Comp_Pass::WasJustResized()
{
	ZoneScoped;

	if (m_ComputeBufferPtr)
	{
		m_UBOComp.image_size = m_ComputeBufferPtr->GetOutputSize();

		NeedNewUBOUpload();
	}
}

void Layering2DModule_Comp_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);

		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);

		Dispatch(vCmdBuffer);
	}
}

bool Layering2DModule_Comp_Pass::CreateUBO()
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

void Layering2DModule_Comp_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBOCompPtr, &m_UBOComp, sizeof(UBOComp));
}

void Layering2DModule_Comp_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBOCompPtr.reset();
	m_UBOComp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
}

bool Layering2DModule_Comp_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);
	return res;
}

bool Layering2DModule_Comp_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U)); // output
	res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_UBOComp_BufferInfos);
	res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]); // input
	res &= AddOrSetWriteDescriptorImage(3U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[1]); // color
	return res;
}

std::string Layering2DModule_Comp_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "Layering2DModule_Comp_Pass";

	SetLocalGroupSize(ct::uvec3(1U, 1U, 1U));

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform writeonly image2D output_buffer;

layout(std140, binding = 1) uniform UBO_Comp
{
	int method;						// 0 -> 7
	float smoothness;				// 0.0 -> 1.0
	float use_input_color_buffer;	// 0.0 -> 1.0 
	uint count_steps;				// 1 -> 200
	float scale_step;				// 0.0 -> 0.1
	float center_offset_step;		// 0.0 -> 50.0
	ivec2 image_size;				
	ivec2 center_offset;
};

layout(binding = 2) uniform sampler2D input_buffer;
layout(binding = 3) uniform sampler2D input_color_buffer;

vec4 getPixel(ivec2 g)
{
    return texelFetch(input_buffer, g, 0);
}

vec4 getColorPixel(ivec2 g)
{
	if (use_input_color_buffer > 0.5)
	{
		vec2 s = textureSize(input_color_buffer, 0);
		vec2 uv = vec2(g) / s; // input_color_buffer can have different size and also 1x1 size..
		return texture(input_color_buffer, uv);
	}
	
	return vec4(0.0);
}

float getValue(vec4 v) 
{
	switch(method)
	{
	case 0: // r
		return v.r;
	case 1: // g
		return v.g;
	case 2: // b
		return v.b;
	case 3: // a
		return v.a;
	case 4: // length(rg)
		return length(v.rg);
	case 5: // length(rgb)
		return length(v.rgb);
	case 6: // length(rgba)
		return length(v);
	case 7: // median(rgb)
		// https://github.com/Chlumsky/msdfgen
		return max(min(v.r, v.g), min(max(v.r, v.g), v.b));
	}
	return v.r;
}

vec4 getLayer(ivec2 p, float scale)
{
	p -= image_size / 2;
	p = ivec2(p * scale);
	p += image_size / 2;
		
	vec4 color = getColorPixel(p);
	color.a = step(0.95, clamp(getValue(getPixel(p)) * 100.0 * smoothness, 0.0, 1.0));
	color.rgb *= color.a;
	return color;
}

void main()
{
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

	vec4 color = vec4(0,0,0,1);

	// layering of same texture
	float fcount = float(max(count_steps, 1));
	float scale = 1.0 + fcount * scale_step;
	float fog = 0.0;
	float fogStep = 1.0 / fcount;
	float offScale = 1.0 + fcount * center_offset_step;
	for (int i = 0 ; i < count_steps ; ++i)
	{
		scale -= scale_step;
		fog += fogStep;
        offScale -= center_offset_step;
		vec4 c = getLayer(coords + ivec2(center_offset * offScale), scale);
		if (c.a > 0.5) // smart merge for avoid overwrite
			color.rgb = c.rgb * fog;
	}
	
	imageStore(output_buffer, coords, color);
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string Layering2DModule_Comp_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<method>" + ct::toStr(m_UBOComp.method) + "</method>\n";
	str += vOffset + "<smoothness>" + ct::toStr(m_UBOComp.smoothness) + "</smoothness>\n";
	str += vOffset + "<layer_count>" + ct::toStr(m_UBOComp.layer_count) + "</layer_count>\n";
	str += vOffset + "<step_scale>" + ct::toStr(m_UBOComp.step_scale) + "</step_scale>\n";

	return str;
}

bool Layering2DModule_Comp_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strParentName == "layering_2d_module")
	{
		if (strName == "method")
			m_UBOComp.method = ct::ivariant(strValue).GetI();
		else if (strName == "smoothness")
			m_UBOComp.smoothness = ct::fvariant(strValue).GetF();
		else if (strName == "layer_count")
			m_UBOComp.layer_count = ct::uvariant(strValue).GetU();
		else if (strName == "step_scale")
			m_UBOComp.step_scale = ct::fvariant(strValue).GetF();
	}

	return true;
}