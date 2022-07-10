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

ShaderPass::ShaderPass(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	ZoneScoped;

	m_RendererType = GenericType::NONE;
	m_VulkanCorePtr = vVulkanCorePtr;
	m_Device = m_VulkanCorePtr->getDevice();
}

ShaderPass::ShaderPass(vkApi::VulkanCorePtr vVulkanCorePtr, const GenericType& vRendererTypeEnum)
{
	ZoneScoped;

	m_RendererType = vRendererTypeEnum;
	m_VulkanCorePtr = vVulkanCorePtr;
	m_Device = m_VulkanCorePtr->getDevice();
}

ShaderPass::ShaderPass(vkApi::VulkanCorePtr vVulkanCorePtr, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool)
{
	ZoneScoped;

	m_RendererType = GenericType::NONE;
	m_VulkanCorePtr = vVulkanCorePtr;
	m_Device = m_VulkanCorePtr->getDevice();
	m_CommandPool = *vCommandPool;
	m_DescriptorPool = *vDescriptorPool;
}

ShaderPass::ShaderPass(vkApi::VulkanCorePtr vVulkanCorePtr, const GenericType& vRendererTypeEnum, vk::CommandPool* vCommandPool, vk::DescriptorPool* vDescriptorPool)
{
	ZoneScoped;

	m_RendererType = vRendererTypeEnum;
	m_VulkanCorePtr = vVulkanCorePtr;
	m_Device = m_VulkanCorePtr->getDevice();
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
	const bool& vMultiPassMode,
	const vk::Format& vFormat,
	const vk::SampleCountFlagBits& vSampleCount)
{
	m_RendererType = GenericType::PIXEL;

	m_CountBuffers = vCountColorBuffer;
	m_SampleCount = vSampleCount;

	Resize(vSize); // will update m_RenderArea and m_Viewport

	ActionBeforeInit();

	m_Loaded = false;

	m_Device = m_VulkanCorePtr->getDevice();
	m_Queue = m_VulkanCorePtr->getQueue(vk::QueueFlagBits::eGraphics);
	m_DescriptorPool = m_VulkanCorePtr->getDescriptorPool();
	m_CommandPool = m_Queue.cmdPools;

	// ca peut ne pas compiler, masi c'est plus bloquant
	// on va plutot mettre un cadre rouge, avec le message d'erreur au survol
	CompilPixel();

	m_FrameBufferPtr = FrameBuffer::Create(m_VulkanCorePtr);
	if (m_FrameBufferPtr->Init(
		vSize, vCountColorBuffer, vUseDepth, vNeedToClear,
		vClearColor, vMultiPassMode, vFormat, vSampleCount)) {
		if (BuildModel()) {
			if (CreateSBO()) {
				if (CreateUBO()) {
					if (CreateRessourceDescriptor()) {
						// si ca compile pas
						// c'est pas bon mais on renvoi true car on va afficher 
						// l'erreur dans le node et on pourra le corriger en editant le shader
						CreatePixelPipeline();
						m_Loaded = true;
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

bool ShaderPass::InitCompute2D(
	const ct::uvec2& vDispatchSize,
	const uint32_t& vCountBuffers,
	const bool& vMultiPassMode,
	const vk::Format& vFormat)
{
	m_RendererType = GenericType::COMPUTE_2D;

	Resize(vDispatchSize); // will update m_RenderArea and m_Viewport

	ActionBeforeInit();

	m_Loaded = false;

	m_Device = m_VulkanCorePtr->getDevice();
	m_Queue = m_VulkanCorePtr->getQueue(vk::QueueFlagBits::eGraphics);
	m_DescriptorPool = m_VulkanCorePtr->getDescriptorPool();
	m_CommandPool = m_Queue.cmdPools;

	m_DispatchSize = ct::uvec3(vDispatchSize.x, vDispatchSize.y, 1);

	// ca peut ne pas compiler, masi c'est plus bloquant
	// on va plutot mettre un cadre rouge, avec le message d'erreur au survol
	CompilCompute();

	m_ComputeBufferPtr = ComputeBuffer::Create(m_VulkanCorePtr);
	if (m_ComputeBufferPtr && 
		m_ComputeBufferPtr->Init(
		vDispatchSize, vCountBuffers, 
		vMultiPassMode, vFormat)) {
		if (BuildModel()) {
			if (CreateSBO()) {
				if (CreateUBO()) {
					if (CreateRessourceDescriptor()) {
						// si ca compile pas
						// c'est pas bon mais on renvoi true car on va afficher 
						// l'erreur dans le node et on pourra le corriger en editant le shader
						CreateComputePipeline();
						m_Loaded = true;
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

bool ShaderPass::InitCompute3D(const ct::uvec3& vDispatchSize)
{
	m_RendererType = GenericType::COMPUTE_3D;

	ActionBeforeInit();

	m_Loaded = false;

	m_Device = m_VulkanCorePtr->getDevice();
	m_Queue = m_VulkanCorePtr->getQueue(vk::QueueFlagBits::eGraphics);
	m_DescriptorPool = m_VulkanCorePtr->getDescriptorPool();
	m_CommandPool = m_Queue.cmdPools;

	m_DispatchSize = vDispatchSize;

	// ca peut ne pas compiler, masi c'est plus bloquant
	// on va plutot mettre un cadre rouge, avec le message d'erreur au survol
	CompilCompute();

	if (BuildModel()) {
		if (CreateSBO()) {
			if (CreateUBO()) {
				if (CreateRessourceDescriptor()) {
					// si ca compile pas
					// c'est pas bon mais on renvoi true car on va afficher 
					// l'erreur dans le node et on pourra le corriger en editant le shader
					CreateComputePipeline();
					m_Loaded = true;
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
	m_ComputeBufferPtr.reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC ////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShaderPass::SetRenderDocDebugName(const char* vLabel, ct::fvec4 vColor)
{
#ifdef VULKAN_DEBUG
	m_RenderDocDebugName = vLabel;
	m_RenderDocDebugColor = vColor;
#else
	UNUSED(vLabel);
	UNUSED(vColor);
#endif
}

void ShaderPass::AllowResize(const bool& vResizing)
{
	m_ResizingIsAllowed = vResizing;
}

void ShaderPass::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	if (m_ResizingIsAllowed)
	{
		if (m_FrameBufferPtr)
		{
			m_FrameBufferPtr->NeedResize(vNewSize, vCountColorBuffer);
		}
		else if (m_ComputeBufferPtr)
		{
			m_ComputeBufferPtr->NeedResize(vNewSize, vCountColorBuffer);
		}
	}
}

void ShaderPass::NeedResize(ct::ivec2* vNewSize)
{
	if (m_ResizingIsAllowed)
	{
		if (m_FrameBufferPtr)
		{
			m_FrameBufferPtr->NeedResize(vNewSize);
		}
		else if (m_ComputeBufferPtr)
		{
			m_ComputeBufferPtr->NeedResize(vNewSize);
		}
	}
}

bool ShaderPass::ResizeIfNeeded()
{
	ZoneScoped;

	if (m_ResizingIsAllowed)
	{
		if (m_FrameBufferPtr && 
			m_FrameBufferPtr->ResizeIfNeeded())
		{
			Resize(m_FrameBufferPtr->GetOutputSize());
			return true;
		}
		else if (
			m_ComputeBufferPtr && 
			m_ComputeBufferPtr->ResizeIfNeeded())
		{
			UpdateBufferInfoInRessourceDescriptor();
			Resize(m_ComputeBufferPtr->GetOutputSize());
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
		if (m_FrameBufferPtr)
		{
			m_RenderArea = vk::Rect2D(vk::Offset2D(), vk::Extent2D(vNewSize.x, vNewSize.y));
			m_Viewport = vk::Viewport(0.0f, 0.0f, static_cast<float>(vNewSize.x), static_cast<float>(vNewSize.y), 0, 1.0f);
		}
		else if (m_ComputeBufferPtr)
		{
			m_DispatchSize = ct::uvec3(vNewSize, 1U);
		}

		m_OutputSize = ct::fvec2((float)vNewSize.x, (float)vNewSize.y);
		m_OutputRatio = m_OutputSize.ratioXY<float>();
	}
}

void ShaderPass::SwapOutputDescriptors()
{
	
}

void ShaderPass::DrawPass(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	if (m_IsShaderCompiled && m_Pipeline && m_PipelineLayout)
	{
		auto devicePtr = m_VulkanCorePtr->getFrameworkDevice().getValidShared();
		if (devicePtr)
		{
			devicePtr->BeginDebugLabel(vCmdBuffer, m_RenderDocDebugName, m_RenderDocDebugColor);
		}

		if (IsPixelRenderer() && m_FrameBufferPtr)
		{
			if (m_FrameBufferPtr->Begin(vCmdBuffer))
			{
				m_FrameBufferPtr->ClearAttachmentsIfNeeded(vCmdBuffer, m_ForceFBOClearing);
				m_ForceFBOClearing = false;

				DrawModel(vCmdBuffer, vIterationNumber);

				m_FrameBufferPtr->End(vCmdBuffer);
			}
		}
		else if (IsCompute2DRenderer() && m_ComputeBufferPtr)
		{
			if (m_ComputeBufferPtr->Begin(vCmdBuffer))
			{
				Compute(vCmdBuffer, vIterationNumber);

				m_ComputeBufferPtr->End(vCmdBuffer);
			}
		}
		else if (IsCompute3DRenderer())
		{
			Compute(vCmdBuffer, vIterationNumber);
		}

		if (devicePtr)
		{
			devicePtr->EndDebugLabel(vCmdBuffer);
		}
	}
}

void ShaderPass::DrawModel(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	UNUSED(vIterationNumber);

	ZoneScoped;

	if (!m_Loaded) return;
	if (!m_IsShaderCompiled) return;

	if (vCmdBuffer)
	{
		CTOOL_DEBUG_BREAK;
	}
}

void ShaderPass::Compute(vk::CommandBuffer* vCmdBuffer, const int& vIterationNumber)
{
	UNUSED(vIterationNumber);

	ZoneScoped;

	if (!m_Loaded) return;
	if (!m_IsShaderCompiled) return;

	if (vCmdBuffer)
	{
		CTOOL_DEBUG_BREAK;
	}
}

bool ShaderPass::BuildModel()
{
	ZoneScoped;

	return true;
}

void ShaderPass::NeedNewModelUpload()
{
	m_NeedNewModelUpdate = true;
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
	ZoneScoped;

	if (vLoaded)
	{
		DestroyModel();
	}

	BuildModel();

	UpdateBufferInfoInRessourceDescriptor();
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
	return m_ComputeShaderCode.m_Code;
}

std::string ShaderPass::GetVertexShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ShaderPass_Vertex";
	return m_VertexShaderCode.m_Code;
}

std::string ShaderPass::GetFragmentShaderCode(std::string& vOutShaderName)
{
	vOutShaderName = "ShaderPass_Fragment";
	return m_FragmentShaderCode.m_Code;
}

bool ShaderPass::IsPixelRenderer()
{
	return (m_RendererType == GenericType::PIXEL);
}

bool ShaderPass::IsCompute2DRenderer()
{
	return (m_RendererType == GenericType::COMPUTE_2D);
}

bool ShaderPass::IsCompute3DRenderer()
{
	return (m_RendererType == GenericType::COMPUTE_3D);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / SPECIFIC UPDATE CODE ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderPass::CompilPixel()
{
	ZoneScoped;

	bool res = false;

	ActionBeforeCompilation();

	// VERTEX CODE

	std::string vertex_name;
	m_VertexShaderCode.m_Code = GetVertexShaderCode(vertex_name);
	assert(!vertex_name.empty());
	if (!m_DontUseShaderFilesOnDisk)
	{
		m_VertexShaderCode.m_FilePathName = "shaders/" + vertex_name + ".vert";
		auto vert_path = FileHelper::Instance()->GetAppPath() + "/" + m_VertexShaderCode.m_FilePathName;
		if (FileHelper::Instance()->IsFileExist(vert_path, true))
		{
			m_VertexShaderCode.m_Code = FileHelper::Instance()->LoadFileToString(vert_path, true);
		}
		else
		{
			FileHelper::Instance()->SaveStringToFile(m_VertexShaderCode.m_Code, vert_path);
		}
	}

	// FRAGMENT CODE

	std::string fragment_name;
	m_FragmentShaderCode.m_Code = GetFragmentShaderCode(fragment_name);
	assert(!fragment_name.empty());
	if (!m_DontUseShaderFilesOnDisk)
	{
		m_FragmentShaderCode.m_FilePathName = "shaders/" + fragment_name + ".frag";
		auto frag_path = FileHelper::Instance()->GetAppPath() + "/" + m_FragmentShaderCode.m_FilePathName;
		if (FileHelper::Instance()->IsFileExist(frag_path, true))
		{
			m_FragmentShaderCode.m_Code = FileHelper::Instance()->LoadFileToString(frag_path, true);
		}
		else
		{
			FileHelper::Instance()->SaveStringToFile(m_FragmentShaderCode.m_Code, frag_path);
		}
	}

	// COMPILATION

	if (!m_VertexShaderCode.m_Code.empty() && 
		!m_FragmentShaderCode.m_Code.empty())
	{
		if (vkApi::VulkanCore::sVulkanShader)
		{
			m_UsedUniforms.clear();
			m_VertexShaderCode.m_SPIRV = vkApi::VulkanCore::sVulkanShader->CompileGLSLString(m_VertexShaderCode.m_Code, "vert", vertex_name);
			m_FragmentShaderCode.m_SPIRV = vkApi::VulkanCore::sVulkanShader->CompileGLSLString(m_FragmentShaderCode.m_Code, "frag", fragment_name);

			m_IsShaderCompiled = !m_VertexShaderCode.m_SPIRV.empty() && !m_FragmentShaderCode.m_SPIRV.empty();

			if (!m_Loaded)
			{
				res = true;
			}
			else if (!m_FragmentShaderCode.m_SPIRV.empty() && !m_VertexShaderCode.m_SPIRV.empty())
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

	// COMPUTE CODE

	std::string compute_name;
	m_ComputeShaderCode.m_Code = GetComputeShaderCode(compute_name);
	assert(!compute_name.empty());
	if (!m_DontUseShaderFilesOnDisk)
	{
		m_ComputeShaderCode.m_FilePathName = "shaders/" + compute_name + ".comp";
		auto comp_path = FileHelper::Instance()->GetAppPath() + "/" + m_ComputeShaderCode.m_FilePathName;
		if (FileHelper::Instance()->IsFileExist(comp_path, true))
		{
			m_ComputeShaderCode.m_Code = FileHelper::Instance()->LoadFileToString(comp_path, true);
		}
		else
		{

			FileHelper::Instance()->SaveStringToFile(m_ComputeShaderCode.m_Code, comp_path);
		}
	}

	// COMPILATION

	if (!m_ComputeShaderCode.m_Code.empty())
	{
		if (vkApi::VulkanCore::sVulkanShader)
		{
			m_UsedUniforms.clear();
			m_ComputeShaderCode.m_SPIRV = vkApi::VulkanCore::sVulkanShader->CompileGLSLString(m_ComputeShaderCode.m_Code, "comp", compute_name);

			m_IsShaderCompiled = !m_ComputeShaderCode.m_SPIRV.empty();

			if (!m_Loaded)
			{
				res = true;
			}
			else if (!m_ComputeShaderCode.m_SPIRV.empty())
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

	if (m_RendererType == GenericType::COMPUTE_2D ||
		m_RendererType == GenericType::COMPUTE_3D)
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

	if (m_ComputeBufferPtr && 
		m_ComputeBufferPtr->IsMultiPassMode())
	{
		SwapOutputDescriptors();
	}

	// update descriptor
	m_Device.updateDescriptorSets(writeDescriptorSets, nullptr);

	// on le met la avant le rendu plutot qu'apres sinon au reload la 1ere
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

void ShaderPass::UpdateShaders(const std::set<std::string>& vFiles)
{
	bool needReCompil = false;

	if (vFiles.find(m_VertexShaderCode.m_FilePathName) != vFiles.end())
	{
		auto shader_path = FileHelper::Instance()->GetAppPath() + "/" + m_VertexShaderCode.m_FilePathName;
		if (FileHelper::Instance()->IsFileExist(shader_path))
		{
			needReCompil = true;
		}

	}
	else if (vFiles.find(m_FragmentShaderCode.m_FilePathName) != vFiles.end())
	{
		auto shader_path = FileHelper::Instance()->GetAppPath() + "/" + m_FragmentShaderCode.m_FilePathName;
		if (FileHelper::Instance()->IsFileExist(shader_path))
		{
			needReCompil = true;
		}
	}
	else if (vFiles.find(m_ComputeShaderCode.m_FilePathName) != vFiles.end())
	{
		auto shader_path = FileHelper::Instance()->GetAppPath() + "/" + m_ComputeShaderCode.m_FilePathName;
		if (FileHelper::Instance()->IsFileExist(shader_path))
		{
			needReCompil = true;
		}
	}

	if (needReCompil)
	{
		ReCompil();
	}
}

void ShaderPass::NeedToClearFBOThisFrame()
{
	m_ForceFBOClearing = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / PUSH CONSTANTS //////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

// to call before create pipeline
void ShaderPass::SetPushConstantRange(const vk::PushConstantRange& vPushConstantRange)
{
	m_Internal_PushConstants = vPushConstantRange;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE / PIPELINE ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ShaderPass::CreateComputePipeline()
{
	ZoneScoped;

	if (m_ComputeShaderCode.m_SPIRV.empty()) return false;

	std::vector<vk::PushConstantRange> push_constants;
	if (m_Internal_PushConstants.size)
	{
		push_constants.push_back(m_Internal_PushConstants);
	}

	m_PipelineLayout =
		m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			1, &m_DescriptorSetLayout,
			(uint32_t)push_constants.size(), push_constants.data()
		));
	
	auto cs = vkApi::VulkanCore::sVulkanShader->CreateShaderModule((VkDevice)m_Device, m_ComputeShaderCode.m_SPIRV);

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

	if (!m_FrameBufferPtr) return false;
	if (!m_FrameBufferPtr->GetRenderPass()) return false;
	if (m_VertexShaderCode.m_SPIRV.empty()) return false;
	if (m_FragmentShaderCode.m_SPIRV.empty()) return false;
	
	std::vector<vk::PushConstantRange> push_constants;
	if (m_Internal_PushConstants.size)
	{
		push_constants.push_back(m_Internal_PushConstants);
	}

	m_PipelineLayout =
		m_Device.createPipelineLayout(vk::PipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(),
			1, &m_DescriptorSetLayout,
			(uint32_t)push_constants.size(), push_constants.data()
		));

	auto vs = vkApi::VulkanCore::sVulkanShader->CreateShaderModule((VkDevice)m_Device, m_VertexShaderCode.m_SPIRV);
	auto fs = vkApi::VulkanCore::sVulkanShader->CreateShaderModule((VkDevice)m_Device, m_FragmentShaderCode.m_SPIRV);

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
