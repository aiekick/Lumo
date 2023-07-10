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
#pragma warning(disable : 4251)

#include <set>
#include <string>
#include <memory>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>
#include <Common/Globals.h>

#include <Utils/Mesh/VertexStruct.h>

#include <Interfaces/OutputSizeInterface.h>

#include <vulkan/vulkan.hpp>
#include <gaia/Texture2D.h>
#include <gaia/VulkanCore.h>
#include <gaia/VulkanDevice.h>
#include <gaia/VulkanShader.h>
#include <gaia/vk_mem_alloc.h>
#include <gaia/VulkanRessource.h>
#include <gaia/VulkanFrameBuffer.h>
#include <gaia/gaia.h>

#include <Base/Base.h>

class COMMON_API FrameBuffer : public OutputSizeInterface
{
public:
	static FrameBufferPtr Create(vkApi::VulkanCorePtr vVulkanCorePtr);

private:
	bool m_NeedNewUBOUpload = true;			// true for first render
	bool m_NeedNewSBOUpload = true;			// true for first render

protected:
	uint32_t m_BufferIdToResize = 0U;								// buffer id to resize (mostly used in compute, because in pixel, all attachments must have same size)
	bool m_IsRenderPassExternal = false;							// true if the renderpass is not created here, but come from external (inportant for not destroy him)
	
	bool m_MultiPassMode = false;
	bool m_CreateRenderPass = false;
	bool m_NeedResize = false;				// will be resized if true
	bool m_Loaded = false;					// if shader operationnel
	bool m_JustReseted = false;				// when shader was reseted
	bool m_FirstRender = true;				// 1er rendu

	uint32_t m_CountBuffers = 0U;			// FRAGMENT count framebuffer color attachment from 0 to 7

	ct::ivec2 m_TemporarySize;				// temporary size before resize can be used by imgui
	int32_t m_TemporaryCountBuffer = 0;		// temporary count before resize can be used by imgui

	bool m_UseDepth = false;				// if depth needed for creation
	bool m_NeedToClear = false;				// if color can be cleared for attachment
	ct::fvec4 m_ClearColor = 0.0f;			// color to clear

	uint32_t m_CurrentFrame = 0U;

	DescriptorImageInfoVector m_FrontDescriptors;
	DescriptorImageInfoVector m_BackDescriptors;
	fvec2Vector m_DescriptorSizes;

	// vulkan creation
	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;	// vulkan core
	vkApi::VulkanQueue m_Queue;					// queue
	vk::Device m_Device;						// device copy

	// FrameBuffer
	std::vector<vkApi::VulkanFrameBuffer> m_FrameBuffers;
	vk::Format m_SurfaceColorFormat = vk::Format::eR32G32B32A32Sfloat;

	// Submition
	std::vector<vk::Semaphore> m_RenderCompleteSemaphores;
	std::vector<vk::Fence> m_WaitFences;
	std::vector<vk::CommandBuffer> m_CommandBuffers;

	// dynamic state
	vk::Rect2D m_RenderArea = {};
	vk::Viewport m_Viewport = {};
	ct::uvec3 m_OutputSize;							// output size for compute stage
	float m_OutputRatio = 1.0f;

	// Renderpass
	vk::RenderPass m_RenderPass = {};

	// pixel format
	vk::Format m_PixelFormat = vk::Format::eR32G32B32A32Sfloat;

	// Multi Sampling
	vk::SampleCountFlagBits m_SampleCount = vk::SampleCountFlagBits::e1; // sampling for primitives

	// clear Color
	std::vector<vk::ClearValue> m_ClearColorValues;

public: // contructor
	FrameBuffer(vkApi::VulkanCorePtr vVulkanCorePtr);
	virtual ~FrameBuffer();

	// init/unit
	bool Init(
		const ct::uvec2& vSize, 
		const uint32_t& vCountColorBuffers, 
		const bool& vUseDepth, 
		const bool& vNeedToClear, 
		const ct::fvec4& vClearColor,
		const bool& vMultiPassMode,
		const vk::Format& vFormat,
		const vk::SampleCountFlagBits& vSampleCount,
		const bool& vCreateRenderPass = true,
		const vk::RenderPass& vExternalRenderPass = nullptr);
	void Unit();

	// resize
	void NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers = nullptr); // to call at any moment

	// not to call at any moment, to call only aftter submit or before any command buffer recording
	// return true, if was resized
	bool ResizeIfNeeded();

	// Merger for merged rendering one FBO in the merger
	bool Begin(vk::CommandBuffer* vCmdBuffer);
	void End(vk::CommandBuffer* vCmdBuffer);

	// get sampler / image / buffer
	vkApi::VulkanFrameBuffer* GetBackFbo();
	std::vector<vkApi::VulkanFrameBufferAttachment>* GetBackBufferAttachments(uint32_t* vMaxBuffers);
	vk::DescriptorImageInfo* GetBackDescriptorImageInfo(const uint32_t& vBindingPoint);
	DescriptorImageInfoVector* GetBackDescriptorImageInfos(fvec2Vector* vOutSizes);

	vkApi::VulkanFrameBuffer* GetFrontFbo();
	std::vector<vkApi::VulkanFrameBufferAttachment>* GetFrontBufferAttachments(uint32_t* vMaxBuffers);
	vk::DescriptorImageInfo* GetFrontDescriptorImageInfo(const uint32_t& vBindingPoint);
	DescriptorImageInfoVector* GetFrontDescriptorImageInfos(fvec2Vector* vOutSizes);
	
	// Get
	vk::Viewport GetViewport() const;
	vk::Rect2D GetRenderArea() const;
	vk::RenderPass* GetRenderPass();
	void SetRenderPass(const vk::RenderPass& vExternalRenderPass);
	vk::SampleCountFlagBits GetSampleCount() const;
	uint32_t GetBuffersCount() const;

	float GetOutputRatio() const override;
	ct::fvec2 GetOutputSize() const override;
	
	void BeginRenderPass(vk::CommandBuffer* vCmdBuffer);
	void ClearAttachmentsIfNeeded(vk::CommandBuffer* vCmdBuffer, const bool& vForce = false); // clear if clear is needed internally (set by ClearAttachments)
	void EndRenderPass(vk::CommandBuffer* vCmdBuffer);

	void ClearAttachments(); // set clear flag for clearing at next render
	void SetClearColorValue(const ct::fvec4& vColor);
	
	void Swap();

protected:
	// Framebuffer
	bool CreateFrameBuffers(
		const ct::uvec2& vSize,
		const uint32_t& vCountColorBuffers,
		const bool& vUseDepth,
		const bool& vNeedToClear,
		const ct::fvec4& vClearColor,
		const vk::Format& vFormat,
		const vk::SampleCountFlagBits& vSampleCount,
		const bool& vCreateRenderPass);
	void DestroyFrameBuffers();
};