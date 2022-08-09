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

#include "ParticlesPointRenderer.h"

#include <functional>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <ImWidgets/ImWidgets.h>
#include <Systems/CommonSystem.h>
#include <Profiler/vkProfiler.hpp>
#include <vkFramework/VulkanCore.h>
#include <vkFramework/VulkanShader.h>
#include <Modules/Renderers/Pass/ParticlesPointRenderer_Mesh_Pass.h>
using namespace vkApi;

#include <Systems/RenderDocController.h>

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<ParticlesPointRenderer> ParticlesPointRenderer::Create(vkApi::VulkanCorePtr vVulkanCorePtr)
{
	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<ParticlesPointRenderer>(vVulkanCorePtr);
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

ParticlesPointRenderer::ParticlesPointRenderer(vkApi::VulkanCorePtr vVulkanCorePtr)
	: BaseRenderer(vVulkanCorePtr)
{

}

ParticlesPointRenderer::~ParticlesPointRenderer()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ParticlesPointRenderer::Init()
{
	ZoneScoped;

	ct::uvec2 map_size = 512;

	m_Loaded = true;

	if (BaseRenderer::InitPixel(map_size))
	{
		m_ParticlesPointRenderer_Mesh_Pass_Ptr = std::make_shared<ParticlesPointRenderer_Mesh_Pass>(m_VulkanCorePtr);
		if (m_ParticlesPointRenderer_Mesh_Pass_Ptr)
		{
			if (m_ParticlesPointRenderer_Mesh_Pass_Ptr->InitPixel(map_size, 1U, true, true, 0.0f,
				false, vk::Format::eR32G32B32A32Sfloat, vk::SampleCountFlagBits::e1))
			{
				AddGenericPass(m_ParticlesPointRenderer_Mesh_Pass_Ptr);
				m_Loaded = true;
			}
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ParticlesPointRenderer::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("Particles Renderer", vCmd);

	return true;
}

bool ParticlesPointRenderer::ExecuteWhenNeeded(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("Particles Renderer", vCmd);

	return true;
}

void ParticlesPointRenderer::NeedResizeByResizeEvent(ct::ivec2* vNewSize, const uint32_t* vCountColorBuffers)
{
	BaseRenderer::NeedResizeByResizeEvent(vNewSize, vCountColorBuffers);
}

bool ParticlesPointRenderer::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("HeatMap", -1.0f, true, true, &m_CanWeRender))
		{
			bool change = false;

			static bool s_RenderingWhenNeeded = false;
			if (ImGui::CheckBoxBoolDefault("Rendering when needed", &s_RenderingWhenNeeded, false))
			{
				SetExecutionWhenNeededOnly(s_RenderingWhenNeeded);
			}

			static bool s_Capture_RenderDocFrame = false;
			ImGui::CheckBoxBoolDefault("Capture Renderdoc Frame with next action", &s_Capture_RenderDocFrame, false);

			if (ImGui::ContrastedButton("Next Frame", nullptr, nullptr, ImGui::GetContentRegionAvail().x))
			{
				change = true;

				if (s_Capture_RenderDocFrame)
				{
					RenderDocController::Instance()->RequestCapture();
				}
			}

			if (m_ParticlesPointRenderer_Mesh_Pass_Ptr)
			{
				change |= m_ParticlesPointRenderer_Mesh_Pass_Ptr->DrawWidgets(vCurrentFrame, vContext);
			}

			if (change)
			{
				NeedNewExecution();
			}

			return change;
		}
	}

	return false;
}

void ParticlesPointRenderer::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void ParticlesPointRenderer::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext); ImGui::SetCurrentContext(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void ParticlesPointRenderer::SetParticles(SceneParticlesWeak vSceneParticles)
{
	ZoneScoped;

	if (m_ParticlesPointRenderer_Mesh_Pass_Ptr)
	{
		m_ParticlesPointRenderer_Mesh_Pass_Ptr->SetParticles(vSceneParticles);
	}
}

vk::DescriptorImageInfo* ParticlesPointRenderer::GetDescriptorImageInfo(const uint32_t& vBindingPoint, ct::fvec2* vOutSize)
{
	ZoneScoped;

	if (m_ParticlesPointRenderer_Mesh_Pass_Ptr)
	{
		return m_ParticlesPointRenderer_Mesh_Pass_Ptr->GetDescriptorImageInfo(vBindingPoint, vOutSize);
	}

	return VK_NULL_HANDLE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ParticlesPointRenderer::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += vOffset + "<heatmap_renderer>\n";
	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	if (m_ParticlesPointRenderer_Mesh_Pass_Ptr)
	{
		str += m_ParticlesPointRenderer_Mesh_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
	}

	str += vOffset + "</heatmap_renderer>\n";

	return str;
}

bool ParticlesPointRenderer::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (m_ParticlesPointRenderer_Mesh_Pass_Ptr)
	{
		m_ParticlesPointRenderer_Mesh_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}