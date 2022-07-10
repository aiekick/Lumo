// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include "FrameBuffer.h"

#include <utility>
#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <vkFramework/VulkanSubmitter.h>

#include <FontIcons/CustomFont.h>

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

using namespace vkApi;

//#define VERBOSE_DEBUG
//#define BLEND_ENABLED

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / STATIC ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

FrameBufferPtr FrameBuffer::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<FrameBuffer>(vVulkanCorePtr);

	return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / CTOR/DTOR ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

FrameBuffer::FrameBuffer(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	m_VulkanCorePtr = vVulkanCorePtr;
}

FrameBuffer::~FrameBuffer()
{
	Unit();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / INIT/UNIT ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

// un mode pour merger
// donc pas de code car pas de shader, pas de m_Pipeline, ni ressources
// mais un command buffer, un fbo et une renderpass

bool FrameBuffer::Init(
	const ct::uvec2& vSize,
	const uint32_t& vCountColorBuffers, 
	const bool& vUseDepth, 
	const bool& vNeedToClear, 
	const ct::fvec4& vClearColor,
	const bool& vMultiPassMode,
	const vk::Format& vFormat,
	const vk::SampleCountFlagBits& vSampleCount,
	const bool& vCreateRenderPass,
	const vk::RenderPass& vExternalRenderPass)
{
	ZoneScoped;

	m_Loaded = false;

	m_Device = m_VulkanCorePtr->getDevice();
	ct::uvec2 size = ct::clamp(vSize, 1u, 8192u);
	if (!size.emptyOR())
	{
		m_MultiPassMode = vMultiPassMode;
		
		m_CreateRenderPass = vCreateRenderPass;

		SetRenderPass(vExternalRenderPass); // can only be set if m_CreateRenderPass is false 

		m_TemporarySize = ct::ivec2(size.x, size.y);
		m_TemporaryCountBuffer = vCountColorBuffers;

		m_Queue = m_VulkanCorePtr->getQueue(vk::QueueFlagBits::eGraphics);

		m_RenderArea = vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_OutputSize.x, m_OutputSize.y));
		m_Viewport = vk::Viewport(0.0f, 0.0f, static_cast<float>(m_OutputSize.x), static_cast<float>(m_OutputSize.y), 0, 1.0f);
		m_OutputSize = ct::uvec3(size.x, size.y, 0);
		m_OutputRatio = ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y).ratioXY<float>();

		m_UseDepth = vUseDepth;
		m_NeedToClear = vNeedToClear;
		m_ClearColor = vClearColor;
		m_SampleCount = vSampleCount;
		m_PixelFormat = vFormat;

		if (CreateFrameBuffers(
			vSize, vCountColorBuffers, 
			m_UseDepth, m_NeedToClear, 
			m_ClearColor, m_PixelFormat, 
			m_SampleCount, m_CreateRenderPass))
		{ 
			// renderpass est créé dans createFrameBuffers
			m_Loaded = true;
		}
	}

	return m_Loaded;
}

void FrameBuffer::Unit()
{
	ZoneScoped;

	m_Device.waitIdle();

	DestroyFrameBuffers();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / RESIZE ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameBuffer::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	if (vNewSize)
	{
		m_TemporarySize = *vNewSize;
		m_NeedResize = true;
	}

	if (vCountColorBuffers)
	{
		m_TemporaryCountBuffer = *vCountColorBuffers;
		m_NeedResize = true;
	}
}

void FrameBuffer::NeedResize(ct::ivec2* vNewSize)
{
	if (vNewSize)
	{
		m_TemporarySize = *vNewSize;
		m_NeedResize = true;
	}
}

bool FrameBuffer::ResizeIfNeeded()
{
	if (m_NeedResize && m_Loaded)
	{
		ZoneScoped;

		DestroyFrameBuffers();
		CreateFrameBuffers(
			ct::uvec2(m_TemporarySize.x, m_TemporarySize.y), m_TemporaryCountBuffer, 
			m_UseDepth, m_NeedToClear, m_ClearColor, m_PixelFormat, 
			m_SampleCount, m_CreateRenderPass);

		m_TemporaryCountBuffer = m_CountBuffers;
		m_TemporarySize = ct::ivec2(m_OutputSize.x, m_OutputSize.y);

		m_RenderArea = vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_TemporarySize.x, m_TemporarySize.y));
		m_Viewport = vk::Viewport(0.0f, 0.0f, static_cast<float>(m_TemporarySize.x), static_cast<float>(m_TemporarySize.y), 0, 1.0f);
		m_OutputRatio = ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y).ratioXY<float>();

		m_NeedResize = false;

		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / RENDER ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FrameBuffer::Begin(vk::CommandBuffer* vCmdBuffer)
{
	if (m_Loaded)
	{
		vCmdBuffer->setViewport(0, 1, &m_Viewport);
		vCmdBuffer->setScissor(0, 1, &m_RenderArea);

		BeginRenderPass(vCmdBuffer);
		
		return true;
	}

	return false;
}

void FrameBuffer::End(vk::CommandBuffer* vCmdBuffer)
{
	if (m_Loaded)
	{
		EndRenderPass(vCmdBuffer);

		Swap();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / RENDER ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FrameBuffer::BeginRenderPass(vk::CommandBuffer* vCmdBuffer)
{
	if (vCmdBuffer)
	{
		auto fbo = GetFrontFbo();

		vCmdBuffer->beginRenderPass(
			vk::RenderPassBeginInfo(
				m_RenderPass,
				fbo->framebuffer,
				m_RenderArea,
				static_cast<uint32_t>(m_ClearColorValues.size()),
				m_ClearColorValues.data()
			),
			vk::SubpassContents::eInline);
	}
}

void FrameBuffer::ClearAttachmentsIfNeeded(vk::CommandBuffer* vCmdBuffer, const bool& vForce)
{
	auto fbo = GetFrontFbo();
	if (fbo->neverCleared || vForce)
	{
		if (fbo->neverToClear || vForce)
		{
			if (vCmdBuffer)
			{
				vCmdBuffer->clearAttachments(
					static_cast<uint32_t>(fbo->attachmentClears.size()),
					fbo->attachmentClears.data(),
					static_cast<uint32_t>(fbo->rectClears.size()),
					fbo->rectClears.data()
				);
			}
		}

		fbo->neverCleared = false;
	}
}

void FrameBuffer::EndRenderPass(vk::CommandBuffer* vCmdBuffer)
{
	if (vCmdBuffer)
	{
		vCmdBuffer->endRenderPass();
	}
}

void FrameBuffer::ClearAttachments()
{
	for (auto& fbo : m_FrameBuffers)
	{
		fbo.neverCleared = true;
	}
}

void FrameBuffer::SetClearColorValue(const ct::fvec4& vColor)
{
	if (!m_ClearColorValues.empty())
	{
		m_ClearColorValues[0] = vk::ClearColorValue(std::array<float, 4>{ vColor.x, vColor.y, vColor.z, vColor.w });
	}
}

void FrameBuffer::Swap()
{
	if (m_MultiPassMode)
	{
		m_CurrentFrame = 1U - m_CurrentFrame;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / GET //////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

vk::Viewport FrameBuffer::GetViewport() const
{
	ZoneScoped;

	return m_Viewport;
}

vk::Rect2D FrameBuffer::GetRenderArea() const
{
	ZoneScoped;

	return m_RenderArea;
}

float FrameBuffer::GetOutputRatio() const
{
	ZoneScoped;

	return m_OutputRatio;
}

vk::RenderPass* FrameBuffer::GetRenderPass()
{
	return &m_RenderPass;
}

void FrameBuffer::SetRenderPass(const vk::RenderPass& vExternalRenderPass)
{
	if (vExternalRenderPass)
	{
		if (!m_CreateRenderPass) // we can set a renderpass only if the creation was not demand
		{
			m_RenderPass = vExternalRenderPass;
		}
		else
		{
			CTOOL_DEBUG_BREAK;
		}
	}
}

vk::SampleCountFlagBits FrameBuffer::GetSampleCount() const
{
	return m_SampleCount;
}

ct::fvec2 FrameBuffer::GetOutputSize() const
{
	return ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y);
}

uint32_t FrameBuffer::GetBuffersCount() const
{
	return m_CountBuffers;
}

vkApi::VulkanFrameBuffer* FrameBuffer::GetBackFbo()
{
	ZoneScoped;
	uint32_t frame = m_CurrentFrame;
	if (m_MultiPassMode)
		frame = 1U - frame;
	return &m_FrameBuffers[frame];
}

vkApi::VulkanFrameBuffer* FrameBuffer::GetFrontFbo()
{
	ZoneScoped;

	return &m_FrameBuffers[m_CurrentFrame];
}

std::vector<vkApi::VulkanFrameBufferAttachment>* FrameBuffer::GetFrontBufferAttachments(uint32_t* vMaxBuffers)
{
	if (vMaxBuffers)
		*vMaxBuffers = m_CountBuffers;
	if (m_FrameBuffers.size() > m_CurrentFrame)
		return &m_FrameBuffers[m_CurrentFrame].attachments;
	return 0;
}

vk::DescriptorImageInfo* FrameBuffer::GetFrontDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	uint32_t maxBuffers = 0U;
	auto fbos = GetFrontBufferAttachments(&maxBuffers);
	if (fbos)
	{
		uint32_t m_PreviewBufferId = vBindingPoint;
		m_PreviewBufferId = ct::clamp<uint32_t>(m_PreviewBufferId, 0U, maxBuffers - 1);
		vkApi::VulkanFrameBufferAttachment* att = &fbos->at(m_PreviewBufferId);
		if (att->sampleCount != vk::SampleCountFlagBits::e1)
		{
			if (m_PreviewBufferId + maxBuffers < fbos->size())
			{
				att = &fbos->at(m_PreviewBufferId + maxBuffers);
			}
		}
		if (att->sampleCount == vk::SampleCountFlagBits::e1)
		{
			return &att->attachmentDescriptorInfo;
		}
	}

	return nullptr;
}

DescriptorImageInfoVector* FrameBuffer::GetFrontDescriptorImageInfos(fvec2Vector* vOutSizes)
{
	uint32_t maxBuffers = 0U;
	auto fbos = GetFrontBufferAttachments(&maxBuffers);
	if (fbos)
	{
		if (m_FrontDescriptors.size() == (size_t)maxBuffers)
		{
			size_t offset = 0U;
			if (m_SampleCount != vk::SampleCountFlagBits::e1)
				offset = (size_t)maxBuffers;

			for (size_t i = 0U; i < (size_t)maxBuffers; ++i)
			{
				m_FrontDescriptors[i] = fbos->at(i + offset).attachmentDescriptorInfo;
				m_DescriptorSizes[i] = ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y);
			}

			if (vOutSizes)
			{
				*vOutSizes = m_DescriptorSizes;
			}
		}
	}

	return &m_FrontDescriptors;
}

vk::DescriptorImageInfo* FrameBuffer::GetBackDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	uint32_t maxBuffers = 0U;
	auto fbos = GetBackBufferAttachments(&maxBuffers);
	if (fbos)
	{
		uint32_t m_PreviewBufferId = vBindingPoint;
		m_PreviewBufferId = ct::clamp<uint32_t>(m_PreviewBufferId, 0U, maxBuffers - 1);
		vkApi::VulkanFrameBufferAttachment* att = &fbos->at(m_PreviewBufferId);
		if (att->sampleCount != vk::SampleCountFlagBits::e1)
		{
			if (m_PreviewBufferId + maxBuffers < fbos->size())
			{
				att = &fbos->at(m_PreviewBufferId + maxBuffers);
			}
		}
		if (att->sampleCount == vk::SampleCountFlagBits::e1)
		{
			return &att->attachmentDescriptorInfo;
		}
	}

	return nullptr;
}

std::vector<vkApi::VulkanFrameBufferAttachment>* FrameBuffer::GetBackBufferAttachments(uint32_t* vMaxBuffers)
{
	if (vMaxBuffers)
		*vMaxBuffers = m_CountBuffers;
	uint32_t frame = m_CurrentFrame;
	if (m_MultiPassMode)
		frame = 1U - frame;
	if (m_FrameBuffers.size() > frame)
		return &m_FrameBuffers[frame].attachments;
	return 0;
}

DescriptorImageInfoVector* FrameBuffer::GetBackDescriptorImageInfos(fvec2Vector* vOutSizes)
{
	uint32_t maxBuffers = 0U;
	auto fbos = GetBackBufferAttachments(&maxBuffers);
	if (fbos)
	{
		if (m_BackDescriptors.size() == (size_t)maxBuffers)
		{
			size_t offset = 0U;
			if (m_SampleCount != vk::SampleCountFlagBits::e1)
				offset = (size_t)maxBuffers;

			for (size_t i = 0U; i < (size_t)maxBuffers; ++i)
			{
				m_BackDescriptors[i] = fbos->at(i + offset).attachmentDescriptorInfo;
				m_DescriptorSizes[i] = ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y);
			}

			if (vOutSizes)
			{
				*vOutSizes = m_DescriptorSizes;
			}
		}
	}

	return &m_BackDescriptors;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / FRAMEBUFFER /////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FrameBuffer::CreateFrameBuffers(
	const ct::uvec2& vSize, 
	const uint32_t& vCountColorBuffers,
	const bool& vUseDepth, 
	const bool& vNeedToClear, 
	const ct::fvec4& vClearColor,
	const vk::Format& vFormat,
	const vk::SampleCountFlagBits& vSampleCount,
	const bool& vCreateRenderPass)
{
	ZoneScoped;

	bool res = false;

	auto countColorBuffers = vCountColorBuffers;
	if (countColorBuffers == 0)
		countColorBuffers = m_CountBuffers;

	if (countColorBuffers > 0 && countColorBuffers <= 8)
	{
		ct::uvec2 size = ct::clamp(vSize, 1u, 8192u);
		if (!size.emptyOR())
		{
			m_CountBuffers = countColorBuffers;
			m_OutputSize = ct::uvec3(size, 0);
			m_RenderArea = vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_OutputSize.x, m_OutputSize.y));
			m_Viewport = vk::Viewport(0.0f, 0.0f, static_cast<float>(m_OutputSize.x), static_cast<float>(m_OutputSize.y), 0, 1.0f);
			m_OutputRatio = ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y).ratioXY<float>();

			m_FrontDescriptors.clear();
			m_FrontDescriptors.resize(m_CountBuffers);

			m_BackDescriptors.clear();
			m_BackDescriptors.resize(m_CountBuffers);

			m_DescriptorSizes.clear();
			m_DescriptorSizes.resize(m_CountBuffers);

			m_ClearColorValues.clear();

			m_FrameBuffers.clear();

			res = true;
			
			m_FrameBuffers.push_back(vkApi::VulkanFrameBuffer{});
			res &= m_FrameBuffers[0U].Init(
				m_VulkanCorePtr, size, m_CountBuffers, m_RenderPass, vCreateRenderPass,
				vUseDepth, vNeedToClear, vClearColor, vFormat, vSampleCount);
			
			if (m_MultiPassMode)
			{
				m_FrameBuffers.push_back(vkApi::VulkanFrameBuffer{});
				res &= m_FrameBuffers[1U].Init(
					m_VulkanCorePtr, size, m_CountBuffers, m_RenderPass, false, // this one will re use the same Renderpass as first one
					vUseDepth, vNeedToClear, vClearColor, vFormat, vSampleCount);
			}

			if (vNeedToClear)
			{
				m_ClearColorValues = m_FrameBuffers[0].clearColorValues;
			}
		}
		else
		{
			LogVarDebug("Debug : Size is empty on one channel at least : x:%u,y:%u", size.x, size.y);
		}
	}
	else
	{
		LogVarDebug("Debug : CountColorBuffer must be between 0 and 8. here => %u", vCountColorBuffers);
	}

	return res;
}

void FrameBuffer::DestroyFrameBuffers()
{
	ZoneScoped;

	m_FrameBuffers.clear();
	if (m_RenderPass)
	{
		m_Device.destroyRenderPass(m_RenderPass);
		m_RenderPass = vk::RenderPass {};
	}
}
