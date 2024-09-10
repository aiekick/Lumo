/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "BillBoardRendererModule_Mesh_Pass.h"

#include <cinttypes>
#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
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
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

BillBoardRendererModule_Mesh_Pass::BillBoardRendererModule_Mesh_Pass(GaiApi::VulkanCoreWeak vVulkanCore)
    : QuadShaderPass(vVulkanCore, MeshShaderPassType::PIXEL) {
    ZoneScoped;

    SetRenderDocDebugName("Quad Pass : BillBoard Renderer", QUAD_SHADER_PASS_DEBUG_COLOR);

    m_DontUseShaderFilesOnDisk = true;
}

BillBoardRendererModule_Mesh_Pass::~BillBoardRendererModule_Mesh_Pass() {
    ZoneScoped;

    Unit();
}

void BillBoardRendererModule_Mesh_Pass::ActionBeforeInit() {
    ZoneScoped;

    m_BlendingEnabled = true;

    auto corePtr = m_VulkanCore.lock();
    assert(corePtr != nullptr);

    for (auto& info : m_ImageInfos) {
        info = *corePtr->getEmptyTexture2DDescriptorImageInfo();
    }
}

bool BillBoardRendererModule_Mesh_Pass::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;

    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);

    bool change = false;

    // change |= DrawResizeWidget();

    static UBO_Frag s_UBO_FragDefault;

    change |= ImGui::SliderFloatDefaultCompact(0.0f, "alpha_power", &m_UBO_Frag.u_alpha_power, 0.0f, 1.0f, 0.5f, 0.0f, "%.3f");
    change |= ImGui::SliderFloatDefaultCompact(0.0f, "scale", &m_UBO_Vert.u_scale, 0.0f, 0.05f, 0.01f, 0.0f, "%.3f");
    change |= ImGui::SliderUIntDefaultCompact(
        0.0f, "Count Instances", &m_UBO_Vert.u_count_instances, m_CountInstances.x, m_CountInstances.y, m_CountInstances.z);
    change |= ImGui::ColorEdit3Default(0.0f, "Tint Color", &m_UBO_Frag.u_tint_color.x, &s_UBO_FragDefault.u_tint_color.x);
    if (change) {
        SetCountInstances(m_UBO_Vert.u_count_instances);

        NeedNewUBOUpload();
    }

    return change;
}

bool BillBoardRendererModule_Mesh_Pass::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool BillBoardRendererModule_Mesh_Pass::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    ZoneScoped;
    assert(vContextPtr);
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// MODEL INPUT /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool BillBoardRendererModule_Mesh_Pass::CanRender() {
    return (!m_SceneModel.expired());
}

void BillBoardRendererModule_Mesh_Pass::SetModel(SceneModelWeak vSceneModel) {
    ZoneScoped;

    m_SceneModel = vSceneModel;

    m_Vertices_Vert_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};

    auto inputModelPtr = m_SceneModel.lock();
    if (inputModelPtr && !inputModelPtr->empty()) {
        auto inputMeshPtr = inputModelPtr->at(0).lock();  // only the first mesh for the moment
        if (inputMeshPtr) {
            m_Vertices_Vert_BufferInfos.buffer = inputMeshPtr->GetVerticesBufferInfo()->buffer;
            m_Vertices_Vert_BufferInfos.offset = inputMeshPtr->GetVerticesBufferInfo()->offset;
            m_Vertices_Vert_BufferInfos.range = inputMeshPtr->GetVerticesBufferInfo()->range;

            m_CountInstances.y = inputMeshPtr->GetVerticesCount();

            UpdateBufferInfoInRessourceDescriptor();
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void BillBoardRendererModule_Mesh_Pass::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas) {
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

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* BillBoardRendererModule_Mesh_Pass::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {
    ZoneScoped;

    if (m_FrameBufferPtr) {
        AutoResizeBuffer(std::dynamic_pointer_cast<OutputSizeInterface>(m_FrameBufferPtr).get(), vOutSize);

        return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BillBoardRendererModule_Mesh_Pass::WasJustResized() {
    ZoneScoped;
}

bool BillBoardRendererModule_Mesh_Pass::CreateUBO() {
    ZoneScoped;

    m_UBO_Vert_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBO_Vert), "BillBoardRendererModule_Mesh_Pass");
    m_UBO_Vert_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
    if (m_UBO_Vert_Ptr) {
        m_UBO_Vert_BufferInfos.buffer = m_UBO_Vert_Ptr->buffer;
        m_UBO_Vert_BufferInfos.range = sizeof(UBO_Vert);
        m_UBO_Vert_BufferInfos.offset = 0;
    }

    m_UBO_Frag_Ptr = VulkanRessource::createUniformBufferObject(m_VulkanCore, sizeof(UBO_Frag), "BillBoardRendererModule_Mesh_Pass");
    m_UBO_Frag_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
    if (m_UBO_Frag_Ptr) {
        m_UBO_Frag_BufferInfos.buffer = m_UBO_Frag_Ptr->buffer;
        m_UBO_Frag_BufferInfos.range = sizeof(UBO_Frag);
        m_UBO_Frag_BufferInfos.offset = 0;
    }

    NeedNewUBOUpload();

    return true;
}

void BillBoardRendererModule_Mesh_Pass::UploadUBO() {
    ZoneScoped;

    VulkanRessource::upload(m_VulkanCore, m_UBO_Vert_Ptr, &m_UBO_Vert, sizeof(UBO_Vert));
    VulkanRessource::upload(m_VulkanCore, m_UBO_Frag_Ptr, &m_UBO_Frag, sizeof(UBO_Frag));
}

void BillBoardRendererModule_Mesh_Pass::DestroyUBO() {
    ZoneScoped;

    m_UBO_Vert_Ptr.reset();
    m_UBO_Vert_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};

    m_UBO_Frag_Ptr.reset();
    m_UBO_Frag_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};
}

bool BillBoardRendererModule_Mesh_Pass::UpdateLayoutBindingInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetLayoutDescriptor(0U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex);
    res &= AddOrSetLayoutDescriptor(1U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex);
    res &= AddOrSetLayoutDescriptor(2U, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex);
    res &= AddOrSetLayoutDescriptor(3U, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
    res &= AddOrSetLayoutDescriptor(4U, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
    return res;
}

bool BillBoardRendererModule_Mesh_Pass::UpdateBufferInfoInRessourceDescriptor() {
    ZoneScoped;

    bool res = true;
    res &= AddOrSetWriteDescriptorBuffer(0U, vk::DescriptorType::eUniformBuffer, CommonSystem::Instance()->GetBufferInfo());
    res &= AddOrSetWriteDescriptorBuffer(1U, vk::DescriptorType::eUniformBuffer, &m_UBO_Vert_BufferInfos);
    res &= AddOrSetWriteDescriptorBuffer(2U, vk::DescriptorType::eStorageBuffer, &m_Vertices_Vert_BufferInfos);
    res &= AddOrSetWriteDescriptorBuffer(3U, vk::DescriptorType::eUniformBuffer, &m_UBO_Frag_BufferInfos);
    res &= AddOrSetWriteDescriptorImage(4U, vk::DescriptorType::eCombinedImageSampler, &m_ImageInfos[0]);
    return res;
}

std::string BillBoardRendererModule_Mesh_Pass::GetVertexShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "BillBoardRendererModule_Mesh_Pass_Vertex";

    return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 vertPosition;
layout(location = 1) in vec2 vertUV;
layout(location = 0) out vec2 v_uv;
)" + CommonSystem::GetBufferObjectStructureHeader(0U) +
           u8R"(
layout(std140, binding = 1) uniform UBO_Vert
{
	float u_scale;
	uint u_count_instances;
};

struct V3N3T3B3T2C4 
{
	float px, py, pz;
	float nx, ny, nz;
	float tax, tay, taz;
	float btax, btay, btaz;
	float tx, ty;
	float cx, cy, cz, cw;
};

layout(std430, binding = 2) readonly buffer VertexInput
{
	V3N3T3B3T2C4 inputVertices[];
};

void main() 
{
	v_uv = vertUV;
	const V3N3T3B3T2C4 quad_vertex = inputVertices[gl_InstanceIndex];
	vec4 quad_pos = model * vec4(quad_vertex.px, quad_vertex.py, quad_vertex.pz, 1.0);
	gl_Position = proj * vec4(vertPosition * u_scale + quad_pos.xy, quad_pos.z, 1.0);
}
)";
}

std::string BillBoardRendererModule_Mesh_Pass::GetFragmentShaderCode(std::string& vOutShaderName) {
    vOutShaderName = "BillBoardRendererModule_Mesh_Pass_Fragment";

    return u8R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 v_uv;

layout(std140, binding = 3) uniform UBO_Frag
{
	float u_alpha_power;
	vec3 u_tint_color;
};

layout(binding = 4) uniform sampler2D billboard_sample;

void main() 
{
	fragColor = 
		texture(billboard_sample, v_uv) * 
			vec4(u_tint_color, u_alpha_power);
}
)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string BillBoardRendererModule_Mesh_Pass::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    ZoneScoped;

    std::string str;

    str += ShaderPass::getXml(vOffset, vUserDatas);

    str += vOffset + "<count_instances>" + ct::toStr(m_UBO_Vert.u_count_instances) + "</count_instances>\n";
    str += vOffset + "<scale>" + ct::toStr(m_UBO_Vert.u_scale) + "</scale>\n";
    str += vOffset + "<alpha_power>" + ct::toStr(m_UBO_Frag.u_alpha_power) + "</alpha_power>\n";
    str += vOffset + "<tint_color>" + m_UBO_Frag.u_tint_color.string() + "</tint_color>\n";

    return str;
}

bool BillBoardRendererModule_Mesh_Pass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
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

    if (strParentName == "billboard_renderer_module") {
        if (strName == "count_instances")
            m_UBO_Vert.u_count_instances = ct::uvariant(strValue).GetU();
        else if (strName == "scale")
            m_UBO_Vert.u_scale = ct::fvariant(strValue).GetF();
        else if (strName == "alpha_power")
            m_UBO_Frag.u_alpha_power = ct::fvariant(strValue).GetF();
        else if (strName == "tint_color")
            m_UBO_Frag.u_tint_color = ct::fvariant(strValue).GetV3();
    }

    return true;
}

void BillBoardRendererModule_Mesh_Pass::AfterNodeXmlLoading() {
    ZoneScoped;

    SetCountInstances(m_UBO_Vert.u_count_instances);
    NeedNewUBOUpload();
}
