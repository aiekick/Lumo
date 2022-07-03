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

#include "ModelSSSModule.h"

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

#include <Modules/Lighting/Pass/ModelSSSModule_Pass.h>

using namespace vkApi;

#define COUNT_BUFFERS 2

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<ModelSSSModule> ModelSSSModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<ModelSSSModule>(vVulkanCorePtr);
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

ModelSSSModule::ModelSSSModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: BaseRenderer(vVulkanCorePtr)
{

}

ModelSSSModule::~ModelSSSModule()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ModelSSSModule::Init()
{
	ZoneScoped;

	ct::uvec2 map_size = 512;

	m_Loaded = true;

	if (BaseRenderer::InitPixel(map_size))
	{
		m_ModelSSSModule_Pass_Ptr = std::make_shared<ModelSSSModule_Pass>(m_VulkanCorePtr);
		if (m_ModelSSSModule_Pass_Ptr)
		{
			// eR8G8B8A8Unorm is used for have nice white and black display
			// unfortunatly not for perf, but the main purpose is for nice widget display
			// or maybe there is a way in glsl to know the component count of a texture
			// so i could modify in this way the shader of imgui
			if (m_ModelSSSModule_Pass_Ptr->InitPixel(map_size, 1U, true, true, 0.0f,
				false, vk::Format::eR8G8B8A8Unorm, vk::SampleCountFlagBits::e1))
			{
				AddGenericPass(m_ModelSSSModule_Pass_Ptr);
				m_Loaded = true;
			}
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ModelSSSModule::Execute(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd)
{
	ZoneScoped;

	if (m_LastExecutedFrame != vCurrentFrame)
	{
		BaseRenderer::Render("Model SSS Module");

		m_LastExecutedFrame = vCurrentFrame;
	}

	return true;
}

bool ModelSSSModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("SSS", -1.0f, true, true, &m_CanWeRender))
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

void ModelSSSModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void ModelSSSModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void ModelSSSModule::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffer)
{
	BaseRenderer::NeedResize(vNewSize, vCountColorBuffer);
}

void ModelSSSModule::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	ZoneScoped;

	if (m_ModelSSSModule_Pass_Ptr)
	{
		m_ModelSSSModule_Pass_Ptr->SetLightGroup(vSceneLightGroup);
	}
}

void ModelSSSModule::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	ZoneScoped;

	if (m_ModelSSSModule_Pass_Ptr)
	{
		return m_ModelSSSModule_Pass_Ptr->SetTexture(vBinding, vImageInfo);
	}
}

vk::DescriptorImageInfo* ModelSSSModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint)
{
	ZoneScoped;

	if (m_ModelSSSModule_Pass_Ptr)
	{
		return m_ModelSSSModule_Pass_Ptr->GetDescriptorImageInfo(0U);
	}

	return nullptr;
}

void ModelSSSModule::UpdateDescriptorsBeforeCommandBuffer()
{
	/*if (m_ModelSSSModule_Pass_Light_SSS_Map_Ptr &&
		m_ModelSSSModule_Pass_Ptr)
	{
		// ligh shadow map in model shadow pass
		m_ModelSSSModule_Pass_Ptr->SetTexture(3U,
			m_ModelSSSModule_Pass_Light_SSS_Map_Ptr->GetDescriptorImageInfo(0U));

		m_ModelSSSModule_Pass_Ptr->SetLighViewMatrix(
			m_ModelSSSModule_Pass_Light_SSS_Map_Ptr->GetLightViewMatrix());
	}*/

	BaseRenderer::UpdateDescriptorsBeforeCommandBuffer();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ModelSSSModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += vOffset + "<model_sss_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	if (m_ModelSSSModule_Pass_Ptr)
	{
		str += m_ModelSSSModule_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
	}

	str += vOffset + "</model_sss_module>\n";

	return str;
}

bool ModelSSSModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "model_sss_module")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();
	}

	if (m_ModelSSSModule_Pass_Ptr)
	{
		m_ModelSSSModule_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}