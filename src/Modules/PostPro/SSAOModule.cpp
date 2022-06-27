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

#include "SSAOModule.h"

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
#include <Generic/FrameBuffer.h>

#include <Modules/PostPro/Pass/SSAOModule_Pass_AO.h>
#include <Modules/PostPro/Pass/SSAOModule_Pass_Blur.h>

using namespace vkApi;

#define COUNT_BUFFERS 2

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<SSAOModule> SSAOModule::Create(vkApi::VulkanCore* vVulkanCore)
{
	auto res = std::make_shared<SSAOModule>(vVulkanCore);
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

SSAOModule::SSAOModule(vkApi::VulkanCore* vVulkanCore)
	: GenericRenderer(vVulkanCore)
{

}

SSAOModule::~SSAOModule()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool SSAOModule::Init()
{
	ZoneScoped;

	ct::uvec2 map_size = 512;

	m_Loaded = true;

	if (GenericRenderer::InitPixel(map_size))
	{
		m_SSAOModule_Pass_AO_Ptr = std::make_shared<SSAOModule_Pass_AO>(m_VulkanCore);
		if (m_SSAOModule_Pass_AO_Ptr)
		{
			if (m_SSAOModule_Pass_AO_Ptr->InitPixel(map_size, 1U, true, true, 0.0f,
				vk::Format::eR8Unorm, vk::SampleCountFlagBits::e1))
			{
				AddGenericPass(m_SSAOModule_Pass_AO_Ptr);
				m_Loaded = true;
			}
		}

		m_SSAOModule_Pass_Blur_Ptr = std::make_shared<SSAOModule_Pass_Blur>(m_VulkanCore);
		if (m_SSAOModule_Pass_Blur_Ptr)
		{
			// eR8G8B8A8Unorm is used for have nice white and black display
			// unfortunatly not for perf, but the main purpose is for nice widget display
			// or maybe there is a way in glsl to know the component count of a texture
			// so i could modify in this way the shader of imgui
			if (m_SSAOModule_Pass_Blur_Ptr->InitPixel(map_size, 1U, true, true, 0.0f,
				vk::Format::eR8G8B8A8Unorm, vk::SampleCountFlagBits::e1)) 
			{
				AddGenericPass(m_SSAOModule_Pass_Blur_Ptr);
				m_Loaded = true;
			}
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool SSAOModule::Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd)
{
	ZoneScoped;

	if (m_LastExecutedFrame != vCurrentFrame)
	{
		GenericRenderer::Render("SSAO", vCmd);

		m_LastExecutedFrame = vCurrentFrame;
	}

	return true;
}

bool SSAOModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("SSAO", -1.0f, true, true, &m_CanWeRender))
		{
			bool change = false;

			for (auto passPtr : m_ShaderPass)
			{
				auto passGuiPtr = dynamic_pointer_cast<GuiInterface>(passPtr);
				if (passGuiPtr)
				{
					change |= passGuiPtr->DrawWidgets(vCurrentFrame, vContext);
				}
			}

			return change;
		}
	}

	return false;
}

void SSAOModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void SSAOModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void SSAOModule::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	GenericRenderer::NeedResize(vNewSize, vCountColorBuffer);
}

void SSAOModule::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	ZoneScoped;

	if (m_Loaded)
	{
		if (m_SSAOModule_Pass_AO_Ptr)
		{
			m_SSAOModule_Pass_AO_Ptr->SetTexture(vBinding, vImageInfo);
		}
	}
}

vk::DescriptorImageInfo* SSAOModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	ZoneScoped;

	if (m_SSAOModule_Pass_Blur_Ptr)
	{
		return m_SSAOModule_Pass_Blur_Ptr->GetDescriptorImageInfo(vBindingPoint);
	}

	return nullptr;
}

void SSAOModule::UpdateShaders(const std::set<std::string>& vFiles)
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

void SSAOModule::UpdateDescriptorsBeforeCommandBuffer()
{
	// on va lier les pass's entre elles
	if (m_SSAOModule_Pass_AO_Ptr && 
		m_SSAOModule_Pass_Blur_Ptr)
	{
		// on va mettre le front de "AO" dans l'input de "BLUR"
		m_SSAOModule_Pass_Blur_Ptr->SetTexture(0U, 
			m_SSAOModule_Pass_AO_Ptr->GetDescriptorImageInfo(0U));
	}

	GenericRenderer::UpdateDescriptorsBeforeCommandBuffer();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string SSAOModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += vOffset + "<ssao_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	for (auto passPtr : m_ShaderPass)
	{
		if (passPtr)
		{
			str += passPtr->getXml(vOffset + "\t", vUserDatas);
		}
	}

	str += vOffset + "</ssao_module>\n";

	return str;
}

bool SSAOModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "ssao_module")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();
	}

	for (auto passPtr : m_ShaderPass)
	{
		if (passPtr)
		{
			passPtr->setFromXml(vElem, vParent, vUserDatas);
		}
	}

	return true;
}