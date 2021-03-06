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

#include "MeshEmitterModule.h"

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

#include <Modules/Emitters/Pass/MeshEmitterModule_Comp_Pass.h>

using namespace vkApi;

//////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

std::shared_ptr<MeshEmitterModule> MeshEmitterModule::Create(vkApi::VulkanCorePtr vVulkanCorePtr, BaseNodeWeak vParentNode)
{
	if (!vVulkanCorePtr) return nullptr;
	auto res = std::make_shared<MeshEmitterModule>(vVulkanCorePtr);
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

MeshEmitterModule::MeshEmitterModule(vkApi::VulkanCorePtr vVulkanCorePtr)
	: BaseRenderer(vVulkanCorePtr)
{

}

MeshEmitterModule::~MeshEmitterModule()
{
	Unit();
}

//////////////////////////////////////////////////////////////
//// INIT / UNIT /////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool MeshEmitterModule::Init()
{
	ZoneScoped;

	ct::uvec2 map_size = 512;

	m_Loaded = false;

	if (BaseRenderer::InitCompute2D(map_size))
	{
		m_MeshEmitterModule_Comp_Pass_Ptr = std::make_shared<MeshEmitterModule_Comp_Pass>(m_VulkanCorePtr);
		if (m_MeshEmitterModule_Comp_Pass_Ptr)
		{
			m_MeshEmitterModule_Comp_Pass_Ptr->SetParentNode(GetParentNode());
			if (m_MeshEmitterModule_Comp_Pass_Ptr->InitCompute2D(map_size / 8U, 1U, false, vk::Format::eR32G32B32A32Sfloat))
			{
				AddGenericPass(m_MeshEmitterModule_Comp_Pass_Ptr);
				m_Loaded = true;
			}
		}
	}

	NeedNewExecution();

	return m_Loaded;
}

//////////////////////////////////////////////////////////////
//// OVERRIDES ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////

bool MeshEmitterModule::ExecuteAllTime(const uint32_t& vCurrentFrame, vk::CommandBuffer* vCmd, BaseNodeState* vBaseNodeState)
{
	ZoneScoped;

	BaseRenderer::Render("Particles Mesh Emitter", vCmd);

	return true;
}

bool MeshEmitterModule::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContext)
{
	assert(vContext);

	//if (m_LastExecutedFrame == vCurrentFrame)
	{
		if (ImGui::CollapsingHeader_CheckBox("Particles Mesh Emitter", -1.0f, true, true, &m_CanWeRender))
		{
			bool change = false;

			if (m_MeshEmitterModule_Comp_Pass_Ptr)
			{
				change |= m_MeshEmitterModule_Comp_Pass_Ptr->DrawWidgets(vCurrentFrame, vContext);
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

void MeshEmitterModule::DrawOverlays(const uint32_t& vCurrentFrame, const ct::frect& vRect, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void MeshEmitterModule::DisplayDialogsAndPopups(const uint32_t& vCurrentFrame, const ct::ivec2& vMaxSize, ImGuiContext* vContext)
{
	assert(vContext);

	if (m_LastExecutedFrame == vCurrentFrame)
	{

	}
}

void MeshEmitterModule::SetModel(SceneModelWeak vSceneModel)
{
	if (m_MeshEmitterModule_Comp_Pass_Ptr)
	{
		m_MeshEmitterModule_Comp_Pass_Ptr->SetModel(vSceneModel);
	}
}

vk::Buffer* MeshEmitterModule::GetTexelBuffer(const uint32_t& vBindingPoint, ct::uvec2* vOutSize)
{
	ZoneScoped;

	if (m_MeshEmitterModule_Comp_Pass_Ptr)
	{
		return m_MeshEmitterModule_Comp_Pass_Ptr->GetTexelBuffer(vBindingPoint, vOutSize);
	}

	return nullptr;
}

vk::BufferView* MeshEmitterModule::GetTexelBufferView(const uint32_t& vBindingPoint, ct::uvec2* vOutSize)
{
	ZoneScoped;

	if (m_MeshEmitterModule_Comp_Pass_Ptr)
	{
		return m_MeshEmitterModule_Comp_Pass_Ptr->GetTexelBufferView(vBindingPoint, vOutSize);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONFIGURATION /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string MeshEmitterModule::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	std::string str;

	str += vOffset + "<mesh_emitter_module>\n";

	str += vOffset + "\t<can_we_render>" + (m_CanWeRender ? "true" : "false") + "</can_we_render>\n";

	if (m_MeshEmitterModule_Comp_Pass_Ptr)
	{
		str += m_MeshEmitterModule_Comp_Pass_Ptr->getXml(vOffset + "\t", vUserDatas);
	}

	str += vOffset + "</mesh_emitter_module>\n";

	return str;
}

bool MeshEmitterModule::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "mesh_emitter_module")
	{
		if (strName == "can_we_render")
			m_CanWeRender = ct::ivariant(strValue).GetB();
	}

	if (m_MeshEmitterModule_Comp_Pass_Ptr)
	{
		m_MeshEmitterModule_Comp_Pass_Ptr->setFromXml(vElem, vParent, vUserDatas);
	}

	return true;
}