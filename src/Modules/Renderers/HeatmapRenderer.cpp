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

#include "HeatmapRenderer.h"

#include <functional>
#include <Gui/MainFrame.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <Modules/Renderers/Pass/HeatmapRenderer_Pass.h>
using namespace vkApi;

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<HeatmapRenderer> HeatmapRenderer::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<HeatmapRenderer>(vVulkanCorePtr);
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

HeatmapRenderer::HeatmapRenderer(vkApi::VulkanCorePtr vVulkanCorePtr)
	: BaseRenderer(vVulkanCorePtr)
{

}

HeatmapRenderer::~HeatmapRenderer()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool HeatmapRenderer::Init()
{
	ZoneScoped;

	ct::uvec2 map_size = 512;

	m_Loaded = true;

	if (BaseRenderer::InitPixel(map_size))
	{
		m_HeatmapRenderer_Pass_Ptr = std::make_shared<HeatmapRenderer_Pass>(m_VulkanCorePtr);
		if (m_HeatmapRenderer_Pass_Ptr)
		{
			if (m_HeatmapRenderer_Pass_Ptr->InitPixel(map_size, 1U, true, true, 0.0f,
				false, vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1))
			{
				AddGenericPass(m_HeatmapRenderer_Pass_Ptr);
				m_Loaded = true;
			}
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool HeatmapRenderer::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd)
{
	ZoneScoped;

	if (m_LastExecutedFrame != vCurrentFrame)
	{
		BaseRenderer::Render("Heatmap Renderer", vCmd);

		m_LastExecutedFrame = vCurrentFrame;
	}

	return true;
}

void HeatmapRenderer::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	BaseRenderer::NeedResize(vNewSize, vCountColorBuffer);
}

bool HeatmapRenderer::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("HeatMap", -1.0f, true, true, &m_CanWeRender))
		{
			bool change = false;

			if (m_HeatmapRenderer_Pass_Ptr)
			{
				return m_HeatmapRenderer_Pass_Ptr->DrawWidgets(vCurrentFrame, vContext);
			}
		}
	}

	return false;
}

void HeatmapRenderer::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void HeatmapRenderer::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void HeatmapRenderer::SetModel(SceneModelWeak vSceneModel)
{
	ZoneScoped;

	if (m_HeatmapRenderer_Pass_Ptr)
	{
		return m_HeatmapRenderer_Pass_Ptr->SetModel(vSceneModel);
	}
}

vk::DescriptorImageInfo* HeatmapRenderer::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	if (m_HeatmapRenderer_Pass_Ptr)
	{
		return m_HeatmapRenderer_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string HeatmapRenderer::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += vOffset + "<heatmap_renderer>\n";
	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	if (m_HeatmapRenderer_Pass_Ptr)
	{
		str += m_HeatmapRenderer_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
	}

	str += vOffset + "</heatmap_renderer>\n";

	return str;
}

bool HeatmapRenderer::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "heatmap_renderer")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();
	}

	if (m_HeatmapRenderer_Pass_Ptr)
	{
		m_HeatmapRenderer_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}