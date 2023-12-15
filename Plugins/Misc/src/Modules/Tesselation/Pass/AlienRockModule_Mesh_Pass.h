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

#pragma once

#include <set>
#include <array>
#include <string>
#include <memory>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <Gaia/gaia.h>
#include <LumoBackend/Base/BaseRenderer.h>
#include <LumoBackend/Base/MeshShaderPass.h>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Core/VulkanDevice.h>

#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/Texture2DInputInterface.h>
#include <LumoBackend/Interfaces/Texture2DOutputInterface.h>

class AlienRockModule_Mesh_Pass : public MeshShaderPass<VertexStruct::P3_N3_C4>,
                                  public Texture2DInputInterface<3>,
                                  public Texture2DOutputInterface,

                                  public NodeInterface {
private:
    struct TriFace {
        size_t v0 = 0U, v1 = 0U, v2 = 0U;
        TriFace() = default;
        TriFace(const size_t& vV0, const size_t& vV1, const size_t& vV2) : v0(vV0), v1(vV1), v2(vV2) {
        }
    };

private:
    std::vector<std::string> m_PolygonModes = {"Fill", "Line", "Point"};
    int32_t m_PolygonModesIndex = 0;

    std::vector<std::string> m_PrimitiveTopologies = {
        "Point List",
        "Line List",
        "Line Strip",
        "Triangle List",
        "Triangle Strip",
        "Triangle Fan",
    };
    int32_t m_PrimitiveTopologiesIndex = 5;  // Triangle Fan

    std::vector<std::string> m_Channels = {"Position", "Normal", "Color"};

    typedef std::map<std::tuple<size_t, size_t>, size_t> CacheDB;
    std::vector<TriFace> m_TriFaces;

    struct UBO_Vert {
        alignas(4) float u_point_size = 1.0f;
    } m_UBO_Vert;
    VulkanBufferObjectPtr m_UBO_Vert_Ptr = nullptr;
    vk::DescriptorBufferInfo m_UBO_Vert_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};

    struct UBO_Tess_Ctrl {
        alignas(4) float u_tesselation_level = 1.0f;
    } m_UBO_Tess_Ctrl;
    VulkanBufferObjectPtr m_UBO_Tess_Ctrl_Ptr = nullptr;
    vk::DescriptorBufferInfo m_UBO_Tess_Ctrl_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};

    struct UBO_Tess_Eval {
        alignas(4) float u_use_height_map = 0.0f;
        alignas(4) float u_displace_factor = 1.0f;
        alignas(4) float u_radius = 1.0f;
        alignas(4) float u_normal_prec = 0.01f;
    } m_UBO_Tess_Eval;
    VulkanBufferObjectPtr m_UBO_Tess_Eval_Ptr = nullptr;
    vk::DescriptorBufferInfo m_UBO_Tess_Eval_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};

    struct UBO_Frag {
        alignas(4) int32_t u_use_debug = 0;
        alignas(4) int32_t u_show_layer = 0;
        alignas(4) int32_t u_show_shaded_wireframe = 0;  // position
        alignas(4) float u_use_normal_map = 0.0f;
        alignas(4) float u_use_color_map = 0.0f;
    } m_UBO_Frag;
    VulkanBufferObjectPtr m_UBO_Frag_Ptr = nullptr;
    vk::DescriptorBufferInfo m_UBO_Frag_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};

    uint32_t m_SubdivisionLevel = 0U;

    bool m_UseIndiceRestriction = false;
    uint32_t m_RestrictedIndicesCountToDraw = 0U;

public:
    AlienRockModule_Mesh_Pass(GaiApi::VulkanCoreWeak vVulkanCore);
    ~AlienRockModule_Mesh_Pass() override;

    void ActionBeforeInit() override;
    void WasJustResized() override;
    void DrawModel(vk::CommandBuffer* vCmdBufferPtr, const int& vIterationNumber) override;

    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, const std::string& vUserDatas = {}) override;

    // Interfaces Setters
    void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize = nullptr, void* vUserDatas = nullptr) override;

    // Interfaces Getters
    vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr, void* vUserDatas = nullptr) override;

    std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
    void AfterNodeXmlLoading() override;

private:
    void CreateCube();
    size_t GetMiddlePoint_Plane(const size_t& p1, const size_t& p2, std::vector<VertexStruct::P3_N3_C4>& vVertices, CacheDB& vCache);
    void Subdivide(const size_t& vSubdivLevel, std::vector<VertexStruct::P3_N3_C4>& vVertices, std::vector<TriFace>& vFaces);
    void CalcNormal(VertexStruct::P3_N3_C4& v0, VertexStruct::P3_N3_C4& v1, VertexStruct::P3_N3_C4& v2);
    void BuildMesh();

protected:
    bool CreateUBO() override;
    void UploadUBO() override;
    void DestroyUBO() override;

    bool UpdateLayoutBindingInRessourceDescriptor() override;
    bool UpdateBufferInfoInRessourceDescriptor() override;

    std::string GetCommonCode();
    std::string GetVertexShaderCode(std::string& vOutShaderName) override;
    std::string GetFragmentShaderCode(std::string& vOutShaderName) override;
    std::string GetTesselationEvaluationShaderCode(std::string& vOutShaderName) override;
    std::string GetTesselationControlShaderCode(std::string& vOutShaderName) override;
};