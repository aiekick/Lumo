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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "VulkanFrameBuffer.h"
#include "VulkanCore.h"
#include <ctools/cTools.h>
#include <ctools/Logger.h>

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

namespace vkApi
{
	VulkanFrameBuffer::VulkanFrameBuffer()
	{
		ZoneScoped;
	}

	VulkanFrameBuffer::~VulkanFrameBuffer()
	{
		ZoneScoped;

		Unit();
	}

	bool VulkanFrameBuffer::Init(
		vkApi::VulkanCore* vVulkanCore,
		ct::uvec2 vSize,
		uint32_t vCountColorBuffer,
		vk::RenderPass& vRenderPass,
		bool vCreateRenderPass,
		bool vUseDepth,
		bool vNeedToClear,
		ct::fvec4 vClearColor,
		vk::Format vFormat,
		vk::SampleCountFlagBits vSampleCount)
	{
		ZoneScoped;

		bool res = false;

		m_VulkanCore = vVulkanCore;
		neverToClear = vNeedToClear;

		if (vCountColorBuffer > 0 && vCountColorBuffer <= 8)
		{
			ct::uvec2 size = ct::clamp(vSize, 1u, 8192u);
			if (!size.emptyOR())
			{
				attachments.clear();
				attachmentViews.clear();
				attachmentClears.clear();
				clearColorValues.clear();
				rectClears.clear();

				auto logDevice = m_VulkanCore->getDevice();

				vk::RenderPass renderPass;

				// necessaire pour une renderpass compatible
				std::vector<vk::AttachmentDescription> attachmentDescriptions;
				std::vector<vk::AttachmentReference> colorReferences;
				std::vector<vk::AttachmentReference> resolveColorReferences;
				std::vector<vk::AttachmentReference> depthReferences;

				sampleCount = vSampleCount;
				format = vFormat;
				mipLevelCount = 1u;
				width = size.x;
				height = size.y;
				ratio = (float)height / (float)width;
				bool useMultiSampling = (sampleCount != vk::SampleCountFlagBits::e1);

				// https://vulkan-tutorial.com/Multisampling
				// on doit ajouter une resolve target
				// pour le sampleCount
				attachments.resize(vCountColorBuffer
					+ (useMultiSampling ? vCountColorBuffer : 0U)
					+ (vUseDepth ? 1U : 0U)
				);

				uint32_t attIndex = 0;

				for (uint32_t j = 0; j < vCountColorBuffer; j++)
				{
					if (attachments[attIndex].InitColor2D(m_VulkanCore, size, format, 1U, vNeedToClear, sampleCount))
					{
						attachmentViews.push_back(attachments[attIndex].attachmentView);
						attachmentDescriptions.push_back(attachments[attIndex].attachmentDescription);
						colorReferences.push_back(vk::AttachmentReference(attIndex, vk::ImageLayout::eColorAttachmentOptimal));
						if (vNeedToClear)
						{
							clearColorValues.push_back(vk::ClearColorValue(std::array<float, 4>{ vClearColor.x, vClearColor.y, vClearColor.z, vClearColor.w }));
							attachmentClears.push_back(vk::ClearAttachment(vk::ImageAspectFlagBits::eColor, attIndex, clearColorValues[attIndex]));
							rectClears.push_back(vk::ClearRect(vk::Rect2D(vk::Offset2D(), vk::Extent2D(width, height)), 0, 1));
						}
					}
					else
					{
						LogVarDebug("Debug : Color Attachement %u cant be loaded...", attIndex);
						return false;
					}
					++attIndex;
				}

				// https://vulkan-tutorial.com/Multisampling
				// on doit ajouter une resolve target
				if (useMultiSampling)
				{
					for (uint32_t j = 0; j < vCountColorBuffer; j++)
					{
						if (attachments[attIndex].InitColor2D(m_VulkanCore, size, format, 1U, vNeedToClear, vk::SampleCountFlagBits::e1))
						{
							attachmentViews.push_back(attachments[attIndex].attachmentView);
							attachmentDescriptions.push_back(attachments[attIndex].attachmentDescription);
							resolveColorReferences.push_back(vk::AttachmentReference(attIndex, vk::ImageLayout::eColorAttachmentOptimal));
							if (vNeedToClear)
							{
								clearColorValues.push_back(vk::ClearColorValue(std::array<float, 4>{ vClearColor.x, vClearColor.y, vClearColor.z, vClearColor.w }));
								//attachmentClears.push_back(vk::ClearAttachment(vk::ImageAspectFlagBits::eColor, attIndex, clearColorValues[attIndex]));
								//rectClears.push_back(vk::ClearRect(vk::Rect2D(vk::Offset2D(), vk::Extent2D(width, height)), 0, 1));
							}
						}
						else
						{
							LogVarDebug("Debug : Resolve Attachement %u cant be loaded...", attIndex);
							return false;
						}
						++attIndex;
					}
				}

				if (vUseDepth)
				{
					depthAttIndex = attIndex;
					if (attachments[attIndex].InitDepth(m_VulkanCore, size, vk::Format::eD32SfloatS8Uint, sampleCount))
					{
						attachmentViews.push_back(attachments[attIndex].attachmentView);
						attachmentDescriptions.push_back(attachments[attIndex].attachmentDescription);
						depthReferences.push_back(vk::AttachmentReference(attIndex, vk::ImageLayout::eDepthStencilAttachmentOptimal));
						if (vNeedToClear)
						{
							clearColorValues.push_back(vk::ClearDepthStencilValue(1.0f, 0u));
							attachmentClears.push_back(vk::ClearAttachment(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, attIndex, clearColorValues[attIndex]));
							rectClears.push_back(vk::ClearRect(vk::Rect2D(vk::Offset2D(), vk::Extent2D(width, height)), 0, 1));
						}
					}
					else
					{
						LogVarDebug("Debug : Depth Attachement can't be loaded");
						return false;
					}
					++attIndex;
				}

				if (vCreateRenderPass)
				{
					std::vector<vk::SubpassDescription> subpasses;

					vk::SubpassDescription subPassDesc = {};
					subPassDesc.flags = vk::SubpassDescriptionFlags();
					subPassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
					subPassDesc.inputAttachmentCount = 0;
					subPassDesc.pInputAttachments = nullptr;
					subPassDesc.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
					subPassDesc.pColorAttachments = colorReferences.data();
					if (sampleCount != vk::SampleCountFlagBits::e1)
						subPassDesc.pResolveAttachments = resolveColorReferences.data();
					else
						subPassDesc.pResolveAttachments = nullptr;
					subPassDesc.preserveAttachmentCount = 0;
					subPassDesc.pPreserveAttachments = nullptr;
					if (vUseDepth)
						subPassDesc.pDepthStencilAttachment = depthReferences.data();
					else
						subPassDesc.pDepthStencilAttachment = nullptr;
					subpasses.push_back(subPassDesc);

					// Use subpass dependencies for layout transitions
					std::array<vk::SubpassDependency, 2> dependencies;
					
					dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
					dependencies[0].dstSubpass = 0;
					dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eFragmentShader;
					dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
					dependencies[0].srcAccessMask = vk::AccessFlagBits::eShaderRead;
					dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
					dependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;

					dependencies[1].srcSubpass = 0;
					dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
					dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
					dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
					dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
					dependencies[1].dstAccessMask = vk::AccessFlagBits::eShaderRead;
					dependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;

					vk::RenderPassCreateInfo renderPassInfo = {};
					renderPassInfo.flags = vk::RenderPassCreateFlags();
					renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
					renderPassInfo.pAttachments = attachmentDescriptions.data();
					renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
					renderPassInfo.pSubpasses = subpasses.data();
					renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
					renderPassInfo.pDependencies = dependencies.data();

					vRenderPass = logDevice.createRenderPass(renderPassInfo);
				}

				vk::FramebufferCreateInfo fboInfo = {};
				fboInfo.flags = vk::FramebufferCreateFlags();
				fboInfo.renderPass = vRenderPass;
				fboInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
				fboInfo.pAttachments = attachmentViews.data();
				fboInfo.width = size.x;
				fboInfo.height = size.y;
				fboInfo.layers = 1;

				framebuffer = logDevice.createFramebuffer(fboInfo);

				res = true;
			}
			else
			{
				LogVarDebug("Debug : Size is empty on one chnannel at least : x:%u,y:%u", size.x, size.y);
			}
		}
		else
		{
			LogVarDebug("Debug : CountColorBuffer must be between 0 and 8. here => %u", vCountColorBuffer);
		}

		return res;
	}

	void VulkanFrameBuffer::Unit()
	{
		ZoneScoped;

		attachmentViews.clear();
		attachments.clear();

		if (m_VulkanCore)
		{
			auto logDevice = m_VulkanCore->getDevice();
			logDevice.destroyFramebuffer(framebuffer);
		}
	}

	VulkanFrameBufferAttachment* VulkanFrameBuffer::GetDepthAttachment()
	{
		if (depthAttIndex < attachments.size())
		{
			return &attachments[depthAttIndex];
		}
		return nullptr;
	}
}