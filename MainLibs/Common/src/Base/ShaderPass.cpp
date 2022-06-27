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

#include "ShaderPass.h"

#include <utility>
#include <functional>

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <vkFramework/VulkanSubmitter.h>

#include <FontIcons/CustomFont.h>

#include <Base/FrameBuffer.h>

#define TRACE_MEMORY
#include <vkProfiler/Profiler.h>

using namespace vkApi;

//#define VERBOSE_DEBUG
//#define BLEND_ENABLED

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / CONSTRUCTOR //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderPass::ShaderPass(vkApi::VulkanCore* vVulkanCore)
{
	ZoneScoped;

	m_RendererType = GenericType::NONE;
	m_VulkanCore = vVulkanCore;
	m_Device = m_VulkanCore->getDevice();
}

ShaderPass::ShaderPass(vkApi::VulkanCore* vVulkanCore, const GenericType& vRendererTypeEnum)
{
	ZoneScoped;

	m_RendererType = vRendererTypeEnum;
	m_VulkanCore = vVulkanCore;
	m_Device = m_VulkanCore->getDevice();
}

ShaderPass::ShaderPass(vkApi::VulkanCore* vVulkanCore, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool)
{
	ZoneScoped;

	m_RendererType = GenericType::NONE;
	m_VulkanCore = vVulkanCore;
	m_Device = m_VulkanCore->getDevice();
	m_CommandPool = *vCommandPool;
	m_DescriptorPool = *vDescriptorPool;
}

ShaderPass::ShaderPass(vkApi::VulkanCore* vVulkanCore, const GenericType& vRendererTypeEnum, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool)
{
	ZoneScoped;

	m_RendererType = vRendererTypeEnum;
	m_VulkanCore = vVulkanCore;
	m_Device = m_VulkanCore->getDevice();
	m_CommandPool = *vCommandPool;
	m_DescriptorPool = *vDescriptorPool;
}

ShaderPass::~ShaderPass()
{
	ZoneScoped;

	Unit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / DURING INIT //////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderPass::ActionBeforeInit()
{

}

void ShaderPass::ActionAfterInitSucceed()
{

}

void ShaderPass::ActionAfterInitFail()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / INIT/UNIT ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderPass::InitPixel(
	const ct::uvec2& vSize,
	const uint32_t& vCountColorBuffer,
	const bool& vUseDepth,
	const bool& vNeedToClear,
	const ct::fvec4& vClearColor,
	const vk::Format& vFormat,
	const vk::SampleCountFlagBits& vSampleCount)
{
	m_RendererType = GenericType::PIXEL;

	m_CountBuffers = vCountColorBuffer;
	m_SampleCount = vSampleCount;

	Resize(vSize); // will update m_RenderArea and m_Viewport

	ActionBeforeInit();

	m_Loaded = false;

	m_Device = m_VulkanCore->getDevice();
	m_Queue = m_VulkanCore->getQueue(vk::QueueFlagBits::eGraphics);
	m_DescriptorPool = m_VulkanCore->getDescriptorPool();
	m_CommandPool = m_Queue.cmdPools;

	m_FrameBufferPtr = FrameBuffer::Create(m_VulkanCore);
	if (m_FrameBufferPtr->Init(vSize, vCountColorBuffer, vUseDepth, vNeedToClear, vClearColor, vFormat, vSampleCount)) {
		if (CompilPixel()) {
			if (BuildModel()) {
				if (CreateSBO()) {
					if (CreateUBO()) {
						if (CreateRessourceDescriptor()) {
							if (CreatePixelPipeline()) {
								m_Loaded = true;
							}
						}
					}
				}
			}
		}
	}

	if (m_Loaded)
	{
		ActionAfterInitSucceed();
	}
	else
	{
		ActionAfterInitFail();
	}

	return m_Loaded;
}

bool ShaderPass::InitCompute(const ct::uvec3& vDispatchSize)
{
	m_RendererType = GenericType::COMPUTE;

	m_DispatchSize = vDispatchSize;

	CTOOL_DEBUG_BREAK;

	return true;
}

void ShaderPass::Unit()
{
	ZoneScoped;

	m_Device.waitIdle();
	DestroyPipeline();
	DestroyRessourceDescriptor();
	DestroyUBO();
	DestroySBO();
	DestroyModel(true);
	m_FrameBufferPtr.reset();
}
void ShaderPass::AllowResize(const bool& vResizing)
{
	m_ResizingIsAllowed = vResizing;
}

void ShaderPass::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	if (m_FrameBufferPtr && m_ResizingIsAllowed)
	{
		m_FrameBufferPtr->NeedResize(vNewSize, vCountColorBuffer);
	}
}

void ShaderPass::NeedResize(ct::ivec2* vNewSize)
{
	if (m_FrameBufferPtr && m_ResizingIsAllowed)
	{
		m_FrameBufferPtr->NeedResize(vNewSize);
	}
}

bool ShaderPass::ResizeIfNeeded()
{
	ZoneScoped;

	if (m_FrameBufferPtr && m_ResizingIsAllowed)
	{
		if (m_FrameBufferPtr->ResizeIfNeeded())
		{
			Resize(m_FrameBufferPtr->GetOutputSize());
			return true;
		}
	}

	return false;
}

void ShaderPass::Resize(const ct::uvec2& vNewSize)
{
	ZoneScoped;

	if (m_ResizingIsAllowed)
	{
		m_RenderArea = vk::Rect2D(vk::Offset2D(), vk::Extent2D(vNewSize.x, vNewSize.y));
		m_Viewport = vk::Viewport(0.0f, 0.0f, static_cast<float>(vNewSize.x), static_cast<float>(vNewSize.y), 0, 1.0f);
		m_OutputSize = ct::fvec2((float)vNewSize.x, (float)vNewSize.y);
		m_OutputRatio = m_OutputSize.ratioXY<float>();
	}
}

void ShaderPass::DrawPass(vk::CommandBuffer* vCmdBuffer)
{
	m_VulkanCore->getFrameworkDevice()->BeginDebugLabel(vCmdBuffer, m_RenderDocDebugName, m_RenderDocDebugColor);

	if (m_FrameBufferPtr->Begin(vCmdBuffer))
	{
		DrawModel(vCmdBuffer, 1U);

		m_FrameBufferPtr->End(vCmdBuffer);
	}

	m_VulkanCore->getFrameworkDevice()->EndDebugLabel(vCmdBuffer);
}

void ShaderPass::DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	UNUSED(vIterationNumber);

	ZoneScoped;

	if (!m_Loaded) return;

	if (vCmdBuffer)
	{
		// for a vertex
		//vCmdBuffer->setLineWidth(m_LineWidth.w);
		//vCmdBuffer->setPrimitiveTopologyEXT(m_PrimitiveTopology);
		//vCmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
		//vCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout, 0, m_DescriptorSet, nullptr);
		//vCmdBuffer->draw(m_CountVertexs.w, m_CountInstances.w, 0, 0);

		// for a mesh
		CTOOL_DEBUG_BREAK;
	}
}

bool ShaderPass::BuildModel()
{
	ZoneScoped;

	return true;
}

void ShaderPass::DestroyModel(const bool& vReleaseDatas)
{
	ZoneScoped;

	if (vReleaseDatas)
	{
		
	}
}

void ShaderPass::UpdateModel(const bool& vLoaded)
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / VERTEX ///////////////////////////////////////////////////////////////////////////////////////
//// count point to draw ///////////////////////////////////////////////////////////////////////////////////
//// count instances ///////////////////////////////////////////////////////////////////////////////////////
//// etc.. /////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderPass::SetCountVertexs(const uint32_t& vCountVertexs)
{
	ZoneScoped;

	m_CountVertexs.w = ct::maxi<uint32_t>(1u, vCountVertexs);
}

void ShaderPass::SetCountInstances(const uint32_t& vCountInstances)
{
	ZoneScoped;

	m_CountInstances.w = ct::maxi<uint32_t>(1u, vCountInstances);
}

void ShaderPass::SetCountIterations(const uint32_t& vCountIterations)
{
	ZoneScoped;

	m_CountIterations.w = ct::maxi<uint32_t>(1u, vCountIterations);
}

void ShaderPass::SetLineWidth(const float& vLineWidth)
{
	ZoneScoped;

	m_LineWidth.w = vLineWidth;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / SHADER ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderPass::GetComputeShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ShaderPass_Compute";
	return m_ComputeShaderCode;
}

std::string ShaderPass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ShaderPass_Vertex";
	return m_FragmentShaderCode;
}

std::string ShaderPass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ShaderPass_Fragment";
	return m_FragmentShaderCode;
}

bool ShaderPass::IsPixelRenderer()
{
	return (m_RendererType == GenericType::PIXEL);
}

bool ShaderPass::IsComputeRenderer()
{
	return (m_RendererType == GenericType::COMPUTE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / SPECIFIC UPDATE CODE ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderPass::CompilPixel()
{
	ZoneScoped;

	bool res = false;

	ActionBeforeCompilation();

	std::string vertex_name;
	auto vertCode = GetVertexShaderCode(vertex_name);
	if (vertex_name.empty())
		vertex_name = "ShaderPass_Vertex";

	std::string fragment_name;
	auto fragCode = GetFragmentShaderCode(fragment_name);
	if (fragment_name.empty())
		fragment_name = "ShaderPass_Fragment";

	FileHelper::Instance()->SaveStringToFile(vertCode, "debug/" + vertex_name + ".glsl");
	FileHelper::Instance()->SaveStringToFile(fragCode, "debug/" + fragment_name + ".glsl");

	if (!vertCode.empty() && !fragCode.empty())
	{
		if (vkApi::VulkanCore::sVulkanShader)
		{
			m_UsedUniforms.clear();
			m_SPIRV_Vert = vkApi::VulkanCore::sVulkanShader->CompileGLSLString(vertCode, "vert", vertex_name);
			m_SPIRV_Frag = vkApi::VulkanCore::sVulkanShader->CompileGLSLString(fragCode, "frag", fragment_name);

			if (!m_Loaded)
			{
				res = true;
			}
			else if (!m_SPIRV_Frag.empty() && !m_SPIRV_Vert.empty())
			{
				m_Device.waitIdle();
				DestroyPipeline();
				DestroyRessourceDescriptor();
				DestroyUBO();
				DestroySBO();
				CreateSBO();
				CreateUBO();
				CreateRessourceDescriptor();
				CreatePixelPipeline();
				res = true;
			}
		}
	}

	ActionAfterCompilation();

	return res;
}

bool ShaderPass::CompilCompute()
{
	ZoneScoped;

	bool res = false;

	ActionBeforeCompilation();

	std::string compute_name;
	auto compCode = GetComputeShaderCode(compute_name);
	if (compute_name.empty())
		compute_name = "ShaderPass_Compute";

	FileHelper::Instance()->SaveStringToFile(compCode, "debug/" + compute_name + ".glsl");

	if (!compCode.empty())
	{
		if (vkApi::VulkanCore::sVulkanShader)
		{
			m_UsedUniforms.clear();
			m_SPIRV_Comp = vkApi::VulkanCore::sVulkanShader->CompileGLSLString(compCode, "comp", compute_name);

			if (!m_Loaded)
			{
				res = true;
			}
			else if (!m_SPIRV_Frag.empty() && !m_SPIRV_Vert.empty())
			{
				m_Device.waitIdle();
				DestroyPipeline();
				DestroyRessourceDescriptor();
				DestroyUBO();
				DestroySBO();
				CreateSBO();
				CreateUBO();
				CreateRessourceDescriptor();
				CreateComputePipeline();
				res = true;
			}
		}
	}

	ActionAfterCompilation();

	return res;
}

bool ShaderPass::ReCompil()
{
	bool res = false;

	if (m_RendererType == GenericType::COMPUTE)
	{
		res = CompilCompute();
	}
	else if (m_RendererType == GenericType::PIXEL)
	{
		res = CompilPixel();
	}

	return res;
}

void ShaderPass::ActionBeforeCompilation()
{

}

void ShaderPass::ActionAfterCompilation()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShaderPass::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	//str += m_UniformWidgets.getXml(vOffset, vUserDatas);

	return str;
}

// return true for continue xml parsing of childs in this node or false for interupt the child exploration
bool ShaderPass::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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
//// PRIVATE / UBO /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderPass::CreateUBO()
{
	ZoneScoped;

	return true;
}

void ShaderPass::NeedNewUBOUpload()
{
	m_NeedNewUBOUpload = true;
}

void ShaderPass::UploadUBO()
{
	ZoneScoped;

	CTOOL_DEBUG_BREAK;
}

void ShaderPass::DestroyUBO()
{
	ZoneScoped;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / SBO /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderPass::CreateSBO()
{
	ZoneScoped;

	return true;
}

void ShaderPass::NeedNewSBOUpload()
{
	m_NeedNewSBOUpload = true;
}

void ShaderPass::UploadSBO()
{
	ZoneScoped;

	CTOOL_DEBUG_BREAK;
}

void ShaderPass::DestroySBO()
{
	ZoneScoped;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / RESSOURCE DESCRIPTORS ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderPass::UpdateBufferInfoInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;

	writeDescriptorSets.clear();

	return res;
}

bool ShaderPass::UpdateLayoutBindingInRessourceDescriptor()
{
	ZoneScoped;

	bool res = true;

	m_LayoutBindings.clear();

	return res;
}

bool ShaderPass::CreateRessourceDescriptor()
{
	ZoneScoped;

	if (UpdateLayoutBindingInRessourceDescriptor())
	{
		m_DescriptorSetLayout =
			m_Device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(
				vk::DescriptorSetLayoutCreateFlags(),
				static_cast<uint32_t>(m_LayoutBindings.size()),
				m_LayoutBindings.data()
			));

		m_DescriptorSet =
			m_Device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo(
				m_DescriptorPool, 1, &m_DescriptorSetLayout))[0];

		if (UpdateBufferInfoInRessourceDescriptor())
		{
			UpdateRessourceDescriptor();
			return true;
		}
	}

	return false;
}

void ShaderPass::UpdateRessourceDescriptor()
{
	ZoneScoped;

	m_Device.waitIdle();

	// il s'agit de maj les buffer info
	// et ca doit ce faire en dehors d'un renderpass
	// ce qui peu etre faire dans un renderpass c'est juste de la maj de data,
	// mais pas des switch de buffer infos

	// update samplers outside of the command buffer
	//assert(!m_This.expired());
	/*m_UniformWidgets.UpdateSamplers(m_This);
	if (IsCompute2DRenderer())
	{
		m_UniformWidgets.UpdateImages(m_This);
	}
	m_UniformWidgets.UpdateBuffers(m_This);
	m_UniformWidgets.UpdateBlocks(m_This, true);*/

	if (m_NeedNewUBOUpload)
	{
		UploadUBO();
		m_NeedNewUBOUpload = false;
	}

	if (m_NeedNewSBOUpload)
	{
		UploadSBO();
		m_NeedNewSBOUpload = false;
	}

	// update descriptor
	m_Device.updateDescriptorSets(writeDescriptorSets, nullptr);

	// on le met la avant le rendu pluto qu'apres sinon au reload la 1ere
	// frame vaut 0 et ca peut reset le shader selon ce qu'on a mis dedans
	// par contre on incremente la frame apres le submit
	//m_UniformWidgets.SetFrame(m_Frame);
}

void ShaderPass::DestroyRessourceDescriptor()
{
	ZoneScoped;

	m_Device.waitIdle();

	if (m_DescriptorPool && m_DescriptorSet)
		m_Device.freeDescriptorSets(m_DescriptorPool, m_DescriptorSet);
	if (m_DescriptorSetLayout)
		m_Device.destroyDescriptorSetLayout(m_DescriptorSetLayout);
	m_DescriptorSet = vk::DescriptorSet{};
	m_DescriptorSetLayout = vk::DescriptorSetLayout{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / PIPELINE ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderPass::CreateComputePipeline()
{
	ZoneScoped;

	if (m_SPIRV_Comp.empty()) return false;

	m_PipelineLayout =
		m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			1, &m_DescriptorSetLayout,
			0, nullptr
		));

	auto cs = vkApi::VulkanCore::sVulkanShader->CreateShaderModule((VkDevice)m_Device, m_SPIRV_Comp);

	m_ShaderCreateInfos.clear();
	m_ShaderCreateInfos.resize(1);
	m_ShaderCreateInfos[0] = vk::PipelineShaderStageCreateInfo(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eCompute,
		cs, "main"
	);

	vk::ComputePipelineCreateInfo computePipeInfo = vk::ComputePipelineCreateInfo()
		.setStage(m_ShaderCreateInfos[0]).setLayout(m_PipelineLayout);
	m_Pipeline = m_Device.createComputePipeline(nullptr, computePipeInfo).value;

	vkApi::VulkanCore::sVulkanShader->DestroyShaderModule((VkDevice)m_Device, cs);

	return true;
}

void ShaderPass::SetInputStateBeforePipelineCreation()
{
	//for a mesh:
	//VertexStruct::P2_T2::GetInputState(m_InputState);

	//for a vertex
	//m_InputState = auto vertexInputState = vk::PipelineVertexInputStateCreateInfo(
	// vk::PipelineVertexInputStateCreateFlags(), // flags
	//	0, nullptr, // vertexBindingDescriptionCount
	//	0, nullptr // vertexAttributeDescriptions
	//	);
}

bool ShaderPass::CreatePixelPipeline()
{
	ZoneScoped;

	if (m_SPIRV_Vert.empty()) return false;
	if (m_SPIRV_Frag.empty()) return false;
	if (!m_FrameBufferPtr) return false;
	if (!m_FrameBufferPtr->GetRenderPass()) return false;

	m_PipelineLayout =
		m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			1, &m_DescriptorSetLayout,
			0, nullptr
		));

	auto vs = vkApi::VulkanCore::sVulkanShader->CreateShaderModule((VkDevice)m_Device, m_SPIRV_Vert);
	auto fs = vkApi::VulkanCore::sVulkanShader->CreateShaderModule((VkDevice)m_Device, m_SPIRV_Frag);

	m_ShaderCreateInfos = {
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eVertex,
			vs, "main"
		),
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eFragment,
			fs, "main"
		)
	};

	// setup fix functions
	auto assemblyState = vk::PipelineInputAssemblyStateCreateInfo(
		vk::PipelineInputAssemblyStateCreateFlags(),
		m_PrimitiveTopology
	);

	auto viewportState = vk::PipelineViewportStateCreateInfo(
		vk::PipelineViewportStateCreateFlags(),
		1, &m_Viewport,
		1, &m_RenderArea
	);

	auto rasterState = vk::PipelineRasterizationStateCreateInfo(
		vk::PipelineRasterizationStateCreateFlags(),
		VK_FALSE,
		VK_FALSE,
		vk::PolygonMode::eFill,
		vk::CullModeFlagBits::eNone,
		vk::FrontFace::eCounterClockwise,
		VK_FALSE,
		0,
		0,
		0,
		m_LineWidth.w
	);

	auto multisampleState = vk::PipelineMultisampleStateCreateInfo(vk::PipelineMultisampleStateCreateFlags());
	multisampleState.rasterizationSamples = m_SampleCount;
	//multisampleState.sampleShadingEnable = VK_TRUE;
	//multisampleState.minSampleShading = 0.2f;

	auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo(
		vk::PipelineDepthStencilStateCreateFlags(),
		VK_TRUE,
		VK_TRUE,
		vk::CompareOp::eLessOrEqual,
		VK_FALSE,
		VK_FALSE,
		vk::StencilOpState(),
		vk::StencilOpState(),
		0,
		0
	);

#ifdef BLEND_ENABLED
	m_BlendAttachmentStates.clear();
	for (uint32_t i = 0; i < m_CountBuffers; ++i)
	{
		m_BlendAttachmentStates.emplace_back(
			VK_TRUE,
			vk::BlendFactor::eSrcAlpha,
			vk::BlendFactor::eOneMinusSrcAlpha,
			vk::BlendOp::eAdd,
			vk::BlendFactor::eOne,
			vk::BlendFactor::eZero,
			vk::BlendOp::eAdd,
			vk::ColorComponentFlags(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA));
	}

	auto colorBlendState = vk::PipelineColorBlendStateCreateInfo(
		vk::PipelineColorBlendStateCreateFlags(),
		VK_FALSE,
		vk::LogicOp::eCopy,
		static_cast<uint32_t>(m_BlendAttachmentStates.size()),
		m_BlendAttachmentStates.data()
	);
#else
	m_BlendAttachmentStates.clear();
	for (uint32_t i = 0; i < m_CountBuffers; ++i)
	{
		m_BlendAttachmentStates.emplace_back(
			VK_FALSE,
			vk::BlendFactor::eZero,
			vk::BlendFactor::eOne,
			vk::BlendOp::eAdd,
			vk::BlendFactor::eZero,
			vk::BlendFactor::eZero,
			vk::BlendOp::eAdd,
			vk::ColorComponentFlags(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA));
	}

	auto colorBlendState = vk::PipelineColorBlendStateCreateInfo(
		vk::PipelineColorBlendStateCreateFlags(),
		VK_FALSE,
		vk::LogicOp::eClear,
		static_cast<uint32_t>(m_BlendAttachmentStates.size()),
		m_BlendAttachmentStates.data()
	);
#endif
	auto dynamicStateList = std::vector<vk::DynamicState>{
		vk::DynamicState::eViewport
		,vk::DynamicState::eScissor
		,vk::DynamicState::eLineWidth
		//,vk::DynamicState::ePrimitiveTopologyEXT
	};

	auto dynamicState = vk::PipelineDynamicStateCreateInfo(
		vk::PipelineDynamicStateCreateFlags(),
		static_cast<uint32_t>(dynamicStateList.size()),
		dynamicStateList.data()
	);

	SetInputStateBeforePipelineCreation();

	m_Pipeline = m_Device.createGraphicsPipeline(
		m_PipelineCache,
		vk::GraphicsPipelineCreateInfo(
			vk::PipelineCreateFlags(),
			static_cast<uint32_t>(m_ShaderCreateInfos.size()),
			m_ShaderCreateInfos.data(),
			&m_InputState.state,
			&assemblyState,
			nullptr,
			&viewportState,
			&rasterState,
			&multisampleState,
			&depthStencilState,
			&colorBlendState,
			&dynamicState,
			m_PipelineLayout,
			*m_FrameBufferPtr->GetRenderPass(),
			0
		)
	).value;

	vkApi::VulkanCore::sVulkanShader->DestroyShaderModule((VkDevice)m_Device, vs);
	vkApi::VulkanCore::sVulkanShader->DestroyShaderModule((VkDevice)m_Device, fs);

	return true;
}

void ShaderPass::DestroyPipeline()
{
	ZoneScoped;

	if (m_PipelineLayout)
		m_Device.destroyPipelineLayout(m_PipelineLayout);
	if (m_Pipeline)
		m_Device.destroyPipeline(m_Pipeline);
	if (m_PipelineCache)
		m_Device.destroyPipelineCache(m_PipelineCache);
	m_PipelineLayout = vk::PipelineLayout {};
	m_Pipeline = vk::Pipeline {};
	m_PipelineCache = vk::PipelineCache {};
}
