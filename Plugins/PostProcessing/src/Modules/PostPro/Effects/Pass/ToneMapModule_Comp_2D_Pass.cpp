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

#include "ToneMapModule_Comp_2D_Pass.h"

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

/*
use the code from https://dmnsgn.github.io/glsl-tone-map/
*/

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<ToneMapModule_Comp_2D_Pass> ToneMapModule_Comp_2D_Pass::Create(const ct::uvec2& vSize, GaiApi::VulkanCoreWeak vVulkanCore) {
    auto res_ptr = std::make_shared<ToneMapModule_Comp_2D_Pass>(vVulkanCore);
    if (!res_ptr->InitCompute2D(vSize, 1U, false, vk::Format::eR32G32B32A32Sfloat)) {
        res_ptr.reset();
    }
    return res_ptr;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

ToneMapModule_Comp_2D_Pass::ToneMapModule_Comp_2D_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : EffectPass(vVulkanCore) {
    SetRenderDocDebugName("Comp Pass : Tone Map", COMPUTE_SHADER_PASS_DEBUG_COLOR);
    m_DontUseShaderFilesOnDisk = true;
}

ToneMapModule_Comp_2D_Pass::~ToneMapModule_Comp_2D_Pass() {
    Unit();
}

void ToneMapModule_Comp_2D_Pass::ActionBeforeInit() {
    // m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);
    m_ToneMap_Algos = {"Aces", "Filmic", "Lottes", "Reinhard", "Reinhard 2", "Uchimura", "Uncharted 2", "Unreal"};
}

void ToneMapModule_Comp_2D_Pass::Compute(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) {
    if (vCmdBufferPtr) {
        vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_Pipeline);
        {
            // VKFPScoped(*vCmdBufferPtr, "ToneMap", "Compute");

            vCmdBufferPtr->bindDescriptorSets(
                vk::PipelineBindPoint::eCompute, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);
            Dispatch(vCmdBufferPtr, "Compute");
        }
    }
}

bool ToneMapModule_Comp_2D_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    bool change = false;

    const float aw = ImGui::GetContentRegionAvail().x;

    if (ImGui::CollapsingHeader_CheckBox("ToneMap##ToneMapModule_Comp_2D_Pass", -1.0f, false, true, IsEffectEnabled())) {
        change |= ImGui::ContrastedComboVectorDefault(aw, "Configs", &m_UBOComp.u_tone_map_algo_idx, m_ToneMap_Algos, 0U);

        switch (m_UBOComp.u_tone_map_algo_idx) {
            case 0:  // Aces
                change |= ImGui::SliderFloatDefaultCompact(aw, "aces a", &m_UBOComp.u_aces_a, 0.0f, 5.0f, m_DefaultUBOComp.u_aces_a);
                change |= ImGui::SliderFloatDefaultCompact(aw, "aces b", &m_UBOComp.u_aces_b, 0.0f, 0.1f, m_DefaultUBOComp.u_aces_b);
                change |= ImGui::SliderFloatDefaultCompact(aw, "aces c", &m_UBOComp.u_aces_c, 0.0f, 5.0f, m_DefaultUBOComp.u_aces_c);
                change |= ImGui::SliderFloatDefaultCompact(aw, "aces d", &m_UBOComp.u_aces_d, 0.0f, 1.0f, m_DefaultUBOComp.u_aces_d);
                change |= ImGui::SliderFloatDefaultCompact(aw, "aces e", &m_UBOComp.u_aces_e, 0.0f, 1.0f, m_DefaultUBOComp.u_aces_e);
                break;
            case 1:  // Filmic
                change |= ImGui::SliderFloatDefaultCompact(aw, "filmic a", &m_UBOComp.u_filmic_a, 0.0f, 0.1f, m_DefaultUBOComp.u_filmic_a);
                change |= ImGui::SliderFloatDefaultCompact(aw, "filmic b", &m_UBOComp.u_filmic_b, 0.0f, 10.0f, m_DefaultUBOComp.u_filmic_b);
                change |= ImGui::SliderFloatDefaultCompact(aw, "filmic c", &m_UBOComp.u_filmic_c, 0.0f, 1.0f, m_DefaultUBOComp.u_filmic_c);
                change |= ImGui::SliderFloatDefaultCompact(aw, "filmic d", &m_UBOComp.u_filmic_d, 0.0f, 2.0f, m_DefaultUBOComp.u_filmic_d);
                change |= ImGui::SliderFloatDefaultCompact(aw, "filmic e", &m_UBOComp.u_filmic_e, 0.0f, 0.1f, m_DefaultUBOComp.u_filmic_e);
                change |= ImGui::SliderFloatDefaultCompact(aw, "filmic f", &m_UBOComp.u_filmic_f, 0.0f, 5.0f, m_DefaultUBOComp.u_filmic_f);
                break;
            case 2:  // Lottes
                change |= ImGui::SliderFloatDefaultCompact(aw, "Lottes a", &m_UBOComp.u_lottes_a, 0.0f, 2.0f, m_DefaultUBOComp.u_lottes_a);
                change |= ImGui::SliderFloatDefaultCompact(aw, "Lottes d", &m_UBOComp.u_lottes_d, 0.0f, 1.0f, m_DefaultUBOComp.u_filmic_a);
                change |=
                    ImGui::SliderFloatDefaultCompact(aw, "Lottes hdr Max", &m_UBOComp.u_lottes_hdrMax, 0.0f, 20.0f, m_DefaultUBOComp.u_lottes_hdrMax);
                change |=
                    ImGui::SliderFloatDefaultCompact(aw, "Lottes mid In", &m_UBOComp.u_lottes_midIn, 0.0f, 0.1f, m_DefaultUBOComp.u_lottes_midIn);
                change |=
                    ImGui::SliderFloatDefaultCompact(aw, "Lottes mid Out", &m_UBOComp.u_lottes_midOut, 0.0f, 0.1f, m_DefaultUBOComp.u_lottes_midOut);
                break;
            case 3:  // Reinhard
            case 4:  // Reinhard 2 (no params)
                change |= ImGui::SliderFloatDefaultCompact(
                    aw, "Reinhard2 L White", &m_UBOComp.u_reinhard2_L_white, 0.0f, 5.0f, m_DefaultUBOComp.u_reinhard2_L_white);
                break;
            case 5:  // Uchimura
                change |= ImGui::SliderFloatDefaultCompact(
                    aw, "Uchimura max brightness", &m_UBOComp.u_uchimura_max_brightness, 0.0f, 5.0f, m_DefaultUBOComp.u_uchimura_max_brightness);
                change |= ImGui::SliderFloatDefaultCompact(
                    aw, "Uchimura contrast", &m_UBOComp.u_uchimura_contrast, 0.0f, 1.0f, m_DefaultUBOComp.u_uchimura_contrast);
                change |= ImGui::SliderFloatDefaultCompact(aw, "Uchimura linear section start", &m_UBOComp.u_uchimura_linear_section_start, 0.0f,
                    1.0f, m_DefaultUBOComp.u_uchimura_linear_section_start);
                change |= ImGui::SliderFloatDefaultCompact(aw, "Uchimura linear section length", &m_UBOComp.u_uchimura_linear_section_length, 0.0f,
                    1.0f, m_DefaultUBOComp.u_uchimura_linear_section_length);
                change |= ImGui::SliderFloatDefaultCompact(
                    aw, "Uchimura black", &m_UBOComp.u_uchimura_black, 0.0f, 5.0f, m_DefaultUBOComp.u_uchimura_black);
                change |= ImGui::SliderFloatDefaultCompact(
                    aw, "Uchimura pedestal", &m_UBOComp.u_uchimura_pedestal, 0.0f, 1.0f, m_DefaultUBOComp.u_uchimura_pedestal);
                break;
            case 6:  // Uncharted 2
                change |=
                    ImGui::SliderFloatDefaultCompact(aw, "Uncharted 2 a", &m_UBOComp.u_uncharted2_a, 0.0f, 1.0f, m_DefaultUBOComp.u_uncharted2_a);
                change |=
                    ImGui::SliderFloatDefaultCompact(aw, "Uncharted 2 b", &m_UBOComp.u_uncharted2_b, 0.0f, 1.0f, m_DefaultUBOComp.u_uncharted2_b);
                change |=
                    ImGui::SliderFloatDefaultCompact(aw, "Uncharted 2 c", &m_UBOComp.u_uncharted2_c, 0.0f, 1.0f, m_DefaultUBOComp.u_uncharted2_c);
                change |=
                    ImGui::SliderFloatDefaultCompact(aw, "Uncharted 2 d", &m_UBOComp.u_uncharted2_d, 0.0f, 1.0f, m_DefaultUBOComp.u_uncharted2_d);
                change |=
                    ImGui::SliderFloatDefaultCompact(aw, "Uncharted 2 e", &m_UBOComp.u_uncharted2_e, 0.0f, 1.0f, m_DefaultUBOComp.u_uncharted2_e);
                change |=
                    ImGui::SliderFloatDefaultCompact(aw, "Uncharted 2 f", &m_UBOComp.u_uncharted2_f, 0.0f, 1.0f, m_DefaultUBOComp.u_uncharted2_f);
                change |=
                    ImGui::SliderFloatDefaultCompact(aw, "Uncharted 2 w", &m_UBOComp.u_uncharted2_w, 0.0f, 20.0f, m_DefaultUBOComp.u_uncharted2_w);
                change |= ImGui::SliderFloatDefaultCompact(
                    aw, "Uncharted 2 Exposure", &m_UBOComp.u_uncharted2_exposure, 0.0f, 5.0f, m_DefaultUBOComp.u_uncharted2_exposure);
                break;
            case 7:  // Unreal
                change |= ImGui::SliderFloatDefaultCompact(aw, "Unreal a", &m_UBOComp.u_unreal_a, 0.0f, 2.0f, m_DefaultUBOComp.u_unreal_a);
                change |= ImGui::SliderFloatDefaultCompact(aw, "Unreal b", &m_UBOComp.u_unreal_b, 0.0f, 2.0f, m_DefaultUBOComp.u_unreal_b);
                break;
        }

        if (change) {
            NeedNewUBOUpload();
        }
    }

    return change;
}

bool ToneMapModule_Comp_2D_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool ToneMapModule_Comp_2D_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void ToneMapModule_Comp_2D_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
    ZoneScoped;

    if (m_Loaded) {
        if (vBindingPoint < m_ImageInfos.size()) {
            if (vImageInfo) {
                if (vTextureSize) {
                    m_ImageInfosSize[vBindingPoint] = *vTextureSize;
                    NeedResizeByHandIfChanged(m_ImageInfosSize[vBindingPoint]);
                }

                m_ImageInfos[vBindingPoint] = *vImageInfo;
            } else {
                auto corePtr = m_VulkanCore.lock();
                assert(corePtr != nullptr);

                m_ImageInfos[vBindingPoint] = *corePtr->getEmptyTexture2DDescriptorImageInfo();
            }
        }
    }
}

vk::DescriptorImageInfo* ToneMapModule_Comp_2D_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    if (m_ComputeBufferPtr) {
        AutoResizeBuffer(std::dynamic_pointer_cast<OutputSizeInterface>(m_ComputeBufferPtr).get(), vOutSize);
        return m_ComputeBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ToneMapModule_Comp_2D_Pass::CreateUBO() {
    ZoneScoped;

    auto size_in_bytes = sizeof(UBOComp);
    m_UBOCompPtr = VulkanRessource::createUniformBufferObject(m_VulkanCore, size_in_bytes, "ToneMapModule_Comp_2D_Pass");
    m_DescriptorBufferInfo_Comp.buffer = m_UBOCompPtr->buffer;
    m_DescriptorBufferInfo_Comp.range = size_in_bytes;
    m_DescriptorBufferInfo_Comp.offset = 0;

    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);

    for (auto& info : m_ImageInfos) {
        info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
    }

    NeedNewUBOUpload();

    return true;
}

void ToneMapModule_Comp_2D_Pass::UploadUBO() {
    ZoneScoped;
    assert(IsEffectEnabled() != nullptr);
    m_UBOComp.u_enabled = (*IsEffectEnabled()) ? 1.0f : 0.0f;
    VulkanRessource::upload(m_VulkanCore, m_UBOCompPtr, &m_UBOComp, sizeof(UBOComp));
}

void ToneMapModule_Comp_2D_Pass::DestroyUBO() {
    ZoneScoped;

    m_UBOCompPtr.reset();
}

bool ToneMapModule_Comp_2D_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute);  // output
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute);
    res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eCompute);
    return res;
}

bool ToneMapModule_Comp_2D_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetWriteDescriptorImage(0U, vk::DescriptorType::eStorageImage, m_ComputeBufferPtr->GetFrontDescriptorImageInfo(0U));  // output
    res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_DescriptorBufferInfo_Comp);
    res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]);  // ssao
    return res;
}

std::string ToneMapModule_Comp_2D_Pass::GetComputeShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "ToneMapModule_Comp_2D_Pass";

    SetLocalGroupSize(ct::uvec3(8U, 8U, 1U));

    return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1 ) in;

layout(binding = 0, rgba32f) uniform writeonly image2D outColor;

layout (std140, binding = 1) uniform UBO_Comp
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

	uint u_enabled; // default is 1
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

void main() {
	const ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	vec4 res = texelFetch(input_map_sampler, coords, 0);	
    if (u_enabled > 0.5) {
	    if (dot(res,res) > 0.0)	{
		    switch(u_tone_map_algo_idx) {
			    case 0: res.rgb = aces(res.rgb); break;
			    case 1: res.rgb = filmic(res.rgb); break;
			    case 2: res.rgb = lottes(res.rgb); break;
			    case 3: res.rgb = reinhard(res.rgb); break;
			    case 4: res.rgb = reinhard2(res.rgb); break;
			    case 5: res.rgb = uchimura(res.rgb); break;
			    case 6: res.rgb = uncharted2(res.rgb); break;
			    case 7: res.rgb = unreal(res.rgb); break;
		    }		
	    } else {
		    // discard;
	    }
    }
	imageStore(outColor, coords, res);
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ToneMapModule_Comp_2D_Pass::GetStructToXMLString(const char* vBalise, float* vStartItem, size_t vCountItem) {
    if (vBalise && strlen(vBalise) > 0U) {
        std::vector<float> arr;
        arr.resize(vCountItem);
        for (size_t idx = 0u; idx < vCountItem; ++idx) {
            arr[idx] = vStartItem[idx];
        }
        return ct::toStr("\t<%s>%s</%s>\n", vBalise, ct::fvariant(arr).GetS(';', "%.3f").c_str(), vBalise);
    }

    return "";
}

std::string ToneMapModule_Comp_2D_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;

    str += vOffset + "<tone_mapping_pass>\n";
    str += ShaderPass::getXml(vOffset + "\t", vUserDatas);
    str += vOffset + "\t<algo>" + ct::toStr(m_UBOComp.u_tone_map_algo_idx) + "</algo>\n";
    str += vOffset + GetStructToXMLString("aces", &m_UBOComp.u_aces_a, 5U);
    str += vOffset + GetStructToXMLString("filmic", &m_UBOComp.u_filmic_a, 6U);
    str += vOffset + GetStructToXMLString("lotted", &m_UBOComp.u_lottes_a, 5U);
    str += vOffset + GetStructToXMLString("reinhard2", &m_UBOComp.u_reinhard2_L_white, 1U);
    str += vOffset + GetStructToXMLString("uchimura", &m_UBOComp.u_uchimura_max_brightness, 6U);
    str += vOffset + GetStructToXMLString("uncharted2", &m_UBOComp.u_uncharted2_a, 8U);
    str += vOffset + GetStructToXMLString("unreal", &m_UBOComp.u_unreal_a, 2U);
    str += vOffset + "\t<enabled>" + ct::toStr(m_UBOComp.u_enabled) + "</enabled>\n";
    str += vOffset + "</tone_mapping_pass>\n";
    return str;
}

void ToneMapModule_Comp_2D_Pass::LoadStructFromToXMLString(const std::string& vXMLString, float* vStartItem, size_t vCountItem) {
    if (!vXMLString.empty()) {
        auto arr = ct::fvariant(vXMLString).GetVectorFloat();
        if (arr.size() == vCountItem) {
            uint32_t idx = 0U;
            for (const auto& a : arr) {
                vStartItem[idx++] = a;
            }
        }
    }
}

bool ToneMapModule_Comp_2D_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "tone_mapping_pass") {
        ShaderPass::setFromXml(vElem, vParent, vUserDatas);
        if (strName == "aces") {
            LoadStructFromToXMLString(strValue, &m_UBOComp.u_aces_a, 5U);
        } else if (strName == "filmic") {
            LoadStructFromToXMLString(strValue, &m_UBOComp.u_filmic_a, 6U);
        } else if (strName == "lotted") {
            LoadStructFromToXMLString(strValue, &m_UBOComp.u_lottes_a, 5U);
        } else if (strName == "reinhard2") {
            LoadStructFromToXMLString(strValue, &m_UBOComp.u_reinhard2_L_white, 1U);
        } else if (strName == "uchimura") {
            LoadStructFromToXMLString(strValue, &m_UBOComp.u_uchimura_max_brightness, 6U);
        } else if (strName == "uncharted2") {
            LoadStructFromToXMLString(strValue, &m_UBOComp.u_uncharted2_a, 8U);
        } else if (strName == "unreal") {
            LoadStructFromToXMLString(strValue, &m_UBOComp.u_unreal_a, 2U);
        } else if (strName == "enabled") {
            m_UBOComp.u_enabled = ct::fvariant(strValue).GetF();
            *IsEffectEnabled() = m_UBOComp.u_enabled;
        } else if (strName == "algo") {
            m_UBOComp.u_tone_map_algo_idx = ct::ivariant(strValue).GetI();
        }
    }

    return true;
}

void ToneMapModule_Comp_2D_Pass::AfterNodeXmlLoading() {
    ZoneScoped;
    NeedNewUBOUpload();
}
