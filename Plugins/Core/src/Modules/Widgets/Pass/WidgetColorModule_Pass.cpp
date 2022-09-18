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

#include "WidgetColorModule_Pass.h"

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

WidgetColorModule_Pass::WidgetColorModule_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: ShaderPass(vVulkanCorePtr)
{
	SetRenderDocDebugName("Comp Pass : Widget Color", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

WidgetColorModule_Pass::~WidgetColorModule_Pass()
{
	Unit();
}

bool WidgetColorModule_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (ImGui::ColorEdit4Default(0.0f, "Color", &m_UBOComp.u_color.x, &m_DefaultColor.x))
	{
		NeedNewUBOUpload();

		return true;
	}

	return false;
}

void WidgetColorModule_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);
}

void WidgetColorModule_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);
}

bool WidgetColorModule_Pass::DrawNodeWidget(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (ImGui::ColorPicker4DefaultForNode(100.0f, "Color", &m_UBOComp.u_color.x, &m_DefaultColor.x))
	{
		NeedNewUBOUpload();

		return true;
	}

	return false;
}

vk::DescriptorImageInfo* WidgetColorModule_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

void WidgetColorModule_Pass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
		Dispatch(vCmdBuffer);
	}
}

bool WidgetColorModule_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBOCompPtr = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBOComp));
	if (m_UBOCompPtr->buffer)
	{
		m_UBOComp_BufferInfo = vk::DescriptorBufferInfo{ m_UBOCompPtr->buffer, 0, sizeof(UBOComp) };
	}
	else
	{
		m_UBOComp_BufferInfo = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	}

	NeedNewUBOUpload();

	return true;
}

void WidgetColorModule_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, m_UBOCompPtr, &m_UBOComp, sizeof(UBOComp));
}

void WidgetColorModule_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBOCompPtr.reset();
}


bool WidgetColorModule_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);
	res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
	return res;
}

bool WidgetColorModule_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;
	res &= AddOrSetWriteDescriptorImage( 0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U)); // output
	res &= AddOrSetWriteDescriptorBuffer( 1U, vk::DescriptorType::eUniformBuffer, &m_UBOComp_BufferInfo);
	return res;
}

std::string WidgetColorModule_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "WidgetColorModule_Pass";

	SetLocalGroupSize(1U);

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColor;

layout (std140, binding = 1) uniform UBO_Comp
{
	vec4 u_color;
};

void main()
{
	imageStore(outColor, ivec2(gl_GlobalInvocationID.xy), u_color); 
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string WidgetColorModule_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<color>" + m_UBOComp.u_color.string() + "</color>\n";
	
	return str;
}

bool WidgetColorModule_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strParentName == "widget_color_module")
	{
		if (strName == "color")
			m_UBOComp.u_color = ct::fvariant(strValue).GetV4();

		NeedNewUBOUpload();
	}

	return true;
}