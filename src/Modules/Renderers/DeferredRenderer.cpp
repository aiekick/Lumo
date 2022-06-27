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

#include "DeferredRenderer.h"

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
#include <Modules/Renderers/Pass/DeferredRenderer_Pass.h>

using namespace vkApi;

#define COUNT_BUFFERS 2

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<DeferredRenderer> DeferredRenderer::Create(vkApi::VulkanCore* vVulkanCore)
{
	auto res = std::make_shared<DeferredRenderer>(vVulkanCore);
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

DeferredRenderer::DeferredRenderer(vkApi::VulkanCore* vVulkanCore)
	: BaseRenderer(vVulkanCore)
{

}

DeferredRenderer::~DeferredRenderer()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool DeferredRenderer::Init()
{
	ZoneScoped;

	ct::uvec2 map_size = 512;

	m_Loaded = true;

	if (BaseRenderer::InitPixel(map_size))
	{
		m_DeferredRenderer_Pass_Ptr = std::make_shared<DeferredRenderer_Pass>(m_VulkanCore);
		if (m_DeferredRenderer_Pass_Ptr)
		{
			if (m_DeferredRenderer_Pass_Ptr->InitPixel(map_size, 1U, true, true, 0.0f,
				vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1))
			{
				AddGenericPass(m_DeferredRenderer_Pass_Ptr);
				m_Loaded = true;
			}
		}
	}
	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool DeferredRenderer::Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd)
{
	ZoneScoped;

	if (m_LastExecutedFrame != vCurrentFrame)
	{
		BaseRenderer::Render("Deferred Renderer", vCmd);

		m_LastExecutedFrame = vCurrentFrame;
	}

	return true;
}

bool DeferredRenderer::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Deferred Renderer", -1.0f, true, true, &m_CanWeRender))
		{
			if (m_DeferredRenderer_Pass_Ptr)
			{
				return m_DeferredRenderer_Pass_Ptr->DrawWidgets(vCurrentFrame, vContext);
			}
		}
	}

	return false;
}

void DeferredRenderer::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void DeferredRenderer::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void DeferredRenderer::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	BaseRenderer::NeedResize(vNewSize, vCountColorBuffer);
}

void DeferredRenderer::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	if (m_DeferredRenderer_Pass_Ptr)
	{
		return m_DeferredRenderer_Pass_Ptr->SetTexture(vBinding, vImageInfo);
	}
}

vk::DescriptorImageInfo* DeferredRenderer::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	if (m_DeferredRenderer_Pass_Ptr)
	{
		return m_DeferredRenderer_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string DeferredRenderer::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	return str;
}

bool DeferredRenderer::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	return true;
}

void DeferredRenderer::UpdateShaders(const std::set<std::string>& vFiles)
{
	for (auto passPtr : m_ShaderPass)
	{
		auto passShaUpdPtr = dynamic_pointer_cast<ShaderUpdateInterface>(passPtr);
		if (passShaUpdPtr)
		{
			passShaUpdPtr->UpdateShaders(vFiles);
		}
	}
}
