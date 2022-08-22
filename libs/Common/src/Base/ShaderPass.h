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
#include <map>
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
	COMPUTE_1D,			// compute image
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
		std::string m_EntryPoint;			// entry point
		vk::ShaderModule m_ShaderModule = nullptr;
		bool m_Used = false;				// say if a sahder mut be take into account
		vk::ShaderStageFlagBits m_ShaderId = vk::ShaderStageFlagBits::eVertex;
	};

	struct DescriptorSetStruct
	{
		vk::DescriptorSet m_DescriptorSet = {};
		vk::DescriptorSetLayout m_DescriptorSetLayout = {};
		std::vector<vk::DescriptorSetLayoutBinding> m_LayoutBindings = {};
		std::vector<vk::WriteDescriptorSet> m_WriteDescriptorSets = {};
	};

	struct PipelineStruct
	{
		vk::PipelineLayout m_PipelineLayout = {};
		vk::Pipeline m_Pipeline = {};
	};

private:
	bool m_NeedNewUBOUpload = false;
	bool m_NeedNewSBOUpload = false;
	bool m_NeedNewModelUpdate = false;

private:
	// compute stage
	ct::uvec3 m_DispatchSize = 1U;			// COMPUTE dispatch size
	ct::uvec3 m_LocalGroupSize = 1U;		// COMPUTE local group size

private:
	bool m_CanDynamicallyChangePrimitiveTopology = false;
	vk::PrimitiveTopology m_BasePrimitiveTopology = vk::PrimitiveTopology::eTriangleList;
	vk::PrimitiveTopology m_DynamicPrimitiveTopology = vk::PrimitiveTopology::eTriangleList;

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
	bool m_ResizingByResizeEventIsAllowed = true;
	bool m_ResizingByHandIsAllowed = true;
	float m_BufferQuality = 1.0f; // smaller value give bigger resolution

	// dynamic state
	vk::Rect2D m_RenderArea = {};
	vk::Viewport m_Viewport = {};
	ct::fvec2 m_OutputSize;
	float m_OutputRatio = 1.0f;

	std::map<vk::ShaderStageFlagBits, std::set<ShaderEntryPoint>> m_ShaderEntryPoints;
	std::map<vk::ShaderStageFlagBits, std::map<ShaderEntryPoint, std::vector<ShaderCode>>> m_ShaderCodes;

	bool m_IsShaderCompiled = false;
	bool m_DescriptorWasUpdated = false;

	// ressources
	std::vector<DescriptorSetStruct> m_DescriptorSets = {DescriptorSetStruct()};
	
	// m_Pipelines[0]
	std::vector<PipelineStruct> m_Pipelines = {PipelineStruct() }; // one entry by default
	vk::PipelineCache m_PipelineCache = {};
	std::vector<vk::PipelineShaderStageCreateInfo> m_ShaderCreateInfos;
	std::vector<vk::PipelineColorBlendAttachmentState> m_BlendAttachmentStates;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_RayTracingShaderGroups;        // Shader groups

	// x:inf, y:sup, z:defaut, w:value
	ct::uvec4 m_CountVertexs = ct::uvec4(0U, 0U, 6U, 6U);			// count vertex to draw
	ct::uvec4 m_CountInstances = ct::uvec4(1U, 1U, 1U, 1U);			// count instances to draw
	ct::uvec4 m_CountIterations = ct::uvec4(0U, 10U, 1U, 1U);		// rendering iterations loop

	std::unordered_map<std::string, bool> m_UsedUniforms;			// Used Uniforms

	GenericType m_RendererType = GenericType::NONE;					// Renderer Type
	
	bool m_BlendingEnabled = false;

	ct::fvec4 m_LineWidth = ct::fvec4(0.0f, 0.0f, 0.0f, 1.0f);		// line width
	vk::PolygonMode m_PolygonMode = vk::PolygonMode::eFill;
	vk::CullModeFlagBits m_CullMode = vk::CullModeFlagBits::eNone;
	vk::FrontFace m_FrontFaceMode = vk::FrontFace::eCounterClockwise;

	VertexStruct::PipelineVertexInputState m_InputState;

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
	virtual bool InitCompute1D(
		const uint32_t& vDispatchSize);
	virtual bool InitCompute2D(
		const ct::uvec2& vDispatchSize,
		const uint32_t& vCountColorBuffers,
		const bool& vMultiPassMode,
		const vk::Format& vFormat);
	virtual bool InitCompute3D(const ct::uvec3& vDispatchSize);
	virtual bool InitRtx( // similar to InitCompute2D
		const ct::uvec2& vDispatchSize,
		const uint32_t& vCountColorBuffers,
		const bool& vMultiPassMode,
		const vk::Format& vFormat);
	virtual void Unit();

	void SetFrameBuffer(FrameBufferWeak vFrameBufferWeak);

	void SetRenderDocDebugName(const char* vLabel, ct::fvec4 vColor);

	/// <summary>
	/// allow or block resize by resize event
	/// </summary>
	/// <param name="vResizing"></param>
	void AllowResizeOnResizeEvents(const bool& vResizing); // allow or block the resize

	/// <summary>
	/// allod of block reisze by hand only
	/// </summary>
	/// <param name="vResizing"></param>
	void AllowResizeByHandOrByInputs(const bool& vResizing); // allow or block the resize

	/// <summary>
	/// to call when from resize event
	/// </summary>
	/// <param name="vNewSize"></param>
	/// <param name="vCountColorBuffers"></param>
	void NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers = nullptr); // to call at any moment

	/// <summary>
	/// will resize if the new size is different
	/// </summary>
	/// <param name="vNewSize"></param>
	/// <returns></returns>
	bool NeedResizeByResizeEventIfChanged(ct::ivec2 vNewSize);

	/// <summary>
	/// to call when from hand
	/// </summary>
	/// <param name="vNewSize"></param>
	/// <param name="vCountColorBuffers"></param>
	void NeedResizeByHand(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers = nullptr); // to call at any moment

	/// <summary>
	/// will resize if the new size is different
	/// </summary>
	bool NeedResizeByHandIfChanged(ct::ivec2 vNewSize);

	/// <summary>
	/// will resize if the resize is need by FBO or compute buffers
	/// </summary>
	/// <returns></returns>
	bool ResizeIfNeeded();

	/// <summary>
	/// to call (herited), inform than a resized was jsut done
	/// </summary>
	virtual void WasJustResized();

	// Renderer Type
	bool IsPixelRenderer();
	bool IsCompute2DRenderer();
	bool IsCompute3DRenderer();
	bool IsRtxRenderer();

	// FBO
	FrameBufferWeak GetFrameBuffer() { return m_FrameBufferPtr; }

	/// <summary>
	/// set the compute local group size
	/// must be set before call to SetDispatchSize functions
	/// </summary>
	/// <param name="vLocalGroupSize"></param>
	void SetLocalGroupSize(const ct::uvec3& vLocalGroupSize);

	/// <summary>
	/// set the dispatch size according to local group size
	/// must be called after call to SetLocalGroupSize
	/// </summary>
	/// <param name="vDispatchSize"></param>
	void SetDispatchSize1D(const uint32_t& vDispatchSize);
	void SetDispatchSize2D(const ct::uvec2& vDispatchSize);
	void SetDispatchSize3D(const ct::uvec3& vDispatchSize);

	/// <summary>
	/// get the dispatch size
	/// </summary>
	const ct::uvec3& GetDispatchSize();

	/// <summary>
	/// will execute vCmdBuffer->disaptch according to the defined dispatchsize
	/// </summary>
	void Dispatch(vk::CommandBuffer* vCmdBuffer);

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
	virtual void SwapMultiPassFrontBackDescriptors(); 

	void DrawPass(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber = 1U);

	/// <summary>
	/// chnage the primitive topology if enebled with m_CanDynamicallyChangePrimitiveTopology
	/// return true if the vPrimitiveTopology was accepted
	/// vForce will recreate the pipeline
	/// </summary>
	/// <returns>return true if the vPrimitiveTopology was accepeted</returns>
	bool ChangeDynamicPrimitiveTopology(const vk::PrimitiveTopology& vPrimitiveTopology, const bool& vForce = false);

	/// <summary>
	/// enable or disable the dynamic change of primitive topology
	/// </summary>
	void SetDynamicallyChangePrimitiveTopology(const bool& vFlag);

	/// <summary>
	/// will define the base primitive topology
	/// </summary>
	/// <param name="vNewSize"></param>
	void SetPrimitveTopology(const vk::PrimitiveTopology& vPrimitiveTopology);

	/// <summary>
	/// get the family of a Primitive Topology
	/// </summary>
	vk::PrimitiveTopology GetPrimitiveTopologyFamily(const vk::PrimitiveTopology& vPrimitiveTopology);

	// draw primitives
	virtual bool CanRender();
	virtual void DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber);
	virtual void Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber);
	virtual void TraceRays(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber);

	virtual bool CanUpdateDescriptors();
	virtual void UpdateRessourceDescriptor();

	// shader update from file
	void UpdateShaders(const std::set<std::string>& vFiles) override;

	void NeedToClearFBOThisFrame();

protected: // IMGUI
	bool DrawResizeWidget();

private:
	void NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers); // to call at any moment
	void NeedResize(ct::ivec2* vNewSize); // to call at any moment
	void Resize(const ct::uvec2& vNewSize);

protected:
	virtual void ActionBeforeCompilation();
	virtual void ActionAfterCompilation();

	void ClearShaderEntryPoints();
	void AddShaderEntryPoints(const vk::ShaderStageFlagBits& vShaderId, const ShaderEntryPoint& vEntryPoint);
	void SetShaderUse(const vk::ShaderStageFlagBits& vShaderId, const ShaderEntryPoint& vEntryPoint, const bool& vUsed);

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

	ShaderCode& AddShaderCode(const ShaderCode& vShaderCode, const ShaderEntryPoint& vEntryPoint);
	ShaderCode CompilShaderCode(
		const vk::ShaderStageFlagBits& vShaderType,
		const std::string& vCode,
		const std::string& vShaderName,
		const std::string& vEntryPoint = "main");
	ShaderCode CompilShaderCode(
		const vk::ShaderStageFlagBits& vShaderType,
		const std::string& vEntryPoint = "main");
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
	virtual bool UpdateRessourceDescriptors();
	virtual bool UpdateLayoutBindingInRessourceDescriptor();
	virtual bool UpdateBufferInfoInRessourceDescriptor();

	void ClearWriteDescriptors();
	void ClearWriteDescriptors(const uint32_t& DescriptorSetIndex);
	bool AddOrSetWriteDescriptorImage(
		const uint32_t& vBinding, 
		const vk::DescriptorType& vType, 
		const vk::DescriptorImageInfo* vImageInfo,
		const uint32_t& vCount = 1U, 
		const uint32_t& DescriptorSetIndex = 0U);
	bool AddOrSetWriteDescriptorBuffer(
		const uint32_t& vBinding, 
		const vk::DescriptorType& vType, 
		const vk::DescriptorBufferInfo* vBufferInfo,
		const uint32_t& vCount = 1U, 
		const uint32_t& DescriptorSetIndex = 0U);
	bool AddOrSetWriteDescriptorBufferView(
		const uint32_t& vBinding, 
		const vk::DescriptorType& vType, 
		const vk::BufferView* vBufferView,
		const uint32_t& vCount = 1U, 
		const uint32_t& DescriptorSetIndex = 0U);
	bool AddOrSetWriteDescriptorNext(
		const uint32_t& vBinding,
		const vk::DescriptorType& vType,
		const void* vNext,
		const uint32_t& vCount = 1U,
		const uint32_t& DescriptorSetIndex = 0U);
	void ClearLayoutDescriptors();
	void ClearLayoutDescriptors(const uint32_t& DescriptorSetIndex);
	bool AddOrSetLayoutDescriptor(
		const uint32_t& vBinding,
		const vk::DescriptorType& vType,
		const vk::ShaderStageFlags& vStage,
		const uint32_t& vCount = 1U,
		const uint32_t& DescriptorSetIndex = 0U);

	// will produce errors eventually only in debug, for dev debugging
	void CheckLayoutBinding(const uint32_t& DescriptorSetIndex = 0U);
	void CheckWriteDescriptors(const uint32_t& DescriptorSetIndex = 0U);

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