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

#include <LumoBackend/Headers/LumoBackendDefs.h>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <LumoBackend/Base/BaseRenderer.h>
#include <LumoBackend/Base/MeshShaderPass.h>
#include <Gaia/gaia.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Gui/ImGuiTexture.h>
#include <Gaia/Core/VulkanDevice.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Resources/Texture2D.h>
#include <Gaia/Resources/VulkanRessource.h>
#include <Gaia/Resources/VulkanFrameBuffer.h>

#include <LumoBackend/Interfaces/GuiInterface.h>
#include <LumoBackend/Interfaces/NodeInterface.h>
#include <LumoBackend/Interfaces/ModelInputInterface.h>
#include <LumoBackend/Interfaces/Texture2DInputInterface.h>
#include <LumoBackend/Interfaces/Texture2DOutputInterface.h>

class ModelRendererModule_Mesh_Pass :
	public MeshShaderPass<VertexStruct::P3_N3_TA3_BTA3_T2_C4>,
	public ModelInputInterface,
	public Texture2DInputInterface<2>,
	public Texture2DOutputInterface,
	public NodeInterface
{
public:
	static std::shared_ptr<ModelRendererModule_Mesh_Pass> Create(const ct::uvec2& vSize, GaiApi::VulkanCoreWeak vVulkanCore);

private:
    std::vector<std::string> m_PolygonModes = {"Fill", "Line", "Point"};
    //int32_t m_PolygonModesIndex = 0;

	std::vector<std::string> m_PrimitiveTopologies = {
        "Point List",
        "Line List",
        "Line Strip",
        "Triangle List",
        "Triangle Strip",
        "Triangle Fan",
    };
	
	std::vector<std::string> m_Channels = {
		"Position", 
		"Normal", 
		"Tangeant", 
		"Bi-Tangeant", 
		"Uv", 
		"Color", 
		"Depth",
	};

    struct UBO_Frag {
        alignas(4) int32_t u_show_layer = 0;
        alignas(4) int32_t u_show_shaded_wireframe = 0;
        alignas(4) float u_use_sampler_mask = 0.0f;
    } m_UBO_Frag;
    VulkanBufferObjectPtr m_UBO_Frag_Ptr = nullptr;
    vk::DescriptorBufferInfo m_UBO_Frag_BufferInfos = vk::DescriptorBufferInfo{VK_NULL_HANDLE, 0, VK_WHOLE_SIZE};

    bool m_UseIndiceRestriction = false;
    uint32_t m_RestrictedIndicesCountToDraw = 0U;

    int32_t m_PrimitiveTopologiesIndex = 3;  // Triangle Fan
	struct UBO_Vert {
		alignas(4) float u_point_size = 1.0f;
	} m_UBO_Vert;
	VulkanBufferObjectPtr m_UBO_Vert_Ptr = nullptr;
	vk::DescriptorBufferInfo m_UBO_Vert_BufferInfos = vk::DescriptorBufferInfo{ VK_NULL_HANDLE, 0, VK_WHOLE_SIZE };

public:
	ModelRendererModule_Mesh_Pass(GaiApi::VulkanCoreWeak vVulkanCore);
	~ModelRendererModule_Mesh_Pass() override;

	void ActionBeforeInit() override;
	void WasJustResized() override;

	void DrawModel(vk::CommandBuffer * vCmdBufferPtr, const int& vIterationNumber) override;
	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
	bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
	bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

	// Interfaces Setters
	void SetModel(SceneModelWeak vSceneModel) override;
    void SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize, void* vUserDatas = nullptr) override;

	// Interfaces Getters
    vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr, void* vUserDatas = nullptr) override;

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
	void AfterNodeXmlLoading() override;

protected:
	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	std::string GetVertexShaderCode(std::string& vOutShaderName) override;
	std::string GetFragmentShaderCode(std::string& vOutShaderName) override;
};