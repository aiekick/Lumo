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
		vkApi::VulkanCorePtr vVulkanCorePtr,
		ct::uvec2 vSize,
		uint32_t vCountColorBuffers,
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

		m_VulkanCorePtr = vVulkanCorePtr;
		needToClear = vNeedToClear;

		if (vCountColorBuffers > 0 && vCountColorBuffers <= 8)
		{
			ct::uvec2 size = ct::clamp(vSize, 1u, 8192u);
			if (!size.emptyOR())
			{
				attachments.clear();
				attachmentViews.clear();
				attachmentClears.clear();
				clearColorValues.clear();
				rectClears.clear();

				auto logDevice = m_VulkanCorePtr->getDevice();

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
				attachments.resize(vCountColorBuffers
					+ (useMultiSampling ? vCountColorBuffers : 0U)
					+ (vUseDepth ? 1U : 0U)
				);

				uint32_t attIndex = 0;

				for (uint32_t j = 0; j < vCountColorBuffers; j++)
				{
					if (attachments[attIndex].InitColor2D(m_VulkanCorePtr, size, format, 1U, vNeedToClear, sampleCount))
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
					for (uint32_t j = 0; j < vCountColorBuffers; j++)
					{
						if (attachments[attIndex].InitColor2D(m_VulkanCorePtr, size, format, 1U, vNeedToClear, vk::SampleCountFlagBits::e1))
						{
							attachmentViews.push_back(attachments[attIndex].attachmentView);
							attachmentDescriptions.push_back(attachments[attIndex].attachmentDescription);
							resolveColorReferences.push_back(vk::AttachmentReference(attIndex, vk::ImageLayout::eColorAttachmentOptimal));
							if (vNeedToClear)
							{
								clearColorValues.push_back(vk::ClearColorValue(std::array<float, 4>{ vClearColor.x, vClearColor.y, vClearColor.z, vClearColor.w }));
								attachmentClears.push_back(vk::ClearAttachment(vk::ImageAspectFlagBits::eColor, attIndex, clearColorValues[attIndex]));
								rectClears.push_back(vk::ClearRect(vk::Rect2D(vk::Offset2D(), vk::Extent2D(width, height)), 0, 1));
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

					// the depth must have the same sampleCount as color, if not the app will crash
					if (attachments[attIndex].InitDepth(m_VulkanCorePtr, size, vk::Format::eD32SfloatS8Uint, sampleCount))
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
					{
						vk::SubpassDescription subPassDesc = {};
						subPassDesc.flags = vk::SubpassDescriptionFlags();
						subPassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
						subPassDesc.inputAttachmentCount = 0;
						subPassDesc.pInputAttachments = nullptr;
						subPassDesc.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
						subPassDesc.pColorAttachments = colorReferences.data();
						subPassDesc.pResolveAttachments = resolveColorReferences.data(); // null if empty +> OK
						subPassDesc.preserveAttachmentCount = 0;
						subPassDesc.pPreserveAttachments = nullptr;
						subPassDesc.pDepthStencilAttachment = depthReferences.data(); // null if empty +> OK
						subpasses.push_back(subPassDesc);
					}

					// Use subpass dependencies for layout transitions
					std::vector<vk::SubpassDependency> dependencies;
					{
						/*{
							vk::SubpassDependency colorDependencie;
							colorDependencie.srcSubpass = VK_SUBPASS_EXTERNAL;
							colorDependencie.dstSubpass = 0;
							colorDependencie.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
							colorDependencie.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
							colorDependencie.srcAccessMask = vk::AccessFlagBits::eNone;
							colorDependencie.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
							colorDependencie.dependencyFlags = vk::DependencyFlagBits::eByRegion;
							dependencies.push_back(colorDependencie);
						}*/

						if (vUseDepth)
						{
							{
								vk::SubpassDependency depthDependencie;
								depthDependencie.srcSubpass = VK_SUBPASS_EXTERNAL;
								depthDependencie.dstSubpass = 0;
								depthDependencie.srcStageMask = vk::PipelineStageFlagBits::eLateFragmentTests;
								depthDependencie.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
								depthDependencie.dstStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests;
								depthDependencie.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
								depthDependencie.dependencyFlags = vk::DependencyFlagBits::eByRegion;
								dependencies.push_back(depthDependencie);
							}

							{
								vk::SubpassDependency depthDependencie;
								depthDependencie.srcSubpass = 0;
								depthDependencie.dstSubpass = VK_SUBPASS_EXTERNAL;
								depthDependencie.srcStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests;
								depthDependencie.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
								depthDependencie.dstStageMask = vk::PipelineStageFlagBits::eLateFragmentTests;
								depthDependencie.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
								depthDependencie.dependencyFlags = vk::DependencyFlagBits::eByRegion;
								dependencies.push_back(depthDependencie);
							}
						}

						{
							vk::SubpassDependency colorDependencie;
							colorDependencie.srcSubpass = VK_SUBPASS_EXTERNAL;
							colorDependencie.dstSubpass = 0;
							colorDependencie.srcStageMask = vk::PipelineStageFlagBits::eFragmentShader;
							colorDependencie.srcAccessMask = vk::AccessFlagBits::eShaderRead;
							colorDependencie.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
							colorDependencie.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
							colorDependencie.dependencyFlags = vk::DependencyFlagBits::eByRegion;
							dependencies.push_back(colorDependencie);
						}

						{
							vk::SubpassDependency colorDependencie;
							colorDependencie.srcSubpass = 0;
							colorDependencie.dstSubpass = VK_SUBPASS_EXTERNAL;
							colorDependencie.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
							colorDependencie.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
							colorDependencie.dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
							colorDependencie.dstAccessMask = vk::AccessFlagBits::eShaderRead;
							colorDependencie.dependencyFlags = vk::DependencyFlagBits::eByRegion;
							dependencies.push_back(colorDependencie); 
						}
					}

					vk::RenderPassCreateInfo renderPassInfo = {};
					{
						renderPassInfo.flags = vk::RenderPassCreateFlags();
						renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
						renderPassInfo.pAttachments = attachmentDescriptions.data();
						renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
						renderPassInfo.pSubpasses = subpasses.data();
						renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
						renderPassInfo.pDependencies = dependencies.data();
					}

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
			LogVarDebug("Debug : CountColorBuffer must be between 0 and 8. here => %u", vCountColorBuffers);
		}

		return res;
	}

	void VulkanFrameBuffer::Unit()
	{
		ZoneScoped;

		attachmentViews.clear();
		attachments.clear();

		if (m_VulkanCorePtr)
		{
			auto logDevice = m_VulkanCorePtr->getDevice();
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