/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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

#include <Utils/Mesh/VertexStruct.h>

#include <Base/Base.h>

enum class GenericType : uint8_t
{
	NONE = 0,
	PIXEL,			// vertex + fragment (shader + m_Pipeline + ubo + fbo + renderpass + command buffer)
	COMPUTE,		// compute (shader + m_Pipeline + ubo + command buffer)
	Count
};

class ShaderPass :
	public conf::ConfigAbstract
{
#ifdef VULKAN_DEBUG
public:
	const char* m_RenderDocDebugName = nullptr;
	ct::fvec4 m_RenderDocDebugColor = 0.0f;
#endif

public:
	void SetRenderDocDebugName(const char* vLabel, ct::fvec4 vColor)
	{
#ifdef VULKAN_DEBUG
		m_RenderDocDebugName = vLabel;
		m_RenderDocDebugColor = vColor;
#else
		UNUSED(vLabel);
		UNUSED(vColor);
#endif
	}

protected:
	bool m_Loaded = false;
	bool m_NeedNewUBOUpload = false;
	bool m_NeedNewSBOUpload = false;

	vkApi::VulkanCore* m_VulkanCore = nullptr;	// vulkan core
	vkApi::VulkanQueue m_Queue;					// queue
	vk::CommandPool m_CommandPool;				// command pool
	vk::DescriptorPool m_DescriptorPool;		// descriptor pool
	vk::Device m_Device;						// device copy

	// Multi Sampling
	vk::SampleCountFlagBits m_SampleCount = vk::SampleCountFlagBits::e1; // sampling for primitives

	uint32_t m_CountBuffers = 0U;			// FRAGMENT count framebuffer color attachment from 0 to 7

	// Framebuffer
	FrameBufferPtr m_FrameBufferPtr = nullptr;
	bool m_ResizingIsAllowed = true;

	// dynamic state
	vk::Rect2D m_RenderArea = {};
	vk::Viewport m_Viewport = {};
	ct::fvec2 m_OutputSize;
	float m_OutputRatio = 1.0f;

	std::vector<unsigned int> m_SPIRV_Vert;							// SPIRV Vertex Code
	std::vector<unsigned int> m_SPIRV_Frag;							// SPIRV Fragment Code
	std::vector<unsigned int> m_SPIRV_Comp;							// SPIRV Compute Code
	std::string m_VertexShaderCode;									// Vertex Shader Code
	std::string m_FragmentShaderCode;								// Fragment Shader Code
	std::string m_ComputeShaderCode;								// Compute Shader Code
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
	ct::uvec4 m_CountVertexs = ct::uvec4(0U, 0U, 0U, 6U);			// count vertex to draw
	ct::uvec4 m_CountInstances = ct::uvec4(0U, 0U, 0U, 1U);			// count instances to draw
	ct::uvec4 m_CountIterations = ct::uvec4(0U, 0U, 0U, 1U);		// rendering iterations loop

	std::unordered_map<std::string, bool> m_UsedUniforms;			// Used Uniforms

	GenericType m_RendererType = GenericType::NONE;					// Renderer Type
	
	ct::fvec4 m_LineWidth = ct::fvec4(0.0f, 0.0f, 0.0f, 1.0f);		// line width
	vk::PrimitiveTopology m_PrimitiveTopology = vk::PrimitiveTopology::eTriangleList;

	VertexStruct::PipelineVertexInputState m_InputState;

public:
	ShaderPass(vkApi::VulkanCore* vVulkanCore);
	ShaderPass(vkApi::VulkanCore* vVulkanCore, const GenericType& vRendererTypeEnum);
	ShaderPass(vkApi::VulkanCore* vVulkanCore, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool);
	ShaderPass(vkApi::VulkanCore* vVulkanCore, const GenericType& vRendererTypeEnum, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool);
	virtual ~ShaderPass();

	// during init
	virtual void ActionBeforeInit();
	virtual void ActionAfterInitSucceed();
	virtual void ActionAfterInitFail();

	// init
	virtual bool InitPixel(
		const ct::uvec2& vSize,
		const uint32_t& vCountColorBuffer,
		const bool& vUseDepth,
		const bool& vNeedToClear,
		const ct::fvec4& vClearColor,
		const vk::Format& vFormat = vk::Format::eR32G32B32A32Sfloat,
		const vk::SampleCountFlagBits& vSampleCount = vk::SampleCountFlagBits::e1);
	virtual bool InitCompute(
		const ct::uvec3& vDispatchSize);
	virtual void Unit();

	void AllowResize(const bool& vResizing); // allow or block the resize
	void NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer); // to call at any moment
	void NeedResize(ct::ivec2* vNewSize); // to call at any moment
	void Resize(const ct::uvec2& vNewSize);
	bool ResizeIfNeeded();

	// Renderer Type
	bool IsPixelRenderer();
	bool IsComputeRenderer();

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

	void DrawPass(vk::CommandBuffer* vCmdBuffer);

	// draw primitives
	virtual void DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber);

	virtual void UpdateRessourceDescriptor();

protected:
	virtual void ActionBeforeCompilation();
	virtual void ActionAfterCompilation();

	// Get Shaders
	virtual std::string GetComputeShaderCode(std::string& vOutShaderName);
	virtual std::string GetVertexShaderCode(std::string& vOutShaderName);
	virtual std::string GetFragmentShaderCode(std::string& vOutShaderName);

	virtual bool CompilPixel();
	virtual bool CompilCompute();
	virtual bool ReCompil();

	virtual bool BuildModel();
	virtual void DestroyModel(const bool& vReleaseDatas = false);
	virtual void UpdateModel(const bool& vLoaded);

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

	// Pipelines
	virtual void SetInputStateBeforePipelineCreation(); // for doing this kind of thing VertexStruct::P2_T2::GetInputState(m_InputState);
	virtual bool CreateComputePipeline();
	virtual bool CreatePixelPipeline();
	void DestroyPipeline();
};