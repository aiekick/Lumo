/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

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

#include "MatcapRenderer_Mesh_Pass.h"

#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
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
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

MatcapRenderer_Mesh_Pass::MatcapRenderer_Mesh_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : ShaderPass(vVulkanCore) {
    SetRenderDocDebugName("Mesh Pass 1 : Matcap", MESH_SHADER_PASS_DEBUG_COLOR);

    m_DontUseShaderFilesOnDisk = true;
}

MatcapRenderer_Mesh_Pass::~MatcapRenderer_Mesh_Pass() {
    Unit();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void MatcapRenderer_Mesh_Pass::DrawModel(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) {
    ZoneScoped;

    if (!m_Loaded)
        return;

    if (vCmdBufferPtr) {
        auto modelPtr = m_SceneModel.lock();
        if (!modelPtr || modelPtr->empty())
            return;

        vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipelines[0].m_Pipeline);
        {
            // VKFPScoped(*vCmdBufferPtr, "MatcapRenderer_Mesh_Pass", "DrawModel");

            vCmdBufferPtr->bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics, m_Pipelines[0].m_PipelineLayout, 0, m_DescriptorSets[0].m_DescriptorSet, nullptr);

            for (auto meshPtr : *modelPtr) {
                if (meshPtr) {
                    vk::DeviceSize offsets = 0;
                    vCmdBufferPtr->bindVertexBuffers(0, meshPtr->GetVerticesBuffer(), offsets);

                    if (meshPtr->GetIndicesCount()) {
                        vCmdBufferPtr->bindIndexBuffer(meshPtr->GetIndicesBuffer(), 0, vk::IndexType::eUint32);
                        vCmdBufferPtr->drawIndexed(meshPtr->GetIndicesCount(), 1, 0, 0, 0);
                    } else {
                        vCmdBufferPtr->draw(meshPtr->GetVerticesCount(), 1, 0, 0);
                    }
                }
            }
        }
    }
}

bool MatcapRenderer_Mesh_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    bool change = false;

    change |= ImGui::CheckBoxFloatDefault("Show Face Normal", &m_UBOFrag.show_face_normal, false);

    if (change) {
        NeedNewUBOUpload();
    }

    DrawInputTexture(m_VulkanCore, "Input Matcap", 0U, m_OutputRatio);

    return change;
}

bool MatcapRenderer_Mesh_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool MatcapRenderer_Mesh_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void MatcapRenderer_Mesh_Pass::SetModel(SceneModelWeak vSceneModel) {
    ZoneScoped;

    m_SceneModel = vSceneModel;
}

void MatcapRenderer_Mesh_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) {
    ZoneScoped;

    if (m_Loaded) {
        if (vBindingPoint < m_ImageInfos.size()) {
            if (vImageInfo) {
                m_ImageInfos[vBindingPoint] = *vImageInfo;
            } else {
                auto corePtr = m_VulkanCore.lock();
                assert(corePtr != nullptr);
                m_ImageInfos[vBindingPoint] = *corePtr->getEmptyTexture2DDescriptorImageInfo();
            }
        }
    }
}

vk::DescriptorImageInfo* MatcapRenderer_Mesh_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize) {
    if (m_FrameBufferPtr) {
        AutoResizeBuffer(std::dynamic_pointer_cast<OutputSizeInterface>(m_FrameBufferPtr).get(), vOutSize);

        return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MatcapRenderer_Mesh_Pass::DestroyModel(const bool& vReleaseDatas) {
    ZoneScoped;

    if (vReleaseDatas) {
        m_SceneModel.reset();
    }
}

bool MatcapRenderer_Mesh_Pass::CreateUBO() {
    ZoneScoped;

    m_UBOVertPtr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBOVert), "MatcapRenderer_Mesh_Pass");
    if (m_UBOVertPtr) {
        m_DescriptorBufferInfo_Vert.buffer = m_UBOVertPtr->buffer;
        m_DescriptorBufferInfo_Vert.range = sizeof(UBOVert);
        m_DescriptorBufferInfo_Vert.offset = 0;
    }

    m_UBOFragPtr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBOFrag), "MatcapRenderer_Mesh_Pass");
    if (m_UBOFragPtr) {
        m_DescriptorBufferInfo_Frag.buffer = m_UBOFragPtr->buffer;
        m_DescriptorBufferInfo_Frag.range = sizeof(UBOFrag);
        m_DescriptorBufferInfo_Frag.offset = 0;
    }

    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);
    for (auto& info : m_ImageInfos) {
        info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
    }

    NeedNewUBOUpload();

    return true;
}

void MatcapRenderer_Mesh_Pass::UploadUBO() {
    ZoneScoped;

    VulkanRessource::upload(m_VulkanCore, m_UBOVertPtr, &m_UBOVert, sizeof(UBOVert));
    VulkanRessource::upload(m_VulkanCore, m_UBOFragPtr, &m_UBOFrag, sizeof(UBOFrag));
}

void MatcapRenderer_Mesh_Pass::DestroyUBO() {
    ZoneScoped;

    m_UBOVertPtr.reset();
    m_UBOFragPtr.reset();
}

bool MatcapRenderer_Mesh_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex);
    res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
    return res;
}

bool MatcapRenderer_Mesh_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo());
    res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_DescriptorBufferInfo_Vert);
    res &= AddOrSetWriteDescriptorBuffer(2U, vk::DescriptorType::eUniformBuffer, &m_DescriptorBufferInfo_Frag);
    res &= AddOrSetWriteDescriptorImage(3U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]);  // matcap
    return res;
}

void MatcapRenderer_Mesh_Pass::SetInputStateBeforePipelineCreation() {
    VertexStruct::P3_N3_TA3_BTA3_T2_C4::GetInputState(m_InputState);
}

std::string MatcapRenderer_Mesh_Pass::GetVertexShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "MatcapRenderer_Mesh_Pass_Vertex";

    return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBiTangent;
layout(location = 4) in vec2 aUv;
layout(location = 5) in vec4 aColor;

layout(location = 0) out vec3 vertPosition;
layout(location = 1) out vec3 vertNormal;
layout(location = 2) out vec2 matcapNormal2D;
)" + CommonSystem::GetBufferObjectStructureHeader(0U) +
           u8R"(
layout (std140, binding = 1) uniform UBO_Vert 
{ 
	mat4 transform;
};

vec2 getMatCap(vec3 pos, vec3 nor)
{
	mat4 modelViewMatrix = view * model;
	mat4 normalMatrix = transpose(inverse(modelViewMatrix));
	vec4 pp = modelViewMatrix * vec4(pos, 1. );
	vec4 nn = normalMatrix * vec4(nor, 1.);
	vec3 rd = normalize( pp.xyz );

	vec3 n = normalize( nn.xyz );
	vec3 r = reflect(rd, n);
	r.y *= -1.0;
	float m = 2. * sqrt(
		pow( r.x, 2. ) +
		pow( r.y, 2. ) +
		pow( r.z + 1., 2. )
	);
	return r.xy / m + .5;
}

void main() 
{
	vertPosition = aPosition;
	vertNormal = aNormal;
	matcapNormal2D = getMatCap(vertPosition, aNormal);
	
	gl_Position = cam * transform * vec4(vertPosition, 1.0);
}
)";
}

std::string MatcapRenderer_Mesh_Pass::GetFragmentShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "MatcapRenderer_Mesh_Pass_Fragment";

    return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec2 matcapNormal2D;
)" + CommonSystem::GetBufferObjectStructureHeader(0U) +
           u8R"(
layout(std140, binding = 2) uniform UBO_Frag 
{ 
	float show_face_normal;
};

layout(binding = 3) uniform sampler2D sampler_matcap;

void main() 
{
	fragColor = vec4(0);

	if (dot(vertPosition, vertPosition) > 0.0)
	{
		vec3 vertice_normal = vertNormal;
		if (show_face_normal > 0.5)
		{
			vertice_normal = normalize(-cross(dFdx(vertPosition), dFdy(vertPosition)));
		}

		vec2 tn = matcapNormal2D;
		fragColor = texture(sampler_matcap, tn);
		fragColor.a = 1.0;
	}
	else
	{
		//discard;
	}
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MatcapRenderer_Mesh_Pass::getXml(const std::string& vOffset, const std::string& /*vUserDatas*/) {
    std::string str;

    str += vOffset + "<show_face_normal>" + (m_UBOFrag.show_face_normal > 0.5f ? "true" : "false") + "</show_face_normal>\n";

    return str;
}

bool MatcapRenderer_Mesh_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& /*vUserDatas*/) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "matcap_renderer") {
        if (strName == "show_face_normal")
            m_UBOFrag.show_face_normal = ct::ivariant(strValue).GetB() ? 1.0f : 0.0f;

        NeedNewUBOUpload();
    }

    return true;
}
