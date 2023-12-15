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

#include "VolumeRenderingModule.h"

#include <cinttypes>
#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <LumoBackend/Graph/Base/BaseNode.h>
#include <ImGuiPack.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <Gaia/Buffer/FrameBuffer.h>

#include <Modules/Rendering/Pass/VolumeRenderingModule_Comp_2D_Pass.h>

using namespace GaiApi;

#ifdef PROFILER_INCLUDE
#include PROFILER_INCLUDE
#endif
#ifndef ZoneScoped
#define ZoneScoped
#endif

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<VolumeRenderingModule> VolumeRenderingModule::Create(GaiApi::VulkanCoreWeak vVulkanCore, BaseNodeWeak vParentNode)
{
	ZoneScoped;

	
	auto res = std::make_shared<VolumeRenderingModule>(vVulkanCore);
	res->SetParentNode(vParentNode);
	res->m_This = res;
	if (!res->Init()) {
		res.reset();
	}

	return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

VolumeRenderingModule::VolumeRenderingModule(GaiApi::VulkanCoreWeak vVulkanCore)
	: BaseRenderer(vVulkanCore)
{
	ZoneScoped;
}

VolumeRenderingModule::~VolumeRenderingModule()
{
	ZoneScoped;

	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool VolumeRenderingModule::Init()
{
	ZoneScoped;

	m_Loaded = false;

	ct::uvec2 map_size = 512;
	if (BaseRenderer::InitCompute2D(map_size)) {
		//SetExecutionWhenNeededOnly(true);
		m_VolumeRenderingModule_Comp_2D_Pass_Ptr = VolumeRenderingModule_Comp_2D_Pass::Create(map_size, m_VulkanCore);
		if (m_VolumeRenderingModule_Comp_2D_Pass_Ptr) {
			// by default but can be changed via widget
			m_VolumeRenderingModule_Comp_2D_Pass_Ptr->AllowResizeOnResizeEvents(true);
			m_VolumeRenderingModule_Comp_2D_Pass_Ptr->AllowResizeByHandOrByInputs(false);
			AddGenericPass(m_VolumeRenderingModule_Comp_2D_Pass_Ptr);
			m_Loaded = true;
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool VolumeRenderingModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;
		BaseRenderer::Render("Volume Rendering", vCmd);
	return true;
}

bool VolumeRenderingModule::ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;
	BaseRenderer::Render("Volume Rendering", vCmd);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// DRAW WIDGETS ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

bool VolumeRenderingModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas)
{
	ZoneScoped;

	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);

	if (m_LastExecutedFrame == vCurrentFrame) {
		if (m_VolumeRenderingModule_Comp_2D_Pass_Ptr) {
	        if (ImGui::CollapsingHeader_CheckBox("Volume Rendering##VolumeRenderingModule_Comp_2D_Pass", -1.0f, false, true, &m_CanWeRender)) {
			    return m_VolumeRenderingModule_Comp_2D_Pass_Ptr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
            }
		}
	}
	return false;
}

bool VolumeRenderingModule::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas)
{
	ZoneScoped;

	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (m_VolumeRenderingModule_Comp_2D_Pass_Ptr) {
			return m_VolumeRenderingModule_Comp_2D_Pass_Ptr->DrawOverlays(vCurrentFrame, vRect, vContextPtr, vUserDatas);
		}
	}

	return false;
}

bool VolumeRenderingModule::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, void* vUserDatas)
{
	ZoneScoped;

	assert(vContextPtr); 
	ImGui::SetCurrentContext(vContextPtr);
	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (m_VolumeRenderingModule_Comp_2D_Pass_Ptr) {
			return m_VolumeRenderingModule_Comp_2D_Pass_Ptr->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
		}
	}

	return false;
}

void VolumeRenderingModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	ZoneScoped;

	// do some code
	
	BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE 3D SLOT INPUT //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void VolumeRenderingModule::SetTexture3D(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec3* vTextureSize) {	
	ZoneScoped;

	if (m_VolumeRenderingModule_Comp_2D_Pass_Ptr) {
		m_VolumeRenderingModule_Comp_2D_Pass_Ptr->SetTexture3D(vBindingPoint, vImageInfo, vTextureSize);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
//// TEXTURE SLOT OUTPUT /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

vk::DescriptorImageInfo* VolumeRenderingModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize, void* vUserDatas) {	
	ZoneScoped;

	if (m_VolumeRenderingModule_Comp_2D_Pass_Ptr) {
		return m_VolumeRenderingModule_Comp_2D_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize, vUserDatas);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string VolumeRenderingModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	ZoneScoped;
	std::string str;
	str += vOffset + "<volume_rendering_module>\n";
	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";
	if (m_VolumeRenderingModule_Comp_2D_Pass_Ptr) {
		str += m_VolumeRenderingModule_Comp_2D_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
	}
	str += vOffset + "</volume_rendering_module>\n";
	return str;
}

bool VolumeRenderingModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "volume_rendering_module")	{
		if (strName == "can_we_render") {
			m_CanWeRender = ct::ivariant(strValue).GetB();
		} 
	}
		if (m_VolumeRenderingModule_Comp_2D_Pass_Ptr) {
			m_VolumeRenderingModule_Comp_2D_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
		}

	return true;
}

void VolumeRenderingModule::AfterNodeXmlLoading()
{
	ZoneScoped;
	if (m_VolumeRenderingModule_Comp_2D_Pass_Ptr) {
		m_VolumeRenderingModule_Comp_2D_Pass_Ptr->AfterNodeXmlLoading();
	}
}
