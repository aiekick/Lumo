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

#include <Base/Base.h>

class ComputeBuffer
{
public:
	static ComputeBufferPtr Create(vkApi::VulkanCore* vVulkanCore);

protected:
	uint32_t m_BufferIdToResize = 0U;								// buffer id to resize (mostly used in compute, because in pixel, all attachments must have same size)
	bool m_IsRenderPassExternal = false;							// true if the renderpass is not created here, but come from external (inportant for not destroy him)
	
	bool m_MultiPassMode = false;

	bool m_NeedResize = false;				// will be resized if true
	bool m_Loaded = false;					// if shader operationnel
	bool m_JustReseted = false;				// when shader was reseted
	bool m_FirstRender = true;				// 1er rendu

	uint32_t m_CountBuffers = 0U;			// count buffers

	ct::ivec2 m_TemporarySize;				// temporary size before resize can be used by imgui
	int32_t m_TemporaryCountBuffer = 0;		// temporary count before resize can be used by imgui

	uint32_t m_CurrentFrame = 0U;

	// vulkan creation
	vkApi::VulkanCore* m_VulkanCore = nullptr;	// vulkan core
	vkApi::VulkanQueue m_Queue;					// queue
	vk::Device m_Device;						// device copy

	// ComputeBuffer
	std::vector<std::vector<Texture2DPtr>> m_ComputeBuffers;
	vk::Format m_Format = vk::Format::eR32G32B32A32Sfloat;

	// Submition
	std::vector<vk::Semaphore> m_RenderCompleteSemaphores;
	std::vector<vk::Fence> m_WaitFences;
	std::vector<vk::CommandBuffer> m_CommandBuffers;

	// dynamic state
	//vk::Rect2D m_RenderArea = {};
	//vk::Viewport m_Viewport = {};
	ct::uvec3 m_OutputSize;							// output size for compute stage
	float m_OutputRatio = 1.0f;

public: // contructor
	ComputeBuffer(vkApi::VulkanCore* vVulkanCore);
	virtual ~ComputeBuffer();

	// init/unit
	bool Init(
		const ct::uvec2& vSize, 
		const uint32_t& vCountBuffers,
		const bool& vMultiPassMode,
		const vk::Format& vFormat);
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
	vk::DescriptorImageInfo* ComputeBuffer::GetBackDescriptorImageInfo(const uint32_t& vBindingPoint);
	vk::DescriptorImageInfo* ComputeBuffer::GetFrontDescriptorImageInfo(const uint32_t& vBindingPoint);
	
	float GetOutputRatio() const;
	ct::fvec2 GetOutputSize() const;
	
	void Swap();

protected:
	// Framebuffer
	bool CreateComputeBuffers(
		const ct::uvec2& vSize,
		const uint32_t& vCountBuffers,
		const vk::Format& vFormat);
	void DestroyComputeBuffers();
};