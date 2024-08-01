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

#include "MeshBuffersModule_Mesh_Pass.h"

#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>

#include <Gaia/Buffer/FrameBuffer.h>

#include <ImWidgets.h>

#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>

using namespace GaiApi;

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//// FIRST PASS //////////////////////////////////////////////
//////////////////////////////////////////////////////////////

MeshBuffersModule_Mesh_Pass::MeshBuffersModule_Mesh_Pass(GaiApi::VulkanCoreWeak vVulkanCore) : ShaderPass(vVulkanCore) {
    SetRenderDocDebugName("Mesh Pass 1 : Mesh Attributes", MESH_SHADER_PASS_DEBUG_COLOR);

    m_DontUseShaderFilesOnDisk = true;
}

MeshBuffersModule_Mesh_Pass::~MeshBuffersModule_Mesh_Pass() {
    Unit();
}

bool MeshBuffersModule_Mesh_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    if (ImGui::CollapsingHeader("Attributes")) {
        bool change = false;

        DrawInputTexture(m_VulkanCore, "Input Mask", 0U, m_OutputRatio);

        return change;
    }

    return false;
}

bool MeshBuffersModule_Mesh_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool MeshBuffersModule_Mesh_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void MeshBuffersModule_Mesh_Pass::SetModel(SceneModelWeak vSceneModel) {
    ZoneScoped;

    m_SceneModel = vSceneModel;
}

void MeshBuffersModule_Mesh_Pass::SetTexture(
    const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
    ZoneScoped;

    if (m_Loaded) {
        if (vBindingPoint < m_ImageInfos.size()) {
            if (vImageInfo) {
                m_ImageInfos[vBindingPoint] = *vImageInfo;

                if ((&m_UBOFrag.use_sampler_mask)[vBindingPoint] < 1.0f) {
                    (&m_UBOFrag.use_sampler_mask)[vBindingPoint] = 1.0f;
                    NeedNewUBOUpload();
                }
            } else {
                if ((&m_UBOFrag.use_sampler_mask)[vBindingPoint] > 0.0f) {
                    (&m_UBOFrag.use_sampler_mask)[vBindingPoint] = 0.0f;
                    NeedNewUBOUpload();
                }

                auto corePtr = m_VulkanCore.lock();
                assert(corePtr != nullptr);
                m_ImageInfos[vBindingPoint] = *corePtr->getEmptyTexture2DDescriptorImageInfo();
            }
        }
    }
}

vk::DescriptorImageInfo* MeshBuffersModule_Mesh_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    if (m_FrameBufferPtr) {
        if (vOutSize) {
            *vOutSize = m_FrameBufferPtr->GetOutputSize();
        }

        return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
    }

    return nullptr;
}

void MeshBuffersModule_Mesh_Pass::DrawModel(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) {
    ZoneScoped;

    if (!m_Loaded)
        return;

    if (vCmdBufferPtr) {
        auto modelPtr = m_SceneModel.lock();
        if (!modelPtr || modelPtr->empty())
            return;

        vCmdBufferPtr->bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipelines[0].m_Pipeline);
        {
            // VKFPScoped(*vCmdBufferPtr, "MeshBuffersModule_Mesh_Pass", "DrawModel");

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

void MeshBuffersModule_Mesh_Pass::DestroyModel(const bool& vReleaseDatas) {
    ZoneScoped;

    if (vReleaseDatas) {
        m_SceneModel.reset();
    }
}

bool MeshBuffersModule_Mesh_Pass::CreateUBO() {
    ZoneScoped;

    m_UBOVertPtr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBOVert), "MeshBuffersModule_Mesh_Pass");
    if (m_UBOVertPtr) {
        m_DescriptorBufferInfo_Vert.buffer = m_UBOVertPtr->buffer;
        m_DescriptorBufferInfo_Vert.range = sizeof(UBOVert);
        m_DescriptorBufferInfo_Vert.offset = 0;
    }

    m_UBOFragPtr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBOFrag), "MeshBuffersModule_Mesh_Pass");
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

void MeshBuffersModule_Mesh_Pass::UploadUBO() {
    ZoneScoped;

    VulkanRessource::upload(m_VulkanCore, m_UBOVertPtr, &m_UBOVert, sizeof(UBOVert));
    VulkanRessource::upload(m_VulkanCore, m_UBOFragPtr, &m_UBOFrag, sizeof(UBOFrag));
}

void MeshBuffersModule_Mesh_Pass::DestroyUBO() {
    ZoneScoped;

    m_UBOVertPtr.reset();
    m_UBOFragPtr.reset();
}

bool MeshBuffersModule_Mesh_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
    return res;
}

bool MeshBuffersModule_Mesh_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo());
    res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_DescriptorBufferInfo_Frag);
    res &= AddOrSetWriteDescriptorImage(2U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]);  // mask
    return res;
}

void MeshBuffersModule_Mesh_Pass::SetInputStateBeforePipelineCreation() {
    VertexStruct::P3_N3_TA3_BTA3_T2_C4::GetInputState(m_InputState);
}

std::string MeshBuffersModule_Mesh_Pass::GetVertexShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "MeshBuffersModule_Vertex_Pass";

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
layout(location = 2) out vec3 vertTangent;
layout(location = 3) out vec3 vertBiTangent;
layout(location = 4) out vec2 vertUv;
layout(location = 5) out vec4 vertColor;
)" + CommonSystem::GetBufferObjectStructureHeader(0U) +
           u8R"(
void main() {
    vertPosition = (model * vec4(aPosition, 1.0)).xyz; 
    vertNormal = transpose(inverse(mat3(model))) * aNormal;
	vertTangent = aTangent;
	vertBiTangent = aBiTangent;
	vertUv = aUv;
	vertColor = aColor;
	gl_Position = cam * vec4(aPosition, 1.0);
}
)";
}

std::string MeshBuffersModule_Mesh_Pass::GetFragmentShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "MeshBuffersModule_Fragment_Pass";

    return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragPos;
layout(location = 1) out vec4 fragNor;
layout(location = 2) out vec4 fragTan;
layout(location = 3) out vec4 fragBTan;
layout(location = 4) out vec4 fragUV;
layout(location = 5) out vec4 fragCol;
layout(location = 6) out vec4 fragDep;

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec3 vertTangent;
layout(location = 3) in vec3 vertBiTangent;
layout(location = 4) in vec2 vertUv;
layout(location = 5) in vec4 vertColor;
)" + CommonSystem::GetBufferObjectStructureHeader(0U) +
           u8R"(
layout (std140, binding = 1) uniform UBO_Vert { 
	float use_sampler_mask;
};
layout(binding = 2) uniform sampler2D mask_map_sampler;

void main() {
	fragPos = vec4(vertPosition, 1.0);
	fragNor = vec4(normalize(vertNormal) * 0.5 + 0.5, 1.0);
	fragTan = vec4(vertTangent, 1.0);
	fragBTan = vec4(vertBiTangent, 1.0);
	fragUV = vec4(vertUv, 0.0, 1.0);
	fragCol = vertColor;
	fragDep = vec4(LinearizeDepth(gl_FragCoord.z / gl_FragCoord.w));
	if (use_sampler_mask > 0.5)	{
		if (texture(mask_map_sampler, vertUv).r < 0.5)	{
			discard;
		}
	}
}
)";
}
