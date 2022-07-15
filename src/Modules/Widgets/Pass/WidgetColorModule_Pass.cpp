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
#include <Modules/Lighting/LightGroupModule.h>

using namespace vkApi;

#define COUNT_BUFFERS 2

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
	assert(vContext);

	ImGui::ColorEdit4Default(0.0f, "Color", &m_UBOComp.u_color.x, &m_DefaultColor.x);

	return false;
}

void WidgetColorModule_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

}

void WidgetColorModule_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

}

bool WidgetColorModule_Pass::DrawNodeWidget(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	return ImGui::ColorEdit4DefaultForNode(0.0f, "Color", & m_UBOComp.u_color.x, & m_DefaultColor.x);
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
		vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipeline);
		vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_PipelineLayout, 0, m_DescriptorSet, nullptr);
		vCmdBuffer->dispatch(m_DispatchSize.x, m_DispatchSize.y, m_DispatchSize.z);
	}
}

bool WidgetColorModule_Pass::CreateUBO()
{
	ZoneScoped;

	m_UBO_Comp = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, sizeof(UBOComp));
	if (m_UBO_Comp->buffer)
	{
		m_UBO_Comp_BufferInfo = vk::DescriptorBufferInfo{ m_UBO_Comp->buffer, 0, sizeof(UBOComp) };
	}
	else
	{
		m_UBO_Comp_BufferInfo = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };
	}

	NeedNewUBOUpload();

	return true;
}

void WidgetColorModule_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, *m_UBO_Comp, &m_UBOComp, sizeof(UBOComp));
}

void WidgetColorModule_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Comp.reset();
}


bool WidgetColorModule_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute);
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute);

	return true;
}

bool WidgetColorModule_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();

	assert(m_ComputeBufferPtr);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eStorageImage, 
		m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U), nullptr); // output
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer,	
		nullptr, &m_UBO_Comp_BufferInfo);

	return true;
}

std::string WidgetColorModule_Pass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "WidgetColorModule_Pass";

	return u8R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColor;

layout (std140, binding = 4) uniform UBO_Comp
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

	//str += vOffset + "<blur_radius>" + ct::toStr(m_UBOComp.u_blur_radius) + "</blur_radius>\n";
	
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

	if (strParentName == "diffuse_module")
	{
		//if (strName == "blur_radius")
		//	m_UBOComp.u_blur_radius = ct::uvariant(strValue).GetU();

		NeedNewUBOUpload();
	}

	return true;
}