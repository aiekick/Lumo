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

#include "SceneMergerModule.h"

#include <cinttypes>
#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <Graph/Base/BaseNode.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanSubmitter.h>
#include <utils/Mesh/VertexStruct.h>
#include <Base/FrameBuffer.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<SceneMergerModule> SceneMergerModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<SceneMergerModule>(vVulkanCorePtr);
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

SceneMergerModule::SceneMergerModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: BaseRenderer(vVulkanCorePtr)
{
	ZoneScoped;
}

SceneMergerModule::~SceneMergerModule()
{
	ZoneScoped;

	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool SceneMergerModule::Init()
{
	ZoneScoped;

	m_Loaded = false;

	ct::uvec2 map_size = 512;

	if (BaseRenderer::InitPixel(map_size))
	{
		//SetExecutionWhenNeededOnly(true);

		AllowResizeOnResizeEvents(true);
		AllowResizeByHandOrByInputs(true);

		m_FrameBufferPtr = FrameBuffer::Create(m_VulkanCorePtr);
		if (m_FrameBufferPtr && m_FrameBufferPtr->Init(
			map_size, 1U, true, true, 0.0f, false,
			vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e2))
		{
			m_Loaded = true;
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool SceneMergerModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("Scene Merger", vCmd);

	return true;
}

bool SceneMergerModule::ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("Scene Merger", vCmd);

	return true;
}

void SceneMergerModule::RenderShaderPasses(vk::CommandBuffer* vCmdBuffer)
{
	if (m_FrameBufferPtr &&
		m_FrameBufferPtr->Begin(vCmdBuffer))
	{
		m_FrameBufferPtr->ClearAttachmentsIfNeeded(vCmdBuffer);

		for (auto pass : m_ShaderPasses)
		{
			auto pass_ptr = pass.getValidShared();
			if (pass_ptr && pass_ptr->StartDrawPass(vCmdBuffer))
			{
				pass_ptr->SetLastExecutedFrame(m_LastExecutedFrame); // for have widgets use

				pass_ptr->DrawModel(vCmdBuffer, 1U);

				pass_ptr->EndDrawPass(vCmdBuffer);
			}
		}

		m_FrameBufferPtr->End(vCmdBuffer);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMergerModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Scene Merger##SceneMergerModule", -1.0f, true, true, &m_CanWeRender))
		{
			bool change = false;

			return change;
		}
	}

	return false;
}

void SceneMergerModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void SceneMergerModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PUBLIC / RESIZE ///////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMergerModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	if (IsResizeableByResizeEvent())
	{
		if (m_FrameBufferPtr)
		{
			m_FrameBufferPtr->NeedResize(vNewSize, vCountColorBuffers);
		}
	}
}

void SceneMergerModule::NeedResizeByHand(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	if (IsResizeableByHand())
	{
		if (m_FrameBufferPtr)
		{
			m_FrameBufferPtr->NeedResize(vNewSize, vCountColorBuffers);
		}
	}
}

bool SceneMergerModule::ResizeIfNeeded()
{
	ZoneScoped;

	if (IsResizeableByHand() ||
		IsResizeableByResizeEvent())
	{
		if (m_FrameBufferPtr &&
			m_FrameBufferPtr->ResizeIfNeeded())
		{
			auto output_size = m_FrameBufferPtr->GetOutputSize();
			for (auto pass : m_ShaderPasses)
			{
				auto pass_ptr = pass.getValidShared();
				if (pass_ptr)
				{
					pass_ptr->UpdatePixel2DViewportSize(output_size);
				}
			}

			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void SceneMergerModule::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{	
	ZoneScoped;
}

void SceneMergerModule::SetShaderPasses(const uint32_t& vSlotID, SceneShaderPassWeak vShaderPasses)
{
	ZoneScoped;

	if (m_FrameBufferPtr)
	{
		if (m_SceneShaderPassContainer.find(vSlotID) != m_SceneShaderPassContainer.end())
		{
			// il faut rediriger la pass vers son FBO natif
			if (vShaderPasses.expired() && !m_SceneShaderPassContainer.at(vSlotID).expired())
			{
				auto scene_pass_ptr = m_SceneShaderPassContainer.at(vSlotID).getValidShared();
				if (scene_pass_ptr)
				{
					for (auto pass : *scene_pass_ptr)
					{
						auto pass_ptr = pass.getValidShared();
						if (pass_ptr)
						{
							pass_ptr->ReSetRenderPassToNative();
						}
					}
				}
			}
		}

		m_JustDeletedSceneShaderPassSlots.clear();

		// on ajoute ou remplace la passe, ici la passe peut etre vide
		if (vShaderPasses.expired())
		{
			if (m_SceneShaderPassContainer.find(vSlotID) != m_SceneShaderPassContainer.end())
			{
				m_SceneShaderPassContainer.erase(vSlotID);
				m_JustDeletedSceneShaderPassSlots.push_back(vSlotID);
			}
		}
		else
		{
			m_SceneShaderPassContainer[vSlotID] = vShaderPasses;
		}

		// rebuilds shader_passes vector
		BaseRenderer::m_ShaderPasses.clear();

		// iterate over scene shader passes
		// and add child passes to BaseRenderer::m_ShaderPasses
		// but if many pass are the sam, we must have only one rendering for perf
		std::set<ShaderPassWeak> m_unique_passes;
		for (auto scene_pass : m_SceneShaderPassContainer)
		{
			auto scene_pass_ptr = scene_pass.second.getValidShared();
			if (scene_pass_ptr)
			{
				for (auto pass : *scene_pass_ptr)
				{
					auto pass_ptr = pass.getValidShared();
					if (pass_ptr && 
						m_unique_passes.find(pass_ptr) == m_unique_passes.end()) // to be sure than no identic pass was inserted before
					{
						// add to set for "already exisiting" test
						m_unique_passes.emplace(pass_ptr);

						// piloted pass, will rebuild the pass pipeline
						pass_ptr->SetRenderPass(m_FrameBufferPtr->GetRenderPass());

						// add the pass for this module
						AddGenericPass(pass_ptr);
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* SceneMergerModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
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

SceneShaderPassContainer& SceneMergerModule::GetSceneShaderPassContainerRef()
{
	ZoneScoped;

	return m_SceneShaderPassContainer;
}

const std::vector<uint32_t>& SceneMergerModule::GetJustDeletedSceneShaderPassSlots() const
{
	return m_JustDeletedSceneShaderPassSlots;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string SceneMergerModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += vOffset + "<scene_merger_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	str += vOffset + "</scene_merger_module>\n";

	return str;
}

bool SceneMergerModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "scene_merger_module")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();
	}

	return true;
}

void SceneMergerModule::AfterNodeXmlLoading()
{
	ZoneScoped;
}
