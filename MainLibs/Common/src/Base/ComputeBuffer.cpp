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

#include "ComputeBuffer.h"

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

ComputeBufferPtr ComputeBuffer::Create(vkApi::VulkanCore* vVulkanCore)
{
	auto res = std::make_shared<ComputeBuffer>(vVulkanCore);

	return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / CTOR/DTOR ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

ComputeBuffer::ComputeBuffer(vkApi::VulkanCore* vVulkanCore)
{
	m_VulkanCore = vVulkanCore;
}

ComputeBuffer::~ComputeBuffer()
{
	Unit();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / INIT/UNIT ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ComputeBuffer::Init(
	const ct::uvec2& vSize,
	const uint32_t& vCountBuffers,
	const bool& vMultiPassMode,
	const vk::Format& vFormat)
{
	ZoneScoped;

	m_Loaded = false;

	m_Device = m_VulkanCore->getDevice();
	ct::uvec2 size = ct::clamp(vSize, 1u, 8192u);
	if (!size.emptyOR())
	{
		m_MultiPassMode = vMultiPassMode;

		m_TemporarySize = ct::ivec2(size.x, size.y);
		m_TemporaryCountBuffer = vCountBuffers;

		m_Queue = m_VulkanCore->getQueue(vk::QueueFlagBits::eGraphics);

		m_OutputSize = ct::uvec3(size.x, size.y, 0);
		m_OutputRatio = ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y).ratioXY<float>();

		m_Format = vFormat;

		if (CreateComputeBuffers(vSize, vCountBuffers, m_Format))
		{ 
			m_Loaded = true;
		}
	}

	return m_Loaded;
}

void ComputeBuffer::Unit()
{
	ZoneScoped;

	m_Device.waitIdle();

	DestroyComputeBuffers();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / RESIZE ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ComputeBuffer::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
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

void ComputeBuffer::NeedResize(ct::ivec2* vNewSize)
{
	if (vNewSize)
	{
		m_TemporarySize = *vNewSize;
		m_NeedResize = true;
	}
}

bool ComputeBuffer::ResizeIfNeeded()
{
	if (m_NeedResize && m_Loaded)
	{
		ZoneScoped;

		DestroyComputeBuffers();
		CreateComputeBuffers(
			ct::uvec2(m_TemporarySize.x, m_TemporarySize.y),
			m_TemporaryCountBuffer, m_Format);

		m_TemporaryCountBuffer = m_CountBuffers;
		m_TemporarySize = ct::ivec2(m_OutputSize.x, m_OutputSize.y);
		m_OutputRatio = ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y).ratioXY<float>();

		m_NeedResize = false;

		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / RENDER ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ComputeBuffer::Begin(vk::CommandBuffer* vCmdBuffer)
{
	if (m_Loaded)
	{
		return true;
	}

	return false;
}

void ComputeBuffer::End(vk::CommandBuffer* vCmdBuffer)
{
	if (m_Loaded)
	{
		Swap();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / RENDER ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ComputeBuffer::Swap()
{
	if (m_MultiPassMode)
	{
		m_CurrentFrame = 1U - m_CurrentFrame;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / GET //////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

float ComputeBuffer::GetOutputRatio() const
{
	ZoneScoped;

	return m_OutputRatio;
}

ct::fvec2 ComputeBuffer::GetOutputSize() const
{
	return ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y);
}

vk::DescriptorImageInfo* ComputeBuffer::GetFrontDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	if (vBindingPoint < m_CountBuffers)
	{
		auto &buffers = m_ComputeBuffers[m_CurrentFrame];
		if (vBindingPoint < buffers.size())
		{
			return &buffers[vBindingPoint]->m_DescriptorImageInfo;
		}
	}
	return nullptr;
}

vk::DescriptorImageInfo* ComputeBuffer::GetBackDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	if (vBindingPoint < m_CountBuffers)
	{
		uint32_t frame = m_CurrentFrame;
		if (m_MultiPassMode)
			frame = 1U - frame;

		auto& buffers = m_ComputeBuffers[frame];
		if (vBindingPoint < buffers.size())
		{
			return &buffers[vBindingPoint]->m_DescriptorImageInfo;
		}
	}
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / FRAMEBUFFER /////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ComputeBuffer::CreateComputeBuffers(
	const ct::uvec2& vSize, 
	const uint32_t& vCountBuffers,
	const vk::Format& vFormat)
{
	ZoneScoped;

	bool res = false;

	auto countColorBuffers = vCountBuffers;
	if (countColorBuffers == 0)
		countColorBuffers = m_CountBuffers;

	if (countColorBuffers > 0 && countColorBuffers <= 8)
	{
		ct::uvec2 size = ct::clamp(vSize, 1u, 8192u);
		if (!size.emptyOR())
		{
			m_CountBuffers = countColorBuffers;
			m_OutputSize = ct::uvec3(size, 0);
			m_OutputRatio = ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y).ratioXY<float>();

			res = true;

			m_ComputeBuffers.clear();
			m_ComputeBuffers.push_back(std::vector<Texture2DPtr>{});
			m_ComputeBuffers[0U].resize(m_CountBuffers);
			for (auto& bufferPtr : m_ComputeBuffers[0U])
			{
				bufferPtr = Texture2D::CreateEmptyImage(m_VulkanCore, size, vFormat);
				res &= (bufferPtr != nullptr);
			}

			if (m_MultiPassMode)
			{
				m_ComputeBuffers.push_back(std::vector<Texture2DPtr>{});
				m_ComputeBuffers[1U].resize(m_CountBuffers);
				for (auto& bufferPtr : m_ComputeBuffers[1U])
				{
					bufferPtr = Texture2D::CreateEmptyImage(m_VulkanCore, size, vFormat);
					res &= (bufferPtr != nullptr);
				}
			}
		}
		else
		{
			LogVarDebug("Debug : Size is empty on one channel at least : x:%u,y:%u", size.x, size.y);
		}
	}
	else
	{
		LogVarDebug("Debug : CountColorBuffer must be between 0 and 8. here => %u", vCountBuffers);
	}

	return res;
}

void ComputeBuffer::DestroyComputeBuffers()
{
	ZoneScoped;

	m_ComputeBuffers.clear();
}
