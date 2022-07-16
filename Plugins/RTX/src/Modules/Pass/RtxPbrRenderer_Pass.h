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

#pragma once

#include <set>
#include <array>
#include <string>
#include <memory>

#include <Headers/Globals.h>

#include <vulkan/vulkan.hpp>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <Base/QuadShaderPass.h>

#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanDevice.h>
#include <vkFramework/vk_mem_alloc.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/ImGuiTexture.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/VulkanFrameBuffer.h>

#include <SceneGraph/SceneMesh.h>

#include <Interfaces/GuiInterface.h>
#include <Interfaces/TaskInterface.h>
#include <Interfaces/ShaderInterface.h>
#include <Interfaces/TextureInputInterface.h>
#include <Interfaces/ResizerInterface.h>
#include <Interfaces/TextureOutputInterface.h>
#include <Interfaces/TextureGroupInputInterface.h>
#include <Interfaces/LightGroupInputInterface.h>
#include <Interfaces/ModelInputInterface.h>

class RtxPbrRenderer_Pass :
	public QuadShaderPass,
	public GuiInterface,
	public TextureInputInterface<5U>,
	public TextureGroupInputInterface<8U>,
	public LightGroupInputInterface,
	public ModelInputInterface,
	public TextureOutputInterface
{
private:
	const vk::DescriptorBufferInfo m_SceneLightGroupDescriptorInfo = { VK_NULL_HANDLE, 0U, VK_WHOLE_SIZE };
	const vk::DescriptorBufferInfo* m_SceneLightGroupDescriptorInfoPtr = &m_SceneLightGroupDescriptorInfo;

	VulkanBufferObjectPtr m_UBO_Frag = nullptr;
	vk::DescriptorBufferInfo m_DescriptorBufferInfo_Frag;

	struct UBOFrag {
		alignas(4) float u_shadow_strength = 0.5f;
		alignas(4) float u_bias = 0.01f;
		alignas(4) float u_poisson_scale = 5000.0f;
		alignas(4) float u_use_pcf = 1.0f;
		alignas(4) float use_sampler_position = 0.0f;		// position
		alignas(4) float use_sampler_normal = 0.0f;			// normal
		alignas(4) float use_sampler_albedo = 0.0f;			// albedo
		alignas(4) float use_sampler_mask = 0.0f;			// mask
		alignas(4) float use_sampler_ssao = 0.0f;			// ao
		alignas(4) float use_sampler_shadow_maps = 0.0f;	// 8 shadow maps
	} m_UBOFrag;

	std::vector<VulkanAccelStructObjectPtr> m_AccelStructure_Bottom_Ptrs;
	VulkanAccelStructObjectPtr m_AccelStructure_Top_Ptr = nullptr;

public:
	RtxPbrRenderer_Pass(vkApi::VulkanCorePtr vVulkanCorePtr);
	~RtxPbrRenderer_Pass() override;

	bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext = nullptr) override;
	void DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext = nullptr) override;
	void DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext = nullptr) override;
	void SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize) override;
	vk::DescriptorImageInfo* GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize = nullptr) override;
	void SetTextures(const uint32_t& vBinding, DescriptorImageInfoVector* vImageInfos, fvec2Vector* vOutSizes) override;
	void SetLightGroup(SceneLightGroupWeak vSceneLightGroup = SceneLightGroupWeak()) override;
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

private:
	bool BuildModel() override;
	void DestroyModel(const bool& vReleaseDatas = false) override;

	bool CreateUBO() override;
	void UploadUBO() override;
	void DestroyUBO() override;

	bool UpdateLayoutBindingInRessourceDescriptor() override;
	bool UpdateBufferInfoInRessourceDescriptor() override;

	std::string GetVertexShaderCode(std::string& vOutShaderName) override;
	std::string GetFragmentShaderCode(std::string& vOutShaderName) override;

protected: // RTX
	// https://developer.nvidia.com/blog/vulkan-raytracing/

	bool CreateBottomLevelAccelerationStructureForMesh(SceneMeshWeak vMesh);
	void DestroyBottomLevelAccelerationStructureForMesh();

	bool CreateTopLevelAccelerationStructure(std::vector<vk::AccelerationStructureInstanceKHR>& vBlasInstances);
	void DestroyTopLevelAccelerationStructure();

	vk::AccelerationStructureInstanceKHR CreateBlasInstance(const uint32_t& blas_id, glm::mat4& mat);
	
	void CreateShaderBindingTables();

	bool CreateRtxPipeline();
	void DestroyRtxPipeline();

	bool CreateShaderBindingTable();
	void DestroyShaderBindingTable();

	/*
	Ray generation shaders (“raygen”) begin all ray tracing work. 
	A raygen shader runs on a 2D grid of threads, much like a compute shader, 
	and is the starting point for tracing rays into the scene. 
	It is also responsible for writing the final output from the ray 
	tracing algorithm out to memory.
	*/
	virtual std::string GetRayGenerationShaderCode(std::string& vOutShaderName);

	/*
	Intersection shaders implement arbitrary ray-primitive intersection algorithms. 
	They are useful to allow applications to intersect rays with different kinds of 
	primitives (e.g., spheres) that do not have built-in support. 
	Triangle primitives have built-in support and don’t require an intersection shader.
	*/
	virtual std::string GetRayIntersectionShaderCode(std::string& vOutShaderName);

	/*
	Miss shaders are invoked when no intersection is found for a given ray.
	*/
	virtual std::string GetRayMissShaderCode(std::string& vOutShaderName);

	/*
	Hit shaders are invoked when a ray-primitive intersection is found. 
	They are responsible for computing the interactions that happen at an intersection 
	point (e.g., light-material interactions for graphics applications) 
	and can spawn new rays as needed.

	There are two kinds of hit shaders: 
	*/

	/*
	any-hit shaders are invoked on all intersections of a ray with scene primitives, 
	in an arbitrary order, and can reject intersections in addition to computing shading data. 
	*/
	virtual std::string GetRayAnyHitShaderCode(std::string& vOutShaderName);

	/*
	Closest-hit shaders invoke only on the closest intersection point along the ray.
	*/
	virtual std::string GetRayClosestHitShaderCode(std::string& vOutShaderName);
};