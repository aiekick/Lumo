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
#include <string>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <vulkan/vulkan.hpp>

#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanDevice.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/vk_mem_alloc.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/VulkanFrameBuffer.h>
#include <Base/ComputeBuffer.h>
#include <SceneGraph/SceneModel.h>
#include <Interfaces/ShaderUpdateInterface.h>

#include <Utils/Mesh/VertexStruct.h>

#include <Base/Base.h>

enum class GenericType : uint8_t
{
	NONE = 0,
	PIXEL,				// vertex + fragment
	COMPUTE_2D,			// compute image
	COMPUTE_3D,			// compute free
	RTX,				// compute free
	Count
};

class ShaderPass :
	public conf::ConfigAbstract,
	public ShaderUpdateInterface
{
#ifdef VULKAN_DEBUG
public:
	const char* m_RenderDocDebugName = nullptr;
	ct::fvec4 m_RenderDocDebugColor = 0.0f;
#endif

protected: // internal struct
	struct ShaderCode
	{
		std::vector<unsigned int> m_SPIRV;	// SPIRV Bytes
		std::string m_Code;					// Shader Code (in clear)
		std::string m_FilePathName;			// file path name on disk drive
	};

	enum ShaderId : uint8_t
	{
		eVertex = 0,
		eFragment,
		eGeometry,
		eTessEval,
		eTessCtrl,
		eCompute,
		eRtxRayGen,
		eRtxRayInt,
		eRtxRayMiss,
		eRtxRayAnyHit,
		eRtxRayClosestHit
	};

private:
	bool m_NeedNewUBOUpload = false;
	bool m_NeedNewSBOUpload = false;
	bool m_NeedNewModelUpdate = false;

protected:
	bool m_Loaded = false;
	bool m_DontUseShaderFilesOnDisk = false;

	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;	// vulkan core
	vkApi::VulkanQueue m_Queue;					// queue
	vk::CommandPool m_CommandPool;				// command pool
	vk::DescriptorPool m_DescriptorPool;		// descriptor pool
	vk::Device m_Device;						// device copy
	
	vk::SampleCountFlagBits m_SampleCount = vk::SampleCountFlagBits::e1;

	uint32_t m_CountColorBuffers = 0U;

	bool m_ForceFBOClearing = false;

	vk::RenderPass* m_RenderPassPtr = nullptr;

	// Framebuffer
	FrameBufferWeak m_FrameBufferWeak; // when loaned
	FrameBufferPtr m_FrameBufferPtr = nullptr;
	ComputeBufferPtr m_ComputeBufferPtr = nullptr;
	bool m_ResizingIsAllowed = true;

	// dynamic state
	vk::Rect2D m_RenderArea = {};
	vk::Viewport m_Viewport = {};
	ct::fvec2 m_OutputSize;
	float m_OutputRatio = 1.0f;


	std::array<ShaderCode, 11U> m_ShaderCodes;

	bool m_IsShaderCompiled = false;

	// ressources
	std::vector<vk::DescriptorSetLayoutBinding> m_LayoutBindings;
	vk::DescriptorSetLayout m_DescriptorSetLayout = {};
	vk::DescriptorSet m_DescriptorSet = {};
	std::vector<vk::WriteDescriptorSet> writeDescriptorSets;

	// m_Pipeline
	vk::PipelineLayout m_PipelineLayout = {};
	vk::Pipeline m_Pipeline = {};
	vk::PipelineCache m_PipelineCache = {};
	std::vector<vk::PipelineShaderStageCreateInfo> m_ShaderCreateInfos;
	std::vector<vk::PipelineColorBlendAttachmentState> m_BlendAttachmentStates;

	ct::uvec3 m_DispatchSize = 1U;									// COMPUTE dispatch size

	// x:inf, y:sup, z:defaut, w:value
	ct::uvec4 m_CountVertexs = ct::uvec4(0U, 0U, 6U, 6U);			// count vertex to draw
	ct::uvec4 m_CountInstances = ct::uvec4(1U, 1U, 1U, 1U);			// count instances to draw
	ct::uvec4 m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);		// rendering iterations loop

	std::unordered_map<std::string, bool> m_UsedUniforms;			// Used Uniforms

	GenericType m_RendererType = GenericType::NONE;					// Renderer Type
	
	ct::fvec4 m_LineWidth = ct::fvec4(0.0f, 0.0f, 0.0f, 1.0f);		// line width
	vk::PrimitiveTopology m_PrimitiveTopology = vk::PrimitiveTopology::eTriangleList;

	VertexStruct::PipelineVertexInputState m_InputState;

private:
	vk::PushConstantRange m_Internal_PushConstants;

public:
	ShaderPass(vkApi::VulkanCorePtr vVulkanCorePtr);
	ShaderPass(vkApi::VulkanCorePtr vVulkanCorePtr, const GenericType& vRendererTypeEnum);
	ShaderPass(vkApi::VulkanCorePtr vVulkanCorePtr, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool);
	ShaderPass(vkApi::VulkanCorePtr vVulkanCorePtr, const GenericType& vRendererTypeEnum, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool);
	virtual ~ShaderPass();

	// during init
	virtual void ActionBeforeInit();
	virtual void ActionAfterInitSucceed();
	virtual void ActionAfterInitFail();

	// init
	virtual bool InitPixelWithoutFBO(
		const ct::uvec2& vSize,
		const uint32_t& vCountColorBuffers,
		vk::RenderPass* vRenderPassPtr,
		const vk::SampleCountFlagBits& vSampleCount = vk::SampleCountFlagBits::e1); // for this one, a compatible fbo must be set before rendering
	virtual bool InitPixel(
		const ct::uvec2& vSize,
		const uint32_t& vCountColorBuffers,
		const bool& vUseDepth,
		const bool& vNeedToClear,
		const ct::fvec4& vClearColor,
		const bool& vMultiPassMode,
		const vk::Format& vFormat = vk::Format::eR32G32B32A32Sfloat,
		const vk::SampleCountFlagBits& vSampleCount = vk::SampleCountFlagBits::e1);
	virtual bool InitCompute2D(
		const ct::uvec2& vDispatchSize,
		const uint32_t& vCountColorBuffers,
		const bool& vMultiPassMode,
		const vk::Format& vFormat);
	virtual bool InitCompute3D(const ct::uvec3& vDispatchSize);
	virtual bool InitRtx(const ct::uvec2& vDispatchSize);
	virtual void Unit();

	void SetFrameBuffer(FrameBufferWeak vFrameBufferWeak);

	void SetRenderDocDebugName(const char* vLabel, ct::fvec4 vColor);

	void AllowResize(const bool& vResizing); // allow or block the resize
	void NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers); // to call at any moment
	void NeedResize(ct::ivec2* vNewSize); // to call at any moment
	void Resize(const ct::uvec2& vNewSize);
	bool ResizeIfNeeded();

	// Renderer Type
	bool IsPixelRenderer();
	bool IsCompute2DRenderer();
	bool IsCompute3DRenderer();
	bool IsRtxRenderer();

	// FBO
	FrameBufferWeak GetFrameBuffer() { return m_FrameBufferPtr; }

	// Set Wigdet limits
	// vertex / count point to draw / count instances / etc..
	void SetCountVertexs(const uint32_t& vCountVertexs);
	void SetCountInstances(const uint32_t& vCountInstances);
	void SetCountIterations(const uint32_t& vCountIterations);
	void SetLineWidth(const float& vLineWidth);

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	// return true for continue xml parsing of childs in this node or false for interupt the child exploration
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

	// swap and write output descriptors imageinfos, in case of multipass
	// xecuted jsut before compute()
	virtual void SwapOutputDescriptors(); 

	void DrawPass(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber = 1U);

	// draw primitives
	virtual void DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber);
	virtual void Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber);

	virtual void UpdateRessourceDescriptor();

	// shader update from file
	void UpdateShaders(const std::set<std::string>& vFiles) override;

	void NeedToClearFBOThisFrame();

protected:
	virtual void ActionBeforeCompilation();
	virtual void ActionAfterCompilation();

	// Get Shaders
	virtual std::string GetVertexShaderCode(std::string& vOutShaderName);
	virtual std::string GetFragmentShaderCode(std::string& vOutShaderName);
	virtual std::string GetGeometryShaderCode(std::string& vOutShaderName);
	virtual std::string GetTesselationEvaluationShaderCode(std::string& vOutShaderName);
	virtual std::string GetTesselationControlShaderCode(std::string& vOutShaderName);
	virtual std::string GetComputeShaderCode(std::string& vOutShaderName);
	virtual std::string GetRayGenerationShaderCode(std::string& vOutShaderName);
	virtual std::string GetRayIntersectionShaderCode(std::string& vOutShaderName);
	virtual std::string GetRayMissShaderCode(std::string& vOutShaderName);
	virtual std::string GetRayAnyHitShaderCode(std::string& vOutShaderName);
	virtual std::string GetRayClosestHitShaderCode(std::string& vOutShaderName);

	ShaderCode CompilShaderCode(const vk::ShaderStageFlagBits& vShaderType);
	virtual bool CompilPixel();
	virtual bool CompilCompute();
	virtual bool CompilRtx();
	virtual bool ReCompil();

	virtual bool BuildModel();
	void NeedNewModelUpdate();
	virtual void DestroyModel(const bool& vReleaseDatas = false);
	void UpdateModel(const bool& vLoaded);

	// Uniform Buffer Object
	virtual bool CreateUBO();
	void NeedNewUBOUpload();
	virtual void UploadUBO();
	virtual void DestroyUBO();

	// Storage Buffer Object
	virtual bool CreateSBO();
	void NeedNewSBOUpload();
	virtual void UploadSBO();
	virtual void DestroySBO();

	// Ressource Descriptor
	virtual bool UpdateLayoutBindingInRessourceDescriptor();
	virtual bool UpdateBufferInfoInRessourceDescriptor();
	bool CreateRessourceDescriptor();
	void DestroyRessourceDescriptor();

	// push constants
	void SetPushConstantRange(const vk::PushConstantRange& vPushConstantRange);

	// Pipelines
	virtual void SetInputStateBeforePipelineCreation(); // for doing this kind of thing VertexStruct::P2_T2::GetInputState(m_InputState);
	virtual bool CreateComputePipeline();
	virtual bool CreatePixelPipeline();
	virtual bool CreateRtxPipeline();
	void DestroyPipeline();
};