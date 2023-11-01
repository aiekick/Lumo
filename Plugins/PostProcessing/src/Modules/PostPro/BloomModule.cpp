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

#include "BloomModule.h"

#include <cinttypes>
#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Gaia/Buffer/FrameBuffer.h>

#include <Modules/PostPro/Pass/BloomModule_Comp_2D_Pass.h>

using namespace GaiApi;

#ifdef PROFILER_INCLUDE
#include <Gaia/gaia.h>
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<BloomModule> BloomModule::Create(GaiApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	ZoneScoped;

	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<BloomModule>(vVulkanCorePtr);
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

BloomModule::BloomModule(GaiApi::VulkanCorePtr vVulkanCorePtr)
	: BaseRenderer(vVulkanCorePtr)
{
	ZoneScoped;
}

BloomModule::~BloomModule()
{
	ZoneScoped;

	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool BloomModule::Init()
{
	ZoneScoped;

	m_Loaded = false;


	ct::uvec2 map_size = 512;

	if (BaseRenderer::InitCompute2D(map_size))
	{
		//SetExecutionWhenNeededOnly(true);

		m_BloomModule_Comp_2D_Pass_Ptr = std::make_shared<BloomModule_Comp_2D_Pass>(m_VulkanCorePtr);
		if (m_BloomModule_Comp_2D_Pass_Ptr)
		{
			// by default but can be changed via widget
			//m_BloomModule_Comp_2D_Pass_Ptr->AllowResizeOnResizeEvents(false);
			//m_BloomModule_Comp_2D_Pass_Ptr->AllowResizeByHandOrByInputs(true);

			if (m_BloomModule_Comp_2D_Pass_Ptr->InitCompute2D(map_size, 2U, false, vk::Format::eR32G32B32A32Sfloat))
			{
				AddGenericPass(m_BloomModule_Comp_2D_Pass_Ptr);
				m_Loaded = true;
			}
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool BloomModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("Bloom", vCmd);

	return true;
}

bool BloomModule::ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("Bloom", vCmd);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool BloomModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	ZoneScoped;

	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);

	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Bloom##BloomModule", -1.0f, true, true, &m_CanWeRender))
		{
			bool change = false;

			if (m_BloomModule_Comp_2D_Pass_Ptr)
			{
				change |= m_BloomModule_Comp_2D_Pass_Ptr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
			}

			return change;
		}

	}

	return false;
}

bool BloomModule::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;

	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);

	if (m_LastExecutedFrame == vCurrentFrame) {
    }
    return false;

}

bool BloomModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	ZoneScoped;

	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
    return false;

}

void BloomModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	// do some code
	
	BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void BloomModule::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{	
	ZoneScoped;
	if (m_BloomModule_Comp_2D_Pass_Ptr)
	{
		m_BloomModule_Comp_2D_Pass_Ptr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* BloomModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{	
	ZoneScoped;
	if (m_BloomModule_Comp_2D_Pass_Ptr)
	{
		return m_BloomModule_Comp_2D_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string BloomModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;

	std::string str;

	str += vOffset + "<bloom_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	if (m_BloomModule_Comp_2D_Pass_Ptr)
	{
		str += m_BloomModule_Comp_2D_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
	}

	str += vOffset + "</bloom_module>\n";

	return str;
}

bool BloomModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "bloom_module")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();

	if (m_BloomModule_Comp_2D_Pass_Ptr)
	{
		m_BloomModule_Comp_2D_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
	}

	}

	return true;
}

void BloomModule::AfterNodeXmlLoading()
{
	if (m_BloomModule_Comp_2D_Pass_Ptr)
	{
		m_BloomModule_Comp_2D_Pass_Ptr->AfterNodeXmlLoading();
	}
}