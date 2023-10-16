/*
Copyright 2022 - 2022 Stephane Cuillerdier(aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissionsand
limitations under the License.
*/

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "PlanetModule.h"

#include <cinttypes>
#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <Graph/Base/BaseNode.h>
#include <ImGuiPack.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Gaia/Buffer/FrameBuffer.h>

#include <Modules/Planet/Pass/PlanetModule_Ground_Mesh_Pass.h>
#include <Modules/Planet/Pass/PlanetModule_Atmosphere_Mesh_Pass.h>
#include <Modules/Planet/Pass/PlanetModule_Water_Mesh_Pass.h>

using namespace GaiApi;

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<PlanetModule> PlanetModule::Create(GaiApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<PlanetModule>(vVulkanCorePtr);
	res->SetParentNode(vParentNode);
	res->m_This = res;
	if (!res->Init())
	{
		res.reset();
	}

	return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

PlanetModule::PlanetModule(GaiApi::VulkanCorePtr vVulkanCorePtr)
	: BaseRenderer(vVulkanCorePtr)
{
	ZoneScoped;
}

PlanetModule::~PlanetModule()
{
	ZoneScoped;

	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool PlanetModule::Init()
{
	ZoneScoped;

	m_Loaded = true;

	ct::uvec2 map_size = 512;

	if (BaseRenderer::InitPixel(map_size))
	{
		//SetExecutionWhenNeededOnly(true);
		m_FrameBufferPtr = FrameBuffer::Create(m_VulkanCorePtr);
		if (m_FrameBufferPtr && m_FrameBufferPtr->Init(
			map_size, 3U, true, true, 0.0f, false, 
			vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e2))
		{
			// ground pass
			m_PlanetModule_Ground_Mesh_Pass_Ptr = std::make_shared<PlanetModule_Ground_Mesh_Pass>(m_VulkanCorePtr);
			if (m_PlanetModule_Ground_Mesh_Pass_Ptr)
			{
				m_PlanetModule_Ground_Mesh_Pass_Ptr->AllowResizeOnResizeEvents(true);
				m_PlanetModule_Ground_Mesh_Pass_Ptr->AllowResizeByHandOrByInputs(false);

				if (m_PlanetModule_Ground_Mesh_Pass_Ptr->InitPixelWithoutFBO(map_size, 3U, true, m_FrameBufferPtr->GetRenderPass(), vk::SampleCountFlagBits::e2))
				{
					//m_PlanetModule_Ground_Mesh_Pass_Ptr->SetFrameBuffer(m_FrameBufferPtr);
					AddGenericPass(m_PlanetModule_Ground_Mesh_Pass_Ptr);
				}
				else
				{
					m_Loaded = false;
				}
			}

			// water pass
			m_PlanetModule_Water_Mesh_Pass_Ptr = std::make_shared<PlanetModule_Water_Mesh_Pass>(m_VulkanCorePtr);
			if (m_PlanetModule_Water_Mesh_Pass_Ptr)
			{
				m_PlanetModule_Water_Mesh_Pass_Ptr->AllowResizeOnResizeEvents(true);
				m_PlanetModule_Water_Mesh_Pass_Ptr->AllowResizeByHandOrByInputs(false);

				if (m_PlanetModule_Water_Mesh_Pass_Ptr->InitPixelWithoutFBO(map_size, 3U, true, m_FrameBufferPtr->GetRenderPass(), vk::SampleCountFlagBits::e2))
				{
					//m_PlanetModule_Water_Mesh_Pass_Ptr->SetFrameBuffer(m_FrameBufferPtr);
					AddGenericPass(m_PlanetModule_Water_Mesh_Pass_Ptr);
				}
				else
				{
					m_Loaded = false;
				}
			}

			// atmosphere pass
			/*m_PlanetModule_Atmosphere_Mesh_Pass_Ptr = std::make_shared<PlanetModule_Atmosphere_Mesh_Pass>(m_VulkanCorePtr);
			if (m_PlanetModule_Atmosphere_Mesh_Pass_Ptr)
			{
				m_PlanetModule_Atmosphere_Mesh_Pass_Ptr->AllowResizeOnResizeEvents(true);
				m_PlanetModule_Atmosphere_Mesh_Pass_Ptr->AllowResizeByHandOrByInputs(false);

				if (m_PlanetModule_Atmosphere_Mesh_Pass_Ptr->InitPixelWithoutFBO(map_size, 3U, true, m_FrameBufferPtr->GetRenderPass(), vk::SampleCountFlagBits::e2))
				{
					m_PlanetModule_Atmosphere_Mesh_Pass_Ptr->SetFrameBuffer(m_FrameBufferPtr);
					AddGenericPass(m_PlanetModule_Atmosphere_Mesh_Pass_Ptr);
				}
				else
				{
					m_Loaded = false;
				}
			}*/
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool PlanetModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("Planet", vCmd);

	return true;
}

bool PlanetModule::ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("Planet", vCmd);

	return true;
}

void PlanetModule::RenderShaderPasses(vk::CommandBuffer* vCmdBuffer)
{
	if (m_FrameBufferPtr &&
		m_FrameBufferPtr->Begin(vCmdBuffer))
	{
		m_FrameBufferPtr->ClearAttachmentsIfNeeded(vCmdBuffer);

		for (auto pass : m_ShaderPasses)
		{
			auto pass_ptr = pass.lock();
			if (pass_ptr && pass_ptr->StartDrawPass(vCmdBuffer))
			{
				pass_ptr->DrawModel(vCmdBuffer, 1U);

				pass_ptr->EndDrawPass(vCmdBuffer);
			}
		}

		m_FrameBufferPtr->End(vCmdBuffer);
	}
}

void PlanetModule::NeedResizeByHand(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	/* not supported by this module
	if (m_FrameBufferPtr)
	{
		m_FrameBufferPtr->NeedResize(vNewSize, vCountColorBuffers);
	}*/
}

void PlanetModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	if (m_FrameBufferPtr)
	{
		m_FrameBufferPtr->NeedResize(vNewSize, vCountColorBuffers);
	}
}

bool PlanetModule::ResizeIfNeeded()
{
	ZoneScoped;

	if (m_FrameBufferPtr && 
		m_FrameBufferPtr->ResizeIfNeeded())
	{
		m_RenderArea = m_FrameBufferPtr->GetRenderArea();
		m_Viewport = m_FrameBufferPtr->GetViewport();
		m_OutputRatio = m_FrameBufferPtr->GetOutputRatio();

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool PlanetModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Planet##PlanetModule", -1.0f, true, true, &m_CanWeRender))
		{
			bool change = false;

			for (auto pass : m_ShaderPasses)
			{
				auto gui_ptr = std::dynamic_pointer_cast<GuiInterface>(pass.lock());
				if (gui_ptr)
				{
					change |= gui_ptr->DrawWidgets(vCurrentFrame, vContext);
				}
			}

			return change;
		}
	}

	return false;
}

void PlanetModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void PlanetModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void PlanetModule::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{	
	ZoneScoped;

	for (auto pass : m_ShaderPasses)
	{
		auto texture_input_ptr = std::dynamic_pointer_cast<TextureInputInterface<3>>(pass.lock());
		if (texture_input_ptr)
		{
			texture_input_ptr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* PlanetModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;

	if (m_FrameBufferPtr)
	{
		if (vOutSize)
		{
			*vOutSize = m_FrameBufferPtr->GetOutputSize();
		}

		return m_FrameBufferPtr->GetFrontDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string PlanetModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += vOffset + "<planet_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	for (auto pass : m_ShaderPasses)
	{
		auto pass_ptr = pass.lock();
		if (pass_ptr)
		{
			str += pass_ptr->getXml(vOffset + "\t", vUserDatas);
		}
	}

	str += vOffset + "</planet_module>\n";

	return str;
}

bool PlanetModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	ZoneScoped;

	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strParentName == "planet_module")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();
	}

	for (auto pass : m_ShaderPasses)
	{
		auto pass_ptr = pass.lock();
		if (pass_ptr)
		{
			pass_ptr->setFromXml(vElem, vParent, vUserDatas);
		}
	}
	
	return true;
}

void PlanetModule::AfterNodeXmlLoading()
{
	ZoneScoped;

	for (auto pass : m_ShaderPasses)
	{
		auto node_pass_ptr = std::dynamic_pointer_cast<NodeInterface>(pass.lock());
		if (node_pass_ptr)
		{
			node_pass_ptr->AfterNodeXmlLoading();
		}
	}
}
