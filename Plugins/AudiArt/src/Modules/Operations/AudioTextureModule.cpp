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

#include "AudioTextureModule.h"

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

#include <Modules/Operations/Pass/AudioTextureModule_Comp_2D_Pass.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<AudioTextureModule> AudioTextureModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<AudioTextureModule>(vVulkanCorePtr);
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

AudioTextureModule::AudioTextureModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: BaseRenderer(vVulkanCorePtr)
{
	ZoneScoped;
}

AudioTextureModule::~AudioTextureModule()
{
	ZoneScoped;

	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool AudioTextureModule::Init()
{
	ZoneScoped;

	m_Loaded = false;

	ct::uvec2 map_size = 512;

	if (BaseRenderer::InitCompute2D(map_size))
	{
		//SetExecutionWhenNeededOnly(true);

		m_AudioTextureModule_Comp_2D_Pass_Ptr = std::make_shared<AudioTextureModule_Comp_2D_Pass>(m_VulkanCorePtr);
		if (m_AudioTextureModule_Comp_2D_Pass_Ptr)
		{
			// by default but can be changed via widget
			//m_AudioTextureModule_Comp_2D_Pass_Ptr->AllowResizeOnResizeEvents(false);
			//m_AudioTextureModule_Comp_2D_Pass_Ptr->AllowResizeByHandOrByInputs(true);

			if (m_AudioTextureModule_Comp_2D_Pass_Ptr->InitCompute2D(map_size, 1U, false, vk::Format::eR32G32B32A32Sfloat))
			{
				AddGenericPass(m_AudioTextureModule_Comp_2D_Pass_Ptr);
				m_Loaded = true;
			}
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool AudioTextureModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("Audio Texture", vCmd);

	return true;
}

bool AudioTextureModule::ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("Audio Texture", vCmd);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool AudioTextureModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Audio Texture##AudioTextureModule", -1.0f, true, true, &m_CanWeRender))
		{
			bool change = false;

			if (m_AudioTextureModule_Comp_2D_Pass_Ptr)
			{
				change |= m_AudioTextureModule_Comp_2D_Pass_Ptr->DrawWidgets(vCurrentFrame, vContext);
			}

			return change;
		}
	}

	return false;
}

void AudioTextureModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void AudioTextureModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void AudioTextureModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	// do some code
	
	BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// SCENEAUDIART INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void AudioTextureModule::SetSceneAudiArt(const std::string& vName, SceneAudiArtWeak vSceneAudiArt)
{	
	ZoneScoped;

	if (m_AudioTextureModule_Comp_2D_Pass_Ptr)
	{
		m_AudioTextureModule_Comp_2D_Pass_Ptr->SetSceneAudiArt(vName, vSceneAudiArt);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* AudioTextureModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;

	if (m_AudioTextureModule_Comp_2D_Pass_Ptr)
	{
		return m_AudioTextureModule_Comp_2D_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string AudioTextureModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += vOffset + "<audio_texture_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	if (m_AudioTextureModule_Comp_2D_Pass_Ptr)
	{
		str += m_AudioTextureModule_Comp_2D_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
	}

	str += vOffset + "</audio_texture_module>\n";

	return str;
}

bool AudioTextureModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "audio_texture_module")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();

		if (m_AudioTextureModule_Comp_2D_Pass_Ptr)
		{
			m_AudioTextureModule_Comp_2D_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
		}
	}

	return true;
}

void AudioTextureModule::AfterNodeXmlLoading()
{
	ZoneScoped;

	if (m_AudioTextureModule_Comp_2D_Pass_Ptr)
	{
		m_AudioTextureModule_Comp_2D_Pass_Ptr->AfterNodeXmlLoading();
	}
}
