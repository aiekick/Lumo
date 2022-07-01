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

#include "BaseRenderer.h"

#include <utility>
#include <functional>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_funcs.hpp>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <vkFramework/VulkanSubmitter.h>
#include <Base/FrameBuffer.h>

#include <FontIcons/CustomFont.h>
#include <Base/Base.h>

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

using namespace vkApi;

//#define VERBOSE_DEBUG
//#define BLEND_ENABLED

#define COUNT_BUFFERS 2

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / CONSTRUCTOR //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

BaseRenderer::BaseRenderer(vkApi::VulkanCore* vVulkanCore)
{
	ZoneScoped;

	m_VulkanCore = vVulkanCore;
	m_Device = m_VulkanCore->getDevice();
}

BaseRenderer::BaseRenderer(vkApi::VulkanCore* vVulkanCore, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool)
{
	ZoneScoped;

	m_VulkanCore = vVulkanCore;
	m_Device = m_VulkanCore->getDevice();
	m_CommandPool = *vCommandPool;
	m_DescriptorPool = *vDescriptorPool;
}

BaseRenderer::~BaseRenderer()
{
	ZoneScoped;

	Unit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / PASS /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BaseRenderer::AddGenericPass(ShaderPassPtr vPass)
{
	if (vPass)
	{
		m_ShaderPass.push_back(vPass);

		return true;
	}
	return false;
}

ShaderPassWeak BaseRenderer::GetGenericPass(const uint32_t& vIdx)
{
	if (m_ShaderPass.size() > vIdx)
	{
		return m_ShaderPass[vIdx];
	}

	return ShaderPassWeak();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / DURING INIT //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BaseRenderer::ActionBeforeInit()
{

}

void BaseRenderer::ActionAfterInitSucceed()
{

}

void BaseRenderer::ActionAfterInitFail()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / INIT/UNIT ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BaseRenderer::InitPixel(const ct::uvec2& vSize)
{
	ZoneScoped;

	ActionBeforeInit();

	m_Loaded = false;

	m_Device = m_VulkanCore->getDevice();
	ct::uvec2 size = ct::clamp(vSize, 1u, 8192u);
	if (!size.emptyOR())
	{
		m_Queue = m_VulkanCore->getQueue(vk::QueueFlagBits::eGraphics);
		m_DescriptorPool = m_VulkanCore->getDescriptorPool();
		m_CommandPool = m_Queue.cmdPools;

		m_OutputSize = ct::uvec3(size.x, size.y, 0);
		m_RenderArea = vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_OutputSize.x, m_OutputSize.y));
		m_Viewport = vk::Viewport(0.0f, 0.0f, static_cast<float>(m_OutputSize.x), static_cast<float>(m_OutputSize.y), 0, 1.0f);
		m_OutputRatio = ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y).ratioXY<float>();

		if (CreateCommanBuffer()) {
			if (CreateSyncObjects()) {
				m_Loaded = true;
			}
		}
	}

	if (m_Loaded)
	{
		m_TracyContext = TracyVkContext(
			m_VulkanCore->getPhysicalDevice(),
			m_Device,
			m_Queue.vkQueue,
			m_CommandBuffers[0]);

		ActionAfterInitSucceed();
	}
	else
	{
		ActionAfterInitFail();
	}

	return m_Loaded;
}

bool BaseRenderer::InitCompute(const ct::uvec2& vSize)
{
	ZoneScoped;

	ActionBeforeInit();

	m_Loaded = false;

	m_Device = m_VulkanCore->getDevice();
	ct::uvec2 size = ct::clamp(vSize, 1u, 8192u);
	if (!size.emptyOR())
	{
		m_UniformSectionToShow = { "COMPUTE" }; // pour afficher les uniforms

		m_Queue = m_VulkanCore->getQueue(vk::QueueFlagBits::eGraphics);
		m_DescriptorPool = m_VulkanCore->getDescriptorPool();
		m_CommandPool = m_Queue.cmdPools;

		m_OutputSize = ct::uvec3(size.x, size.y, 1);
		m_RenderArea = vk::Rect2D(vk::Offset2D(), vk::Extent2D(m_OutputSize.x, m_OutputSize.y));
		m_Viewport = vk::Viewport(0.0f, 0.0f, static_cast<float>(m_OutputSize.x), static_cast<float>(m_OutputSize.y), 0, 1.0f);
		m_OutputRatio = ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y).ratioXY<float>();

		if (CreateCommanBuffer()) {
			if (CreateSyncObjects()) {
				m_Loaded = true;
			}
		}
	}

	if (m_Loaded)
	{
		m_TracyContext = TracyVkContext(
			m_VulkanCore->getPhysicalDevice(),
			m_Device,
			m_Queue.vkQueue,
			m_CommandBuffers[0]);

		ActionAfterInitSucceed();
	}
	else
	{
		ActionAfterInitFail();
	}

	return m_Loaded;
}

void BaseRenderer::Unit()
{
	ZoneScoped;

	m_Device.waitIdle();

	m_ShaderPass.clear();
	DestroySyncObjects();
	DestroyCommanBuffer();

	if (m_Loaded)
	{
		if (m_TracyContext)
		{
			TracyVkDestroy(m_TracyContext);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / RESIZE ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BaseRenderer::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	for (auto passPtr : m_ShaderPass)
	{
		if (passPtr)
		{
			passPtr->NeedResize(vNewSize, vCountColorBuffer);
		}
	}
}

void BaseRenderer::NeedResize(ct::ivec2* vNewSize)
{
	for (auto passPtr : m_ShaderPass)
	{
		if (passPtr)
		{
			passPtr->NeedResize(vNewSize);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / RENDER ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BaseRenderer::Render(const char* vSectionLabel, vk::CommandBuffer* vCmdBuffer)
{
	ZoneScoped;

	if (m_CanWeRender || m_JustReseted)
	{
		auto cmd = GetCommandBuffer();
		if (cmd)
		{
			if (BeginRender(vSectionLabel))
			{
				for (auto passPtr : m_ShaderPass)
				{
					if (passPtr)
					{
						passPtr->DrawPass(cmd);
					}
				}

				m_VulkanCore->getFrameworkDevice()->EndDebugLabel(cmd);
				
				EndRender();
			}
		}
	}
}

void BaseRenderer::UpdateDescriptorsBeforeCommandBuffer()
{
	for (auto passPtr : m_ShaderPass)
	{
		if (passPtr)
		{
			passPtr->UpdateRessourceDescriptor();
		}
	}
}

bool BaseRenderer::ResizeIfNeeded()
{
	bool resized = false;
	
	if (!m_ShaderPass.empty())
	{
		for (auto passPtr : m_ShaderPass)
		{
			if (passPtr)
			{
				// only the last pass can be an output and must
				// be take into account for get
				// the renderarea and viewport
				resized = passPtr->ResizeIfNeeded();
			}
		}

		if (resized)
		{
			// only the last pass can be an output and must
			// be take into account for get
			// the renderarea and viewport
			auto passPtr = m_ShaderPass.at(m_ShaderPass.size() - 1U);
			if (passPtr)
			{
				auto fboPtr = passPtr->GetFrameBuffer().getValidShared();
				if (fboPtr)
				{
					m_RenderArea = fboPtr->GetRenderArea();
					m_Viewport = fboPtr->GetViewport();
					m_OutputRatio = fboPtr->GetOutputRatio();
				}
			}
		}
	}

	return resized;
}

bool BaseRenderer::BeginRender(const char* vSectionLabel)
{
	ZoneScoped;

	if (!m_Loaded) return false;

	ResizeIfNeeded();

	if (ResetFence())
	{
		auto cmd = GetCommandBuffer();
		if (cmd)
		{
			BeginTracyFrame("BaseRenderer");

			UpdateDescriptorsBeforeCommandBuffer();

			ResetCommandBuffer();
			BeginPixelCommandBuffer(vSectionLabel);
			
			return true;
		}
	}

	return false;
}

void BaseRenderer::EndRender()
{
	ZoneScoped;

	EndCommandBuffer();

	SubmitPixel();
	if (WaitFence())
	{
		Swap();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / RENDER ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BaseRenderer::ResetFence()
{
	ZoneScoped;

	if (!m_Loaded) return false;

	return m_Device.resetFences(1, &m_WaitFences[m_CurrentFrame]) == vk::Result::eSuccess;
}

bool BaseRenderer::WaitFence()
{
	ZoneScoped;

	if (!m_Loaded) return false;

	return m_Device.waitForFences(1, &m_WaitFences[m_CurrentFrame], VK_TRUE, UINT64_MAX) == vk::Result::eSuccess;
}

vk::CommandBuffer* BaseRenderer::GetCommandBuffer()
{
	return &m_CommandBuffers[m_CurrentFrame];
}

void BaseRenderer::BeginTracyFrame(const char* vFrameName)
{
	FrameMarkNamed(vFrameName);
}

void BaseRenderer::ResetCommandBuffer()
{
	auto cmd = GetCommandBuffer();
	cmd->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
}

void BaseRenderer::BeginPixelCommandBuffer(const char* vSectionLabel)
{
	auto cmd = GetCommandBuffer();
	cmd->begin(vk::CommandBufferBeginInfo());

	if (vSectionLabel)
	{
		m_VulkanCore->getFrameworkDevice()->BeginDebugLabel(cmd, vSectionLabel, GENERIC_RENDERER_DEBUG_COLOR);
		m_DebugLabelWasUsed = true;
	}

	{
		TracyVkZone(m_TracyContext, *cmd, "Record Pixel Command buffer");
	}
}

void BaseRenderer::BeginComputeCommandBuffer(const char* vSectionLabel)
{
	auto cmd = GetCommandBuffer();
	cmd->begin(vk::CommandBufferBeginInfo());

	if (vSectionLabel)
	{
		m_VulkanCore->getFrameworkDevice()->BeginDebugLabel(cmd, vSectionLabel, GENERIC_RENDERER_DEBUG_COLOR);
		m_DebugLabelWasUsed = true;
	}

	{
		TracyVkZone(m_TracyContext, *cmd, "Record Compute Command buffer");
	}
}

void BaseRenderer::EndCommandBuffer()
{
	auto cmd = GetCommandBuffer();
	if (cmd)
	{
		if (m_DebugLabelWasUsed)
		{
			m_VulkanCore->getFrameworkDevice()->EndDebugLabel(cmd);
			m_DebugLabelWasUsed = false;
		}

		DoBeforeEndCommandBuffer(cmd);

		{
			TracyVkCollect(m_TracyContext, *cmd);
		}
		cmd->end();
	}
}

void BaseRenderer::SubmitPixel()
{
	ZoneScoped;

	if (!m_Loaded) return;

	vk::SubmitInfo submitInfo;
	vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	submitInfo
		.setPWaitDstStageMask(&waitDstStageMask)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&m_CommandBuffers[m_CurrentFrame])
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(&m_RenderCompleteSemaphores[0]);

	if (!m_FirstRender)
	{
		submitInfo
			.setWaitSemaphoreCount(1)
			.setPWaitSemaphores(&m_RenderCompleteSemaphores[0]);
	}
	else
	{
		m_FirstRender = false;
	}

	m_FirstTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::system_clock::now().time_since_epoch()).count();

	VulkanSubmitter::Submit(m_VulkanCore, vk::QueueFlagBits::eGraphics, submitInfo, m_WaitFences[m_CurrentFrame]);
}

void BaseRenderer::SubmitCompute()
{
	ZoneScoped;

	if (!m_Loaded) return;

	vk::SubmitInfo submitInfo;
	vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eComputeShader;
	submitInfo
		.setPWaitDstStageMask(&waitDstStageMask)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&m_CommandBuffers[m_CurrentFrame])
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(&m_RenderCompleteSemaphores[0]);

	if (!m_FirstRender)
	{
		submitInfo
			.setWaitSemaphoreCount(1)
			.setPWaitSemaphores(&m_RenderCompleteSemaphores[0]);
	}
	else
	{
		m_FirstRender = false;
	}

	m_FirstTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::system_clock::now().time_since_epoch()).count();

	VulkanSubmitter::Submit(m_VulkanCore, vk::QueueFlagBits::eCompute, submitInfo, m_WaitFences[m_CurrentFrame]);
}

void BaseRenderer::Swap()
{
	ZoneScoped;

	if (!m_Loaded) return;

	m_CurrentFrame = 1 - m_CurrentFrame;

	m_SecondTimeMark = std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::system_clock::now().time_since_epoch()).count();

	static float deltaPrec = 1.0f / 1000.0f; // on cree ca pour eviter de diviser, multiplier est plus low cost
	m_DeltaTime = (m_SecondTimeMark - m_FirstTimeMark) * deltaPrec;

	++m_Frame;
	m_JustReseted = false;
}

void BaseRenderer::ResetFrame()
{
	//ClearAttachments();
	m_Frame = 0;
	m_JustReseted = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / GET //////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

vk::Viewport BaseRenderer::GetViewport() const
{
	ZoneScoped;

	return m_Viewport;
}

vk::Rect2D BaseRenderer::GetRenderArea() const
{
	ZoneScoped;

	return m_RenderArea;
}

ct::fvec2 BaseRenderer::GetOutputSize() const
{
	return ct::fvec2((float)m_OutputSize.x, (float)m_OutputSize.y);
}

float BaseRenderer::GetOutputRatio() const
{
	return m_OutputRatio;
}

void BaseRenderer::UpdateShaders(const std::set<std::string>& vFiles)
{
	for (auto passPtr : m_ShaderPass)
	{
		if (passPtr)
		{
			passPtr->UpdateShaders(vFiles);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string BaseRenderer::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	//str += m_UniformWidgets.getXml(vOffset, vUserDatas);

	return str;
}

// return true for continue xml parsing of childs in this node or false for interupt the child exploration
bool BaseRenderer::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	//m_UniformWidgets.setFromXml(vElem, vParent, vUserDatas);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / COMMANDBUFFER ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BaseRenderer::DoBeforeEndCommandBuffer(vk::CommandBuffer* vCmdBuffer)
{

}

bool BaseRenderer::CreateCommanBuffer()
{
	ZoneScoped;

	m_CommandBuffers = m_Device.allocateCommandBuffers(
		vk::CommandBufferAllocateInfo(
			m_CommandPool,
			vk::CommandBufferLevel::ePrimary,
			COUNT_BUFFERS
		)
	);

	return true;
}

void BaseRenderer::DestroyCommanBuffer()
{
	ZoneScoped;

	if (!m_CommandBuffers.empty())
	{
		m_Device.freeCommandBuffers(m_CommandPool, m_CommandBuffers);
		m_CommandBuffers.clear();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / SYNC OBJECTS ////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BaseRenderer::CreateSyncObjects()
{
	ZoneScoped;

	m_RenderCompleteSemaphores.resize(COUNT_BUFFERS);
	m_WaitFences.resize(COUNT_BUFFERS);
	for (size_t i = 0; i < COUNT_BUFFERS; ++i)
	{
		m_RenderCompleteSemaphores[i] = m_Device.createSemaphore(vk::SemaphoreCreateInfo());
		m_WaitFences[i] = m_Device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
	}

	return true;
}

void BaseRenderer::DestroySyncObjects()
{
	ZoneScoped;

	for (size_t i = 0; i < COUNT_BUFFERS; ++i)
	{
		if (i < m_RenderCompleteSemaphores.size())
			m_Device.destroySemaphore(m_RenderCompleteSemaphores[i]);
		if (i < m_WaitFences.size())
			m_Device.destroyFence(m_WaitFences[i]);
	}

	m_RenderCompleteSemaphores.clear();
	m_WaitFences.clear();
}
