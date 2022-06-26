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
#include <memory>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <Utils/Mesh/VertexStruct.h>

#include <vulkan/vulkan.hpp>
#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanDevice.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/vk_mem_alloc.h>
#include <vkFramework/VulkanRessource.h>
#include <vkFramework/VulkanFrameBuffer.h>

#include <Generic/Generic.h>

class FrameBuffer
{
public:
	static FrameBufferPtr Create(vkApi::VulkanCore* vVulkanCore);

private:
	bool m_NeedNewUBOUpload = true;			// true for first render
	bool m_NeedNewSBOUpload = true;			// true for first render

protected:
	uint32_t m_BufferIdToResize = 0U;								// buffer id to resize (mostly used in compute, because in pixel, all attachments must have same size)
	bool m_IsRenderPassExternal = false;							// true if the renderpass is not created here, but come from external (inportant for not destroy him)
	
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

	// vulkan creation
	vkApi::VulkanCore* m_VulkanCore = nullptr;	// vulkan core
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
	FrameBuffer(vkApi::VulkanCore* vVulkanCore);
	virtual ~FrameBuffer();

	// init/unit
	bool Init(
		const ct::uvec2& vSize, 
		const uint32_t& vCountColorBuffer, 
		const bool& vUseDepth, 
		const bool& vNeedToClear, 
		const ct::fvec4& vClearColor,
		const vk::Format& vFormat,
		const vk::SampleCountFlagBits& vSampleCount);
	void Unit();

	// resize
	void NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer); // to call at any moment
	void NeedResize(ct::ivec2* vNewSize); // to call at any moment

	// not to call at any moment, to call only aftter submit or before any command buffer recording
	// return true, if was resized
	bool ResizeIfNeeded();

	// Merger for merged rendering one FBO in the merger
	bool Begin(vk::CommandBuffer* vCmdBuffer);
	void End(vk::CommandBuffer* vCmdBuffer);

	// get sampler / image / buffer
	vkApi::VulkanFrameBuffer* GetBackFbo();
	std::vector<vkApi::VulkanFrameBufferAttachment>* GetBackBufferAttachments(uint32_t* vMaxBuffers);
	vk::DescriptorImageInfo* FrameBuffer::GetBackDescriptorImageInfo(const uint32_t& vBindingPoint);
	
	vkApi::VulkanFrameBuffer* GetFrontFbo();
	std::vector<vkApi::VulkanFrameBufferAttachment>* GetFrontBufferAttachments(uint32_t* vMaxBuffers);
	vk::DescriptorImageInfo* FrameBuffer::GetFrontDescriptorImageInfo(const uint32_t& vBindingPoint);
	
	// Get
	vk::Viewport GetViewport() const;
	vk::Rect2D GetRenderArea() const;
	float GetOutputRatio() const;
	vk::RenderPass* GetRenderPass();
	vk::SampleCountFlagBits GetSampleCount() const;
	ct::fvec2 GetOutputSize() const;
	
	void BeginRenderPass(vk::CommandBuffer* vCmdBuffer);
	void ClearAttachmentsIfNeeded(vk::CommandBuffer* vCmdBuffer); // clear if clear is needed internally (set by ClearAttachments)
	void EndRenderPass(vk::CommandBuffer* vCmdBuffer);

	void ClearAttachments(); // set clear flag for clearing at next render
	void SetClearColorValue(const ct::fvec4& vColor);
	
protected:
	// Framebuffer
	bool CreateFrameBuffers(
		const ct::uvec2& vSize,
		const uint32_t& vCountColorBuffer,
		const bool& vUseDepth,
		const bool& vNeedToClear,
		const ct::fvec4& vClearColor,
		const vk::Format& vFormat,
		const vk::SampleCountFlagBits& vSampleCount);
	void DestroyFrameBuffers();
};