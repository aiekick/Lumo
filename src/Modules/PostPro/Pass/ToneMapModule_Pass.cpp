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

#include "ToneMapModule_Pass.h"

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

/*
use the code from https://dmnsgn.github.io/glsl-tone-map/
*/

//////////////////////////////////////////////////////////////
//// ToneMap SECOND PASS : BLUR ////////////////////////////
//////////////////////////////////////////////////////////////

ToneMapModule_Pass::ToneMapModule_Pass(vkApi::VulkanCorePtr vVulkanCorePtr)
	: QuadShaderPass(vVulkanCorePtr, MeshShaderPassType::PIXEL)
{
	SetRenderDocDebugName("Quad Pass : Tone Map", QUAD_SHADER_PASS_DEBUG_COLOR);

	m_DontUseShaderFilesOnDisk = true;
}

ToneMapModule_Pass::~ToneMapModule_Pass()
{
	Unit();
}

void ToneMapModule_Pass::ActionBeforeInit()
{
	m_ToneMap_Algos = { "Aces", "Filmic",  "Lottes",  "Reinhard",  "Reinhard 2",  "Uchimura",  "Uncharted 2",  "Unreal" };

}

bool ToneMapModule_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	bool change = false;

	const float aw = ImGui::GetContentRegionAvail().x;

	change |= ImGui::ContrastedComboVectorDefault(aw, "Configs", &m_UBOFrag.u_tone_map_algo_idx, m_ToneMap_Algos, 0U);

	switch (m_UBOFrag.u_tone_map_algo_idx)
	{
	case 0: // Aces
		change |= ImGui::SliderFloatDefaultCompact(aw, "aces a", &m_UBOFrag.u_aces_a, 0.0f, 5.0f, m_DefaultUBOFrag.u_aces_a);
		change |= ImGui::SliderFloatDefaultCompact(aw, "aces b", &m_UBOFrag.u_aces_b, 0.0f, 0.1f, m_DefaultUBOFrag.u_aces_b);
		change |= ImGui::SliderFloatDefaultCompact(aw, "aces c", &m_UBOFrag.u_aces_c, 0.0f, 5.0f, m_DefaultUBOFrag.u_aces_c);
		change |= ImGui::SliderFloatDefaultCompact(aw, "aces d", &m_UBOFrag.u_aces_d, 0.0f, 1.0f, m_DefaultUBOFrag.u_aces_d);
		change |= ImGui::SliderFloatDefaultCompact(aw, "aces e", &m_UBOFrag.u_aces_e, 0.0f, 1.0f, m_DefaultUBOFrag.u_aces_e);
		break;
	case 1: // Filmic
		change |= ImGui::SliderFloatDefaultCompact(aw, "filmic a", &m_UBOFrag.u_filmic_a, 0.0f, 0.1f, m_DefaultUBOFrag.u_filmic_a);
		change |= ImGui::SliderFloatDefaultCompact(aw, "filmic b", &m_UBOFrag.u_filmic_b, 0.0f, 10.0f, m_DefaultUBOFrag.u_filmic_b);
		change |= ImGui::SliderFloatDefaultCompact(aw, "filmic c", &m_UBOFrag.u_filmic_c, 0.0f, 1.0f, m_DefaultUBOFrag.u_filmic_c);
		change |= ImGui::SliderFloatDefaultCompact(aw, "filmic d", &m_UBOFrag.u_filmic_d, 0.0f, 2.0f, m_DefaultUBOFrag.u_filmic_d);
		change |= ImGui::SliderFloatDefaultCompact(aw, "filmic e", &m_UBOFrag.u_filmic_e, 0.0f, 0.1f, m_DefaultUBOFrag.u_filmic_e);
		change |= ImGui::SliderFloatDefaultCompact(aw, "filmic f", &m_UBOFrag.u_filmic_f, 0.0f, 5.0f, m_DefaultUBOFrag.u_filmic_f);
		break;
	case 2: // Lottes
		change |= ImGui::SliderFloatDefaultCompact(aw, "Lottes a", &m_UBOFrag.u_lottes_a, 0.0f, 2.0f, m_DefaultUBOFrag.u_lottes_a);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Lottes d", &m_UBOFrag.u_lottes_d, 0.0f, 1.0f, m_DefaultUBOFrag.u_filmic_a);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Lottes hdr Max", &m_UBOFrag.u_lottes_hdrMax, 0.0f, 20.0f, m_DefaultUBOFrag.u_lottes_hdrMax);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Lottes mid In", &m_UBOFrag.u_lottes_midIn, 0.0f, 0.1f, m_DefaultUBOFrag.u_lottes_midIn);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Lottes mid Out", &m_UBOFrag.u_lottes_midOut, 0.0f, 0.1f, m_DefaultUBOFrag.u_lottes_midOut);
		break;
	case 3: // Reinhard
	case 4: // Reinhard 2 (no params)
		change |= ImGui::SliderFloatDefaultCompact(aw, "Reinhard2 L White", &m_UBOFrag.u_reinhard2_L_white, 0.0f, 5.0f, m_DefaultUBOFrag.u_reinhard2_L_white);
		break;
	case 5: // Uchimura
		change |= ImGui::SliderFloatDefaultCompact(aw, "Uchimura max brightness", &m_UBOFrag.u_uchimura_max_brightness, 0.0f, 5.0f, m_DefaultUBOFrag.u_uchimura_max_brightness);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Uchimura contrast", &m_UBOFrag.u_uchimura_contrast, 0.0f, 1.0f, m_DefaultUBOFrag.u_uchimura_contrast);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Uchimura linear section start", &m_UBOFrag.u_uchimura_linear_section_start, 0.0f, 1.0f, m_DefaultUBOFrag.u_uchimura_linear_section_start);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Uchimura linear section length", &m_UBOFrag.u_uchimura_linear_section_length, 0.0f, 1.0f, m_DefaultUBOFrag.u_uchimura_linear_section_length);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Uchimura black", &m_UBOFrag.u_uchimura_black, 0.0f, 5.0f, m_DefaultUBOFrag.u_uchimura_black);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Uchimura pedestal", &m_UBOFrag.u_uchimura_pedestal, 0.0f, 1.0f, m_DefaultUBOFrag.u_uchimura_pedestal);
		break;
	case 6: // Uncharted 2
		change |= ImGui::SliderFloatDefaultCompact(aw, "Uncharted 2 a", &m_UBOFrag.u_uncharted2_a, 0.0f, 1.0f, m_DefaultUBOFrag.u_uncharted2_a);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Uncharted 2 b", &m_UBOFrag.u_uncharted2_b, 0.0f, 1.0f, m_DefaultUBOFrag.u_uncharted2_b);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Uncharted 2 c", &m_UBOFrag.u_uncharted2_c, 0.0f, 1.0f, m_DefaultUBOFrag.u_uncharted2_c);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Uncharted 2 d", &m_UBOFrag.u_uncharted2_d, 0.0f, 1.0f, m_DefaultUBOFrag.u_uncharted2_d);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Uncharted 2 e", &m_UBOFrag.u_uncharted2_e, 0.0f, 1.0f, m_DefaultUBOFrag.u_uncharted2_e);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Uncharted 2 f", &m_UBOFrag.u_uncharted2_f, 0.0f, 1.0f, m_DefaultUBOFrag.u_uncharted2_f);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Uncharted 2 w", &m_UBOFrag.u_uncharted2_w, 0.0f, 20.0f, m_DefaultUBOFrag.u_uncharted2_w);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Uncharted 2 Exposure", &m_UBOFrag.u_uncharted2_exposure, 0.0f, 5.0f, m_DefaultUBOFrag.u_uncharted2_exposure);
		break;
	case 7: // Unreal
		change |= ImGui::SliderFloatDefaultCompact(aw, "Unreal a", &m_UBOFrag.u_unreal_a, 0.0f, 2.0f, m_DefaultUBOFrag.u_unreal_a);
		change |= ImGui::SliderFloatDefaultCompact(aw, "Unreal b", &m_UBOFrag.u_unreal_b, 0.0f, 2.0f, m_DefaultUBOFrag.u_unreal_b);
		break;
	}

	if (change)
	{
		NeedNewUBOUpload();
	}

	return change;
}

void ToneMapModule_Pass::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

}

void ToneMapModule_Pass::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

}

void ToneMapModule_Pass::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
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

vk::DescriptorImageInfo* ToneMapModule_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_FrameBufferPtr)
	{
		if (vOutSize)
		{
			*vOutSize = m_FrameBufferPtr->GetOutputSize();
		}

		return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	CTOOL_DEBUG_BREAK;

	return nullptr;
}

bool ToneMapModule_Pass::CreateUBO()
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

void ToneMapModule_Pass::UploadUBO()
{
	ZoneScoped;

	VulkanRessource::upload(m_VulkanCorePtr, *m_UBO_Frag, &m_UBOFrag, sizeof(UBOFrag));
}

void ToneMapModule_Pass::DestroyUBO()
{
	ZoneScoped;

	m_UBO_Frag.reset();
}

bool ToneMapModule_Pass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	m_LayoutBindings.clear();
	m_LayoutBindings.emplace_back(1U, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);
	m_LayoutBindings.emplace_back(2U, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);

	return true;
}

bool ToneMapModule_Pass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	writeDescriptorSets.clear();
	writeDescriptorSets.emplace_back(m_DescriptorSet, 1U, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &m_DescriptorBufferInfo_Frag);
	writeDescriptorSets.emplace_back(m_DescriptorSet, 2U, 0, 1, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0], nullptr); // ssao

	return true;
}

std::string ToneMapModule_Pass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ToneMapModule_Vertex";

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

std::string ToneMapModule_Pass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ToneMapModule_Pass";

	return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 v_uv;

layout (std140, binding = 1) uniform UBO_Frag
{
	uint u_tone_map_algo_idx; // default is 0 (aces)

	float u_aces_a;			// default is 2.51
	float u_aces_b;			// default is 0.03
	float u_aces_c;			// default is 2.43
	float u_aces_d;			// default is 0.59
	float u_aces_e;			// default is 0.14

	float u_filmic_a;			// default is 0.004
	float u_filmic_b;			// default is 6.2
	float u_filmic_c;			// default is 0.5
	float u_filmic_d;			// default is 1.7
	float u_filmic_e;			// default is 0.06
	float u_filmic_f;			// default is 2.2

	float u_lottes_a;		// default is 1.6
	float u_lottes_d;		// default is 0.977
	float u_lottes_hdrMax;	// default is 8.0
	float u_lottes_midIn;	// default is 0.18
	float u_lottes_midOut;	// default is 0.267

	float u_reinhard2_L_white;	// default is 4.0

	float u_uchimura_max_brightness;		// max display brightness // default is 1.0
	float u_uchimura_contrast;				// contrast // default is 1.0
	float u_uchimura_linear_section_start;	// linear section start // default is 0.22
	float u_uchimura_linear_section_length;  // linear section length // default is 0.4
	float u_uchimura_black;					// black // default is 1.33
	float u_uchimura_pedestal;				// pedestal // default is 0.0

	float u_uncharted2_a;	// default is 0.15
	float u_uncharted2_b;	// default is 0.50
	float u_uncharted2_c;	// default is 0.10
	float u_uncharted2_d;	// default is 0.20
	float u_uncharted2_e;	// default is 0.02
	float u_uncharted2_f;	// default is 0.30
	float u_uncharted2_w;	// default is 11.2
	float u_uncharted2_exposure;	// default is 2.0

	float u_unreal_a;	// default is 0.154
	float u_unreal_b;	// default is 1.019
};
layout(binding = 2) uniform sampler2D input_map_sampler;

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

// Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
vec3 aces(vec3 x) {
  //const float a = 2.51;
  //const float b = 0.03;
  //const float c = 2.43;
  //const float d = 0.59;
  //const float e = 0.14;
  return clamp((x * (u_aces_a * x + u_aces_b)) / (x * (u_aces_c * x + u_aces_d) + u_aces_e), 0.0, 1.0);
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

// Filmic Tonemapping Operators http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 filmic(vec3 x) {
  vec3 X = max(vec3(0.0), x - u_filmic_a);
  vec3 result = (X * (u_filmic_b * X + u_filmic_c)) / (X * (u_filmic_b * X + u_filmic_d) + u_filmic_e);
  return pow(result, vec3(u_filmic_f));
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

// Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
vec3 lottes(vec3 x) {
  const vec3 a = vec3(u_lottes_a);
  const vec3 d = vec3(u_lottes_d);
  const vec3 hdrMax = vec3(u_lottes_hdrMax);
  const vec3 midIn = vec3(u_lottes_midIn);
  const vec3 midOut = vec3(u_lottes_midOut);

  const vec3 b =
      (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
      ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
  const vec3 c =
      (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
      ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

  return pow(x, a) / (pow(x, a * d) * b + c);
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

vec3 reinhard(vec3 x) {
  return x / (1.0 + x);
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

vec3 reinhard2(vec3 x) {
  //const float L_white = 4.0;

  return (x * (1.0 + x / (u_reinhard2_L_white * u_reinhard2_L_white))) / (1.0 + x);
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

// Uchimura 2017, "HDR theory and practice"
// Math: https://www.desmos.com/calculator/gslcdxvipg
// Source: https://www.slideshare.net/nikuque/hdr-theory-and-practicce-jp
vec3 uchimura(vec3 x, float P, float a, float m, float l, float c, float b) {
  float l0 = ((P - m) * l) / a;
  float L0 = m - m / a;
  float L1 = m + (1.0 - m) / a;
  float S0 = m + l0;
  float S1 = m + a * l0;
  float C2 = (a * P) / (P - S1);
  float CP = -C2 / P;

  vec3 w0 = vec3(1.0 - smoothstep(0.0, m, x));
  vec3 w2 = vec3(step(m + l0, x));
  vec3 w1 = vec3(1.0 - w0 - w2);

  vec3 T = vec3(m * pow(x / m, vec3(c)) + b);
  vec3 S = vec3(P - (P - S1) * exp(CP * (x - S0)));
  vec3 L = vec3(m + a * (x - m));

  return T * w0 + L * w1 + S * w2;
}

vec3 uchimura(vec3 x) {
  const float P = u_uchimura_max_brightness;  // max display brightness
  const float a = u_uchimura_contrast;  // contrast
  const float m = u_uchimura_linear_section_start; // linear section start
  const float l = u_uchimura_linear_section_length;  // linear section length
  const float c = u_uchimura_black; // black
  const float b = u_uchimura_pedestal;  // pedestal

  return uchimura(x, P, a, m, l, c, b);
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

vec3 uncharted2Tonemap(vec3 x) {
  float A = u_uncharted2_a;
  float B = u_uncharted2_b;
  float C = u_uncharted2_c;
  float D = u_uncharted2_d;
  float E = u_uncharted2_e;
  float F = u_uncharted2_f;
  float W = u_uncharted2_w;
  return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 uncharted2(vec3 color) {
  float exposureBias = u_uncharted2_exposure;
  vec3 curr = uncharted2Tonemap(exposureBias * color);
  vec3 whiteScale = 1.0 / uncharted2Tonemap(vec3(u_uncharted2_w));
  return curr * whiteScale;
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

// Unreal 3, Documentation: "Color Grading"
// Adapted to be close to Tonemap_ACES, with similar range
// Gamma 2.2 correction is baked in, don't use with sRGB conversion!
vec3 unreal(vec3 x) {
  return x / (x + u_unreal_a) * u_unreal_b;
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void main() 
{
	fragColor = vec4(0.0);
	
	vec4 c = texture(input_map_sampler, v_uv);
	if (dot(c,c) > 0.0)
	{
		switch(u_tone_map_algo_idx)
		{
		case 0: fragColor.rgb = aces(c.rgb); break;
		case 1: fragColor.rgb = filmic(c.rgb); break;
		case 2: fragColor.rgb = lottes(c.rgb); break;
		case 3: fragColor.rgb = reinhard(c.rgb); break;
		case 4: fragColor.rgb = reinhard2(c.rgb); break;
		case 5: fragColor.rgb = uchimura(c.rgb); break;
		case 6: fragColor.rgb = uncharted2(c.rgb); break;
		case 7: fragColor.rgb = unreal(c.rgb); break;
		}
		
		fragColor.a = c.a;
	}
	else
	{
		discard;
	}
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ToneMapModule_Pass::GetStructToXMLString(const char* vBalise, float* vStartItem, size_t vCountItem)
{
	if (vBalise && strlen(vBalise) > 0U)
	{
		std::vector<float> arr; arr.resize(vCountItem);
		for (size_t idx = 0u; idx < vCountItem; ++idx) { arr[idx] = vStartItem[idx]; }
		return ct::toStr("<%s>%s</%s>\n", vBalise, ct::fvariant(arr).GetS(';', "%.3f").c_str(), vBalise);
	}

	return "";
}

std::string ToneMapModule_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/)
{
	std::string str;

	str += vOffset + "<algo>" + ct::toStr(m_UBOFrag.u_tone_map_algo_idx) + "</algo>\n";
	str += vOffset + GetStructToXMLString("aces", &m_UBOFrag.u_aces_a, 5U);
	str += vOffset + GetStructToXMLString("filmic", &m_UBOFrag.u_filmic_a, 6U);
	str += vOffset + GetStructToXMLString("lotted", &m_UBOFrag.u_lottes_a, 5U);
	str += vOffset + GetStructToXMLString("reinhard2", &m_UBOFrag.u_reinhard2_L_white, 1U);
	str += vOffset + GetStructToXMLString("uchimura", &m_UBOFrag.u_uchimura_max_brightness, 6U);
	str += vOffset + GetStructToXMLString("uncharted2", &m_UBOFrag.u_uncharted2_a, 8U);
	str += vOffset + GetStructToXMLString("unreal", &m_UBOFrag.u_unreal_a, 2U);

	return str;
}

void ToneMapModule_Pass::LoadStructFromToXMLString(const std::string& vXMLString, float* vStartItem, size_t vCountItem)
{
	if (!vXMLString.empty())
	{
		auto arr = ct::fvariant(vXMLString).GetVectorFloat();
		if (arr.size() == vCountItem)
		{
			uint32_t idx = 0U;
			for (const auto& a : arr)
			{
				vStartItem[idx++] = a;
			}
		}
	}
}
bool ToneMapModule_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/)
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

	if (strParentName == "tone_map_module")
	{
		if (strName == "aces")
			LoadStructFromToXMLString(strValue, &m_UBOFrag.u_aces_a, 5U);
		else if (strName == "filmic")
			LoadStructFromToXMLString(strValue, &m_UBOFrag.u_filmic_a, 6U);
		else if (strName == "lotted")
			LoadStructFromToXMLString(strValue, &m_UBOFrag.u_lottes_a, 5U);
		else if (strName == "reinhard2")
			LoadStructFromToXMLString(strValue, &m_UBOFrag.u_reinhard2_L_white, 1U);
		else if (strName == "uchimura")
			LoadStructFromToXMLString(strValue, &m_UBOFrag.u_uchimura_max_brightness, 6U);
		else if (strName == "uncharted2")
			LoadStructFromToXMLString(strValue, &m_UBOFrag.u_uncharted2_a, 8U);
		else if (strName == "unreal")
			LoadStructFromToXMLString(strValue, &m_UBOFrag.u_unreal_a, 2U);


		NeedNewUBOUpload();
	}

	return true;
}