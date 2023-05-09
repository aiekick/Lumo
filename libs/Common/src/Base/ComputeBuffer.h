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
#include <memory>

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <Utils/Mesh/VertexStruct.h>

#include <Interfaces/OutputSizeInterface.h>

#include <vulkan/vulkan.hpp>
#include <vkFramework/Texture2D.h>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanDevice.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/vk_mem_alloc.h>
#include <vkFramework/VulkanRessource.h>

#include <Base/Base.h>

class ComputeBuffer : public OutputSizeInterface
{
public:
	static ComputeBufferPtr Create(vkApi::VulkanCorePtr vVulkanCorePtr);

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
	vkApi::VulkanCorePtr m_VulkanCorePtr = nullptr;	// vulkan core
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
	ComputeBuffer(vkApi::VulkanCorePtr vVulkanCorePtr);
	virtual ~ComputeBuffer();

	// init/unit
	bool Init(
		const ct::uvec2& vSize, 
		const uint32_t& vCountColorBuffers,
		const bool& vMultiPassMode,
		const vk::Format& vFormat);
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
	vk::DescriptorImageInfo* GetBackDescriptorImageInfo(const uint32_t& vBindingPoint);
	vk::DescriptorImageInfo* GetFrontDescriptorImageInfo(const uint32_t& vBindingPoint);
	
	uint32_t GetBuffersCount() const;
	bool IsMultiPassMode() const;

	float GetOutputRatio() const override;
	ct::fvec2 GetOutputSize() const override;

	void Swap();

protected:
	// Framebuffer
	bool CreateComputeBuffers(
		const ct::uvec2& vSize,
		const uint32_t& vCountColorBuffers,
		const vk::Format& vFormat);
	void DestroyComputeBuffers();
};