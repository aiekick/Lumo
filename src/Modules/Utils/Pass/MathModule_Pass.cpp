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
#include <FontIcons/CustomFont.h>

#include <imgui_node_editor/NodeEditor/Include/imgui_node_editor.h>
namespace nd = ax::NodeEditor;

using namespace vkApi;

#define COUNT_BUFFERS 2

//////////////////////////////////////////////////////////////
//// SSAO SECOND PASS : BLUR /////////////////////////////////
//////////////////////////////////////////////////////////////

MathModule_Pass::MathModule_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: QuadShaderPass(vVulkanCorePtr, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Frag Pass : Math", COMPUTE_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

MathModule_Pass::~MathModule_Pass()
{
	Unit();
}

void MathModule_Pass::ActionBeforeInit()
{
	m_Functions.clear();

	// unary
	m_Functions.push_back(MathModuleEntry("cos", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("sin", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("tan", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("acos", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("asin", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("atan", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("acosh", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("asinh", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("atanh", 1U, "v", "", ""));

	m_Functions.push_back(MathModuleEntry("abs", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("floor", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("ceil", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("fract", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("round", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("roundEven", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("sqrt", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("sign", 1U, "v", "", ""));

	m_Functions.push_back(MathModuleEntry("dFdx", 1U, "v", "", "")); // need fragment
	m_Functions.push_back(MathModuleEntry("dFdxCoarse", 1U, "v", "", "")); // need fragment
	m_Functions.push_back(MathModuleEntry("dFdxFine", 1U, "v", "", "")); // need fragment

	m_Functions.push_back(MathModuleEntry("dFdy", 1U, "v", "", "")); // need fragment
	m_Functions.push_back(MathModuleEntry("dFdyCoarse", 1U, "v", "", "")); // need fragment
	m_Functions.push_back(MathModuleEntry("dFdyFine", 1U, "v", "", "")); // need fragment

	m_Functions.push_back(MathModuleEntry("fwidth", 1U, "v", "", "")); // need fragment
	m_Functions.push_back(MathModuleEntry("fwidthCoarse", 1U, "v", "", "")); // need fragment
	m_Functions.push_back(MathModuleEntry("fwidthFine", 1U, "v", "", "")); // need fragment

	m_Functions.push_back(MathModuleEntry("length", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("normalize", 1U, "v", "", ""));

	m_Functions.push_back(MathModuleEntry("log", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("log2", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("exp", 1U, "v", "", ""));
	m_Functions.push_back(MathModuleEntry("exp2", 1U, "v", "", ""));

	// binary
	m_Functions.push_back(MathModuleEntry("atan 2", 1U, "x", "y", ""));
	m_Functions.push_back(MathModuleEntry("cross", 2U, "x", "y", ""));
	m_Functions.push_back(MathModuleEntry("dot", 2U, "x", "y", ""));
	m_Functions.push_back(MathModuleEntry("distance", 2U, "p0", "p1", ""));
	m_Functions.push_back(MathModuleEntry("reflect", 2U, "I", "N", ""));
	m_Functions.push_back(MathModuleEntry("pow", 2U, "x", "y", ""));
	m_Functions.push_back(MathModuleEntry("mod", 2U, "x", "y", ""));
	m_Functions.push_back(MathModuleEntry("step", 2U, "edge", "x", ""));

	m_Functions.push_back(MathModuleEntry("add", 2U, "a", "b", ""));
	m_Functions.push_back(MathModuleEntry("sub", 2U, "a", "b", ""));
	m_Functions.push_back(MathModuleEntry("mul", 2U, "a", "b", ""));
	m_Functions.push_back(MathModuleEntry("div", 2U, "a", "b", ""));

	m_Functions.push_back(MathModuleEntry("min", 2U, "x", "y", ""));
	m_Functions.push_back(MathModuleEntry("max", 2U, "x", "y", ""));

	m_Functions.push_back(MathModuleEntry("smooth abs", 2U, "v", "k", ""));

	// ternary
	m_Functions.push_back(MathModuleEntry("clamp", 3U, "x", "minVal", "maxVal"));
	m_Functions.push_back(MathModuleEntry("mix", 3U, "x", "y", "a"));
	m_Functions.push_back(MathModuleEntry("smoothstep", 3U, "edge0", "edge1", "x"));
	m_Functions.push_back(MathModuleEntry("refract", 3U, "I", "N", "eta"));
	m_Functions.push_back(MathModuleEntry("faceforward", 3U, "N", "I", "Nref"));
}

bool MathModule_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (ImGui::CollapsingHeader("Math", ImGuiTreeNodeFlags_DefaultOpen))
	{
		return DrawCombo(0.0f);
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

void MathModule_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (vBinding < m_ImageInfos.size())
		{
			if (vImageInfo)
			{
				m_ImageInfos[vBinding] = *vImageInfo;

				if (vTextureSize)
				{
					m_ImageInfosSize[vBinding] = *vTextureSize;

					ResizeToMaxOfTexturesIfNeeded();
				}

				if ((&m_UBOFrag.u_use_input_0)[vBinding] < 1.0f)
				{
					(&m_UBOFrag.u_use_input_0)[vBinding] = 1.0f;

					NeedNewUBOUpload();
				}


			}
			else
			{
				if ((&m_UBOFrag.u_use_input_0)[vBinding] > 0.0f)
				{
					(&m_UBOFrag.u_use_input_0)[vBinding] = 0.0f;
					
					NeedNewUBOUpload();
				}

				m_ImageInfos[vBinding] = m_VulkanCorePtr->getEmptyTextureDescriptorImageInfo();
			}
		}
	}
}

vk::DescriptorImageInfo* MathModule_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

bool MathModule_Pass::CreateUBO()
{
	ZoneScoped;

	auto size_in_bytes = sizeof(UBOFrag);
	m_UBO_Frag = VulkanRessource::createUniformBufferObject(m_VulkanCorePtr, size_in_bytes);
	m_DescriptorBufferInfo_Frag.buffer = m_UBO_Frag->buffer;
	m_DescriptorBufferInfo_Frag.range = size_in_bytes;
	m_DescriptorBufferInfo_Frag.offset = 0;

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

	VulkanRessource::upload(m_VulkanCorePtr, *m_UBO_Frag, &m_UBOFrag, sizeof(UBOFrag));
}

void MathModule_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Frag.reset();
}

bool MathModule_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(0U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);

	// max 3 input
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(3U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);

	return true;
}

bool MathModule_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();
	writeDescriptorSets.emplace_back(m_DescriptorSet, 0U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Frag);

	// max 3 input
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], nullptr);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[1], nullptr);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 3U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[2], nullptr);

	return true;
}

std::string MathModule_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "MathModule_Pass_Vertex";

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

std::string MathModule_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "MathModule_Pass_Fragment";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 v_uv;

layout (std140, binding = 0) uniform UBO_Frag 
{ 
	int u_Function_index;
	float u_use_input_0;
	float u_use_input_1;
	float u_use_input_2;
};

layout(binding = 1) uniform sampler2D input_0_map_sampler;
layout(binding = 2) uniform sampler2D input_1_map_sampler;
layout(binding = 3) uniform sampler2D input_2_map_sampler;

void main() 
{
	fragColor = vec4(0);
	
	vec4 input_0 = vec4(0);
	vec4 input_1 = vec4(0);
	vec4 input_2 = vec4(0);
	
	if (u_use_input_0 > 0.5) { input_0 = texture(input_0_map_sampler, v_uv); }
	if (u_use_input_1 > 0.5) { input_1 = texture(input_1_map_sampler, v_uv); }
	if (u_use_input_2 > 0.5) { input_2 = texture(input_2_map_sampler, v_uv); }
	
	switch(u_Function_index)
	{
		// unary
		case 0: fragColor = cos(input_0); break;
		case 1: fragColor = sin(input_0); break;
		case 2: fragColor = tan(input_0); break;
		case 3: fragColor = acos(input_0); break;
		case 4: fragColor = asin(input_0); break;
		case 5: fragColor = atan(input_0); break;
		case 6: fragColor = acosh(input_0); break;
		case 7: fragColor = asinh(input_0); break;
		case 8: fragColor = atanh(input_0); break;
		
		case 9: fragColor = abs(input_0); break;
		case 10: fragColor = floor(input_0); break;
		case 11: fragColor = ceil(input_0); break;
		case 12: fragColor = fract(input_0); break;
		case 13: fragColor = round(input_0); break;
		case 14: fragColor = roundEven(input_0); break;
		case 15: fragColor = sqrt(input_0); break;
		case 16: fragColor = sign(input_0); break;
		
		case 17: fragColor = dFdx(input_0); break;
		case 18: fragColor = dFdxCoarse(input_0); break;
		case 19: fragColor = dFdxFine(input_0); break;
		case 20: fragColor = dFdy(input_0); break;
		case 21: fragColor = dFdyCoarse(input_0); break;
		case 22: fragColor = dFdyFine(input_0); break;
		case 23: fragColor = fwidth(input_0); break;
		case 24: fragColor = fwidthCoarse(input_0); break;
		case 25: fragColor = fwidthFine(input_0); break;

		case 26: fragColor = vec4(length(input_0)); break;
		case 27: fragColor = vec4(normalize(input_0.xyz), 1.0); break;
		
		case 28: fragColor = log(input_0); break;
		case 29: fragColor = log2(input_0); break;
		case 30: fragColor = exp(input_0); break;
		case 31: fragColor = exp2(input_0); break;

		// binary
		case 32: fragColor = atan(input_0, input_1); break;
		case 33: fragColor = vec4(cross(input_0.xyz, input_1.xyz), 1.0); break;
		case 34: fragColor = vec4(dot(input_0, input_1)); break;
		case 35: fragColor = vec4(distance(input_0, input_1)); break;
		case 36: fragColor = reflect(input_0, input_1); break;
		case 37: 
		{
			if (input_0.x > 0.0f && 
				input_0.y > 0.0f && 
				input_0.z > 0.0f && 
				input_0.w > 0.0f &&
				input_1.x > 0.0f &&
				input_1.y > 0.0f &&
				input_1.z > 0.0f &&
				input_1.w > 0.0f)
			{
				fragColor = pow(input_0, input_1);
			}
			break;
		}		
		case 38: fragColor = mod(input_0, input_1); break;
		case 39: fragColor = step(input_0, input_1); break;

		case 40: fragColor = input_0 + input_1; break;
		case 41: fragColor = input_0 - input_1; break;
		case 42: fragColor = input_0 * input_1; break;
		case 43:
		{
			if (input_1.x != 0.0f &&
				input_1.y != 0.0f &&
				input_1.z != 0.0f &&
				input_1.w != 0.0f)
			{
				fragColor = input_0 / input_1;
			}
			break;
		}		
		case 44: fragColor = min(input_0, input_1); break;
		case 45: fragColor = max(input_0, input_1); break;

		case 46: fragColor = sqrt(input_0 * input_0 + abs(input_1)); break;

		// ternary
		case 47: fragColor = clamp(input_0, input_1, input_2); break;
		case 48: fragColor = mix(input_0, input_1, input_2); break;
		case 49: fragColor = smoothstep(input_0, input_1, input_2); break;
		case 50: fragColor = vec4(refract(input_0.xyz, input_1.xyz, input_2.x), 1.0); break;
		case 51: fragColor = vec4(faceforward(input_0.xyz, input_1.xyz, input_2.xyz), 1.0); break;
	}
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MathModule_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<function>" + ct::toStr(m_UBOFrag.u_Function_index) + "</function>\n";

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

	if (strParentName == "math_module")
	{
		if (strName == "function")
			m_UBOFrag.u_Function_index = ct::ivariant(strValue).GetI();

		NeedNewUBOUpload();
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MathModule_Pass::DrawNodeWidget(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(m_UBOFrag.u_Function_index < (int32_t)m_Functions.size());

	ImGui::Text(std::get<0>(m_Functions.at(m_UBOFrag.u_Function_index)).c_str());

	return false;
}

bool MathModule_Pass::DrawCombo(const float vWidth)
{
	bool change = false;

	if (!m_Functions.empty())
	{
		float px = ImGui::GetCursorPosX();

		ImGui::PushID(++ImGui::CustomStyle::Instance()->pushId);

		change = ImGui::ContrastedButton(ICON_NDP_RESET);
		if (change)
			m_UBOFrag.u_Function_index = 0;

		ImGui::CustomSameLine();

		//nd::Suspend();

		change |= ImGui::ContrastedCombo(vWidth, "##Function", &m_UBOFrag.u_Function_index, [](void* data, int idx, const char** out_text)
			{
				*out_text = std::get<0>(((const std::vector<MathModuleEntry>*)data)->at(idx)).c_str();
				return true;
			}, (void*)&m_Functions, (int)m_Functions.size(), -1);


		//nd::Resume();

		ImGui::PopID();
	}

	if (change)
	{
		NeedNewUBOUpload();
	}

	return change;
}

uint32_t MathModule_Pass::GetComponentCount()
{
	if (m_UBOFrag.u_Function_index < (int32_t)m_Functions.size())
	{
		return std::get<1>(m_Functions.at(m_UBOFrag.u_Function_index));
	}

	return 0U;
}

std::string MathModule_Pass::GetInputName(const uint32_t& vIdx)
{
	if (m_UBOFrag.u_Function_index < (int32_t)m_Functions.size() && vIdx < 3U)
	{
		const auto& tu = m_Functions.at(m_UBOFrag.u_Function_index);
		switch (vIdx)
		{
		case 0: return std::get<2>(tu);
		case 1: return std::get<3>(tu);
		case 2: return std::get<4>(tu);
		}
	}

	return "";
}

/// <summary>
/// on va ajuster l'output au plus grand des inputs
/// on veut ni etre trop grand, ni etre trop petit, juste egal au plus rgand des trois
/// </summary>
void MathModule_Pass::ResizeToMaxOfTexturesIfNeeded()
{
	if (m_FrameBufferPtr)
	{
		ct::fvec2 max_size;
		for (const auto& size : m_ImageInfosSize)
		{
			if (size > max_size)
			{
				max_size = size;
			}
		}

		if (!max_size.emptyAND())
		{
			// need resize to max of input textures ?
			ct::fvec2 current_size = m_FrameBufferPtr->GetOutputSize();
			if (IS_FLOAT_DIFFERENT(max_size.x, current_size.x) &&
				IS_FLOAT_DIFFERENT(max_size.y, current_size.y))
			{
				// yes !
				ct::ivec2 new_size = ct::ivec2((int32_t)max_size.x, (int32_t)max_size.y);
				QuadShaderPass::NeedResize(&new_size);
			}
		}
	}
}
