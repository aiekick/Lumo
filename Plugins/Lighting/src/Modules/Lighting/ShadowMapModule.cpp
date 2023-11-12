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

#include "ShadowMapModule.h"

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

#include <Modules/Lighting/Pass/ShadowMapModule_Mesh_Pass.h>

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

std::shared_ptr<ShadowMapModule> ShadowMapModule::Create(GaiApi::VulkanCorePtr vVulkanCorePtr)
{
	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<ShadowMapModule>(vVulkanCorePtr);
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

ShadowMapModule::ShadowMapModule(GaiApi::VulkanCorePtr vVulkanCorePtr)
	: BaseRenderer(vVulkanCorePtr)
{

}

ShadowMapModule::~ShadowMapModule()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ShadowMapModule::Init()
{
	ZoneScoped;

	ct::uvec2 map_size = 512;

	m_Loaded = false;

	if (BaseRenderer::InitPixel(map_size))
	{
		m_ImageInfos.resize(8U);

		bool res = true;
		for (size_t i = 0U; i < 8U; ++i)
		{
			m_FrameBuffers[i] = FrameBuffer::Create(m_VulkanCorePtr);
			if (m_FrameBuffers[i])
			{
				if (i == 0U)
				{
					// we create the renderpass only for the first
					res &= m_FrameBuffers[i]->Init(1024U, 1U, true, true, 0.0f,
						false, vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1);
				}
				else
				{
					auto renderpassPtr = m_FrameBuffers[0]->GetRenderPass();
					if (renderpassPtr)
					{
						res &= m_FrameBuffers[i]->Init(1024U, 1U, true, true, 0.0f,
							false, vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1, false, *renderpassPtr);
					}
				}
			}
		}

		if (res)
		{
			m_ShadowMapModule_Mesh_Pass_Ptr = std::make_shared<ShadowMapModule_Mesh_Pass>(m_VulkanCorePtr);
			if (m_ShadowMapModule_Mesh_Pass_Ptr)
			{
				if (m_ShadowMapModule_Mesh_Pass_Ptr->InitPixelWithoutFBO(
					1024U, 1U, false, m_FrameBuffers[0]->GetRenderPass(),
					vk::SampleCountFlagBits::e1))
				{
					m_ShadowMapModule_Mesh_Pass_Ptr->AllowResizeByHandOrByInputs(true);
					m_ShadowMapModule_Mesh_Pass_Ptr->AllowResizeOnResizeEvents(false); // 1024 is fixed
					AddGenericPass(m_ShadowMapModule_Mesh_Pass_Ptr);
					m_Loaded = true;
				}
			}
		}
	}

	return m_Loaded;
}

void ShadowMapModule::Unit()
{
	m_ImageInfos.clear();

	// the renderpass in the FBO[0], must be destroyed in last
	// if not, will cause errors, because the renderpass loaned to other fbo will be invalid
	for (size_t i = 0U; i < 8U; ++i)
	{
		m_FrameBuffers[i].reset();
	}

	BaseRenderer::Unit();
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void ShadowMapModule::RenderShaderPasses(vk::CommandBuffer* vCmdBufferPtr)
{
	/*
	here we must run the model for each lights
	so for each pass :
	- set the fbo, (the pipelines of all fbo's already have the renderpass of the first fbo)
	- set the light id
	*/

	if (m_ShadowMapModule_Mesh_Pass_Ptr)
	{
		auto lightGroupPtr = m_SceneLightGroupWeak.lock();
		if (lightGroupPtr)
		{
			uint32_t idx = 0U;
			for (auto lightPtr : *lightGroupPtr)
			{
				if (lightPtr)
				{
					m_ShadowMapModule_Mesh_Pass_Ptr->SetFrameBuffer(m_FrameBuffers[idx]);
					m_ShadowMapModule_Mesh_Pass_Ptr->SetLightIdToUse(idx);
					m_ShadowMapModule_Mesh_Pass_Ptr->DrawPass(vCmdBufferPtr);
				}

				++idx;
			}
		}
	}
}

bool ShadowMapModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("Shadow Map Module");

	return true;
}

bool ShadowMapModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Shadow", -1.0f, true, true, &m_CanWeRender))
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

bool ShadowMapModule::DrawOverlays(
    const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, const std::string& vUserDatas) {
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
    return false;
}

bool ShadowMapModule::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, const std::string& vUserDatas)
{
	assert(vContextPtr); ImGui::SetCurrentContext(vContextPtr);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
    return false;
}

void ShadowMapModule::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

void ShadowMapModule::SetModel(SceneModelWeak vSceneModel)
{
	ZoneScoped;

	if (m_ShadowMapModule_Mesh_Pass_Ptr)
	{
		m_ShadowMapModule_Mesh_Pass_Ptr->SetModel(vSceneModel);
	}
}

void ShadowMapModule::SetLightGroup(SceneLightGroupWeak vSceneLightGroup)
{
	ZoneScoped;

	m_SceneLightGroupWeak = vSceneLightGroup;

	if (m_ShadowMapModule_Mesh_Pass_Ptr)
	{
		m_ShadowMapModule_Mesh_Pass_Ptr->SetLightGroup(vSceneLightGroup);
	}
}

SceneLightGroupWeak ShadowMapModule::GetLightGroup()
{
	ZoneScoped;

	if (m_ShadowMapModule_Mesh_Pass_Ptr)
	{
		return m_ShadowMapModule_Mesh_Pass_Ptr->GetLightGroup();
	}

	return SceneLightGroupWeak();
}

DescriptorImageInfoVector* ShadowMapModule::GetDescriptorImageInfos(const uint32_t& vBindingPoint, fvec2Vector* vOutSizes)
{
	for (size_t i = 0U; i < 8U; ++i)
	{
		if (m_FrameBuffers[i])
		{
			auto descPtr = m_FrameBuffers[i]->GetFrontDescriptorImageInfo(0U);
			if (descPtr)
			{
				m_ImageInfos[i] = *descPtr;
			}
		}
	}

	return &m_ImageInfos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ShadowMapModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += vOffset + "<shadow_map_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	for (auto pass : m_ShaderPasses)
	{
		auto pass_ptr = pass.lock();
		if (pass_ptr)
		{
			str += pass_ptr->getXml(vOffset + "\t", vUserDatas);
		}
	}

	str += vOffset + "</shadow_map_module>\n";

	return str;
}

bool ShadowMapModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "shadow_map_module")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();
	}

	for (auto pass : m_ShaderPasses)
	{
		auto pass_ptr = pass.lock();
		if (pass_ptr)
		{
			pass_ptr->setFromXml(vElem, vParent, vUserDatas);
		}
	}

	return true;
}