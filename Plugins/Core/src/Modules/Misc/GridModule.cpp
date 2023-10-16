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

#include "GridModule.h"

#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>
#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Modules/Misc/Pass/GridModule_Vertex_Pass.h>
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

std::shared_ptr<GridModule> GridModule::Create(GaiApi::VulkanCorePtr vVulkanCorePtr)
{
	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<GridModule>(vVulkanCorePtr);
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

GridModule::GridModule(GaiApi::VulkanCorePtr vVulkanCorePtr)
	: TaskRenderer(vVulkanCorePtr)
{
	m_SceneShaderPassPtr = SceneShaderPass::Create();
}

GridModule::~GridModule()
{
	Unit();

	m_SceneShaderPassPtr.reset();
}

//////////////////////////////////////////////////////////////
//// INIT ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool GridModule::Init()
{
	ZoneScoped;

	ct::uvec2 map_size = 512;

	m_Loaded = true;

	if (TaskRenderer::InitPixel(map_size))
	{
		m_GridModule_Vertex_Pass_Ptr = std::make_shared<GridModule_Vertex_Pass>(m_VulkanCorePtr);
		if (m_GridModule_Vertex_Pass_Ptr)
		{
			if (m_GridModule_Vertex_Pass_Ptr->InitPixel(map_size, 1U, true, true, 0.0f, 
				false, false, vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e2))
			{
				AddGenericPass(m_GridModule_Vertex_Pass_Ptr);
				m_SceneShaderPassPtr->Add(m_GridModule_Vertex_Pass_Ptr);
				m_Loaded = true;
			}
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// PUBLIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool GridModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	TaskRenderer::Render("Grid Module", vCmd);
	
	return true;
}

void GridModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	TaskRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

bool GridModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	ZoneScoped;

	if (IsTheGoodFrame(vCurrentFrame))
	{
		if (ImGui::CollapsingHeader_CheckBox("Grid/Axis Module", -1.0f, true, true, &m_CanWeRender))
		{
			if (m_GridModule_Vertex_Pass_Ptr)
			{
				return m_GridModule_Vertex_Pass_Ptr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
			}
		}
	}

	return false;
}

bool GridModule::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
    return false;
}

bool GridModule::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
    return false;
}

vk::DescriptorImageInfo* GridModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	if (m_GridModule_Vertex_Pass_Ptr)
	{
		return m_GridModule_Vertex_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

SceneShaderPassWeak GridModule::GetShaderPasses(const uint32_t& vSlotID)
{
	return m_SceneShaderPassPtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GridModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += vOffset + "<grid_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	for (auto pass : m_ShaderPasses)
	{
		auto& pass_ptr = pass.lock();
		if (pass_ptr)
		{
			str += pass_ptr->getXml(vOffset + "\t", vUserDatas);
		}
	}

	str += vOffset + "</grid_module>\n";

	return str;
}

bool GridModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "grid_module")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();
	}

	for (auto pass : m_ShaderPasses)
	{
		auto& pass_ptr = pass.lock();
		if (pass_ptr)
		{
			pass_ptr->setFromXml(vElem, vParent, vUserDatas);
		}
	}

	return true;
}