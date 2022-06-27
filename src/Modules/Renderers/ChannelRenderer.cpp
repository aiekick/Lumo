/*
MIT License

Copyright (c) 2022-2022 Stephane Cuillerdier (aka aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ChannelRenderer.h"

#include <functional>
#include <Gui/MainFrame.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>

#include <Modules/Renderers/Pass/ChannelRenderer_Pass.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<ChannelRenderer> ChannelRenderer::Create(vkApi::VulkanCore* vVulkanCore)
{
	auto res = std::make_shared<ChannelRenderer>(vVulkanCore);
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

ChannelRenderer::ChannelRenderer(vkApi::VulkanCore* vVulkanCore)
	: BaseRenderer(vVulkanCore)
{

}

ChannelRenderer::~ChannelRenderer()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ChannelRenderer::Init()
{
	ZoneScoped;

	ct::uvec2 map_size = 512;

	m_Loaded = true;

	if (BaseRenderer::InitPixel(map_size))
	{
		m_ChannelRenderer_Pass_Ptr = std::make_shared<ChannelRenderer_Pass>(m_VulkanCore);
		if (m_ChannelRenderer_Pass_Ptr)
		{
			if (m_ChannelRenderer_Pass_Ptr->InitPixel(map_size, 1U, true, true, 0.0f,
				vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1))
			{
				AddGenericPass(m_ChannelRenderer_Pass_Ptr);
				m_Loaded = true;
			}
		}
	}

	return m_Loaded;
}
//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ChannelRenderer::Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer *vCmd)
{
	ZoneScoped;

	if (m_LastExecutedFrame != vCurrentFrame)
	{
		BaseRenderer::Render("Channel Renderer", vCmd);

		m_LastExecutedFrame = vCurrentFrame;
	}

	return true;
}

void ChannelRenderer::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	BaseRenderer::NeedResize(vNewSize, vCountColorBuffer);
}

bool ChannelRenderer::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Channels", -1.0f, true, true, &m_CanWeRender))
		{
			if (m_ChannelRenderer_Pass_Ptr)
			{
				return m_ChannelRenderer_Pass_Ptr->DrawWidgets(vCurrentFrame, vContext);
			}
		}
	}

	return false;
}

void ChannelRenderer::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void ChannelRenderer::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void ChannelRenderer::SetModel(SceneModelWeak vSceneModel)
{
	ZoneScoped;

	if (m_ChannelRenderer_Pass_Ptr)
	{
		return m_ChannelRenderer_Pass_Ptr->SetModel(vSceneModel);
	}
}

vk::DescriptorImageInfo* ChannelRenderer::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	if (m_ChannelRenderer_Pass_Ptr)
	{
		return m_ChannelRenderer_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ChannelRenderer::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += vOffset + "<channel_renderer>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";
	
	if (m_ChannelRenderer_Pass_Ptr)
	{
		str += m_ChannelRenderer_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
	}

	str += vOffset + "</channel_renderer>\n";

	return str;
}

bool ChannelRenderer::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "channel_renderer")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();
	}

	if (m_ChannelRenderer_Pass_Ptr)
	{
		m_ChannelRenderer_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}