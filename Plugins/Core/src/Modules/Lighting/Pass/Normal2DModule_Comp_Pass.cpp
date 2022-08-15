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

#include "Normal2DModule_Comp_Pass.h"

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

Normal2DModule_Comp_Pass::Normal2DModule_Comp_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : Normal From Texture", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

Normal2DModule_Comp_Pass::~Normal2DModule_Comp_Pass()
{
	Unit();
}

bool Normal2DModule_Comp_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	ZoneScoped;

	ImGui::SetCurrentContext(vContext);

	const float aw = ImGui::GetContentRegionAvail().x;

	bool change = false;

	change |= ImGui::ContrastedComboVectorDefault(aw, "Method", &m_UBOComp.method, m_MethodNames, 0);

	change |= ImGui::SliderFloatDefaultCompact(aw, "Smoothness", &m_UBOComp.smoothness, -1.0f, 1.0f, 0.5f);

	if (change)
	{
		NeedNewUBOUpload();
	}

	return change;
}

void Normal2DModule_Comp_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void Normal2DModule_Comp_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

}

void Normal2DModule_Comp_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBindingPoint < m_ImageInfos.size())
		{
			if (vTextureSize)
			{
				m_ImageInfosSize[vBindingPoint] = *vTextureSize;

				if (vBindingPoint == 0U)
				{
					NeedResizeByHandIfChanged(m_ImageInfosSize[0]);
				}
			}

			if (vImageInfo)
			{
				m_ImageInfos[vBindingPoint] = *vImageInfo;
			}
			else
			{
				m_ImageInfos[vBindingPoint] = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* Normal2DModule_Comp_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

void Normal2DModule_Comp_Pass::WasJustResized()
{
	ZoneScoped;

	if (m_ComputeBufferPtr)
	{
		m_UBOComp.image_size = m_ComputeBufferPtr->GetOutputSize();

		NeedNewUBOUpload();
	}
}

void Normal2DModule_Comp_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipeline);

		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_PipelineLayout, 0, m_DescriptorSet, nullptr);

		Dispatch(vCmdBuffer);
	}
}

bool Normal2DModule_Comp_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBOCompPtr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBOComp));
	m_UBO_Comp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	if (m_UBOCompPtr)
	{
		m_UBO_Comp_BufferInfos.buffer = m_UBOCompPtr->buffer;
		m_UBO_Comp_BufferInfos.range = sizeof(UBOComp);
		m_UBO_Comp_BufferInfos.offset = 0;
	}

	for (auto& info : m_ImageInfos)
	{
		info = *m_VulkanCorePtr->getEmptyTexture2DDescriptorImageInfo();
	}

	NeedNewUBOUpload();

	return true;
}

void Normal2DModule_Comp_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBOCompPtr, &m_UBOComp, sizeof(UBOComp));
}

void Normal2DModule_Comp_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBOCompPtr.reset();
	m_UBO_Comp_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
}

bool Normal2DModule_Comp_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute);
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute);

	return true;
}

bool Normal2DModule_Comp_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();

	assert(m_ComputeBufferPtr);

	writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eStorageImage, 
		m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U), nullptr); // output

	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer,
		nullptr, &m_UBO_Comp_BufferInfos);

	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eCombinedImageSampler,
		&m_ImageInfos[0], nullptr); // input 0

	return true;
}

std::string Normal2DModule_Comp_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "Normal2DModule_Comp_Pass";

	SetLocalGroupSize(ct::uvec3(32U, 32U, 1U));

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform image2D colorBuffer;

layout(std140, binding = 1) uniform UBO_Comp
{
	int method;			// 0 -> 7
	float smoothness;	// 0 -> 1
	ivec2 image_size;
};

layout(binding = 2) uniform sampler2D inputBuffer;

vec4 getPixel(ivec2 g, int x, int y)
{
    return texelFetch(inputBuffer, g + ivec2(x,y), 0);
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

void main()
{
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

	float e = smoothness * 100.0 / min(image_size.x, image_size.y);
	float f = getValue(getPixel(coords, 0, 0));
	float fx = (f-getValue(getPixel(coords, 1, 0)))/e;
	float fy = (f-getValue(getPixel(coords, 0, 1)))/e;
	vec4 color = vec4(normalize(vec3(0,0,1) - vec3(fx,fy,0.0)) * 0.5 + 0.5, 1.0);
	
	imageStore(colorBuffer, coords, color); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string Normal2DModule_Comp_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<method>" + ct::toStr(m_UBOComp.method) + "</method>\n";
	str += vOffset + "<smoothness>" + ct::toStr(m_UBOComp.smoothness) + "</smoothness>\n";

	return str;
}

bool Normal2DModule_Comp_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strParentName == "normal_2d_module")
	{
		if (strName == "method")
			m_UBOComp.method = ct::ivariant(strValue).GetI();
		else if (strName == "smoothness")
			m_UBOComp.smoothness = ct::fvariant(strValue).GetF();
	}

	return true;
}