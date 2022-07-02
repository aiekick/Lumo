// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#define COUNT_INTERNAL_BUFFERS 2

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / STATIC ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

FrameBufferPtr FrameBuffer::Create(vkApi::VulkanCore* vVulkanCore)
{
	auto res = std::make_shared<FrameBuffer>(vVulkanCore);

	return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / CTOR/DTOR ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

FrameBuffer::FrameBuffer(vkApi::VulkanCore* vVulkanCore)
{
	m_VulkanCore = vVulkanCore;
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
	const uint32_t& vCountColorBuffer, 
	const bool& vUseDepth, 
	const bool& vNeedToClear, 
	const ct::fvec4& vClearColor,
	const vk::Format& vFormat,
	const vk::SampleCountFlagBits& vSampleCount)
{
	ZoneScoped;

	m_Loaded = false;

	m_Device = m_VulkanCore->getDevice();
	ct::uvec2 size = ct::clamp(vSize, 1u, 8192u);
	if (!size.emptyOR())
	{
		m_TemporarySize = ct::ivec2(size.x, size.y);
		m_TemporaryCountBuffer = vCountColorBuffer;

		m_Queue = m_VulkanCore->getQueue(vk::QueueFlagBits::eGraphics);

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
			vSize, vCountColorBuffer, 
			m_UseDepth, m_NeedToClear, 
			m_ClearColor, m_PixelFormat, m_SampleCount))
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

void FrameBuffer::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	if (vNewSize)
	{
		m_TemporarySize = *vNewSize;
		m_NeedResize = true;
	}

	if (vCountColorBuffer)
	{
		m_TemporaryCountBuffer = *vCountColorBuffer;
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
			m_UseDepth, m_NeedToClear, m_ClearColor, m_PixelFormat, m_SampleCount);

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
		ClearAttachmentsIfNeeded(vCmdBuffer);
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

void FrameBuffer::ClearAttachmentsIfNeeded(vk::CommandBuffer* vCmdBuffer)
{
	auto fbo = GetFrontFbo();
	if (fbo->neverCleared)
	{
		if (fbo->neverToClear)
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
	m_CurrentFrame = 1U - m_CurrentFrame;
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

vk::SampleCountFlagBits FrameBuffer::GetSampleCount() const
{
	return m_SampleCount;
}

ct::fvec2 FrameBuffer::GetOutputSize() const
{
	return ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y);
}

vkApi::VulkanFrameBuffer* FrameBuffer::GetBackFbo()
{
	ZoneScoped;

	return &m_FrameBuffers[1 - m_CurrentFrame];
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

std::vector<vkApi::VulkanFrameBufferAttachment>* FrameBuffer::GetBackBufferAttachments(uint32_t* vMaxBuffers)
{
	if (vMaxBuffers)
		*vMaxBuffers = m_CountBuffers;
	if (m_FrameBuffers.size() > (1 - m_CurrentFrame))
		return &m_FrameBuffers[1 - m_CurrentFrame].attachments;
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / FRAMEBUFFER /////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FrameBuffer::CreateFrameBuffers(
	const ct::uvec2& vSize, 
	const uint32_t& vCountColorBuffer,
	const bool& vUseDepth, 
	const bool& vNeedToClear, 
	const ct::fvec4& vClearColor,
	const vk::Format& vFormat,
	const vk::SampleCountFlagBits& vSampleCount)
{
	ZoneScoped;

	bool res = false;

	auto countColorBuffers = vCountColorBuffer;
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

			m_ClearColorValues.clear();

			m_FrameBuffers.clear();
			m_FrameBuffers.resize(COUNT_INTERNAL_BUFFERS);

			res = true;

			for (int i = 0; i < COUNT_INTERNAL_BUFFERS; ++i)
			{
				// on cree la renderpass que pour le 1er fbo, apres on reutilisé la meme
				res &= m_FrameBuffers[i].Init(
					m_VulkanCore, size, m_CountBuffers, m_RenderPass, i == 0, 
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
		LogVarDebug("Debug : CountColorBuffer must be between 0 and 8. here => %u", vCountColorBuffer);
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
