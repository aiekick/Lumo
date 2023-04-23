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

#include "AlienRockModule.h"

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

#include <Modules/Tesselation/Pass/AlienRockModule_Mesh_Pass.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<AlienRockModule> AlienRockModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<AlienRockModule>(vVulkanCorePtr);
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

AlienRockModule::AlienRockModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: BaseRenderer(vVulkanCorePtr)
{
	ZoneScoped;
}

AlienRockModule::~AlienRockModule()
{
	ZoneScoped;

	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool AlienRockModule::Init()
{
	ZoneScoped;

	m_Loaded = false;

	ct::uvec2 map_size = 512;

	if (BaseRenderer::InitPixel(map_size))
	{
		//SetExecutionWhenNeededOnly(true);

		m_AlienRockModule_Mesh_Pass_Ptr = std::make_shared<AlienRockModule_Mesh_Pass>(m_VulkanCorePtr);
		if (m_AlienRockModule_Mesh_Pass_Ptr)
		{
			// by default but can be changed via widget
			m_AlienRockModule_Mesh_Pass_Ptr->AllowResizeOnResizeEvents(true);
			m_AlienRockModule_Mesh_Pass_Ptr->AllowResizeByHandOrByInputs(false);


			if (m_AlienRockModule_Mesh_Pass_Ptr->InitPixel(map_size, 1U, true, true, 0.0f,
				false, false, vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1))
			{
				AddGenericPass(m_AlienRockModule_Mesh_Pass_Ptr);
				m_Loaded = true;
			}
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool AlienRockModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("Alien Rock", vCmd);

	return true;
}

bool AlienRockModule::ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("Alien Rock", vCmd);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool AlienRockModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Alien Rock##AlienRockModule", -1.0f, true, true, &m_CanWeRender))
		{
			bool change = false;

			if (m_AlienRockModule_Mesh_Pass_Ptr)
			{
				change |= m_AlienRockModule_Mesh_Pass_Ptr->DrawWidgets(vCurrentFrame, vContext);
			}

			return change;
		}
	}

	return false;
}

void AlienRockModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void AlienRockModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	ZoneScoped;

	assert(vContext); 
	ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void AlienRockModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	// do some code
	
	BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* AlienRockModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;

	if (m_AlienRockModule_Mesh_Pass_Ptr)
	{
		return m_AlienRockModule_Mesh_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string AlienRockModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += vOffset + "<alien_rock_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	if (m_AlienRockModule_Mesh_Pass_Ptr)
	{
		str += m_AlienRockModule_Mesh_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
	}

	str += vOffset + "</alien_rock_module>\n";

	return str;
}

bool AlienRockModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "alien_rock_module")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();

		if (m_AlienRockModule_Mesh_Pass_Ptr)
		{
			m_AlienRockModule_Mesh_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
		}
	}

	return true;
}

void AlienRockModule::AfterNodeXmlLoading()
{
	ZoneScoped;

	if (m_AlienRockModule_Mesh_Pass_Ptr)
	{
		m_AlienRockModule_Mesh_Pass_Ptr->AfterNodeXmlLoading();
	}
}
