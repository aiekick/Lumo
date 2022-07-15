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

#include "MathModule.h"

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

#include <Modules/Utils/Pass/MathModule_Pass.h>

using namespace vkApi;

#define COUNT_BUFFERS 2

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<MathModule> MathModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	auto res = std::make_shared<MathModule>(vVulkanCorePtr);
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

MathModule::MathModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: BaseRenderer(vVulkanCorePtr)
{

}

MathModule::~MathModule()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool MathModule::Init()
{
	ZoneScoped;

	ct::uvec2 map_size = 512;

	m_Loaded = true;

	if (BaseRenderer::InitCompute2D(map_size))
	{
		m_MathModule_Pass_Ptr = std::make_shared<MathModule_Pass>(m_VulkanCorePtr);
		if (m_MathModule_Pass_Ptr)
		{
			if (m_MathModule_Pass_Ptr->InitCompute2D(map_size, 1U, false, vk::Format::eR32G32B32A32Sfloat))
			{
				AddGenericPass(m_MathModule_Pass_Ptr);
				m_Loaded = true;
			}
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool MathModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	if (m_LastExecutedFrame != vCurrentFrame)
	{
		BaseRenderer::Render("Math", vCmd);

		m_LastExecutedFrame = vCurrentFrame;
	}

	return true;
}

bool MathModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Math", -1.0f, true, true, &m_CanWeRender))
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

void MathModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void MathModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void MathModule::NeedResize(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	BaseRenderer::NeedResize(vNewSize, vCountColorBuffers);
}

void MathModule::SetTexture(const uint32_t& vBinding, vk::DescriptorImageInfo* vImageInfo)
{
	ZoneScoped;

	if (m_MathModule_Pass_Ptr)
	{
		m_MathModule_Pass_Ptr->SetTexture(vBinding, vImageInfo);
	}
}

vk::DescriptorImageInfo* MathModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	ZoneScoped;

	if (m_MathModule_Pass_Ptr)
	{
		return m_MathModule_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MathModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += vOffset + "<blur_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	for (auto passPtr : m_ShaderPass)
	{
		if (passPtr)
		{
			str += passPtr->getXml(vOffset + "\t", vUserDatas);
		}
	}

	str += vOffset + "</blur_module>\n";

	return str;
}

bool MathModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "blur_module")
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