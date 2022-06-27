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

#include "ModelShadowModule.h"

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

#include <Modules/Lighting/Pass/ModelShadowModule_Pass.h>

using namespace vkApi;

#define COUNT_BUFFERS 2

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<ModelShadowModule> ModelShadowModule::Create(vkApi::VulkanCore* vVulkanCore)
{
	auto res = std::make_shared<ModelShadowModule>(vVulkanCore);
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

ModelShadowModule::ModelShadowModule(vkApi::VulkanCore* vVulkanCore)
	: BaseRenderer(vVulkanCore)
{

}

ModelShadowModule::~ModelShadowModule()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ModelShadowModule::Init()
{
	ZoneScoped;

	ct::uvec2 map_size = 512;

	m_Loaded = true;

	if (BaseRenderer::InitPixel(map_size))
	{
		m_ModelShadowModule_Pass_Ptr = std::make_shared<ModelShadowModule_Pass>(m_VulkanCore);
		if (m_ModelShadowModule_Pass_Ptr)
		{
			// eR8G8B8A8Unorm is used for have nice white and black display
			// unfortunatly not for perf, but the main purpose is for nice widget display
			// or maybe there is a way in glsl to know the component count of a texture
			// so i could modify in this way the shader of imgui
			if (m_ModelShadowModule_Pass_Ptr->InitPixel(map_size, 1U, true, true, 0.0f,
				vk::Format::eR8G8B8A8Unorm, vk::SampleCountFlagBits::e1))
			{
				AddGenericPass(m_ModelShadowModule_Pass_Ptr);
				m_Loaded = true;
			}
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ModelShadowModule::Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd)
{
	ZoneScoped;

	if (m_LastExecutedFrame != vCurrentFrame)
	{
		BaseRenderer::Render("Model Shadow Module");

		m_LastExecutedFrame = vCurrentFrame;
	}

	return true;
}

bool ModelShadowModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Shadow", -1.0f, true, true, &m_CanWeRender))
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

void ModelShadowModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void ModelShadowModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void ModelShadowModule::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	BaseRenderer::NeedResize(vNewSize, vCountColorBuffer);
}

void ModelShadowModule::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	ZoneScoped;

	if (m_ModelShadowModule_Pass_Ptr)
	{
		m_ModelShadowModule_Pass_Ptr->SetLightGroup(vSceneLightGroup);
	}
}

void ModelShadowModule::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	ZoneScoped;

	if (m_ModelShadowModule_Pass_Ptr)
	{
		return m_ModelShadowModule_Pass_Ptr->SetTexture(vBinding, vImageInfo);
	}
}

vk::DescriptorImageInfo* ModelShadowModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	ZoneScoped;

	if (m_ModelShadowModule_Pass_Ptr)
	{
		return m_ModelShadowModule_Pass_Ptr->GetDescriptorImageInfo(0U);
	}

	return nullptr;
}

void ModelShadowModule::UpdateShaders(const std::set<std::string>& vFiles)
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

void ModelShadowModule::UpdateDescriptorsBeforeCommandBuffer()
{
	/*if (m_ModelShadowModule_Pass_Light_Shadow_Map_Ptr && 
		m_ModelShadowModule_Pass_Ptr)
	{
		// ligh shadow map in model shadow pass
		m_ModelShadowModule_Pass_Ptr->SetTexture(3U,
			m_ModelShadowModule_Pass_Light_Shadow_Map_Ptr->GetDescriptorImageInfo(0U));

		m_ModelShadowModule_Pass_Ptr->SetLighViewMatrix(
			m_ModelShadowModule_Pass_Light_Shadow_Map_Ptr->GetLightViewMatrix());
	}*/

	BaseRenderer::UpdateDescriptorsBeforeCommandBuffer();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ModelShadowModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += vOffset + "<model_shadow_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	if (m_ModelShadowModule_Pass_Ptr)
	{
		return m_ModelShadowModule_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
	}

	str += vOffset + "</model_shadow_module>\n";

	return str;
}

bool ModelShadowModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "model_shadow_module")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();
	}

	if (m_ModelShadowModule_Pass_Ptr)
	{
		return m_ModelShadowModule_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}