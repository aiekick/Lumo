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

#include "ParticlesSimulationModule.h"

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

#include <Modules/Simulation/Pass/ParticlesSimulationModule_Pass.h>

using namespace vkApi;

#define COUNT_BUFFERS 2

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<ParticlesSimulationModule> ParticlesSimulationModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<ParticlesSimulationModule>(vVulkanCorePtr);
	res->m_This = res;
	res->SetParentNode(vParentNode);
	if (!res->Init())
	{
		res.reset();
	}
	return res;
}

//////////////////////////////////////////////////////////////
//// CTOR / DTOR /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

ParticlesSimulationModule::ParticlesSimulationModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: BaseRenderer(vVulkanCorePtr)
{

}

ParticlesSimulationModule::~ParticlesSimulationModule()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ParticlesSimulationModule::Init()
{
	ZoneScoped;

	ct::uvec2 map_size = 512;

	m_Loaded = false;

	if (BaseRenderer::InitCompute2D(map_size))
	{
		m_ParticlesSimulationModule_Pass_Ptr = std::make_shared<ParticlesSimulationModule_Pass>(m_VulkanCorePtr);
		if (m_ParticlesSimulationModule_Pass_Ptr)
		{
			m_ParticlesSimulationModule_Pass_Ptr->SetParentNode(GetParentNode());
			if (m_ParticlesSimulationModule_Pass_Ptr->InitCompute2D(map_size / 8U, 1U, false, vk::Format::eR32G32B32A32Sfloat))
			{
				AddGenericPass(m_ParticlesSimulationModule_Pass_Ptr);
				m_Loaded = true;
			}
		}
	}

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool ParticlesSimulationModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("ParticlesSimulation", vCmd);

	return true;
}

bool ParticlesSimulationModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("ParticlesSimulation", -1.0f, true, true, &m_CanWeRender))
		{
			bool change = false;

			if (m_ParticlesSimulationModule_Pass_Ptr)
			{
				change |= m_ParticlesSimulationModule_Pass_Ptr->DrawWidgets(vCurrentFrame, vContext);
			}

			return change;
		}
	}

	return false;
}

void ParticlesSimulationModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void ParticlesSimulationModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void ParticlesSimulationModule::SetTexelBuffer(const uint32_t& vBinding, vk::Buffer* vTexelBuffer, ct::uvec2* vTexelBufferSize)
{
	ZoneScoped;

	if (m_ParticlesSimulationModule_Pass_Ptr)
	{
		m_ParticlesSimulationModule_Pass_Ptr->SetTexelBuffer(vBinding, vTexelBuffer, vTexelBufferSize);
	}
}

void ParticlesSimulationModule::SetTexelBufferView(const uint32_t& vBinding, vk::BufferView* vTexelBufferView, ct::uvec2* vTexelBufferSize)
{
	ZoneScoped;

	if (m_ParticlesSimulationModule_Pass_Ptr)
	{
		m_ParticlesSimulationModule_Pass_Ptr->SetTexelBufferView(vBinding, vTexelBufferView, vTexelBufferSize);
	}
}

vk::Buffer* ParticlesSimulationModule::GetTexelBuffer(const uint32_t& vBindingPoint, ct::uvec2* vOutSize)
{
	ZoneScoped;

	if (m_ParticlesSimulationModule_Pass_Ptr)
	{
		return m_ParticlesSimulationModule_Pass_Ptr->GetTexelBuffer(vBindingPoint, vOutSize);
	}

	return nullptr;
}

vk::BufferView* ParticlesSimulationModule::GetTexelBufferView(const uint32_t& vBindingPoint, ct::uvec2* vOutSize)
{
	ZoneScoped;

	if (m_ParticlesSimulationModule_Pass_Ptr)
	{
		return m_ParticlesSimulationModule_Pass_Ptr->GetTexelBufferView(vBindingPoint, vOutSize);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ParticlesSimulationModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += vOffset + "<diffuse_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	if (m_ParticlesSimulationModule_Pass_Ptr)
	{
		str += m_ParticlesSimulationModule_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
	}

	str += vOffset + "</diffuse_module>\n";

	return str;
}

bool ParticlesSimulationModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "diffuse_module")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();
	}

	if (m_ParticlesSimulationModule_Pass_Ptr)
	{
		m_ParticlesSimulationModule_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}