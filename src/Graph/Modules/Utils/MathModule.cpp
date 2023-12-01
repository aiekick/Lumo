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

#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets.h>
#include <LumoBackend/Systems/CommonSystem.h>

#include <Gaia/Core/VulkanCore.h>
#include <Gaia/Shader/VulkanShader.h>
#include <Gaia/Core/VulkanSubmitter.h>
#include <LumoBackend/Utils/Mesh/VertexStruct.h>
#include <cinttypes>
#include <Gaia/Buffer/FrameBuffer.h>

#include <Graph/Modules/Utils/Pass/MathModule_Quad_Pass.h>

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

std::shared_ptr<MathModule> MathModule::Create(GaiApi::VulkanCoreWeak vVulkanCore)
{
	auto res = std::make_shared<MathModule>(vVulkanCore);
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

MathModule::MathModule(GaiApi::VulkanCoreWeak vVulkanCore)
	: BaseRenderer(vVulkanCore)
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

	ct::uvec2 map_size = 1U;

	m_Loaded = true;

	if (BaseRenderer::InitPixel(map_size))
	{
		m_MathModule_Quad_Pass_Ptr = std::make_shared<MathModule_Quad_Pass>(m_VulkanCore);
		if (m_MathModule_Quad_Pass_Ptr)
		{
			// by default but can be changed via widget
			m_MathModule_Quad_Pass_Ptr->AllowResizeByHandOrByInputs(true);
			m_MathModule_Quad_Pass_Ptr->AllowResizeOnResizeEvents(false);
			
			if (m_MathModule_Quad_Pass_Ptr->InitPixel(map_size, 1U, true, true, 0.0f,
				false, false, vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1))
			{
				AddGenericPass(m_MathModule_Quad_Pass_Ptr);
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

	BaseRenderer::Render("Math", vCmd);

	return true;
}

bool MathModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	//if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Math", -1.0f, true, true, &m_CanWeRender))
		{
			bool change = false;

			for (auto pass : m_ShaderPasses)
			{
				auto passGuiPtr = dynamic_pointer_cast<GuiInterface>(pass.lock());
				if (passGuiPtr)
				{
                    change |= passGuiPtr->DrawWidgets(vCurrentFrame, vContextPtr, vUserDatas);
				}
			}

			return change;
		}
	}

	return false;
}

bool MathModule::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
    return false;
}

bool MathModule::DrawDialogsAndPopups(
    const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
    return false;
}

void MathModule::SetTexture(const uint32_t& vBindingPoint, vk::DescriptorImageInfo* vImageInfo, ct::fvec2* vTextureSize)
{
	ZoneScoped;

	if (m_MathModule_Quad_Pass_Ptr)
	{
		m_MathModule_Quad_Pass_Ptr->SetTexture(vBindingPoint, vImageInfo, vTextureSize);
	}
}

vk::DescriptorImageInfo* MathModule::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	ZoneScoped;

	if (m_MathModule_Quad_Pass_Ptr)
	{
		return m_MathModule_Quad_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MathModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += vOffset + "<math_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	if (m_MathModule_Quad_Pass_Ptr)
	{
		str += m_MathModule_Quad_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
	}

	str += vOffset + "</math_module>\n";

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

	if (strParentName == "math_module")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();
	}

	if (m_MathModule_Quad_Pass_Ptr)
	{
		return m_MathModule_Quad_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}

bool MathModule::DrawNodeWidget(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr)
{
	//if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (m_MathModule_Quad_Pass_Ptr)
		{
			return m_MathModule_Quad_Pass_Ptr->DrawNodeWidget(vCurrentFrame, vContextPtr);
		}
	}

	return false;
}

uint32_t MathModule::GetComponentCount()
{
	if (m_MathModule_Quad_Pass_Ptr)
	{
		return m_MathModule_Quad_Pass_Ptr->GetComponentCount();
	}

	return 0U;
}

std::string MathModule::GetInputName(const uint32_t& vIdx)
{
	if (m_MathModule_Quad_Pass_Ptr)
	{
		return m_MathModule_Quad_Pass_Ptr->GetInputName(vIdx);
	}

	return "";
}