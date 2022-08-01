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

#include "WidgetColorModule.h"

#include <functional>
#include <Gui/MainFrame.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <vkFramework/VulkanSubmitter.h>
#include <utils/Mesh/VertexStruct.h>
#include <cinttypes>
#include <Base/FrameBuffer.h>

#include <Modules/Widgets/Pass/WidgetColorModule_Pass.h>

using namespace vkApi;

#define COUNT_BUFFERS 2

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<WidgetColorModule> WidgetColorModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<WidgetColorModule>(vVulkanCorePtr);
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

WidgetColorModule::WidgetColorModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: BaseRenderer(vVulkanCorePtr)
{

}

WidgetColorModule::~WidgetColorModule()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool WidgetColorModule::Init()
{
	ZoneScoped;

	ct::uvec2 map_size = 1;

	m_Loaded = false;

	if (BaseRenderer::InitCompute2D(map_size))
	{
		m_WidgetColorModule_Pass_Ptr = std::make_shared<WidgetColorModule_Pass>(m_VulkanCorePtr);
		if (m_WidgetColorModule_Pass_Ptr)
		{
			if (m_WidgetColorModule_Pass_Ptr->InitCompute2D(map_size, 1U, false, vk::Format::eR32G32B32A32Sfloat))
			{
				AddGenericPass(m_WidgetColorModule_Pass_Ptr);
				m_Loaded = true;
			}
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

// todo : to put the rendering in when needed, so when the suer chnage the color only
// so use ExecuteWhenNeeded

bool WidgetColorModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("WidgetColor", vCmd);

	return true;
}

bool WidgetColorModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	//if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Widget Color", -1.0f, true, true, &m_CanWeRender))
		{
			bool change = false;

			if (m_WidgetColorModule_Pass_Ptr)
			{
				change |= m_WidgetColorModule_Pass_Ptr->DrawWidgets(vCurrentFrame, vContext);
			}

			return change;
		}
	}

	return false;
}

void WidgetColorModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void WidgetColorModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

bool WidgetColorModule::DrawNodeWidget(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	//if (m_LastExecutedFrame == vCurrentFrame)
	{
		bool change = false;

		if (m_WidgetColorModule_Pass_Ptr)
		{
			change |= m_WidgetColorModule_Pass_Ptr->DrawNodeWidget(vCurrentFrame, vContext);
		}

		return change;
	}

	return false;
}

vk::DescriptorImageInfo* WidgetColorModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	ZoneScoped;

	if (m_WidgetColorModule_Pass_Ptr)
	{
		return m_WidgetColorModule_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string WidgetColorModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += vOffset + "<widget_color_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	if (m_WidgetColorModule_Pass_Ptr)
	{
		str += m_WidgetColorModule_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
	}

	str += vOffset + "</widget_color_module>\n";

	return str;
}

bool WidgetColorModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "widget_color_module")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();
	}

	if (m_WidgetColorModule_Pass_Ptr)
	{
		m_WidgetColorModule_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}